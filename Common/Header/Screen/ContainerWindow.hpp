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

#ifndef XCSOAR_SCREEN_CONTAINER_WINDOW_HXX
#define XCSOAR_SCREEN_CONTAINER_WINDOW_HXX

#include "Screen/PaintWindow.hpp"

/**
 * A container for more #Window objects.  It is also derived from
 * #PaintWindow, because you might want to paint a border between the
 * child windows.
 */
class ContainerWindow : public PaintWindow {
protected:
  virtual Brush *on_color(Window &window, Canvas &canvas);

#ifndef ENABLE_SDL
  virtual LRESULT on_message(HWND hWnd, UINT message,
                             WPARAM wParam, LPARAM lParam);
#endif

#ifdef ENABLE_SDL
public:
  void update_child(const Window &child) {
    canvas.copy(child.get_left(), child.get_top(),
                child.get_canvas().get_width(),
                child.get_canvas().get_height(),
                child.get_canvas(), 0, 0);
    canvas.update(child.get_left(), child.get_top(),
                  canvas.get_width(), canvas.get_height());
  }
#endif
};

#endif
