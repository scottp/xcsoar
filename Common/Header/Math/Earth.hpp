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

/*! \file
 * \brief Library for calculating Earth dimensions
 *
 * This library provides general functions for calculating dimensions
 * on the Earth with GPS coordinates.
 */

#ifndef XCSOAR_MATH_EARTH_HPP
#define XCSOAR_MATH_EARTH_HPP

#include "GeoPoint.hpp"
#include <stdlib.h> // for NULL

/**
 * Finds cross track error in meters and closest point P4 between P3
 * and desired track P1-P2.  Very slow function!
 */
double CrossTrackError(GEOPOINT loc1,
                       GEOPOINT loc2,
                       GEOPOINT loc3,
                       GEOPOINT *loc4);

/**
 * Calculates projected distance from P3 along line P1-P2.
 */
double ProjectedDistance(GEOPOINT loc1,
                         GEOPOINT loc2,
                         GEOPOINT loc3);

void DistanceBearing(GEOPOINT loc1,
                     GEOPOINT loc2,
                     double *Distance, double *Bearing);

double Distance(GEOPOINT loc1,
                GEOPOINT loc2);

double Bearing(GEOPOINT loc1,
               GEOPOINT loc2);

double DoubleDistance(GEOPOINT loc1,
                      GEOPOINT loc2,
		      GEOPOINT loc3);

void FindLatitudeLongitude(GEOPOINT loc,
                           double Bearing, double Distance,
                           GEOPOINT *loc_out);

#endif
