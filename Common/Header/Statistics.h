#ifndef STATISTICS_H
#define STATISTICS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include "leastsqs.h"

class Statistics {
 public:

  enum {
    STYLE_BLUETHIN,
    STYLE_REDTHICK,
    STYLE_DASHGREEN,
    STYLE_MEDIUMBLACK,
    STYLE_THINDASHPAPER
  };

  LeastSquares ThermalAverage;
  LeastSquares Wind_x;
  LeastSquares Wind_y;
  LeastSquares Altitude;
  LeastSquares Altitude_Base;
  LeastSquares Altitude_Ceiling;

  static void DrawBarChart(HDC hdc, RECT rc, LeastSquares* lsdata);

  static void DrawLineGraph(HDC hdc, RECT rc, LeastSquares* lsdata,
                            int Style);
  static void DrawTrend(HDC hdc, RECT rc, LeastSquares* lsdata,
                        int Style);
  static void DrawTrendN(HDC hdc, RECT rc, LeastSquares* lsdata,
                         int Style);

  static void DrawLine(HDC hdc, RECT rc, double xmin, double ymin,
                       double xmax, double ymax, int Style);

  static void ScaleYFromData(RECT rc, LeastSquares* lsdata);
  static void ScaleXFromData(RECT rc, LeastSquares* lsdata);
  static void ScaleYFromValue(RECT rc, double val);
  static void ScaleXFromValue(RECT rc, double val);

  static void StyleLine(HDC hdc, POINT l1, POINT l2, int Style);

  static double yscale;
  static double xscale;
  static double y_min, x_min;
  static double x_max, y_max;
  static bool unscaled_x;
  static bool unscaled_y;
  static void ResetScale();

  static void DrawXGrid(HDC hdc, RECT rc, double ticstep, double zero,
                        int Style);
  static void DrawYGrid(HDC hdc, RECT rc, double ticstep, double zero,
                        int Style);


  ///

    static void RenderBarograph(HDC hdc, RECT rc);
    static void RenderClimb(HDC hdc, RECT rc);
    static void RenderGlidePolar(HDC hdc, RECT rc);
};


LRESULT CALLBACK AnalysisProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


#endif