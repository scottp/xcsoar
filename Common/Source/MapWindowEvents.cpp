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

#include "MapWindow.h"
#include "SettingsUser.hpp"
#include "GaugeVarioAltA.h"
#include "InfoBoxManager.h"
#include "Protection.hpp"
#include "InputEvents.h"
#include "Dialogs.h"
#include <stdlib.h>

/////////////////////////////////////////
// called from calculation thread!!

void MapWindow::SwitchZoomClimb(void) {

  //static double CruiseMapScale = 10;
  static double CruiseMapScale = MapWindow::RequestMapScale*2; // VNT 090621
  //static double ClimbMapScale = 0.25;
  static double ClimbMapScale = MapWindow::RequestMapScale/20;
  static bool last_isclimb = false;
  static bool last_targetpan = false;

  bool isclimb = (DisplayMode == dmCircling);

  LockTaskData();
  bool my_target_pan = TargetPan;
  UnlockTaskData();

  if (my_target_pan != last_targetpan) {
    if (my_target_pan) {
      // save starting values
      if (isclimb) {
        ClimbMapScale = MapScale;
      } else {
        CruiseMapScale = MapScale;
      }
    } else {
      // restore scales
      if (isclimb) {
        RequestMapScale = LimitMapScale(ClimbMapScale);
      } else {
        RequestMapScale = LimitMapScale(CruiseMapScale);
      }
      BigZoom = true;
    }
    last_targetpan = my_target_pan;
    return;
  }

  if (!my_target_pan && CircleZoom) {
    if (isclimb != last_isclimb) {
      if (isclimb) {
        // save cruise scale
        CruiseMapScale = MapScale;
        // switch to climb scale
        RequestMapScale = LimitMapScale(ClimbMapScale);
      } else {
        // leaving climb
        // save cruise scale
        ClimbMapScale = MapScale;
        RequestMapScale = LimitMapScale(CruiseMapScale);
        // switch to climb scale
      }
      BigZoom = true;
      last_isclimb = isclimb;
    }
  }
}

/////////////////////////////////////////
// called from UI

void MapWindow::ToggleFullScreenStart() {
  // ok, save the state.
  MapFullScreen = askFullScreen;

  // show infoboxes immediately

  if (MapFullScreen) {
    MapRect = MapRectBig;
    HideInfoBoxes();
  } else {
    MapRect = MapRectSmall;
    ShowInfoBoxes();
  }
  GaugeVario::Show(!MapFullScreen);

}


void MapWindow::RequestToggleFullScreen() {
  askFullScreen = !askFullScreen;
  RefreshMap();
}

void MapWindow::RequestFullScreen(bool full) {
  askFullScreen = full;
  RefreshMap();
}

// called from UI or input event handler (same thread)

void MapWindow::Event_AutoZoom(int vswitch) {
  if (vswitch== -1) {
    AutoZoom = !AutoZoom;
  } else {
    AutoZoom = (vswitch != 0); // 0 off, 1 on
  }

  if (AutoZoom) {
    if (EnablePan) {
      EnablePan = false;
      InputEvents::setMode(TEXT("default"));
      StoreRestoreFullscreen(false);
    }
  }
  RefreshMap();
}


void MapWindow::Event_PanCursor(int dx, int dy) {
  int X= (MapRect.right+MapRect.left)/2;
  int Y= (MapRect.bottom+MapRect.top)/2;
  double Xstart, Ystart, Xnew, Ynew;

  Screen2LatLon(X, Y, Xstart, Ystart);

  X+= (MapRect.right-MapRect.left)*dx/4;
  Y+= (MapRect.bottom-MapRect.top)*dy/4;
  Screen2LatLon(X, Y, Xnew, Ynew);

  if (EnablePan) {
    PanLongitude += Xstart-Xnew;
    PanLatitude += Ystart-Ynew;
  }
  RefreshMap();
}


/* Event_TerrainToplogy Changes
   0       Show
   1       Toplogy = ON
   2       Toplogy = OFF
   3       Terrain = ON
   4       Terrain = OFF
   -1      Toggle through 4 stages (off/off, off/on, on/off, on/on)
   -2      Toggle terrain
   -3      Toggle toplogy
*/
void MapWindow::Event_TerrainTopology(int vswitch) {
  char val;

  if (vswitch== -1) { // toggle through 4 possible options
    val = 0;
    if (EnableTopology) val++;
    if (EnableTerrain) val += (char)2;
    val++;
    if (val>3) val=0;
    EnableTopology = ((val & 0x01) == 0x01);
    EnableTerrain  = ((val & 0x02) == 0x02);
    RefreshMap();

  } else if (vswitch == -2) { // toggle terrain
    EnableTerrain = !EnableTerrain;
    RefreshMap();

  } else if (vswitch == -3) { // toggle topology
    EnableTopology = !EnableTopology;
    RefreshMap();

  } else if (vswitch == 1) { // Turn on toplogy
    EnableTopology = true;
    RefreshMap();

  } else if (vswitch == 2) { // Turn off toplogy
    EnableTopology = false;
    RefreshMap();

  } else if (vswitch == 3) { // Turn on terrain
    EnableTerrain = true;
    RefreshMap();

  } else if (vswitch == 4) { // Turn off terrain
    EnableTerrain = false;
    RefreshMap();

  } else if (vswitch == 0) { // Show terrain/Topology
    // ARH Let user know what's happening
    TCHAR buf[128];

    if (EnableTopology)
      _stprintf(buf, TEXT("\r\n%s / "), gettext(TEXT("ON")));
    else
      _stprintf(buf, TEXT("\r\n%s / "), gettext(TEXT("OFF")));

    if (EnableTerrain)
      _stprintf(buf+_tcslen(buf), TEXT("%s"), gettext(TEXT("ON")));
    else
      _stprintf(buf+_tcslen(buf), TEXT("%s"), gettext(TEXT("OFF")));
    DoStatusMessage(TEXT("Topology / Terrain"), buf);
  }
}


void MapWindow::Event_SetZoom(double value) {

  static double lastRequestMapScale = RequestMapScale;

  RequestMapScale = LimitMapScale(value);
  if (lastRequestMapScale != RequestMapScale){
    lastRequestMapScale = RequestMapScale;
    BigZoom = true;
    RefreshMap();
  }
}


void MapWindow::Event_ScaleZoom(int vswitch) {

  static double lastRequestMapScale = RequestMapScale;
  double value = RequestMapScale;
  static int nslow=0;

  // For best results, zooms should be multiples or roots of 2

  if (ScaleListCount > 0){
    value = FindMapScale(RequestMapScale);
    value = StepMapScale(-vswitch);
  } else {

    if (abs(vswitch)>=4) {
      nslow++;
      if (nslow %2 != 0) {
        // JMW disabled        return;
      }
      if (vswitch==4) {
        vswitch = 1;
      }
      if (vswitch==-4) {
        vswitch = -1;
      }
    }
    if (vswitch==1) { // zoom in a little
      value /= 1.414;
    }
    if (vswitch== -1) { // zoom out a little
      value *= 1.414;
    }
    if (vswitch==2) { // zoom in a lot
      value /= 2.0;
    }
    if (vswitch== -2) { // zoom out a lot
      value *= 2.0;
    }

  }
  RequestMapScale = LimitMapScale(value);

  if (lastRequestMapScale != RequestMapScale){
    lastRequestMapScale = RequestMapScale;
    BigZoom = true;
    RefreshMap();

    //    DrawMapScale(hdcScreen, MapRect, true);
    // JMW this is bad, happening from wrong thread.
  }
}