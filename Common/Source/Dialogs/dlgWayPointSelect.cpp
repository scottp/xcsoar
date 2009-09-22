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

#include "Dialogs/Internal.hpp"
#include "Math/Earth.hpp"
#include "MainWindow.hpp"
#include "Blackboard.hpp"
#include "InfoBoxLayout.h"
#include "Compatibility/string.h"
#include "Math/FastMath.h"
#include "DataField/Base.hpp"
#include "WayPointList.hpp"
#include "Components.hpp"

#include <assert.h>
#include <stdlib.h>

typedef struct{
  int Index;
  double Distance;
  double Direction;
  int    DirectionErr;
  int    Type;
  int    FileIdx;
  unsigned int FourChars;
} WayPointSelectInfo_t;

static GEOPOINT Location;

static WndForm *wf=NULL;
static WndListFrame *wWayPointList=NULL;
static WndOwnerDrawFrame *wWayPointListEntry = NULL;

static const TCHAR NameFilter[] = TEXT("*ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
static unsigned NameFilterIdx=0;

static double DistanceFilter[] = {0.0, 25.0, 50.0, 75.0, 100.0, 150.0, 250.0, 500.0, 1000.0};
static unsigned DistanceFilterIdx=0;

#define DirHDG -1

static int DirectionFilter[] = {0, DirHDG, 360, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330};
static unsigned DirectionFilterIdx=0;
static int lastHeading=0;

static const TCHAR *TypeFilter[] = {TEXT("*"), TEXT("Airport"), TEXT("Landable"),
				    TEXT("Turnpoint"), TEXT("File 1"), TEXT("File 2")};
static unsigned TypeFilterIdx=0;

static unsigned UpLimit=0;
static unsigned LowLimit=0;

static int ItemIndex = -1;

static int SelectedWayPointFileIdx = 0;


static void OnWaypointListEnter(WindowControl * Sender,
				WndListFrame::ListInfo_t *ListInfo){
	(void)Sender; (void)ListInfo;
  if (ItemIndex != -1) {
    wf->SetModalResult(mrOK);
  }
  else
    wf->SetModalResult(mrCancel);
}


static WayPointSelectInfo_t *WayPointSelectInfo=NULL;

static int _cdecl WaypointNameCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->FourChars < ((WayPointSelectInfo_t *)elem2)->FourChars)
    return (-1);
  if (((WayPointSelectInfo_t *)elem1)->FourChars > ((WayPointSelectInfo_t *)elem2)->FourChars)
    return (+1);
  return (0);
}

static int _cdecl WaypointDistanceCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->Distance < ((WayPointSelectInfo_t *)elem2)->Distance)
    return (-1);
  if (((WayPointSelectInfo_t *)elem1)->Distance > ((WayPointSelectInfo_t *)elem2)->Distance)
    return (+1);
  return (0);
}

static int _cdecl WaypointAirportCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->Type & (AIRPORT))
    return (-1);
  return (+1);
}

static int _cdecl WaypointLandableCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->Type & (AIRPORT | LANDPOINT))
    return (-1);
  return (+1);
}

static int _cdecl WaypointWayPointCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->Type & (TURNPOINT))
    return (-1);
  return (+1);
}

static int _cdecl WaypointFileIdxCompare(const void *elem1, const void *elem2 ){
  if (((WayPointSelectInfo_t *)elem1)->FileIdx != SelectedWayPointFileIdx)
    return (+1);
  return (-1);
}

static int _cdecl WaypointDirectionCompare(const void *elem1, const void *elem2 ){

  int a, a1, a2;

  a = DirectionFilter[DirectionFilterIdx];
  if (a == DirHDG){
    a = iround(XCSoarInterface::Calculated().Heading);
    lastHeading = a;
  }

  a1 = (int)(((WayPointSelectInfo_t *)elem1)->Direction - a);
  a2 = (int)(((WayPointSelectInfo_t *)elem2)->Direction - a);

  if (a1 > 180)
    a1 -=360;

  if (a1 < -180)
    a1 +=360;

  if (a2 > 180)
    a2 -=360;

  if (a2 < -180)
    a2 +=360;

  a1 = abs(a1);
  a2 = abs(a2);

  ((WayPointSelectInfo_t *)elem1)->DirectionErr = a1;
  ((WayPointSelectInfo_t *)elem2)->DirectionErr = a2;

  if (a1 < a2)
    return (-1);
  if (a1 > a2)
    return (+1);

  return (0);
}

static void PrepareData(void){

  TCHAR sTmp[5];

  WayPointSelectInfo = (WayPointSelectInfo_t*)malloc(sizeof(WayPointSelectInfo_t) *
                                                     way_points.get_count());

  for (unsigned i = 0; way_points.verify_index(i); ++i) {
    const WAYPOINT &way_point = way_points.get(i);

    WayPointSelectInfo[i].Index = i;

    DistanceBearing(Location,
                    way_point.Location,
                    &(WayPointSelectInfo[i].Distance),
                    &(WayPointSelectInfo[i].Direction));
    WayPointSelectInfo[i].Distance *= DISTANCEMODIFY;

    _tcsncpy(sTmp, way_point.Name, 4);
    sTmp[4] = '\0';
    _tcsupr(sTmp);

    WayPointSelectInfo[i].FourChars =
                    (((DWORD)sTmp[0] & 0xff) << 24)
                  + (((DWORD)sTmp[1] & 0xff) << 16)
                  + (((DWORD)sTmp[2] & 0xff) << 8)
                  + (((DWORD)sTmp[3] & 0xff) );

    WayPointSelectInfo[i].Type = way_point.Flags;

    WayPointSelectInfo[i].FileIdx = way_point.FileNum;

  }

  qsort(WayPointSelectInfo, UpLimit,
      sizeof(WayPointSelectInfo_t), WaypointNameCompare);

}


static void UpdateList(void){

//  TCHAR sTmp[128];
  bool distancemode = false;

  ItemIndex = 0;

  UpLimit = way_points.get_count();
  LowLimit =0;

  if (TypeFilterIdx == 1){
    qsort(WayPointSelectInfo, way_points.get_count(),
        sizeof(WayPointSelectInfo_t), WaypointAirportCompare);
    for (unsigned i = 0; way_points.verify_index(i); ++i) {
      if (!(WayPointSelectInfo[i].Type & (AIRPORT))){
        UpLimit = i;
        break;
      }
    }
  }

  if (TypeFilterIdx == 2){
    qsort(WayPointSelectInfo, way_points.get_count(),
        sizeof(WayPointSelectInfo_t), WaypointLandableCompare);
    for (unsigned i = 0; way_points.verify_index(i); ++i) {
      if (!(WayPointSelectInfo[i].Type & (AIRPORT | LANDPOINT))){
        UpLimit = i;
        break;
      }
    }
  }

  if (TypeFilterIdx == 3){
    qsort(WayPointSelectInfo, way_points.get_count(),
        sizeof(WayPointSelectInfo_t), WaypointWayPointCompare);
    for (unsigned i = 0; way_points.verify_index(i); ++i) {
      if (!(WayPointSelectInfo[i].Type & (TURNPOINT))){
        UpLimit = i;
        break;
      }
    }
  }

  if (TypeFilterIdx == 4 || TypeFilterIdx == 5){
    // distancemode = true;
    SelectedWayPointFileIdx = TypeFilterIdx-4;
    qsort(WayPointSelectInfo, way_points.get_count(),
        sizeof(WayPointSelectInfo_t), WaypointFileIdxCompare);
    for (unsigned i = 0; way_points.verify_index(i); ++i) {
      if (WayPointSelectInfo[i].FileIdx != SelectedWayPointFileIdx){
        UpLimit = i;
        break;
      }
    }
  }

  if (DistanceFilterIdx != 0){
    distancemode = true;
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointDistanceCompare);
    for (unsigned i = 0; i < UpLimit; i++){
      if (WayPointSelectInfo[i].Distance > DistanceFilter[DistanceFilterIdx]){
        UpLimit = i;
        break;
      }
    }
  }

  if (DirectionFilterIdx != 0){
    distancemode = true;
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointDirectionCompare);
    for (unsigned i = 0; i < UpLimit; i++){
      if (WayPointSelectInfo[i].DirectionErr > 18){
        UpLimit = i;
        break;
      }
    }
  }

  if (NameFilterIdx != 0){
    TCHAR sTmp[8];
    LowLimit = UpLimit;
    qsort(WayPointSelectInfo, UpLimit,
        sizeof(WayPointSelectInfo_t), WaypointNameCompare);
    sTmp[0] = NameFilter[NameFilterIdx];
    sTmp[1] = '\0';
    sTmp[2] = '\0';
    _tcsupr(sTmp);

    unsigned i;
    for (i = 0; i < UpLimit; i++){
      if ((BYTE)(WayPointSelectInfo[i].FourChars >> 24) >= (sTmp[0]&0xff)){
        LowLimit = i;
        break;
      }
    }

    for (; i<UpLimit; i++){
      if ((BYTE)(WayPointSelectInfo[i].FourChars >> 24) != (sTmp[0]&0xff)){
        UpLimit = i;
        break;
      }
    }
  }

  if (!distancemode) {
    qsort(&WayPointSelectInfo[LowLimit], UpLimit-LowLimit,
	  sizeof(WayPointSelectInfo_t), WaypointNameCompare);
  } else {
    qsort(&WayPointSelectInfo[LowLimit], UpLimit-LowLimit,
	  sizeof(WayPointSelectInfo_t), WaypointDistanceCompare);
  }

  wWayPointList->ResetList();
  wWayPointList->Redraw();

}


static WndProperty *wpName;
static WndProperty *wpDistance;
static WndProperty *wpDirection;

static void FilterMode(bool direction) {
  if (direction) {
    DistanceFilterIdx=0;
    DirectionFilterIdx=0;
    if (wpDistance) {
      wpDistance->GetDataField()->Set(TEXT("*"));
      wpDistance->RefreshDisplay();
    }
    if (wpDirection) {
      wpDirection->GetDataField()->Set(TEXT("*"));
      wpDirection->RefreshDisplay();
    }
  } else {
    NameFilterIdx=0;
    if (wpName) {
      wpName->GetDataField()->Set(TEXT("**"));
      wpName->RefreshDisplay();
    }
  }
}



static void OnFilterName(DataField *Sender, DataField::DataAccessKind_t Mode){

  TCHAR sTmp[12];

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    break;
    case DataField::daChange:
    break;
    case DataField::daInc:
      NameFilterIdx++;
      if (NameFilterIdx > sizeof(NameFilter)/sizeof(NameFilter[0])-2)
        NameFilterIdx = 1;
      FilterMode(true);
      UpdateList();
    break;
    case DataField::daDec:
      if (NameFilterIdx == 0)
        NameFilterIdx = sizeof(NameFilter)/sizeof(NameFilter[0])-1;
      else
        NameFilterIdx--;
      FilterMode(true);
      UpdateList();
    break;
  }

  _stprintf(sTmp, TEXT("%c*"), NameFilter[NameFilterIdx]);
  Sender->Set(sTmp);

}



static void OnFilterDistance(DataField *Sender, DataField::DataAccessKind_t Mode){

  TCHAR sTmp[12];

  switch(Mode){
    case DataField::daGet:
      Sender->Set(TEXT("25"));
    break;
    case DataField::daPut:
    break;
    case DataField::daChange:
    break;
    case DataField::daInc:
      DistanceFilterIdx++;
      if (DistanceFilterIdx > sizeof(DistanceFilter)/sizeof(DistanceFilter[0])-1)
        DistanceFilterIdx = 0;
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
      if (DistanceFilterIdx == 0)
        DistanceFilterIdx = sizeof(DistanceFilter)/sizeof(DistanceFilter[0])-1;
      else
        DistanceFilterIdx--;
      FilterMode(false);
      UpdateList();
    break;
  }

  if (DistanceFilterIdx == 0)
    _stprintf(sTmp, TEXT("%c"), '*');
  else
    _stprintf(sTmp, TEXT("%.0f%s"),
              DistanceFilter[DistanceFilterIdx],
              Units::GetDistanceName());
  Sender->Set(sTmp);
}


static void SetDirectionData(DataField *Sender){

  TCHAR sTmp[12];

  if (Sender == NULL){
    Sender = wpDirection->GetDataField();
  }

  if (DirectionFilterIdx == 0)
    _stprintf(sTmp, TEXT("%c"), '*');
  else if (DirectionFilterIdx == 1){
    int a = iround(XCSoarInterface::Calculated().Heading);
    if (a <=0)
      a += 360;
    _stprintf(sTmp, TEXT("HDG(%d")TEXT(DEG)TEXT(")"), a);
  }else
    _stprintf(sTmp, TEXT("%d")TEXT(DEG), DirectionFilter[DirectionFilterIdx]);

  Sender->Set(sTmp);

}

static void OnFilterDirection(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
      Sender->Set(TEXT("*"));
    break;
    case DataField::daPut:
    break;
    case DataField::daChange:
    break;
    case DataField::daInc:
      DirectionFilterIdx++;
      if (DirectionFilterIdx > sizeof(DirectionFilter)/sizeof(DirectionFilter[0])-1)
        DirectionFilterIdx = 0;
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
      if (DirectionFilterIdx == 0)
        DirectionFilterIdx = sizeof(DirectionFilter)/sizeof(DirectionFilter[0])-1;
      else
        DirectionFilterIdx--;
      FilterMode(false);
      UpdateList();
    break;
  }

  SetDirectionData(Sender);

}

static void OnFilterType(DataField *Sender, DataField::DataAccessKind_t Mode){

  TCHAR sTmp[12];

  switch(Mode){
    case DataField::daGet:
      Sender->Set(TEXT("*"));
    break;
    case DataField::daPut:
    break;
    case DataField::daChange:
    break;
    case DataField::daInc:
      TypeFilterIdx++;
      if (TypeFilterIdx > sizeof(TypeFilter)/sizeof(TypeFilter[0])-1)
        TypeFilterIdx = 0;
      FilterMode(false);
      UpdateList();
    break;
    case DataField::daDec:
      if (TypeFilterIdx == 0)
        TypeFilterIdx = sizeof(TypeFilter)/sizeof(TypeFilter[0])-1;
      else
        TypeFilterIdx--;
      FilterMode(false);
      UpdateList();
    break;
  }

  _stprintf(sTmp, TEXT("%s"), TypeFilter[TypeFilterIdx]);

  Sender->Set(sTmp);

}

static int DrawListIndex=0;

static void
OnPaintListItem(WindowControl *Sender, Canvas &canvas)
{
  (void)Sender;
  int n = UpLimit - LowLimit;
  TCHAR sTmp[12];

  if (DrawListIndex < n){

    int i = LowLimit + DrawListIndex;
    const WAYPOINT &way_point = way_points.get(WayPointSelectInfo[i].Index);

// Sleep(100);

    int w0, w1, w2, w3, x1, x2, x3;
    if (InfoBoxLayout::landscape) {
      w0 = 202*InfoBoxLayout::scale;
    } else {
      w0 = 225*InfoBoxLayout::scale;
    }
    w1 = canvas.text_width(TEXT("XXX"));
    w2 = canvas.text_width(TEXT(" 000km"));
    w3 = canvas.text_width(TEXT(" 000")TEXT(DEG));

    x1 = w0-w1-w2-w3;

    canvas.text_clipped(2 * InfoBoxLayout::scale, 2 * InfoBoxLayout::scale,
                        x1 - InfoBoxLayout::scale * 5,
                        way_point.Name);

    sTmp[0] = '\0';
    sTmp[1] = '\0';
    sTmp[2] = '\0';

    if (way_point.Flags & HOME){
      sTmp[0] = 'H';
    }else
    if (way_point.Flags & AIRPORT){
      sTmp[0] = 'A';
    }else
    if (way_point.Flags & LANDPOINT){
      sTmp[0] = 'L';
    }

    if (way_point.Flags & TURNPOINT){
      if (sTmp[0] == '\0')
        sTmp[0] = 'T';
      else
        sTmp[1] = 'T';
    }

    // left justified
    canvas.text_opaque(x1, 2 * InfoBoxLayout::scale, sTmp);

    // right justified after waypoint flags
    _stprintf(sTmp, TEXT("%.0f%s"),
              WayPointSelectInfo[i].Distance,
              Units::GetDistanceName());
    x2 = w0-w3-canvas.text_width(sTmp);
    canvas.text_opaque(x2, 2 * InfoBoxLayout::scale, sTmp);

    // right justified after distance
    _stprintf(sTmp, TEXT("%d")TEXT(DEG),
	      iround(WayPointSelectInfo[i].Direction));
    x3 = w0-canvas.text_width(sTmp);
    canvas.text_opaque(x3, 2 * InfoBoxLayout::scale, sTmp);
  } else {
    if (DrawListIndex == 0){
      _stprintf(sTmp, TEXT("%s"), gettext(TEXT("No Match!")));
      canvas.text_opaque(2 * InfoBoxLayout::scale, 2 * InfoBoxLayout::scale,
                         sTmp);
    }
  }

}

// DrawListIndex = number of things to draw
// ItemIndex = current selected item


static void OnWpListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
	if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = UpLimit-LowLimit;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}


static void OnWPSCloseClicked(WindowControl * Sender){
	(void)Sender;
  ItemIndex = -1;
  wf->SetModalResult(mrCancel);
}

static int OnTimerNotify(WindowControl * Sender) {
  (void)Sender;
  if (DirectionFilterIdx == 1){
    int a;
    a = (lastHeading - iround(XCSoarInterface::Calculated().Heading));
    if (abs(a) > 0){
      UpdateList();
      SetDirectionData(NULL);
      wpDirection->RefreshDisplay();
    }
  }
  return 0;
}

static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){

  WndProperty* wp;
  unsigned NewIndex = TypeFilterIdx;

  (void)lParam;
  (void)Sender;

  wp = ((WndProperty *)wf->FindByName(TEXT("prpFltType")));

  switch(wParam & 0xffff){
    case VK_F1:
      NewIndex = 0;
    break;
    case VK_F2:
      NewIndex = 2;
    break;
    case VK_F3:
      NewIndex = 3;
    break;
  }

  if (TypeFilterIdx != NewIndex){
    TypeFilterIdx = NewIndex;
    FilterMode(false);
    UpdateList();
    wp->GetDataField()->SetAsString(TypeFilter[TypeFilterIdx]);
    wp->RefreshDisplay();
  }

  return(1);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnFilterName),
  DeclareCallBackEntry(OnFilterDistance),
  DeclareCallBackEntry(OnFilterDirection),
  DeclareCallBackEntry(OnFilterType),
  DeclareCallBackEntry(OnPaintListItem),
  DeclareCallBackEntry(OnWpListInfo),
  DeclareCallBackEntry(NULL)
};

int dlgWayPointSelect(const GEOPOINT &location, const int type, const int FilterNear){

  UpLimit = 0;
  LowLimit = 0;
  ItemIndex = -1;

  Location = location;

  if (type > -1){
    TypeFilterIdx = type;
  }
  if (FilterNear){
    DistanceFilterIdx = 1;
  }

  if (!InfoBoxLayout::landscape) {
    wf = dlgLoadFromXML(CallBackTable,
                        TEXT("dlgWayPointSelect_L.xml"),
                        XCSoarInterface::main_window,
                        TEXT("IDR_XML_WAYPOINTSELECT_L"));
  } else {
    wf = dlgLoadFromXML(CallBackTable,
                        TEXT("dlgWayPointSelect.xml"),
                        XCSoarInterface::main_window,
                        TEXT("IDR_XML_WAYPOINTSELECT"));
  }

  if (!wf) return -1;

  assert(wf!=NULL);

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->
   FindByName(TEXT("cmdClose")))->
    SetOnClickNotify(OnWPSCloseClicked);

  wWayPointList = (WndListFrame*)wf->FindByName(TEXT("frmWayPointList"));
  assert(wWayPointList!=NULL);
  wWayPointList->SetBorderKind(BORDERLEFT);
  wWayPointList->SetEnterCallback(OnWaypointListEnter);

  wWayPointListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmWayPointListEntry"));
  assert(wWayPointListEntry!=NULL);
  wWayPointListEntry->SetCanFocus(true);

  wpName = (WndProperty*)wf->FindByName(TEXT("prpFltName"));
  wpDistance = (WndProperty*)wf->FindByName(TEXT("prpFltDistance"));
  wpDirection = (WndProperty*)wf->FindByName(TEXT("prpFltDirection"));

  PrepareData();
  UpdateList();

  wf->SetTimerNotify(OnTimerNotify);

  if ((wf->ShowModal() == mrOK) && (UpLimit > LowLimit) &&
      (ItemIndex >= 0)  // JMW fixed bug, was >0
      && ((unsigned)ItemIndex < (UpLimit - LowLimit))) {
    ItemIndex = WayPointSelectInfo[LowLimit + ItemIndex].Index;
  }else
    ItemIndex = -1;

  free(WayPointSelectInfo);

  delete wf;

  wf = NULL;

  return(ItemIndex);

}
