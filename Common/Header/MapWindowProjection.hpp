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
#if !defined(XCSOAR_MAPWINDOW_PROJECTION_H)
#define XCSOAR_MAPWINDOW_PROJECTION_H

#include "Math/FastMath.h"
#include "Units.hpp"
#include "SettingsComputer.hpp"
#include "SettingsUser.hpp"
#include "Screen/shapelib/mapprimitive.h"
#include "Sizes.h"
#include "GeoPoint.hpp"
#include <windef.h>

struct NMEA_INFO;
struct DERIVED_INFO;
class Canvas;

class MapWindowProjection {
 public:
  MapWindowProjection();

  // used by terrain renderer, topology and airspace

  void    Screen2LonLat(const int &x, const int &y,
                        GEOPOINT &location) const;

  void    LonLat2Screen(const GEOPOINT &location,
                        POINT &sc) const;
  void    LonLat2Screen(const pointObj* const ptin, POINT *ptout,
                        const int n,
                        const int skip) const;

  POINT   GetOrigScreen(void) const { return Orig_Screen; }
  POINT   GetOrigAircraft(void) const { return Orig_Aircraft; }
  GEOPOINT GetPanLocation() const { return PanLocation; }
  double  GetDisplayAngle() const { return DisplayAngle; }

  rectObj CalculateScreenBounds(double scale) const;

  double  GetScreenDistanceMeters(void) const;

  double GetScreenScaleToLonLat() const {
    return InvDrawScale;
  }
  double GetMapScaleUser() const { // Topology
    return MapScale;
  }
  double GetMapScaleKM() const {
    return MapScale*0.001/DISTANCEMODIFY;
  }
  RECT GetMapRectBig() const {
    return MapRectBig;
  }
  RECT GetMapRect() const {
    return MapRect;
  }

  // used by waypoint nearest routine
  bool WaypointInScaleFilter(int i) const;

  // drawing functions
  void DrawGreatCircle(Canvas &canvas,
                       const GEOPOINT &loc_start, const GEOPOINT &loc_end);

  rectObj* getSmartBounds() {
    return &smart_bounds_active;
  }

  // called on receipt of new data, to trigger projection/scale change functions
  void ExchangeBlackboard(const NMEA_INFO &nmea_info,
			  const DERIVED_INFO &derived_info,
			  const SETTINGS_MAP &settings_map);

  DisplayMode_t GetDisplayMode() const {
    return DisplayMode;
  }

  unsigned DistanceMetersToScreen(const double x) {
    return iround(_scale_meters_to_screen*x);
  }

 protected:
  DisplayMode_t DisplayMode;

  // helpers
  bool PointVisible(const POINT &P) const;
  bool LonLatVisible(const GEOPOINT &loc) const;
  bool PointInRect(const double &x, const double &y,
		   const rectObj &bounds) const;

  bool LonLat2ScreenIfVisible(const GEOPOINT &loc,
			      POINT *sc) const;

  rectObj   screenbounds_latlon;
  RECT   MapRectSmall;
  RECT   MapRectBig;
  RECT   MapRect;
  GEOPOINT PanLocation;
  POINT  Orig_Screen, Orig_Aircraft;

  double DisplayAngle;
  double DisplayAircraftAngle;

  // scale/display stuff
  void   CalculateOrigin(const RECT rc,
			 const NMEA_INFO &nmea_info,
			 const DERIVED_INFO &derived_info,
			 const SETTINGS_COMPUTER &settings_computer,
			 const SETTINGS_MAP &settings_map);

  void      InitialiseScaleList(const SETTINGS_MAP &settings);

  // 4 = x*30/1000
  double DistancePixelsToMeters(const double x) const {
    return x*MapScale/(DISTANCEMODIFY*GetMapResolutionFactor());
  }
  //
  double RequestDistancePixelsToMeters(const double x) const {
    return x*_RequestedMapScale/(DISTANCEMODIFY*GetMapResolutionFactor());
  }
  double DistanceScreenToUser(const unsigned x) const {
    return x*MapScale/GetMapResolutionFactor();
  }
  double RequestMapScale(double x, const SETTINGS_MAP &settings_map) {
    _RequestedMapScale = LimitMapScale(x, settings_map);
    return _RequestedMapScale;
  }
  double GetRequestedMapScale() const {
    return _RequestedMapScale;
  }
  double GetLonLatToScreenScale() const {
    return DrawScale;
  }
  bool IsOriginCentered() const {
    return _origin_centered;
  }

  bool SmartBounds(const bool force);
  double    StepMapScale(int Step) const;
public:
  bool HaveScaleList() const {
    return ScaleListCount>0;
  }
  double    StepMapScale(int Step);

 private:

  double DrawScale;
  double InvDrawScale;

  double _scale_meters_to_screen; // speedup
  double MapScale;
  double _RequestedMapScale;

  void   ModifyMapScale(const SETTINGS_MAP &settings_map);

  void   UpdateMapScale(const NMEA_INFO &nmea_info,
			const DERIVED_INFO &derived_info,
			const SETTINGS_MAP &settings_map);
  void   CalculateOrientationTargetPan(const NMEA_INFO &nmea_info,
				       const DERIVED_INFO &derived_info,
				       const SETTINGS_MAP &settings);
  void   CalculateOrientationNormal(const NMEA_INFO &nmea_info,
				    const DERIVED_INFO &derived_info,
				    const SETTINGS_MAP &settings);

  bool   _origin_centered;
  double    LimitMapScale(double value,
			  const SETTINGS_MAP &settings);
  double    FindMapScale(double Value);
  int       ScaleCurrent;
  double    ScaleList[SCALELISTSIZE];
  int       ScaleListCount;
  int GetMapResolutionFactor() const;

  rectObj smart_bounds_active;
  double smart_range_active;
};

#endif
