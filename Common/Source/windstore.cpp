/***********************************************************************
**
**   windstore.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by Andr� Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/*
NOTE: Some portions copyright as above

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
#include "windstore.h"
#include "XCSoar.h"
#include "Units.hpp"
#include "NMEA/Info.h"
#include "NMEA/Derived.hpp"

#include <math.h>

WindStore::WindStore() {
  //create the lists
  windlist = new WindMeasurementList();
  updated = true;
}


WindStore::~WindStore(){
  delete windlist;
}


/**
  * Called with new measurements. The quality is a measure for how
  * good the measurement is. Higher quality measurements are more
  * important in the end result and stay in the store longer.
  */
void
WindStore::slot_measurement(const NMEA_INFO *nmeaInfo,
                            DERIVED_INFO *derivedInfo,
                            Vector windvector, int quality)
{
  updated = true;
  windlist->addMeasurement(nmeaInfo->Time, windvector, nmeaInfo->Altitude, quality);
  //we may have a new wind value, so make sure it's emitted if needed!
  recalculateWind(nmeaInfo, derivedInfo);
}


/**
  * Called if the altitude changes.
  * Determines where measurements are stored and may result in a
  * newWind signal.
  */

void
WindStore::slot_Altitude(const NMEA_INFO *nmeaInfo,
                         DERIVED_INFO *derivedInfo)
{
  if ((fabs(nmeaInfo->Altitude-_lastAltitude)>100.0)||(updated)) {
    //only recalculate if there is a significant change
    recalculateWind(nmeaInfo, derivedInfo);

    updated = false;
    _lastAltitude=nmeaInfo->Altitude;
  }
}


Vector WindStore::getWind(double Time, double h, bool *found) {
  return windlist->getWind(Time, h, found);
}

/** Recalculates the wind from the stored measurements.
  * May result in a newWind signal. */

void
WindStore::recalculateWind(const NMEA_INFO *nmeaInfo,
                           DERIVED_INFO *derivedInfo)
{
  bool found;
  Vector CurWind= windlist->getWind(nmeaInfo->Time,
                                    nmeaInfo->Altitude, &found);

  if (found) {
    if ((fabs(CurWind.x-_lastWind.x)>1.0) ||
	(fabs(CurWind.y-_lastWind.y)>1.0) || updated) {
      _lastWind=CurWind;

      updated = false;
      _lastAltitude=nmeaInfo->Altitude;

      newWind(nmeaInfo, derivedInfo, CurWind);
    }
  } // otherwise, don't change anything

}


void
WindStore::newWind(const NMEA_INFO *nmeaInfo, DERIVED_INFO *derivedInfo,
                   Vector &wind)
{
  //
  double mag = sqrt(wind.x*wind.x+wind.y*wind.y);
  double bearing;

  if (wind.y == 0 && wind.x == 0)
    bearing = 0;
  else
    bearing = atan2(wind.y, wind.x)*RAD_TO_DEG;

  if (mag<30) { // limit to reasonable values
    derivedInfo->WindSpeed = mag;
    if (bearing<0) {
      bearing += 360;
    }
    derivedInfo->WindBearing = bearing;
  } else {
    // TODO code: give warning, wind estimate bogus or very strong!
  }

#ifdef DEBUG_WIND
  DebugStore("%f %f 0 # wind estimate\n",wind.x,wind.y);
#endif

}
