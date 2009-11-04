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

#ifndef XCSOAR_SCREEN_BUTTON_WINDOW_HXX
#define XCSOAR_SCREEN_BUTTON_WINDOW_HXX

#ifdef ENABLE_SDL

#include "Screen/PaintWindow.hpp"

/**
 * A clickable button.
 */
class ButtonWindow : public PaintWindow
{
  const TCHAR *text;
  unsigned id;
  bool down;
  Font font;

public:
  ButtonWindow():down(false) {}
  ~ButtonWindow() { reset(); }

public:
  void set(ContainerWindow &parent, const TCHAR *text, unsigned id,
           int left, int top, unsigned width, unsigned height);
  void reset();

protected:
  virtual bool on_mouse_down(int x, int y);
  virtual bool on_mouse_up(int x, int y);
  virtual void on_paint(Canvas &canvas);
};

#else /* !ENABLE_SDL */

#include "Screen/Window.hpp"

#include <tchar.h>

/**
 * A clickable button.
 */
class ButtonWindow : public Window {
public:
  void set(ContainerWindow &parent, const TCHAR *text, unsigned id,
           int left, int top, unsigned width, unsigned height) {
    Window::set(&parent, _T("BUTTON"), text,
                left, top, width, height);

    ::SetWindowLong(hWnd, GWL_ID, id);
  }
};

#endif /* !ENABLE_SDL */

#endif
