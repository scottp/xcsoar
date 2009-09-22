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

#ifndef XCSOAR_SNAIL_TRAIL_HPP
#define XCSOAR_SNAIL_TRAIL_HPP

#include "Screen/shapelib/mapshape.h"
#include "GPSClock.hpp"
#include "Poco/RWLock.h"
#include "Sizes.h"

#include <windef.h>

struct NMEA_INFO;
struct DERIVED_INFO;

typedef struct _SNAIL_POINT
{
  float Latitude;
  float Longitude;
  float Vario;
  double Time;
  short Colour;
  BOOL Circling;
  bool FarVisible;
  double DriftFactor;
} SNAIL_POINT;

class SnailTrail {
public:
  SnailTrail();
  void AddPoint(const NMEA_INFO *Basic, const DERIVED_INFO *Calculated);

  int getIndex() const {
    return indexNext;
  }
  const SNAIL_POINT& getPoint(int index) {
    return TrailPoints[index];
  }

  void ScanVisibility(rectObj *bounds);
  GPSClock clock;

  void ReadLock() {
    lock.readLock();
  }
  void Unlock() {
    lock.unlock();
  }
private:
  SNAIL_POINT TrailPoints[TRAILSIZE];
  int indexNext;
  Poco::RWLock lock;
};

#endif
