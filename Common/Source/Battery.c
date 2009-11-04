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

#include "Battery.h"

#if defined(HAVE_BLANK) && !defined(GNAV) && !defined(WINDOWSPC)

/** Battery percentage (default = 100%) */
int PDABatteryPercent = 100;
/** Battery temperature (default = 0�C (?)) */
int PDABatteryTemperature = 0;

/** Warning time before battery is empty */
DWORD BatteryWarningTime = 0;

/**
 * Reads the battery information into the BATTERYINFO struct
 * @param pBatteryInfo Pointer to the BATTERYINFO struct
 * @return Success of the reading
 */
DWORD GetBatteryInfo(BATTERYINFO* pBatteryInfo) {
  // set default return value
  DWORD result = 0;

  // check incoming pointer
  if (NULL == pBatteryInfo) {
    return 0;
  }

  SYSTEM_POWER_STATUS_EX2 sps;

  // request the power status
  result = GetSystemPowerStatusEx2(&sps, sizeof(sps), TRUE);

  // only update the caller if the previous call succeeded
  if (0 != result) {
    pBatteryInfo->acStatus = sps.ACLineStatus;
    pBatteryInfo->chargeStatus = sps.BatteryFlag;
    pBatteryInfo->BatteryLifePercent = sps.BatteryLifePercent;
    // VENTA get everything ready for PNAs battery control
    pBatteryInfo->BatteryVoltage = sps.BatteryVoltage;
    pBatteryInfo->BatteryAverageCurrent = sps.BatteryAverageCurrent;
    pBatteryInfo->BatteryCurrent = sps.BatteryCurrent;
    pBatteryInfo->BatterymAHourConsumed = sps.BatterymAHourConsumed;
    pBatteryInfo->BatteryTemperature = sps.BatteryTemperature;
  }

  return result;
}

#endif
