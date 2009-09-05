/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000 - 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}

*/

#include "GlideComputer.hpp"
#include "McReady.h"
#include "WindZigZag.h"
#include "windanalyser.h"
#include <math.h>
#include "ReplayLogger.hpp"
#include "GlideComputer.hpp"
#include "Protection.hpp"
#include "Units.hpp"
#include "SettingsComputer.hpp"
#include "SettingsUser.hpp"
#include "Math/LowPassFilter.hpp"
#include "Math/Earth.hpp"
#include "Math/Geometry.hpp"
#include "Math/FastMath.h"
#include "RasterTerrain.h"
#include "Calibration.hpp"
#include "Airspace.h"
#include "AirspaceWarning.h"
#include "GlideSolvers.hpp"
#include "Task.h" // only needed for check of activewaypoint
#include "SettingsTask.hpp"
#include "LocalTime.hpp"
#include "MapWindowProjection.hpp"
#include "InputEvents.h"
#include "Components.hpp"
#include "Interface.hpp"
#include "Atmosphere.h"
#include "LogFile.hpp"

bool WasFlying = false; // VENTA3 used by auto QFE: do not reset QFE
			//   if previously in flight. So you can check
			//   QFE on the ground, otherwise it turns to
			//   zero at once!

GlideComputerAirData::GlideComputerAirData()
{
  InitLDRotary(&rotaryLD);

  //JMW TODO enhancement: seed initial wind store with start conditions
  // SetWindEstimate(Calculated().WindSpeed, Calculated().WindBearing, 1);
}

void GlideComputerAirData::ResetFlight(const bool full) 
{
  GlidePolar::SetCruiseEfficiency(1.0);
}

void GlideComputerAirData::Initialise() 
{
  CalibrationInit();
}


void GlideComputerAirData::ProcessBasic() {
  Heading();
  EnergyHeightNavAltitude();
  TerrainHeight();
  Vario();
  PredictNextPosition();
  BallastDump();
}


void GlideComputerAirData::ProcessVertical() {

  Turning();
  LastThermalStats();
  LD();
  CruiseLD();
  SetCalculated().AverageLD= CalculateLDRotary(&Calculated(), &rotaryLD); // AverageLD
  Average30s();

  AverageClimbRate();
  AverageThermal();
  ThermalGain();

}


#define D_AUTOWIND_CIRCLING 1
#define D_AUTOWIND_ZIGZAG 2

int AutoWindMode= D_AUTOWIND_CIRCLING;
// 0: Manual
// 1: Circling
// 2: ZigZag
// 3: Both

void GlideComputerAirData::DoWindZigZag() {
  // update zigzag wind
  if (((AutoWindMode & D_AUTOWIND_ZIGZAG)==D_AUTOWIND_ZIGZAG)
      && (!ReplayLogger::IsEnabled())) {
    double zz_wind_speed;
    double zz_wind_bearing;
    int quality;
    quality = WindZigZagUpdate(&Basic(), &Calculated(),
			       &zz_wind_speed,
			       &zz_wind_bearing);
    if (quality>0) {
      SetWindEstimate(zz_wind_speed, zz_wind_bearing, quality);
      Vector v_wind;
      v_wind.x = zz_wind_speed*cos(zz_wind_bearing*DEG_TO_RAD);
      v_wind.y = zz_wind_speed*sin(zz_wind_bearing*DEG_TO_RAD);

      windanalyser.slot_newEstimate(&Basic(), 
				    &SetCalculated(), v_wind, quality);
    }
  }
}


void GlideComputerAirData::DoWindCirclingMode(const bool left) {
  if ((AutoWindMode & D_AUTOWIND_CIRCLING)==D_AUTOWIND_CIRCLING) {
    windanalyser.slot_newFlightMode(&Basic(), 
				    &Calculated(),
				    left, 0);
  }
}


void GlideComputerAirData::DoWindCirclingSample() {
  ScopeLock protect(mutexGlideComputer);
  if ((AutoWindMode & D_AUTOWIND_CIRCLING)==D_AUTOWIND_CIRCLING) {
    windanalyser.slot_newSample(&Basic(), 
				&SetCalculated());
  }
}


void GlideComputerAirData::DoWindCirclingAltitude() {
  if (AutoWindMode>0) {
    ScopeLock protect(mutexGlideComputer);
    windanalyser.slot_Altitude(&Basic(), 
			       &SetCalculated());
  }
}


void GlideComputerAirData::SetWindEstimate(const double wind_speed,
					    const double wind_bearing,
					    const int quality) {
  Vector v_wind;
  v_wind.x = wind_speed*cos(wind_bearing*3.1415926/180.0);
  v_wind.y = wind_speed*sin(wind_bearing*3.1415926/180.0);
  ScopeLock protect(mutexGlideComputer);
  windanalyser.slot_newEstimate(&Basic(), 
				&SetCalculated(),
				v_wind, quality);
}


///////////


void GlideComputerAirData::AverageClimbRate()
{
  if (Basic().AirspeedAvailable && Basic().VarioAvailable
      && (!Calculated().Circling)) {

    int vi = iround(Basic().IndicatedAirspeed);

    if ((vi<=0) || (vi>= SAFTEYSPEED)) {
      // out of range
      return;
    }
    if (Basic().AccelerationAvailable) {
      if (fabs(fabs(Basic().Gload)-1.0)>0.25) {
        // G factor too high
        return;
      }
    }
    if (Basic().TrueAirspeed>0) {
      // TODO: Check this is correct for TAS/IAS
      double ias_to_tas = Basic().IndicatedAirspeed/Basic().TrueAirspeed;
      double w_tas = Basic().Vario*ias_to_tas;

      SetCalculated().AverageClimbRate[vi]+= w_tas;
      SetCalculated().AverageClimbRateN[vi]++;
    }
  }
}


#ifdef NEWCLIMBAV
ClimbAverageCalculator climbAverageCalculator;
void GlideComputerAirData::Average30s()
{
  Calculated().Average30s = climbAverageCalculator.GetAverage(Basic().Time, 
								 Basic().Altitude, 30);
  Calculated().NettoAverage30s = Calculated().Average30s;
}

#endif

void GlideComputerAirData::Average30s()
{
  static double LastTime = 0;
  static double Altitude[30];
  static double Vario[30];
  static double NettoVario[30];
  int Elapsed, i;
  long index = 0;
  double Gain;
  static int num_samples = 0;
  static BOOL lastCircling = false;

  if(Basic().Time > LastTime)
    {

      if (Calculated().Circling != lastCircling) {
        num_samples = 0;
        // reset!
      }
      lastCircling = Calculated().Circling;

      Elapsed = (int)(Basic().Time - LastTime);
      for(i=0;i<Elapsed;i++)
        {
          index = (long)LastTime + i;
          index %= 30;

          Altitude[index] = Calculated().NavAltitude;
	  if (Basic().NettoVarioAvailable) {
	    NettoVario[index] = Basic().NettoVario;
	  } else {
	    NettoVario[index] = Calculated().NettoVario;
	  }
	  if (Basic().VarioAvailable) {
	    Vario[index] = Basic().Vario;
	  } else {
	    Vario[index] = Calculated().Vario;
	  }

          if (num_samples<30) {
            num_samples ++;
          }

        }

      double Vave = 0;
      double NVave = 0;
      int j;
      for (i=0; i< num_samples; i++) {
        j = (index - i) % 30;
        if (j<0) {
          j += 30;
        }
        Vave += Vario[j];
	NVave += NettoVario[j];
      }
      if (num_samples) {
        Vave /= num_samples;
        NVave /= num_samples;
      }

      if (!Basic().VarioAvailable) {
        index = ((long)Basic().Time - 1)%30;
        Gain = Altitude[index];

        index = ((long)Basic().Time)%30;
        Gain = Gain - Altitude[index];

        Vave = Gain/30;
      }
      SetCalculated().Average30s =
        LowPassFilter(Calculated().Average30s,Vave,0.8);
      SetCalculated().NettoAverage30s =
        LowPassFilter(Calculated().NettoAverage30s,NVave,0.8);

#ifdef DEBUGAVERAGER
      if (Calculated().Flying) {
        DebugStore("%d %g %g %g # averager\r\n",
                num_samples,
                Calculated().Vario,
                Calculated().Average30s, Calculated().NettoAverage30s);
      }
#endif

    }
  else
    {
      if (Basic().Time<LastTime) {
	// gone back in time
	for (i=0; i<30; i++) {
	  Altitude[i]= 0;
	  Vario[i]=0;
	  NettoVario[i]=0;
	}
      }
    }
  LastTime = Basic().Time;
}

void GlideComputerAirData::AverageThermal()
{
  if (Calculated().ClimbStartTime>=0) {
    if(Basic().Time > Calculated().ClimbStartTime)
      {
        double Gain =
          Calculated().NavAltitude+Calculated().EnergyHeight
            - Calculated().ClimbStartAlt;
        SetCalculated().AverageThermal  =
          Gain / (Basic().Time - Calculated().ClimbStartTime);
      }
  }
}

void GlideComputerAirData::MaxHeightGain()
{
  if (!Calculated().Flying) return;

  if (Calculated().MinAltitude>0) {
    double height_gain = Calculated().NavAltitude - Calculated().MinAltitude;
    SetCalculated().MaxHeightGain = max(height_gain, Calculated().MaxHeightGain);
  } else {
    SetCalculated().MinAltitude = Calculated().NavAltitude;
  }
  SetCalculated().MinAltitude = min(Calculated().NavAltitude, Calculated().MinAltitude);
}


void GlideComputerAirData::ThermalGain()
{
  if (Calculated().ClimbStartTime>=0) {
    if(Basic().Time >= Calculated().ClimbStartTime)
      {
        SetCalculated().ThermalGain =
          Calculated().NavAltitude + Calculated().EnergyHeight
          - Calculated().ClimbStartAlt;
      }
  }
}



void GlideComputerAirData::LD()
{
  static double LastLat = 0;
  static double LastLon = 0;
  static double LastTime = 0;
  static double LastAlt = 0;

  if (Basic().Time<LastTime) {
    LastTime = Basic().Time;
    SetCalculated().LDvario = INVALID_GR;
    SetCalculated().LD = INVALID_GR;
  }
  if(Basic().Time >= LastTime+1.0)
    {
      double DistanceFlown;
      DistanceBearing(Basic().Latitude, Basic().Longitude,
                      LastLat, LastLon,
                      &DistanceFlown, NULL);

      SetCalculated().LD = UpdateLD(Calculated().LD,
				    DistanceFlown,
				    LastAlt - Calculated().NavAltitude, 0.1);

      InsertLDRotary(&Calculated(), 
		     &rotaryLD,(int)DistanceFlown, (int)Calculated().NavAltitude);

      LastLat = Basic().Latitude;
      LastLon = Basic().Longitude;
      LastAlt = Calculated().NavAltitude;
      LastTime = Basic().Time;
    }

  // LD instantaneous from vario, updated every reading..
  if (Basic().VarioAvailable && Basic().AirspeedAvailable
      && Calculated().Flying) {
    SetCalculated().LDvario = UpdateLD(Calculated().LDvario,
                                   Basic().IndicatedAirspeed,
                                   -Basic().Vario,
                                   0.3);
  } else {
    SetCalculated().LDvario = INVALID_GR;
  }
}


void GlideComputerAirData::CruiseLD()
{
  if(!Calculated().Circling)
    {
      double DistanceFlown;

      if (Calculated().CruiseStartTime<0) {
        SetCalculated().CruiseStartLat = Basic().Latitude;
        SetCalculated().CruiseStartLong = Basic().Longitude;
        SetCalculated().CruiseStartAlt = Calculated().NavAltitude;
        SetCalculated().CruiseStartTime = Basic().Time;
      } else {

        DistanceBearing(Basic().Latitude, Basic().Longitude,
                        Calculated().CruiseStartLat,
                        Calculated().CruiseStartLong, &DistanceFlown, NULL);
        SetCalculated().CruiseLD =
          UpdateLD(Calculated().CruiseLD,
                   DistanceFlown,
                   Calculated().CruiseStartAlt - Calculated().NavAltitude,
                   0.5);
      }
    }
}

//////


void GlideComputerAirData::Heading()
{
  double x0, y0, mag;
  static double LastTime = 0;
  static double lastHeading = 0;

  if ((Basic().Speed>0)||(Calculated().WindSpeed>0)) {

    x0 = fastsine(Basic().TrackBearing)*Basic().Speed;
    y0 = fastcosine(Basic().TrackBearing)*Basic().Speed;
    x0 += fastsine(Calculated().WindBearing)*Calculated().WindSpeed;
    y0 += fastcosine(Calculated().WindBearing)*Calculated().WindSpeed;

    if (!Calculated().Flying) {
      // don't take wind into account when on ground
      SetCalculated().Heading = Basic().TrackBearing;
    } else {
      SetCalculated().Heading = AngleLimit360(atan2(x0,y0)*RAD_TO_DEG);
    }

    // calculate turn rate in wind coordinates
    if(Basic().Time > LastTime) {
      double dT = Basic().Time - LastTime;

      SetCalculated().TurnRateWind = AngleLimit180(Calculated().Heading
                                               - lastHeading)/dT;

      lastHeading = Calculated().Heading;
    }
    LastTime = Basic().Time;

    // calculate estimated true airspeed
    mag = isqrt4((unsigned long)(x0*x0*100+y0*y0*100))/10.0;
    SetCalculated().TrueAirspeedEstimated = mag;

    // estimate bank angle (assuming balanced turn)
    double angle = atan(DEG_TO_RAD*Calculated().TurnRateWind*
			Calculated().TrueAirspeedEstimated/9.81);

    SetCalculated().BankAngle = RAD_TO_DEG*angle;
    SetCalculated().Gload = 1.0/max(0.001,fabs(cos(angle)));

    // estimate pitch angle (assuming balanced turn)
    SetCalculated().PitchAngle = RAD_TO_DEG*
      atan2(Calculated().GPSVario-Calculated().Vario,
	    Calculated().TrueAirspeedEstimated);

    DoWindZigZag();

  } else {
    SetCalculated().Heading = Basic().TrackBearing;
  }
}


void GlideComputerAirData::EnergyHeightNavAltitude()
{
  // Determine which altitude to use for nav functions
  if (EnableNavBaroAltitude && Basic().BaroAltitudeAvailable) {
    SetCalculated().NavAltitude = Basic().BaroAltitude;
  } else {
    SetCalculated().NavAltitude = Basic().Altitude;
  }

  double ias_to_tas;
  double V_tas;

  if (Basic().AirspeedAvailable && (Basic().IndicatedAirspeed>0)) {
    ias_to_tas = Basic().TrueAirspeed/Basic().IndicatedAirspeed;
    V_tas = Basic().TrueAirspeed;
  } else {
    ias_to_tas = 1.0;
    V_tas = Calculated().TrueAirspeedEstimated;
  }
  double V_bestld_tas = GlidePolar::Vbestld*ias_to_tas;
  double V_mc_tas = Calculated().VMacCready*ias_to_tas;
  V_tas = max(V_tas, V_bestld_tas);
  double V_target = max(V_bestld_tas, V_mc_tas);
  SetCalculated().EnergyHeight =
    (V_tas*V_tas-V_target*V_target)/(9.81*2.0);
}


void GlideComputerAirData::TerrainHeight()
{
  short Alt = 0;

  terrain.Lock();
  // want most accurate rounding here
  terrain.SetTerrainRounding(0,0);
  Alt = terrain.GetTerrainHeight(Basic().Latitude,
				 Basic().Longitude);
  terrain.Unlock();

  if(Alt<0) {
    Alt = 0;
    if (Alt <= TERRAIN_INVALID) {
      SetCalculated().TerrainValid = false;
    } else {
      SetCalculated().TerrainValid = true;
    }
    SetCalculated().TerrainAlt = 0;
  } else {
    SetCalculated().TerrainValid = true;
    SetCalculated().TerrainAlt = Alt;
  }
  SetCalculated().AltitudeAGL = Calculated().NavAltitude - Calculated().TerrainAlt;

  if (!FinalGlideTerrain) {
    SetCalculated().TerrainBase = Calculated().TerrainAlt;
  }
}


void GlideComputerAirData::Vario()
{
  static double LastTime = 0;
  static double LastAlt = 0;
  static double LastAltTE = 0;
  static double h0last = 0;

  if(Basic().Time <= LastTime) {
    LastTime = Basic().Time;
  } else {
    double Gain = Calculated().NavAltitude - LastAlt;
    double GainTE = (Calculated().EnergyHeight+Basic().Altitude) - LastAltTE;
    double dT = (Basic().Time - LastTime);
    // estimate value from GPS
    SetCalculated().GPSVario = Gain / dT;
    SetCalculated().GPSVarioTE = GainTE / dT;

    double dv = (Calculated().TaskAltitudeDifference-h0last)
      /(Basic().Time-LastTime);
    SetCalculated().DistanceVario = LowPassFilter(Calculated().DistanceVario,
                                              dv, 0.1);

    h0last = Calculated().TaskAltitudeDifference;

    LastAlt = Calculated().NavAltitude;
    LastAltTE = Calculated().EnergyHeight+Basic().Altitude;
    LastTime = Basic().Time;

  }

  if (!Basic().VarioAvailable || ReplayLogger::IsEnabled()) {
    SetCalculated().Vario = Calculated().GPSVario;

  } else {
    // get value from instrument
    SetCalculated().Vario = Basic().Vario;
    // we don't bother with sound here as it is polled at a
    // faster rate in the DoVarioCalcs methods

    CalibrationUpdate(&Basic(), &Calculated());
  }
}


///////

void
GlideComputerAirData::SpeedToFly(const double mc_setting,
				 const double cruise_efficiency)
{
  double n;
  // get load factor
  if (Basic().AccelerationAvailable) {
    n = fabs(Basic().Gload);
  } else {
    n = fabs(Calculated().Gload);
  }

  // calculate optimum cruise speed in current track direction
  // this still makes use of mode, so it should agree with
  // Vmcready if the track bearing is the best cruise track
  // this does assume g loading of 1.0

  // this is basically a dolphin soaring calculator

  double delta_mc;
  double risk_mc;
  if (Calculated().TaskAltitudeDifference> -120) {
    risk_mc = mc_setting;
  } else {
    risk_mc =
      GlidePolar::MacCreadyRisk(Calculated().NavAltitude+Calculated().EnergyHeight
                                -SAFETYALTITUDEBREAKOFF-Calculated().TerrainBase,
                                Calculated().MaxThermalHeight,
                                mc_setting);
  }
  SetCalculated().MacCreadyRisk = risk_mc;

  if (EnableBlockSTF) {
    delta_mc = risk_mc;
  } else {
    delta_mc = risk_mc-Calculated().NettoVario;
  }

  if (1 || (Calculated().Vario <= risk_mc)) {
    // thermal is worse than mc threshold, so find opt cruise speed

    double VOptnew;

    if (!ValidTaskPoint(ActiveWayPoint) || !Calculated().FinalGlide) {
      // calculate speed as if cruising, wind has no effect on opt speed
      GlidePolar::MacCreadyAltitude(delta_mc,
                                    100.0, // dummy value
                                    Basic().TrackBearing,
                                    0.0,
                                    0.0,
                                    NULL,
                                    &VOptnew,
                                    false,
                                    NULL, 0, cruise_efficiency);
    } else {
      GlidePolar::MacCreadyAltitude(delta_mc,
                                    100.0, // dummy value
                                    Basic().TrackBearing,
                                    Calculated().WindSpeed,
                                    Calculated().WindBearing,
                                    0,
                                    &VOptnew,
                                    true,
                                    NULL, 1.0e6, cruise_efficiency);
    }

    // put low pass filter on VOpt so display doesn't jump around
    // too much
    if (Calculated().Vario <= risk_mc) {
      SetCalculated().VOpt = max(Calculated().VOpt,
			     GlidePolar::Vminsink*sqrt(n));
    } else {
      SetCalculated().VOpt = max(Calculated().VOpt,
			     GlidePolar::Vminsink);
    }
    SetCalculated().VOpt = LowPassFilter(Calculated().VOpt,VOptnew, 0.6);

  } else {
    // this thermal is better than maccready, so fly at minimum sink
    // speed
    // calculate speed of min sink adjusted for load factor
    SetCalculated().VOpt = GlidePolar::Vminsink*sqrt(n);
  }

  SetCalculated().STFMode = !Calculated().Circling;
}

void
GlideComputerAirData::NettoVario()
{
  double n;
  // get load factor
  if (Basic().AccelerationAvailable) {
    n = fabs(Basic().Gload);
  } else {
    n = fabs(Calculated().Gload);
  }

  // calculate sink rate of glider for calculating netto vario

  bool replay_disabled = !ReplayLogger::IsEnabled();

  double glider_sink_rate;
  if (Basic().AirspeedAvailable && replay_disabled) {
    glider_sink_rate= GlidePolar::SinkRate(max(GlidePolar::Vminsink,
					       Basic().IndicatedAirspeed), n);
  } else {
    // assume zero wind (Speed=Airspeed, very bad I know)
    // JMW TODO accuracy: adjust for estimated airspeed
    glider_sink_rate= GlidePolar::SinkRate(max(GlidePolar::Vminsink,
					       Basic().Speed), n);
  }
  SetCalculated().GliderSinkRate = glider_sink_rate;

  if (Basic().NettoVarioAvailable && replay_disabled) {
    SetCalculated().NettoVario = Basic().NettoVario;
  } else {
    if (Basic().VarioAvailable && replay_disabled) {
      SetCalculated().NettoVario = Basic().Vario - glider_sink_rate;
    } else {
      SetCalculated().NettoVario = Calculated().Vario - glider_sink_rate;
    }
  }
}


bool
GlideComputerAirData::ProcessVario()
{
  static double LastTime = 0;
  const double mc = GlidePolar::GetMacCready();
  const double ce = GlidePolar::GetCruiseEfficiency();

  NettoVario();
  SpeedToFly(mc, ce);

  // has GPS time advanced?
  if(Basic().Time <= LastTime)
    {
      LastTime = Basic().Time;
      return false;
    }
  LastTime = Basic().Time;

  return true;
}


bool
GlideComputerAirData::FlightTimes()
{
  static double LastTime = 0;

  if ((Basic().Time != 0) && (Basic().Time <= LastTime))
    // 20060519:sgi added (Basic().Time != 0) dueto alwas return here
    // if no GPS time available
    {

      if ((Basic().Time<LastTime) && (!Basic().NAVWarning)) {
	// Reset statistics.. (probably due to being in IGC replay mode)
        ResetFlight(false);
      }

      LastTime = Basic().Time;
      return false;
    }

  LastTime = Basic().Time;

  double t = DetectStartTime(&Basic(), &Calculated());
  if (t>0) {
    SetCalculated().FlightTime = t;
  }

  TakeoffLanding();

  return true;
}

bool
GlideComputerAirData::ProcessIdle(const MapWindowProjection &map_projection)
{
  TerrainFootprint(map_projection.GetScreenDistanceMeters());

  static double lastTime = 0;
  if (Basic().Time<= lastTime) {
    lastTime = Basic().Time-6;
  } else {
    // calculate airspace warnings every 6 seconds
    AirspaceWarning(map_projection);
  }
}


void
GlideComputerAirData::TakeoffLanding()
{
  static int time_in_flight = 0;
  static int time_on_ground = 0;

  if (Basic().Speed>1.0) {
    // stop system from shutting down if moving
    XCSoarInterface::InterfaceTimeoutReset();
  }
  if (!Basic().NAVWarning) {
    if (Basic().Speed> TAKEOFFSPEEDTHRESHOLD) {
      time_in_flight++;
      time_on_ground=0;
    } else {
      if ((Calculated().AltitudeAGL<300)&&(Calculated().TerrainValid)) {
        time_in_flight--;
      } else if (!Calculated().TerrainValid) {
        time_in_flight--;
      }
      time_on_ground++;
    }
  }

  time_in_flight = min(60, max(0,time_in_flight));
  time_on_ground = min(30, max(0,time_on_ground));

  // JMW logic to detect takeoff and landing is as follows:
  //   detect takeoff when above threshold speed for 10 seconds
  //
  //   detect landing when below threshold speed for 30 seconds
  //
  // TODO accuracy: make this more robust by making use of terrain height data
  // if available

  if ((time_on_ground<=10)||(ReplayLogger::IsEnabled())) {
    // Don't allow 'OnGround' calculations if in IGC replay mode
    SetCalculated().OnGround = FALSE;
  }

  if (!Calculated().Flying) {
    // detect takeoff
    if (time_in_flight>10) {
      OnTakeoff();
    }
    if (time_on_ground>10) {
      SetCalculated().OnGround = TRUE;
      DoAutoQNH(&Basic(), &Calculated());
      // Do not reset QFE after landing.
      if (!WasFlying) {
	QFEAltitudeOffset=Basic().Altitude; // VENTA3 Automatic QFE
      }
    }
  } else {
    // detect landing
    if (time_in_flight==0) {
      // have been stationary for a minute
      OnLanding();
    }
  }
}


void GlideComputerAirData::OnLanding()
{
  InputEvents::processGlideComputer(GCE_LANDING);

  // JMWX  restore data calculated at finish so
  // user can review flight as at finish line
  
  // VENTA3 TODO maybe reset WasFlying to false, so that QFE is reset
  // though users can reset by hand anyway anytime..
  
  if (Calculated().ValidFinish) {
    RestoreFinish();
  }
  SetCalculated().Flying = FALSE;
}



void GlideComputerAirData::OnTakeoff()
{
  SetCalculated().Flying = TRUE;
  WasFlying=true; // VENTA3
  InputEvents::processGlideComputer(GCE_TAKEOFF);
  // reset stats on takeoff
  ResetFlight();
  
  SetCalculated().TakeOffTime= Basic().Time;
  
  // save stats in case we never finish
  SaveFinish();
}


// airspace stuff
void
GlideComputerAirData::PredictNextPosition()
{
  if(Calculated().Circling)
    {
      SetCalculated().NextLatitude = Basic().Latitude;
      SetCalculated().NextLongitude = Basic().Longitude;
      SetCalculated().NextAltitude =
        Calculated().NavAltitude + Calculated().Average30s * WarningTime;
    }
  else
    {
      FindLatitudeLongitude(Basic().Latitude,
                            Basic().Longitude,
                            Basic().TrackBearing,
                            Basic().Speed*WarningTime,
                            &SetCalculated().NextLatitude,
                            &SetCalculated().NextLongitude);

      if (Basic().BaroAltitudeAvailable) {
        SetCalculated().NextAltitude =
          Basic().BaroAltitude + Calculated().Average30s * WarningTime;
      } else {
        SetCalculated().NextAltitude =
          Calculated().NavAltitude + Calculated().Average30s * WarningTime;
      }
    }
    // MJJ TODO Predict terrain altitude
    SetCalculated().NextAltitudeAGL = 
      Calculated().NextAltitude - Calculated().TerrainAlt;

}

bool GlobalClearAirspaceWarnings = false;


void
GlideComputerAirData::AirspaceWarning(const MapWindowProjection &map_projection)
{
  unsigned int i;

  if(!AIRSPACEWARNINGS)
      return;

  static bool position_is_predicted = false;

  if (GlobalClearAirspaceWarnings == true) {
    GlobalClearAirspaceWarnings = false;
    SetCalculated().IsInAirspace = false;
  }

  position_is_predicted = !position_is_predicted;
  // every second time step, do predicted position rather than
  // current position

  double alt;
  double agl;
  double lat;
  double lon;

  if (position_is_predicted) {
    alt = Calculated().NextAltitude;
    agl = Calculated().NextAltitudeAGL;
    lat = Calculated().NextLatitude;
    lon = Calculated().NextLongitude;
  } else {
    if (Basic().BaroAltitudeAvailable) {
      alt = Basic().BaroAltitude;
    } else {
      alt = Basic().Altitude;
    }
    agl = Calculated().AltitudeAGL;
    lat = Basic().Latitude;
    lon = Basic().Longitude;
  }

  // JMW TODO enhancement: FindAirspaceCircle etc should sort results, return
  // the most critical or closest.

  if (AirspaceCircle) {
    for (i=0; i<NumberOfAirspaceCircles; i++) {

      if ((((AirspaceCircle[i].Base.Base != abAGL) 
	    && (alt >= AirspaceCircle[i].Base.Altitude))
           || ((AirspaceCircle[i].Base.Base == abAGL) 
	       && (agl >= AirspaceCircle[i].Base.AGL)))
          && (((AirspaceCircle[i].Top.Base != abAGL) 
	       && (alt < AirspaceCircle[i].Top.Altitude))
           || ((AirspaceCircle[i].Top.Base == abAGL) 
	       && (agl < AirspaceCircle[i].Top.AGL)))) {

        if ((iAirspaceMode[AirspaceCircle[i].Type] >= 2) &&
	    InsideAirspaceCircle(lon, lat, i)) {

          AirspaceWarnListAdd(&Basic(), &Calculated(), map_projection,
                              position_is_predicted, 1, i, false);
        }
      }
    }
  }

  // repeat process for areas

  if (AirspaceArea) {
    for (i=0; i<NumberOfAirspaceAreas; i++) {

      if ((((AirspaceArea[i].Base.Base != abAGL) 
	    && (alt >= AirspaceArea[i].Base.Altitude))
           || ((AirspaceArea[i].Base.Base == abAGL) 
	       && (agl >= AirspaceArea[i].Base.AGL)))
          && (((AirspaceArea[i].Top.Base != abAGL) 
	       && (alt < AirspaceArea[i].Top.Altitude))
           || ((AirspaceArea[i].Top.Base == abAGL) 
	       && (agl < AirspaceArea[i].Top.AGL)))) {

        if ((iAirspaceMode[AirspaceArea[i].Type] >= 2)
            && InsideAirspaceArea(lon, lat, i)){

          AirspaceWarnListAdd(&Basic(), &Calculated(), map_projection,
                              position_is_predicted, 0, i, false);
        }
      }
    }
  }

  AirspaceWarnListProcess(&Basic(), &Calculated(), map_projection);
}



void
GlideComputerAirData::TerrainFootprint(double screen_range)
{
  if (FinalGlideTerrain) {

    double bearing, distance;
    double lat, lon;
    bool out_of_range;

    // estimate max range (only interested in at most one screen distance away)
    // except we need to scan for terrain base, so 20km search minimum is required
    double mymaxrange = max(20000.0, screen_range);

    SetCalculated().TerrainBase = Calculated().TerrainAlt;

    for (int i=0; i<=NUMTERRAINSWEEPS; i++) {
      bearing = (i*360.0)/NUMTERRAINSWEEPS;
      distance = FinalGlideThroughTerrain(bearing,
					  &Basic(),
					  &Calculated(), &lat, &lon,
					  mymaxrange, &out_of_range,
					  &SetCalculated().TerrainBase);
      if (out_of_range) {
	FindLatitudeLongitude(Basic().Latitude, Basic().Longitude,
			      bearing,
			      mymaxrange*20,
			      &lat, &lon);
      }
      SetCalculated().GlideFootPrint[i].x = lon;
      SetCalculated().GlideFootPrint[i].y = lat;
    }
    SetCalculated().Experimental = Calculated().TerrainBase;
  }
}


bool BallastTimerActive = false;
int BallastSecsToEmpty = 120;

void
GlideComputerAirData::BallastDump()
{
  static double BallastTimeLast = -1;

  if (BallastTimerActive) {
    // JMW only update every 5 seconds to stop flooding the devices
    if (Basic().Time > BallastTimeLast+5) {
      double BALLAST = GlidePolar::GetBallast();
      double BALLAST_last = BALLAST;
      double dt = Basic().Time - BallastTimeLast;
      double percent_per_second = 1.0/max(10.0, BallastSecsToEmpty);
      BALLAST -= dt*percent_per_second;
      if (BALLAST<0) {
	BallastTimerActive = false;
	BALLAST = 0.0;
      }
      GlidePolar::SetBallast(BALLAST);
      if (fabs(BALLAST-BALLAST_last)>0.05) { // JMW update on 5 percent!
	GlidePolar::UpdatePolar(true);
      }
      BallastTimeLast = Basic().Time;
    }
  } else {
    BallastTimeLast = Basic().Time;
  }
}


////


#define CRUISE 0
#define WAITCLIMB 1
#define CLIMB 2
#define WAITCRUISE 3

#define MinTurnRate  4
#define CruiseClimbSwitch 15
#define ClimbCruiseSwitch 10
#define THERMAL_TIME_MIN 45.0

void
GlideComputerAirData::SwitchZoomClimb(bool isclimb, bool left)
{
  DoWindCirclingMode(left);
}

void
GlideComputerAirData::PercentCircling(const double Rate)
{
  // JMW circling % only when really circling,
  // to prevent bad stats due to flap switches and dolphin soaring

  if (Calculated().Circling && (Rate>MinTurnRate)) {
    //    timeCircling += (Basic->Time-LastTime);
    SetCalculated().timeCircling+= 1.0;
    SetCalculated().TotalHeightClimb += Calculated().GPSVario;
    ThermalBand();
  } else {
    //    timeCruising += (Basic->Time-LastTime);
    SetCalculated().timeCruising+= 1.0;
  }

  if (Calculated().timeCruising+Calculated().timeCircling>1) {
    SetCalculated().PercentCircling =
      100.0*(Calculated().timeCircling)/(Calculated().timeCruising+
                                        Calculated().timeCircling);
  } else {
    SetCalculated().PercentCircling = 0.0;
  }
}


void
GlideComputerAirData::Turning()
{
  static double LastTrack = 0;
  static double StartTime  = 0;
  static double StartLong = 0;
  static double StartLat = 0;
  static double StartAlt = 0;
  static double StartEnergyHeight = 0;
  static double LastTime = 0;
  static int MODE = CRUISE;
  static bool LEFT = FALSE;
  double Rate;
  static double LastRate=0;
  double dRate;
  double dT;

  if (!Calculated().Flying) return;

  if(Basic().Time <= LastTime) {
    LastTime = Basic().Time;
    return;
  }
  dT = Basic().Time - LastTime;
  LastTime = Basic().Time;

  Rate = AngleLimit180(Basic().TrackBearing-LastTrack)/dT;

  if (dT<2.0) {
    // time step ok

    // calculate acceleration
    dRate = (Rate-LastRate)/dT;

    double dtlead=0.3;
    // integrate assuming constant acceleration, for one second
    SetCalculated().NextTrackBearing = Basic().TrackBearing
      + dtlead*(Rate+0.5*dtlead*dRate);
    // s = u.t+ 0.5*a*t*t

    SetCalculated().NextTrackBearing =
      AngleLimit360(Calculated().NextTrackBearing);

  } else {
    // time step too big, so just take it at last measurement
    SetCalculated().NextTrackBearing = Basic().TrackBearing;
  }

  SetCalculated().TurnRate = Rate;

  // JMW limit rate to 50 deg per second otherwise a big spike
  // will cause spurious lock on circling for a long time
  if (Rate>50) {
    Rate = 50;
  }
  if (Rate<-50) {
    Rate = -50;
  }

  // average rate, to detect essing
  static double rate_history[60];
  double rate_ave=0;
  for (int i=59; i>0; i--) {
    rate_history[i] = rate_history[i-1];
    rate_ave += rate_history[i];
  }
  rate_history[0] = Rate;
  rate_ave /= 60;

  SetCalculated().Essing = fabs(rate_ave)*100/MinTurnRate;
  if (fabs(rate_ave)< MinTurnRate*2) {
    //    Calculated().Essing = rate_ave;
  }

  Rate = LowPassFilter(LastRate,Rate,0.3);
  LastRate = Rate;

  if(Rate <0)
    {
      if (LEFT) {
        // OK, already going left
      } else {
        LEFT = true;
      }
      Rate *= -1;
    } else {
    if (!LEFT) {
      // OK, already going right
    } else {
      LEFT = false;
    }
  }

  PercentCircling(Rate);

  LastTrack = Basic().TrackBearing;

  bool forcecruise = false;
  bool forcecircling = false;
  if (EnableExternalTriggerCruise && !(ReplayLogger::IsEnabled())) {
    forcecircling = triggerClimbEvent.test();
    forcecruise = !forcecircling;
  }

  switch(MODE) {
  case CRUISE:
    if((Rate >= MinTurnRate)||(forcecircling)) {
      StartTime = Basic().Time;
      StartLong = Basic().Longitude;
      StartLat  = Basic().Latitude;
      StartAlt  = Calculated().NavAltitude;
      StartEnergyHeight  = Calculated().EnergyHeight;
      MODE = WAITCLIMB;
    }
    if (forcecircling) {
      MODE = WAITCLIMB;
    } else {
      break;
    }
  case WAITCLIMB:
    if (forcecruise) {
      MODE = CRUISE;
      break;
    }
    if((Rate >= MinTurnRate)||(forcecircling)) {
      if( ((Basic().Time  - StartTime) > CruiseClimbSwitch)|| forcecircling) {
        SetCalculated().Circling = TRUE;
        // JMW Transition to climb
        MODE = CLIMB;
        SetCalculated().ClimbStartLat = StartLat;
        SetCalculated().ClimbStartLong = StartLong;
        SetCalculated().ClimbStartAlt = StartAlt+StartEnergyHeight;
        SetCalculated().ClimbStartTime = StartTime;

	OnClimbBase(StartAlt);

        // consider code: InputEvents GCE - Move this to InputEvents
        // Consider a way to take the CircleZoom and other logic
        // into InputEvents instead?
        // JMW: NO.  Core functionality must be built into the
        // main program, unable to be overridden.
        SwitchZoomClimb(true, LEFT);
        InputEvents::processGlideComputer(GCE_FLIGHTMODE_CLIMB);
      }
    } else {
      // nope, not turning, so go back to cruise
      MODE = CRUISE;
    }
    break;
  case CLIMB:
    DoWindCirclingSample();

    if((Rate < MinTurnRate)||(forcecruise)) {
      StartTime = Basic().Time;
      StartLong = Basic().Longitude;
      StartLat  = Basic().Latitude;
      StartAlt  = Calculated().NavAltitude;
      StartEnergyHeight  = Calculated().EnergyHeight;
      // JMW Transition to cruise, due to not properly turning
      MODE = WAITCRUISE;
    }
    if (forcecruise) {
      MODE = WAITCRUISE;
    } else {
      break;
    }
  case WAITCRUISE:
    if (forcecircling) {
      MODE = CLIMB;
      break;
    }
    if((Rate < MinTurnRate) || forcecruise) {
      if( ((Basic().Time  - StartTime) > ClimbCruiseSwitch) || forcecruise) {
        SetCalculated().Circling = FALSE;

        // Transition to cruise
        MODE = CRUISE;
        SetCalculated().CruiseStartLat = StartLat;
        SetCalculated().CruiseStartLong = StartLong;
        SetCalculated().CruiseStartAlt = StartAlt;
        SetCalculated().CruiseStartTime = StartTime;

 	InitLDRotary(&rotaryLD);

	OnClimbCeiling();

        SwitchZoomClimb(false, LEFT);
        InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);
      }

      //if ((Basic().Time  - StartTime) > ClimbCruiseSwitch/3) {
      // reset thermal locator if changing thermal cores
      // thermallocator.Reset();
      //}

    } else {
      // JMW Transition back to climb, because we are turning again
      MODE = CLIMB;
    }
    break;
  default:
    // error, go to cruise
    MODE = CRUISE;
  }

  // generate new wind vector if altitude changes or a new
  // estimate is available
  DoWindCirclingAltitude();

  if (EnableThermalLocator) {
    if (Calculated().Circling) {
      ScopeLock protect(mutexGlideComputer);
      thermallocator.AddPoint(Basic().Time, Basic().Longitude, Basic().Latitude,
			      Calculated().NettoVario);
      thermallocator.Update(Basic().Time, Basic().Longitude, Basic().Latitude,
			    Calculated().WindSpeed, Calculated().WindBearing,
			    Basic().TrackBearing,
			    &SetCalculated().ThermalEstimate_Longitude,
			    &SetCalculated().ThermalEstimate_Latitude,
			    &SetCalculated().ThermalEstimate_W,
			    &SetCalculated().ThermalEstimate_R);
    } else {
      SetCalculated().ThermalEstimate_W = 0;
      SetCalculated().ThermalEstimate_R = -1;
      ScopeLock protect(mutexGlideComputer);
      thermallocator.Reset();
    }
  }

  // update atmospheric model
  CuSonde::updateMeasurements(&Basic(), &Calculated());

}


///////////

void 
GlideComputerAirData::ThermalSources()
{
  double ground_longitude;
  double ground_latitude;
  double ground_altitude;

  {
    ScopeLock protect(mutexGlideComputer);
    thermallocator.EstimateThermalBase(Calculated().ThermalEstimate_Longitude,
				       Calculated().ThermalEstimate_Latitude,
				       Calculated().NavAltitude,
				       Calculated().LastThermalAverage,
				       Calculated().WindSpeed,
				       Calculated().WindBearing,
				       &ground_longitude,
				       &ground_latitude,
				       &ground_altitude
				       );
  }
  if (ground_altitude>0) {
    double tbest=0;
    int ibest=0;

    for (int i=0; i<MAX_THERMAL_SOURCES; i++) {
      if (Calculated().ThermalSources[i].LiftRate<0.0) {
	ibest = i;
	break;
      }
      double dt = Basic().Time - Calculated().ThermalSources[i].Time;
      if (dt> tbest) {
	tbest = dt;
	ibest = i;
      }
    }
    SetCalculated().ThermalSources[ibest].LiftRate = Calculated().LastThermalAverage;
    SetCalculated().ThermalSources[ibest].Latitude = ground_latitude;
    SetCalculated().ThermalSources[ibest].Longitude = ground_longitude;
    SetCalculated().ThermalSources[ibest].GroundHeight = ground_altitude;
    SetCalculated().ThermalSources[ibest].Time = Basic().Time;
  }
}

void
GlideComputerAirData::LastThermalStats()
{
  static int LastCircling = FALSE;

  if((Calculated().Circling == FALSE) && (LastCircling == TRUE)
     && (Calculated().ClimbStartTime>=0))
    {
      double ThermalTime = Calculated().CruiseStartTime
        - Calculated().ClimbStartTime;

      if(ThermalTime >0)
        {
          double ThermalGain = Calculated().CruiseStartAlt + Calculated().EnergyHeight
            - Calculated().ClimbStartAlt;

          if (ThermalGain>0) {
            if (ThermalTime>THERMAL_TIME_MIN) {

	      SetCalculated().LastThermalAverage = ThermalGain/ThermalTime;
	      SetCalculated().LastThermalGain = ThermalGain;
	      SetCalculated().LastThermalTime = ThermalTime;

	      OnDepartedThermal();

              if (EnableThermalLocator) {
                ThermalSources();
              }
            }
	  }
	}
    }
  LastCircling = Calculated().Circling;
}

void
GlideComputerAirData::ThermalBand()
{
  static double LastTime = 0;
  if(Basic().Time <= LastTime)
    {
      LastTime = Basic().Time;
      return;
    }
  LastTime = Basic().Time;

  // JMW TODO accuracy: Should really work out dt here,
  //           but i'm assuming constant time steps
  double dheight = Calculated().NavAltitude
    -SAFETYALTITUDEBREAKOFF
    -Calculated().TerrainBase; // JMW EXPERIMENTAL

  int index, i, j;

  if (dheight<0) {
    return; // nothing to do.
  }
  if (Calculated().MaxThermalHeight==0) {
    SetCalculated().MaxThermalHeight = dheight;
  }

  // only do this if in thermal and have been climbing
  if ((!Calculated().Circling)||(Calculated().Average30s<0)) return;

  if (dheight > Calculated().MaxThermalHeight) {

    // moved beyond ceiling, so redistribute buckets
    double max_thermal_height_new;
    double tmpW[NUMTHERMALBUCKETS];
    int tmpN[NUMTHERMALBUCKETS];
    double h;

    // calculate new buckets so glider is below max
    double hbuk = Calculated().MaxThermalHeight/NUMTHERMALBUCKETS;

    max_thermal_height_new = max(1, Calculated().MaxThermalHeight);
    while (max_thermal_height_new<dheight) {
      max_thermal_height_new += hbuk;
    }

    // reset counters
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      tmpW[i]= 0.0;
      tmpN[i]= 0;
    }
    // shift data into new buckets
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      h = (i)*(Calculated().MaxThermalHeight)/(NUMTHERMALBUCKETS);
      // height of center of bucket
      j = iround(NUMTHERMALBUCKETS*h/max_thermal_height_new);

      if (j<NUMTHERMALBUCKETS) {
        if (Calculated().ThermalProfileN[i]>0) {
          tmpW[j] += Calculated().ThermalProfileW[i];
          tmpN[j] += Calculated().ThermalProfileN[i];
        }
      }
    }
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      SetCalculated().ThermalProfileW[i]= tmpW[i];
      SetCalculated().ThermalProfileN[i]= tmpN[i];
    }
    SetCalculated().MaxThermalHeight= max_thermal_height_new;
  }

  index = min(NUMTHERMALBUCKETS-1,
	      iround(NUMTHERMALBUCKETS*(dheight/max(1.0,
		     Calculated().MaxThermalHeight))));

  SetCalculated().ThermalProfileW[index]+= Calculated().Vario;
  SetCalculated().ThermalProfileN[index]++;
}


