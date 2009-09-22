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
#include "Units.hpp"
#include "McReady.h"
#include "Device/device.h"
#include "Math/FastMath.h"
#include "DataField/Base.hpp"
#include "MainWindow.hpp"

static WndForm *wf=NULL;

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static double VegaDemoW = 0.0;
static double VegaDemoV = 0.0;
static bool VegaDemoAudioClimb = true;


static void VegaWriteDemo(void) {
  static DWORD fpsTime = 0;
  DWORD fpsTimeNew = ::GetTickCount();
  if (fpsTimeNew-fpsTime>250) {
    fpsTime = fpsTimeNew;
  } else {
    return;
  }

  TCHAR dbuf[100];
  wsprintf(dbuf, TEXT("PDVDD,%d,%d"),
	   iround(VegaDemoW*10),
	   iround(VegaDemoV*10));
  VarioWriteNMEA(dbuf);
};



static void OnVegaDemoW(DataField *Sender,
			 DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      VegaDemoW = Sender->GetAsFloat()/LIFTMODIFY;
      VegaWriteDemo();
    break;
  }
}


static void OnVegaDemoV(DataField *Sender,
			 DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      VegaDemoV = Sender->GetAsFloat()/SPEEDMODIFY;
      VegaWriteDemo();
    break;
  }
}


static void OnVegaDemoAudioClimb(DataField *Sender,
			 DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      VegaDemoAudioClimb = (Sender->GetAsInteger()==1);
      VegaWriteDemo();
    break;
  }
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnVegaDemoW),
  DeclareCallBackEntry(OnVegaDemoV),
  DeclareCallBackEntry(OnVegaDemoAudioClimb),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void dlgVegaDemoShowModal(void){
  wf = dlgLoadFromXML(CallBackTable,
                      TEXT("dlgVegaDemo.xml"),
		      XCSoarInterface::main_window,
		      TEXT("IDR_XML_VEGADEMO"));

  WndProperty* wp;

  if (!wf) return;

  VarioWriteNMEA(TEXT("PDVSC,S,DemoMode,0"));
  VarioWriteNMEA(TEXT("PDVSC,S,DemoMode,3"));

  wp = (WndProperty*)wf->FindByName(TEXT("prpVegaDemoW"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(VegaDemoW*LIFTMODIFY);
    wp->GetDataField()->SetUnits(Units::GetVerticalSpeedName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpVegaDemoV"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(VegaDemoV*SPEEDMODIFY);
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpVegaDemoAudioClimb"));
  if (wp) {
    wp->GetDataField()->Set(VegaDemoAudioClimb);
    wp->RefreshDisplay();
  }

  //////

  wf->ShowModal();

  // deactivate demo.
  VarioWriteNMEA(TEXT("PDVSC,S,DemoMode,0"));

  delete wf;

  wf = NULL;

}

