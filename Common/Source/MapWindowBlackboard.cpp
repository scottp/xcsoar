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
#include "MapWindowBlackboard.hpp"

void
MapWindowBlackboard::ReadSettingsComputer(const SETTINGS_COMPUTER
					      &settings)
{
  memcpy(&settings_computer,&settings,sizeof(SETTINGS_COMPUTER));
}

void
MapWindowBlackboard::ReadSettingsMap(const SETTINGS_MAP
				     &settings)
{
  memcpy(&settings_map,&settings,sizeof(SETTINGS_MAP));
}

void
MapWindowBlackboard::ReadBlackboard(const NMEA_INFO &nmea_info,
				    const DERIVED_INFO &derived_info)
{
  memcpy(&gps_info,&nmea_info,sizeof(NMEA_INFO));
  memcpy(&calculated_info,&derived_info,sizeof(DERIVED_INFO));
}

