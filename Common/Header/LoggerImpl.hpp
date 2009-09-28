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

#if !defined(XCSOAR_LOGGER_IMPL_HPP)
#define XCSOAR_LOGGER_IMPL_HPP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include "SettingsComputer.hpp"
#include "Device/device.h"

#define MAX_LOGGER_BUFFER 60

struct NMEA_INFO;

typedef struct LoggerBuffer {
  double Latitude;
  double Longitude;
  double Altitude;
  double BaroAltitude;
  short Day;
  short Month;
  short Year;
  short Hour;
  short Minute;
  short Second;
  int SatelliteIDs[MAXSATELLITES];
} LoggerBuffer_T;

class LoggerImpl {
public:
  LoggerImpl(); 

public:
  void LogPoint(const NMEA_INFO &gps_info);
  bool CheckDeclaration(void);
  bool isTaskDeclared() const;
  bool isLoggerActive() const;
  bool LoggerClearFreeSpace(const NMEA_INFO &gps_info);
  void LinkGRecordDLL(void);
  bool LoggerGActive() const;
  void guiStartLogger(const NMEA_INFO& gps_info, 
                      const SETTINGS_COMPUTER& settings,
                      bool noAsk = false);
  void guiToggleLogger(const NMEA_INFO& gps_info, 
                       const SETTINGS_COMPUTER& settings,
                       bool noAsk = false);
  void guiStopLogger(const NMEA_INFO &gps_info,
                     bool noAsk = false);
  void LoggerDeviceDeclare();
  void LoggerNote(const TCHAR *text);
  void clearBuffer();

private:
  void StartLogger(const NMEA_INFO &gps_info, 
                   const SETTINGS_COMPUTER &settings,
                   const TCHAR *strAssetNumber);

  void AddDeclaration(double Lattitude, double Longditude, const TCHAR *ID);
  void StartDeclaration(const NMEA_INFO &gps_info, 
                        const int numturnpoints);
  void EndDeclaration(void);
  void LoggerHeader(const NMEA_INFO &gps_info);
  
  void StopLogger(const NMEA_INFO &gps_info);
  bool IGCWriteRecord(const char *szIn, const TCHAR *);
  
  bool LogFRecordToFile(const int SatelliteIDs[], short Hour, short Minute,
                        short Second, bool bAlways);
  
  bool LogFRecord(const NMEA_INFO &gps_info, bool bAlways);
  
  void SetFRecordLastTime(double dTime);
  double GetFRecordLastTime(void);
  void ResetFRecord(void);
  
  bool LoggerDeclare(PDeviceDescriptor_t dev, Declaration_t *decl);
  void LoggerGInit();
private:
  void LogPointToFile(const NMEA_INFO& gps_info);
  void ResetFRecord_Internal(void);
  void LogPointToBuffer(const NMEA_INFO &gps_info);
  void LoggerGStop(TCHAR* szLoggerFileName);
private:
  bool LoggerActive;
  bool DeclaredToDevice;
  double FRecordLastTime;
  TCHAR szLoggerFileName[MAX_PATH];
  TCHAR szFLoggerFileName[MAX_PATH];
  TCHAR szFLoggerFileNameRoot[MAX_PATH];
  char szLastFRecord[MAX_IGC_BUFF];
  int NumLoggerBuffered;
  LoggerBuffer_T FirstPoint;
  LoggerBuffer_T LoggerBuffer[MAX_LOGGER_BUFFER];
};

#endif


