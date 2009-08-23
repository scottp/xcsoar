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
#ifndef INFOBOX_MANAGER_H
#define INFOBOX_MANAGER_H

#include "XCSoar.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct _SCREEN_INFO
{
  UnitGroup_t UnitGroup;
  TCHAR Description[DESCRIPTION_SIZE +1];
  TCHAR Title[TITLE_SIZE + 1];
  InfoBoxFormatter *Formatter;
  void (*Process)(int UpDown);
  char next_screen;
  char prev_screen;
} SCREEN_INFO;

extern SCREEN_INFO Data_Options[];
extern int InfoType[MAXINFOWINDOWS]; //
extern int  InfoFocus;
extern const int NUMSELECTSTRINGS;
extern int numInfoWindows;

void InfoBoxesSetDirty(bool is_dirty);
void DeleteInfoBoxFormatters();
bool InfoBoxClick(HWND wmControl, bool display_locked);
void InfoBoxFocus(bool display_locked);
void InfoBoxProcessTimer(void);
void InfoBoxDrawIfDirty(void);
void InfoBoxFocusSetMaxTimeOut(void);
void ShowInfoBoxes();
void HideInfoBoxes();
void DefocusInfoBox(void);
void Event_SelectInfoBox(int i);
void Event_ChangeInfoBoxType(int i);
void DoInfoKey(int keycode);

#endif