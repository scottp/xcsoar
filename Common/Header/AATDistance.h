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

#ifndef AATDISTANCE_H
#define AATDISTANCE_H

#include "Thread/Mutex.hpp"
#include "Protection.hpp"
#include "Sizes.h"
#include "GeoPoint.hpp"

#define MAXNUM_AATDISTANCE 50

class AATDistance {
public:
  AATDistance();
  void Reset();

  void AddPoint(const GEOPOINT &location,
                const unsigned taskwaypoint,
		const double aatclosedistance);
  double DistanceCovered(const GEOPOINT &location,
                         const int taskwaypoint,
			 const double aatclosedistance);
  double LegDistanceAchieved(int taskwaypoint);
  bool HasEntered(int taskwaypoint);
  void ResetEnterTrigger(int taskwaypoint);

private:

  double DistanceCovered_internal(const GEOPOINT &location,
                                  const bool insector,
				  const double aatclosedistance);
  double DistanceCovered_inside(const GEOPOINT &location,
				const double aatclosedistance);
  double DistanceCovered_outside(const GEOPOINT &location,
				 const double aatclosedistance);
  double distance_achieved(const int taskwaypoint, const int jbest,
                           const GEOPOINT &location);

  void UpdateSearch(int taskwaypoint);
  void ThinData(int taskwaypoint);

  double max_achieved_distance;

  bool has_entered[MAXTASKPOINTS];
  double distancethreshold[MAXTASKPOINTS];
  double legdistance_achieved[MAXTASKPOINTS];
  // index to max distance sample to task point n
  int imax[MAXTASKPOINTS][MAXNUM_AATDISTANCE];

  double Dmax[MAXTASKPOINTS][MAXNUM_AATDISTANCE];

  GEOPOINT loc_points[MAXTASKPOINTS][MAXNUM_AATDISTANCE];
  int best[MAXTASKPOINTS];
  int num_points[MAXTASKPOINTS];

  void ShiftTargetFromBehind(const GEOPOINT &location,
                             const int taskwaypoint, const double aatclosedistance);
  void ShiftTargetFromInFront(const GEOPOINT &location,
                              const int taskwaypoint, const double aatclosedistance);
  void ShiftTargetOutside(const GEOPOINT &location,
                          int taskwaypoint);
 private:
  Mutex mutexAAT;
  void Lock() {
    mutexAAT.Lock();
  }
  void Unlock() {
    mutexAAT.Unlock();
  }
};

#endif

