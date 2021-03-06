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
#include "InfoBoxLayout.h"
#include "Screen/Graphics.hpp"
#include "MainWindow.hpp"

#include <assert.h>

static WndForm *wf=NULL;
static WndListFrame *wAirspacePatternsList=NULL;
static WndOwnerDrawFrame *wAirspacePatternsListEntry = NULL;

static int ItemIndex = -1;


static void UpdateList(void){
  wAirspacePatternsList->ResetList();
  wAirspacePatternsList->Redraw();
}

static int DrawListIndex=0;

static void
OnAirspacePatternsPaintListItem(WindowControl *Sender, Canvas &canvas)
{
  (void)Sender;
  if ((DrawListIndex < NUMAIRSPACEBRUSHES) &&(DrawListIndex>=0)) {
    int i = DrawListIndex;

    canvas.black_pen();
    canvas.set_background_color(Color(0xFF, 0xFF, 0xFF));
    canvas.select(MapGfx.GetAirspaceBrush(i));
    canvas.set_text_color(Color(0x00,0x00, 0x00));
    canvas.rectangle(100 * InfoBoxLayout::scale,
                     2 * InfoBoxLayout::scale,
                     180 * InfoBoxLayout::scale,
                     22 * InfoBoxLayout::scale);
  }
}


static void OnAirspacePatternsListEnter(WindowControl * Sender,
				WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;
  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex>=NUMAIRSPACEBRUSHES) {
    ItemIndex = NUMAIRSPACEBRUSHES-1;
  }
  if (ItemIndex>=0) {
    wf->SetModalResult(mrOK);
  }
}


static void OnAirspacePatternsListInfo(WindowControl * Sender,
			       WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = NUMAIRSPACEBRUSHES;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}

static void OnCloseClicked(WindowControl * Sender){
  (void)Sender;
  ItemIndex = -1;
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAirspacePatternsPaintListItem),
  DeclareCallBackEntry(OnAirspacePatternsListInfo),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


int dlgAirspacePatternsShowModal(void){

  ItemIndex = -1;

  if (!InfoBoxLayout::landscape) {
    wf = dlgLoadFromXML(CallBackTable,
                        TEXT("dlgAirspacePatterns_L.xml"),
                        XCSoarInterface::main_window,
                        TEXT("IDR_XML_AIRSPACEPATTERNS_L"));
  } else {
    wf = dlgLoadFromXML(CallBackTable,
                        TEXT("dlgAirspacePatterns.xml"),
                        XCSoarInterface::main_window,
                        TEXT("IDR_XML_AIRSPACEPATTERNS"));
  }

  if (!wf) return -1;

  assert(wf!=NULL);

  wAirspacePatternsList = (WndListFrame*)wf->FindByName(TEXT("frmAirspacePatternsList"));
  assert(wAirspacePatternsList!=NULL);
  wAirspacePatternsList->SetBorderKind(BORDERLEFT);
  wAirspacePatternsList->SetEnterCallback(OnAirspacePatternsListEnter);

  wAirspacePatternsListEntry = (WndOwnerDrawFrame*)wf->
    FindByName(TEXT("frmAirspacePatternsListEntry"));
  assert(wAirspacePatternsListEntry!=NULL);
  wAirspacePatternsListEntry->SetCanFocus(true);

  UpdateList();

  wf->ShowModal();

  // now retrieve back the properties...

  delete wf;

  wf = NULL;

  return ItemIndex;
}

