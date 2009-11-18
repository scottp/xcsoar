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

#include "Protection.hpp"
#include "MainWindow.hpp"
#include "SettingsComputer.hpp"
#include "SettingsUser.hpp"
#include "SettingsTask.hpp"
#include "RasterTerrain.h"
#include "Waypointparser.h"
#include "AirfieldDetails.h"
#include "Airspace.h"
#include "TopologyStore.h"
#include "McReady.h"
#include "Dialogs.h"
#include "Device/device.h"
#include "Message.h"
#include "Polar/Historical.hpp"
#include "TopologyStore.h"
#include "Components.hpp"
#include "Interface.hpp"
#include "Asset.hpp"

bool COMPORTCHANGED = false;
bool MAPFILECHANGED = false;
bool AIRSPACEFILECHANGED = false;
bool AIRFIELDFILECHANGED = false;
bool WAYPOINTFILECHANGED = false;
bool TERRAINFILECHANGED = false;
bool TOPOLOGYFILECHANGED = false;
bool POLARFILECHANGED = false;
bool LANGUAGEFILECHANGED = false;
bool STATUSFILECHANGED = false;
bool INPUTFILECHANGED = false;

void SettingsEnter() {
  draw_thread->suspend();
  // This prevents the map and calculation threads from doing anything
  // with shared data while it is being changed (also prevents drawing)

  MAPFILECHANGED = false;
  AIRSPACEFILECHANGED = false;
  AIRFIELDFILECHANGED = false;
  WAYPOINTFILECHANGED = false;
  TERRAINFILECHANGED = false;
  TOPOLOGYFILECHANGED = false;
  POLARFILECHANGED = false;
  LANGUAGEFILECHANGED = false;
  STATUSFILECHANGED = false;
  INPUTFILECHANGED = false;
  COMPORTCHANGED = false;
}

void SettingsLeave() {
  if (!globalRunningEvent.test()) return;

  XCSoarInterface::main_window.map.set_focus();

  // mutexing.Lock everything here prevents the calculation thread
  // from running, while shared data is potentially reloaded.

  mutexBlackboard.Lock();

/*
  if (MAPFILECHANGED) { printf("MAPFILECHANGED\n"); }
  if (AIRSPACEFILECHANGED) { printf("AIRSPACEFILECHANGED\n"); }
  if (AIRFIELDFILECHANGED) { printf("AIRFIELDFILECHANGED\n"); }
  if (WAYPOINTFILECHANGED) { printf("WAYPOINTFILECHANGED\n"); }
  if (TERRAINFILECHANGED) { printf("TERRAINFILECHANGED\n"); }
  if (TOPOLOGYFILECHANGED) { printf("TOPOLOGYFILECHANGED\n"); }
  if (POLARFILECHANGED) { printf("POLARFILECHANGED\n"); }
  if (LANGUAGEFILECHANGED) { printf("LANGUAGEFILECHANGED\n"); }
  if (STATUSFILECHANGED) { printf("STATUSFILECHANGED\n"); }
  if (INPUTFILECHANGED) { printf("INPUTFILECHANGED\n"); }
  if (COMPORTCHANGED) { printf("COMPORTCHANGED\n"); }
*/

  if(MAPFILECHANGED) {
    AIRSPACEFILECHANGED = true;
    AIRFIELDFILECHANGED = true;
    WAYPOINTFILECHANGED = true;
    TERRAINFILECHANGED = true;
    TOPOLOGYFILECHANGED = true;
  }

  if((WAYPOINTFILECHANGED) || (TERRAINFILECHANGED) || (AIRFIELDFILECHANGED)) {
    task.ClearTask();

    // re-load terrain
    terrain.CloseTerrain();
    terrain.OpenTerrain();

    // re-load waypoints
    ReadWayPoints(way_points, terrain);
    ReadAirfieldFile();

    // re-set home
    if (WAYPOINTFILECHANGED || TERRAINFILECHANGED) {
      SetHome(way_points, terrain, XCSoarInterface::SetSettingsComputer(),
          WAYPOINTFILECHANGED);
    }

    terrain.ServiceFullReload(XCSoarInterface::Basic().Location);

    task.RefreshTask(XCSoarInterface::SetSettingsComputer());
  }

  if (TOPOLOGYFILECHANGED) {
    topology->Close();
    topology->Open();
  }

  if(AIRSPACEFILECHANGED) {
    CloseAirspace();
    ReadAirspace();
    SortAirspace();
  }

  if (POLARFILECHANGED) {
    CalculateNewPolarCoef();
    GlidePolar::UpdatePolar(false, XCSoarInterface::SettingsComputer());
  }

  if (AIRFIELDFILECHANGED
      || AIRSPACEFILECHANGED
      || WAYPOINTFILECHANGED
      || TERRAINFILECHANGED
      || TOPOLOGYFILECHANGED
      ) {
    XCSoarInterface::CloseProgressDialog();
    XCSoarInterface::main_window.map.set_focus();
  }

  mutexBlackboard.Unlock();

  if(COMPORTCHANGED) {
    devRestart();
  }

  draw_thread->resume();
  // allow map and calculations threads to continue on their merry way
}


void SystemConfiguration(void) {
  if (!is_simulator() && XCSoarInterface::LockSettingsInFlight
      && XCSoarInterface::Calculated().Flying) {
    Message::AddMessage(TEXT("Settings locked in flight"));
    return;
  }

  SettingsEnter();
  dlgConfigurationShowModal();
  SettingsLeave();
}

