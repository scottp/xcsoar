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
#if !defined(XCSOAR_MAPWINDOW_H)
#define XCSOAR_MAPWINDOW_H

#include "MapWindowProjection.hpp"
#include "MapWindowTimer.hpp"
#include "Airspace.h"
#include "Thread/Trigger.hpp"
#include "Thread/Mutex.hpp"
#include "Screen/BufferCanvas.hpp"
#include "Screen/MaskedPaintWindow.hpp"
#include "Screen/LabelBlock.hpp"
#include "MapWindowBlackboard.hpp"
#include "PeriodClock.hpp"
#include "DrawThread.hpp"
#include "Task.h"

typedef struct _THERMAL_SOURCE_VIEW
{
  POINT Screen;
  bool Visible;
} THERMAL_SOURCE_VIEW;

struct ZoomClimb_t {
  double CruiseMapScale;
  double ClimbMapScale;
  bool last_isclimb;
  bool last_targetpan;
};

class GaugeCDI;
class TerrainRenderer;

class MapWindow
: public MaskedPaintWindow,
  public MapWindowProjection,
  public MapWindowBlackboard,
  public MapWindowTimer
{
  PeriodClock mouse_down_clock;

  GaugeCDI *cdi;
  TerrainRenderer *terrain_renderer;

 public:
  MapWindow();
  virtual ~MapWindow();

  static bool register_class(HINSTANCE hInstance);

  void set(ContainerWindow &parent,
           const RECT _MapRectSmall, const RECT _MapRectBig);

  // used by dlgTarget
  bool TargetDragged(double *longitude, double *latitude);

  // use at startup
  void SetMapRect(RECT rc) {
    move(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    SetRect(&MapRect, 0, 0, rc.right - rc.left, rc.bottom - rc.top);
  }

  void ApplyScreenSize();

  // input events or reused code
  void ExchangeBlackboard();

  ////////////////////////////////////////////////////////////////////

 private:

  void DrawThreadLoop ();
  void DrawThreadInitialise (void);
  Mutex    mutexBuffer;

  // state
  BOOL     Initialised;

  void     ReadBlackboard(const NMEA_INFO &nmea_info,
			  const DERIVED_INFO &derived_info);
  void     SendBlackboard(const NMEA_INFO &nmea_info,
			  const DERIVED_INFO &derived_info);

  // display management
  void          RefreshMap();
  void          SwitchZoomClimb(void);

  // state/localcopy/local data
  GEOPOINT      TargetDrag_Location;
  int           TargetDrag_State;

  POINT         Groundline[NUMTERRAINSWEEPS+1];
  TaskScreen_t   task_screen;
  StartScreen_t  task_start_screen;
  ZoomClimb_t    zoomclimb;

  THERMAL_SOURCE_VIEW ThermalSources[MAX_THERMAL_SOURCES];

  // projection
  bool      BigZoom;
  bool      FullScreen;
  void      StoreRestoreFullscreen(bool);

  // interface handlers
  int ProcessVirtualKey(int X, int Y, long keytime, short vkmode);

  // display element functions

  void ScanVisibilityWaypoints(rectObj *bounds_active);
  void ScanVisibilityAirspace(rectObj *bounds_active);
  void ScanVisibility(rectObj *bounds_active);

  void CalculateScreenPositions(POINT Orig, RECT rc,
                                       POINT *Orig_Aircraft);
  void CalculateScreenPositionsTask();
  void CalculateScreenPositionsWaypoints();
  void CalculateScreenPositionsGroundline();
  void CalculateScreenPositionsAirspace();
  void CalculateScreenPositionsAirspaceCircle(AIRSPACE_CIRCLE& circ);
  void CalculateScreenPositionsAirspaceArea(AIRSPACE_AREA& area);
  void CalculateScreenPositionsThermalSources();
  void MapWaypointLabelSortAndRender(Canvas &canvas);

  // display renderers
  void DrawAircraft(Canvas &canvas);
  void DrawCrossHairs(Canvas &canvas);
  void DrawGlideCircle(Canvas &canvas, const POINT Orig, const RECT rc); // VENTA3
  void DrawBestCruiseTrack(Canvas &canvas);
  void DrawCompass(Canvas &canvas, const RECT rc);
  void DrawHorizon(Canvas &canvas, const RECT rc);
  void DrawWindAtAircraft2(Canvas &canvas, POINT Orig, RECT rc);
  void DrawAirSpace(Canvas &canvas, const RECT rc, Canvas &buffer);
  void DrawWaypoints(Canvas &canvas);
  void DrawFlightMode(Canvas &canvas, const RECT rc);
  void DrawGPSStatus(Canvas &canvas, const RECT rc);
  double DrawTrail(Canvas &canvas);
  void DrawTeammate(Canvas &canvas);
  void DrawTrailFromTask(Canvas &canvas, const double TrailFirstTime);
  void DrawOffTrackIndicator(Canvas &canvas);
  void DrawProjectedTrack(Canvas &canvas);
  void DrawTask(Canvas &canvas, RECT rc);
  void DrawThermalEstimate(Canvas &canvas);
  void DrawTaskAAT(Canvas &canvas, const RECT rc, Canvas &buffer);
  void DrawAbortedTask(Canvas &canvas);

  void DrawBearing(Canvas &canvas, int bBearingValid);
  void DrawMapScale(Canvas &canvas, const RECT rc,
			   const bool ScaleChangeFeedback);
  void DrawMapScale2(Canvas &canvas, const RECT rc);
  void DrawFinalGlide(Canvas &canvas, const RECT rc);
  void DrawThermalBand(Canvas &canvas, const RECT rc);
  void DrawGlideThroughTerrain(Canvas &canvas);
  void DrawTerrainAbove(Canvas &hDC, const RECT rc, Canvas &buffer);
  void DrawCDI();
  void DrawSpotHeights(Canvas &canvas);
  //  void DrawSpeedToFly(HDC hDC, RECT rc);
  void DrawFLARMTraffic(Canvas &canvas);
  double    findMapScaleBarSize(const RECT rc);
  void ClearAirSpace(Canvas &dc, bool fill);

  // thread, main functions
  void Render(Canvas &canvas, const RECT rc);
  bool Idle(const bool force=false);

  // graphics vars

  BufferCanvas draw_canvas;
  BufferCanvas buffer_canvas;

  LabelBlock label_block;
 public:
  bool checkLabelBlock(RECT rc);
  LabelBlock *getLabelBlock() {
    return &label_block;
  }
  bool draw_masked_bitmap_if_visible(Canvas &canvas,
				     Bitmap &bitmap,
				     const GEOPOINT &loc,
				     unsigned width,
				     unsigned height,
				     POINT *sc=NULL);
 protected:
  virtual bool on_create();
  virtual bool on_destroy();
  virtual bool on_resize(unsigned width, unsigned height);
  virtual bool on_mouse_double(int x, int y);
  virtual bool on_mouse_move(int x, int y);
  virtual bool on_mouse_down(int x, int y);
  virtual bool on_mouse_up(int x, int y);
  virtual bool on_key_down(unsigned key_code);
  virtual void on_paint(Canvas& canvas);
  virtual bool on_setfocus();
 private:
  void RenderStart(Canvas &canvas, const RECT rc);
  void RenderBackground(Canvas &canvas, const RECT rc);
  void RenderMapLayer(Canvas &canvas, const RECT rc);
  void RenderAreas(Canvas &canvas, const RECT rc);
  void RenderTrail(Canvas &canvas, const RECT rc);
  void RenderTask(Canvas &canvas, const RECT rc);
  void RenderGlide(Canvas &canvas, const RECT rc);
  void RenderAirborne(Canvas &canvas, const RECT rc);
  void RenderSymbology_upper(Canvas &canvas, const RECT rc);
  void RenderSymbology_lower(Canvas &canvas, const RECT rc);

  friend class DrawThread;
};

#endif
