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

#ifndef XCSOAR_SCREEN_WINDOW_HPP
#define XCSOAR_SCREEN_WINDOW_HPP

#include "Screen/Font.hpp"

#ifdef ENABLE_SDL
#include "Screen/BufferCanvas.hpp"
#include "Screen/Timer.hpp"
#endif /* ENABLE_SDL */

class ContainerWindow;

/**
 * A Window is a portion on the screen which displays something, and
 * which optionally interacts with the user.  To draw custom graphics
 * into a Window, derive your class from #PaintWindow.
 */
class Window {
public:
#ifdef ENABLE_SDL
  friend class SDLTimer;
  typedef SDLTimer *timer_t;
#else
  typedef UINT_PTR timer_t;
#endif

protected:
#ifdef ENABLE_SDL
  ContainerWindow *parent;
  int left, top;
  BufferCanvas canvas;
#else
  HWND hWnd;
  WNDPROC prev_wndproc;
#endif

private:
  /* copy constructor not allowed */
  Window(const Window &window) {}
  Window &operator=(const Window &window) { return *this; }

public:
#ifdef ENABLE_SDL
  Window():parent(NULL) {}
#else
  Window():hWnd(NULL), prev_wndproc(NULL) {}
#endif
  ~Window() {
    reset();
  }

#ifndef ENABLE_SDL
  operator HWND() const {
    return hWnd;
  };
#endif

public:
  bool defined() const {
#ifdef ENABLE_SDL
    return canvas.defined();
#else
    return hWnd != NULL;
#endif
  }

#ifdef ENABLE_SDL
  int get_top() const {
    return top;
  }

  int get_left() const {
    return left;
  }

  const Canvas &get_canvas() const {
    return canvas;
  }
#endif

  void set(ContainerWindow *parent, LPCTSTR cls, LPCTSTR text,
           int left, int top, unsigned width, unsigned height,
           DWORD style, DWORD ex_style=0);

  void set(ContainerWindow *parent, LPCTSTR cls, LPCTSTR text,
           int left, int top, unsigned width, unsigned height,
           bool center = false, bool notify = false, bool show = true,
           bool tabstop = false, bool border = false);

#ifndef ENABLE_SDL
  void created(HWND _hWnd);
#endif

  void reset();

  void move(int left, int top) {
#ifdef ENABLE_SDL
    this->left = left;
    this->top = top;
#else
    ::SetWindowPos(hWnd, NULL, left, top, 0, 0,
                   SWP_NOSIZE | SWP_NOZORDER |
                   SWP_NOACTIVATE | SWP_NOOWNERZORDER);
#endif
  }

  void move(int left, int top, unsigned width, unsigned height) {
#ifdef ENABLE_SDL
#else /* !ENABLE_SDL */
    ::SetWindowPos(hWnd, NULL, left, top, width, height,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    // XXX store new size?
#endif
  }

  void insert_after(HWND hWnd2, bool show=true) {
#ifdef ENABLE_SDL
    // XXX
#else
    ::SetWindowPos(hWnd, hWnd2, 0, 0, 0, 0,
                   SWP_NOMOVE|SWP_NOSIZE|(show?SWP_SHOWWINDOW:SWP_HIDEWINDOW));
#endif
  }

  void bring_to_top() {
#ifdef ENABLE_SDL
    // XXX
#else
    ::BringWindowToTop(hWnd);
#endif
  }

  void set_font(const Font &font) {
#ifdef ENABLE_SDL
    // XXX
#else
    ::SendMessage(hWnd, WM_SETFONT,
                  (WPARAM)font.native(), MAKELPARAM(TRUE,0));
#endif
  }

  void show() {
#ifdef ENABLE_SDL
    // XXX
#else
    ::ShowWindow(hWnd, SW_SHOW);
#endif
  }

  void hide() {
#ifdef ENABLE_SDL
    // XXX
#else
    ::ShowWindow(hWnd, SW_HIDE);
#endif
  }

  /**
   * Can this window get user input?
   */
  bool is_enabled() const {
#ifdef ENABLE_SDL
    return true;
#else
    return ::IsWindowEnabled(hWnd);
#endif
  }

  /**
   * Specifies whether this window can get user input.
   */
  void set_enabled(bool enabled) {
#ifdef ENABLE_SDL
    // XXX
#else
    ::EnableWindow(hWnd, enabled);
#endif
  }

  void set_focus() {
#ifdef ENABLE_SDL
    // XXX
#else
    ::SetFocus(hWnd);
#endif
  }

  bool has_focus() const {
#ifdef ENABLE_SDL
    return true; // XXX
#else
    return hWnd == ::GetFocus();
#endif
  }

  void set_capture() {
#ifdef ENABLE_SDL
    // XXX
#else
    ::SetCapture(hWnd);
#endif
  }

  void release_capture() {
#ifdef ENABLE_SDL
    // XXX
#else
    ::ReleaseCapture();
#endif
  }

#ifndef ENABLE_SDL
  WNDPROC set_wndproc(WNDPROC wndproc)
  {
    return (WNDPROC)::SetWindowLong(hWnd, GWL_WNDPROC, (LONG)wndproc);
  }

  void set_userdata(LONG value)
  {
    ::SetWindowLong(hWnd, GWL_USERDATA, value);
  }

  void set_userdata(void *value)
  {
    // XXX on 64 bit machines?
    set_userdata((LONG)(size_t)value);
  }

  LONG get_userdata() const
  {
    return ::GetWindowLong(hWnd, GWL_USERDATA);
  }

  void *get_userdata_pointer() const
  {
    // XXX on 64 bit machines?
    return (void *)get_userdata();
  }
#endif /* !ENABLE_SDL */

  timer_t set_timer(unsigned id, unsigned ms)
  {
#ifdef ENABLE_SDL
    return new SDLTimer(*this, ms);
#else
    ::SetTimer(hWnd, id, ms, NULL);
    return id;
#endif
  }

  void kill_timer(timer_t id)
  {
#ifdef ENABLE_SDL
    delete id;
#else
    ::KillTimer(hWnd, id);
#endif
  }

  const RECT get_position() const
  {
    RECT rc;
#ifdef ENABLE_SDL
    // XXX
#else
    ::GetWindowRect(hWnd, &rc);
#endif
    return rc;
  }

  const RECT get_client_rect() const
  {
    RECT rc;
#ifdef ENABLE_SDL
    // XXX
#else
    ::GetClientRect(hWnd, &rc);
#endif
    return rc;
  }

#ifndef ENABLE_SDL
  static LONG get_userdata(HWND hWnd) {
    return ::GetWindowLong(hWnd, GWL_USERDATA);
  }

  static void *get_userdata_pointer(HWND hWnd) {
    // XXX on 64 bit machines?
    return (void *)get_userdata(hWnd);
  }

  /**
   * Converts a #HWND into a #Window pointer, without checking if that
   * is legal.
   */
  static Window *get_unchecked(HWND hWnd) {
    return (Window *)get_userdata_pointer(hWnd);
  }

  /**
   * Converts a #HWND into a #Window pointer.  Returns NULL if the
   * HWND is not a Window peer.  This only works for windows which
   * have called install_wndproc().
   */
  static Window *get(HWND hWnd) {
    WNDPROC wndproc = (WNDPROC)::GetWindowLong(hWnd, GWL_WNDPROC);
    return wndproc == WndProc
      ? get_unchecked(hWnd)
      : NULL;
  }
#endif

  void send_command(const Window &from) {
#ifdef ENABLE_SDL
    // XXX
#else /* !ENABLE_SDL */
    ::SendMessage(hWnd, WM_COMMAND, (WPARAM)0, (LPARAM)from.hWnd);
#endif /* !ENABLE_SDL */
  }

  void send_user(unsigned id) {
#ifdef ENABLE_SDL
    SDL_Event event;
    event.user.type = SDL_USEREVENT + id;
    event.user.code = 0;
    event.user.data1 = this;
    event.user.data2 = NULL;

    ::SDL_PushEvent(&event);
#else /* !ENABLE_SDL */
    ::PostMessage(hWnd, WM_USER + id, (WPARAM)0, (LPARAM)0);
#endif /* !ENABLE_SDL */
  }

protected:
  /**
   * @return true on success, false if the window should not be
   * created
   */
  virtual bool on_create();

  virtual bool on_destroy();
  virtual bool on_close();
  virtual bool on_resize(unsigned width, unsigned height);
  virtual bool on_mouse_move(int x, int y, unsigned keys);
  virtual bool on_mouse_down(int x, int y);
  virtual bool on_mouse_up(int x, int y);
  virtual bool on_mouse_double(int x, int y);
  virtual bool on_key_down(unsigned key_code);
  virtual bool on_key_up(unsigned key_code);
  virtual bool on_command(HWND hWnd, unsigned id, unsigned code);
  virtual bool on_setfocus();
  virtual bool on_killfocus();
  virtual bool on_timer(timer_t id);
  virtual bool on_user(unsigned id);


#ifndef ENABLE_SDL
  /**
   * Called by on_message() when the message was not handled by any
   * virtual method.  Calls the default handler.  This function is
   * virtual, because the Dialog class will have to override it -
   * dialogs have slightly different semantics.
   */
  virtual LRESULT on_unhandled_message(HWND hWnd, UINT message,
                                       WPARAM wParam, LPARAM lParam);

  virtual LRESULT on_message(HWND hWnd, UINT message,
                             WPARAM wParam, LPARAM lParam);
#endif /* !ENABLE_SDL */

public:
#ifdef ENABLE_SDL
  void install_wndproc() {
    // XXX
  }
#else /* !ENABLE_SDL */
  /**
   * This static method reads the Window* object from GWL_USERDATA and
   * calls on_message().
   */
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                                  WPARAM wParam, LPARAM lParam);

  /**
   * Installs Window::WndProc() has the WNDPROC.  This enables the
   * methods on_*() methods, which may be implemented by sub classes.
   */
  void install_wndproc() {
    set_userdata(this);
    prev_wndproc = set_wndproc(WndProc);
  }
#endif /* !ENABLE_SDL */
};

#endif
