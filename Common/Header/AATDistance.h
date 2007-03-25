/*
Copyright_License {

  XCSoar Glide Computer - http://xcsoar.sourceforge.net/
  Copyright (C) 2000 - 2005

        M Roberts (original release)
        Robin Birch <robinb@ruffnready.co.uk>
        Samuel Gisiger <samuel.gisiger@triadis.ch>
        Jeff Goodenough <jeff@enborne.f2s.com>
        Alastair Harrison <aharrison@magic.force9.co.uk>
        Scott Penrose <scottp@dd.com.au>
        John Wharington <jwharington@bigfoot.com>

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

#ifndef AATDISTANCE_H
#define AATDISTANCE_H

#include "stdafx.h"
#include "Calculations.h"
#include "Utils.h"

#define MAXNUM_AATDISTANCE 50

class AATDistance {
public:
  AATDistance();
  void Reset();

  void AddPoint(double longitude, double latitude, int taskwaypoint);
  double DistanceCovered(double longitude, double latitude, int taskwaypoint);
  double LegDistanceAchieved(int taskwaypoint);
  bool HasEntered(int taskwaypoint);
private:

  double DistanceCovered_internal(double longitude, double latitude,
                                  bool insector);

  void UpdateSearch(int taskwaypoint);
  void ThinData(int taskwaypoint);

  double max_achieved_distance;

  double distancethreshold[MAXTASKPOINTS];
  double legdistance_achieved[MAXTASKPOINTS];
  // index to max distance sample to task point n
  int imax[MAXTASKPOINTS][MAXNUM_AATDISTANCE];

  double Dmax[MAXTASKPOINTS][MAXNUM_AATDISTANCE];

  double lat_points[MAXTASKPOINTS][MAXNUM_AATDISTANCE];
  double lon_points[MAXTASKPOINTS][MAXNUM_AATDISTANCE];
  int num_points[MAXTASKPOINTS];

};

#endif
