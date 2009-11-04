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

#ifndef XCSOAR_NMEA_DERIVED_H
#define XCSOAR_NMEA_DERIVED_H

#include "Screen/shapelib/mapshape.h"
#include "GeoPoint.hpp"

#define NUMTHERMALBUCKETS 10
#define MAX_THERMAL_SOURCES 20

struct THERMAL_SOURCE_INFO
{
  GEOPOINT Location;
  double GroundHeight;
  double LiftRate;
  double Time;
};

typedef enum {
  CRUISE= 0,
  WAITCLIMB,
  CLIMB,
  WAITCRUISE
} CirclingMode_t;


struct DERIVED_INFO
{
  double Vario; /**< Vertical speed */
  double LD; /**< Instant glide ratio */
  double CruiseLD; /**< Glide ratio while in Cruise mode */
  int	 AverageLD; /**< Average glide ratio */
  double VMacCready; /**< MacCready speed */
  double Average30s; /**< Average vertical speed of 30s */
  double NettoAverage30s; /**< average vertical speed (of the airmass) of 30s */
  double BestCruiseTrack;
  double AverageThermal; /**< Average vertical speed in the thermal */
  double AdjustedAverageThermal;
  double ThermalGain; /**< Altitude gained while in the thermal */
  double LastThermalAverage; /**< Average vertical speed in the last thermal */
  double LastThermalGain; /**< Altitude gained while in the last thermal */
  double LastThermalTime; /**< Time spend in the last thermal */
  GEOPOINT ClimbStartLocation; /**< StartLocation of the current/last climb */
  double ClimbStartAlt; /**< StartAltitude of the current/last climb */
  double ClimbStartTime; /**< StartTime of the current/last climb */
  GEOPOINT CruiseStartLocation; /**< StartLocation of the current/last cruise */
  double CruiseStartAlt; /**< StartAltitude of the current/last cruise */
  double CruiseStartTime; /**< StartTime of the current/last cruise */
  double WindSpeed; /**< Wind speed */
  double WindBearing; /**< Wind bearing */
  double Bearing;
  double TerrainAlt; /**< Terrain altitude */
  bool   TerrainValid;
  double Heading;
  double AltitudeAGL; /**< Altitude over terrain */
  int    Circling;
  int    FinalGlide;
  int    Flying;
  int    TimeOnGround;
  int    TimeInFlight;
  bool   LandableReachable;
  double NextAltitudeRequired;
  double NextAltitudeRequired0; // mc=0
  double NextAltitudeDifference;
  double NextAltitudeDifference0; // difference with mc=0
  double FinalAltitudeRequired;
  double FinalAltitudeDifference;
  double TaskDistanceToGo;
  double TaskDistanceCovered;
  double TaskTimeToGo;
  double TaskStartTime; /**< StartTime of the task */
  double TaskSpeed;
  double TaskSpeedInstantaneous;
  double TaskAltitudeRequired;
  double TaskAltitudeDifference;
  double TaskAltitudeDifference0; // difference with mc=0
  double TaskAltitudeRequiredFromStart;
  double LDFinish;
  double LDNext;
  double LegDistanceToGo;
  double LegDistanceCovered;
  double LegTimeToGo;
  double LegStartTime;
  double LegSpeed;
  GEOPOINT NextLocation; /**< Predicted position after airspace warning time */
  double NextAltitude; /**< Predicted altitude after airspace warning time */
  double NextAltitudeAGL; /**< Predicted altitude over terrain after airspace warning time */
  double AATMaxDistance;
  double AATMinDistance;
  double AATTargetDistance;
  double AATTimeToGo;
  double AATMaxSpeed;
  double AATTargetSpeed;
  double AATMinSpeed;
  double PercentCircling; /**< Circling/Cruise ratio in percent */

  GEOPOINT TerrainWarningLocation;

  // JMW moved calculated waypoint info here

  double WaypointBearing;
  double WaypointDistance;
  double WaypointSpeed;

  // JMW thermal band data
  double MaxThermalHeight;
  int    ThermalProfileN[NUMTHERMALBUCKETS];
  double ThermalProfileW[NUMTHERMALBUCKETS];

  double NettoVario;

  // optimum speed to fly instantaneously
  double VOpt;

  // JMW estimated track bearing at next time step
  double NextTrackBearing;

  // whether Speed-To-Fly audio are valid or not
  bool STFMode;

  // JMW energy height excess to slow to best glide speed
  double EnergyHeight;

  // Turn rate in global coordinates
  double TurnRate;
  double SmoothedTurnRate;

  double TurnStartTime;
  GEOPOINT TurnStartLocation;
  double TurnStartAltitude;
  double TurnStartEnergyHeight;
  CirclingMode_t TurnMode;

  // reflects whether aircraft is in a start/finish/aat/turn sector
  bool IsInSector;
  bool IsInAirspace;
  bool InFinishSector;
  bool InStartSector;
  int StartSectorWaypoint;

  unsigned ActiveTaskPoint;
  int ReadyWayPoint;

  bool OnGround; /**< Detects when glider is on ground for several seconds */

  double NavAltitude; /**< Altitude used for navigation (GPS or Baro) */
  bool ValidStart;
  double TaskStartSpeed;
  double TaskStartAltitude;
  bool ValidFinish;

  double LDvario;

  GEOPOINT ThermalEstimate_Location;
  double ThermalEstimate_W;
  double ThermalEstimate_R;

  THERMAL_SOURCE_INFO ThermalSources[MAX_THERMAL_SOURCES];

  pointObj GlideFootPrint[NUMTERRAINSWEEPS+1];

  TCHAR OwnTeamCode[10];
  double TeammateBearing;
  double TeammateRange;
  GEOPOINT TeammateLocation;
  TCHAR  TeammateCode[10]; // auto-detected, see also in settings computer.h
  bool   TeammateCodeValid;

  double FlightTime; /**< Time of flight */
  double TakeOffTime; /**< Time of takeoff */

  double AverageClimbRate[200];
  long AverageClimbRateN[200];

  double HomeDistance; /**< Distance to home airport */
  double HomeRadial;


  double ZoomDistance;
  double TaskSpeedAchieved;
  double TrueAirspeedEstimated;

  double timeCruising;
  double timeCircling;

  double MinAltitude;
  double MaxHeightGain;

  // Turn rate in wind coordinates
  double GPSVario;
  double TurnRateWind;
  double BankAngle;
  double PitchAngle;
  double GPSVarioTE;
  double MacCreadyRisk;
  double TaskTimeToGoTurningNow;
  double TotalHeightClimb;
  double DistanceVario;
  double GliderSinkRate;
  double Gload;
  double Essing;
  double TerrainBase; /**< Lowest height within glide range */
  double TermikLigaPoints;
  double GRFinish;	// GRadient to final destination, 090203
			// Note: we don't need GRNext since this value is used when going to a landing
			// point, which is always a final glide.
  int    BestAlternate;
  double TimeSunset;
  double Experimental;
  // JMW note, new items should go at the bottom of this struct before experimental!
};

#endif

