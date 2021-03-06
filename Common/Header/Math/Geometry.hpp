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

#ifndef XCSOAR_MATH_GEOMETRY_HPP
#define XCSOAR_MATH_GEOMETRY_HPP

void irotate(int &xin, int &yin, const double &angle);
void irotatescale(int &xin, int &yin, const double &angle, const double &scale,
                  double &x, double &y);
void rotate(double &xin, double &yin, const double &angle);
void frotate(float &xin, float &yin, const float &angle);
void rotatescale(double &xin, double &yin,
                 const double &angle, const double &scale);
void frotatescale(float &xin, float &yin,
                  const float &angle, const float &scale);

bool AngleInRange(double Angle0, double Angle1, double x, bool is_signed=false);
double AngleLimit180(double theta);
double AngleLimit360(double theta);

double Reciprocal(double InBound);
double BiSector(double InBound, double OutBound);
double HalfAngle(double Angle0, double Angle1);

#endif
