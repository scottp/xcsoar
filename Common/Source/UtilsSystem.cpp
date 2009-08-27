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

#include "UtilsSystem.hpp"
#include "Utils.h"
#include "UtilsText.hpp"
#include "XCSoar.h"
#include <assert.h>
#include "Interface.hpp"
#include "Asset.hpp"
#include "Registry.hpp"
#include "LocalPath.hpp"

#ifdef PNA
#include "LogFile.hpp"
#include "SettingsUser.hpp"
#endif

#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#endif


#if !defined(GNAV) || (WINDOWSPC>0)
typedef DWORD (_stdcall *GetIdleTimeProc) (void);
GetIdleTimeProc GetIdleTime;
#endif

int MeasureCPULoad() {
#if (!defined(GNAV) || (WINDOWSPC>0)) && !defined(__MINGW32__)
  static bool init=false;
  if (!init) {
    // get the pointer to the function
    GetIdleTime = (GetIdleTimeProc)
      GetProcAddress(LoadLibrary(_T("coredll.dll")),
		     _T("GetIdleTime"));
    init=true;
  }
  if (!GetIdleTime) return 0;
#endif

#if defined(GNAV) && defined(__MINGW32__)
  // JMW GetIdleTime() not defined?
  return 100;
#else
  static int pi;
  static int PercentIdle;
  static int PercentLoad;
  static bool start=true;
  static DWORD dwStartTick;
  static DWORD dwIdleSt;
  static DWORD dwStopTick;
  static DWORD dwIdleEd;
  if (start) {
    dwStartTick = GetTickCount();
    dwIdleSt = GetIdleTime();
  }
  if (!start) {
    dwStopTick = GetTickCount();
    dwIdleEd = GetIdleTime();
    pi = ((100 * (dwIdleEd - dwIdleSt))/(dwStopTick - dwStartTick));
    PercentIdle = (PercentIdle+pi)/2;
  }
  start = !start;
  PercentLoad = 100-PercentIdle;
  return PercentLoad;
#endif
}


long CheckFreeRam(void) {
  MEMORYSTATUS    memInfo;
  // Program memory
  memInfo.dwLength = sizeof(memInfo);
  GlobalMemoryStatus(&memInfo);

  //	   memInfo.dwTotalPhys,
  //	   memInfo.dwAvailPhys,
  //	   memInfo.dwTotalPhys- memInfo.dwAvailPhys);

  return memInfo.dwAvailPhys;
}


#if (WINDOWSPC>0)
#if _DEBUG
_CrtMemState memstate_s1;
#endif
#endif

void MemCheckPoint()
{
#if (WINDOWSPC>0)
#if _DEBUG
  _CrtMemCheckpoint( &memstate_s1 );
#endif
#endif
}


void MemLeakCheck() {
#if (WINDOWSPC>0)
#if _DEBUG
  _CrtMemState memstate_s2, memstate_s3;

   // Store a 2nd memory checkpoint in s2
   _CrtMemCheckpoint( &memstate_s2 );

   if ( _CrtMemDifference( &memstate_s3, &memstate_s1, &memstate_s2 ) ) {
     _CrtMemDumpStatistics( &memstate_s3 );
     _CrtMemDumpAllObjectsSince(&memstate_s1);
   }

  _CrtCheckMemory();
#endif
#endif
}


///////////////

// This is necessary to be called periodically to get rid of
// memory defragmentation, since on pocket pc platforms there is no
// automatic defragmentation.
void MyCompactHeaps() {
#if (WINDOWSPC>0)||(defined(GNAV) && !defined(__MINGW32__))
  HeapCompact(GetProcessHeap(),0);
#else
  typedef DWORD (_stdcall *CompactAllHeapsFn) (void);
  static CompactAllHeapsFn CompactAllHeaps = NULL;
  static bool init=false;
  if (!init) {
    // get the pointer to the function
    CompactAllHeaps = (CompactAllHeapsFn)
      GetProcAddress(LoadLibrary(_T("coredll.dll")),
		     _T("CompactAllHeaps"));
    init=true;
  }
  if (CompactAllHeaps) {
    CompactAllHeaps();
  }
#endif
}


unsigned long FindFreeSpace(const TCHAR *path) {
  // returns number of kb free on destination drive

  ULARGE_INTEGER FreeBytesAvailableToCaller;
  ULARGE_INTEGER TotalNumberOfBytes;
  ULARGE_INTEGER TotalNumberOfFreeBytes;
  if (GetDiskFreeSpaceEx(path,
			 &FreeBytesAvailableToCaller,
			 &TotalNumberOfBytes,
			 &TotalNumberOfFreeBytes)) {
    return FreeBytesAvailableToCaller.LowPart/1024;
  } else {
    return 0;
  }
}

#ifndef DISABLEAUDIO
#include "mmsystem.h"
#endif

BOOL PlayResource (const TCHAR* lpName)
{
#ifdef DISABLEAUDIO
  return false;
#else
  BOOL bRtn;
  LPTSTR lpRes;
  HANDLE hResInfo, hRes;

  // TODO code: Modify to allow use of WAV Files and/or Embedded files

  if (_tcsstr(lpName, TEXT(".wav"))) {
    bRtn = sndPlaySound (lpName, SND_ASYNC | SND_NODEFAULT );

  } else {

    // Find the wave resource.
    hResInfo = FindResource (hInst, lpName, TEXT("WAVE"));

    if (hResInfo == NULL)
      return FALSE;

    // Load the wave resource.
    hRes = LoadResource (hInst, (HRSRC)hResInfo);

    if (hRes == NULL)
      return FALSE;

    // Lock the wave resource and play it.
    lpRes = (LPTSTR)LockResource ((HGLOBAL)hRes);

    if (lpRes != NULL)
      {
	bRtn = sndPlaySound (lpRes, SND_MEMORY | SND_ASYNC | SND_NODEFAULT );
      }
    else
      bRtn = 0;
  }
  return bRtn;
#endif
}

void CreateDirectoryIfAbsent(const TCHAR *filename) {
  TCHAR fullname[MAX_PATH];

  LocalPath(fullname, filename);

  DWORD fattr = GetFileAttributes(fullname);

  if ((fattr != 0xFFFFFFFF) &&
      (fattr & FILE_ATTRIBUTE_DIRECTORY)) {
    // directory exists
  } else {
    CreateDirectory(fullname, NULL);
  }

}


bool FileExistsW(const TCHAR *FileName){
  FILE *file = _tfopen(FileName, TEXT("r"));
  if (file == NULL)
    return(FALSE);

  fclose(file);

  return(TRUE);

}

bool FileExistsA(const char *FileName){
  FILE *file = fopen(FileName, "r");
  if (file != NULL) {
    fclose(file);
    return(TRUE);
  }
  return FALSE;
}



bool RotateScreen() {
#if (WINDOWSPC>0)
  return false;
#else
  //
  // Change the orientation of the screen
  //
#ifdef GNAV
  DEVMODE DeviceMode;

  memset(&DeviceMode, 0, sizeof(DeviceMode));
  DeviceMode.dmSize=sizeof(DeviceMode);
  DeviceMode.dmFields = DM_DISPLAYORIENTATION;
  DeviceMode.dmDisplayOrientation = DMDO_90;
  //Put your desired position right here.

  if (DISP_CHANGE_SUCCESSFUL ==
      ChangeDisplaySettingsEx(NULL, &DeviceMode, NULL, CDS_RESET, NULL))
    return true;
  else
    return false;
#else
  return false;
#endif
#endif

}


#ifdef PNA
// VENTA-ADDON MODELTYPE

//
//	Check if the model type is encoded in the executable file name
//
//  GlobalModelName is a global variable, shown during startup and used for printouts only.
//  In order to know what model you are using, GlobalModelType is used.
//
//  This "smartname" facility is used to override the registry/config Model setup to force
//  a model type to be used, just in case. The model types may not follow strictly those in
//  config menu, nor be updated. Does'nt hurt though.
//
void SmartGlobalModelType() {

	GlobalModelType=MODELTYPE_PNA;	// default for ifdef PNA by now!

	if ( GetGlobalModelName() )
	{
		ConvToUpper(GlobalModelName);

		if ( !_tcscmp(GlobalModelName,_T("PNA"))) {
					GlobalModelType=MODELTYPE_PNA_PNA;
					_tcscpy(GlobalModelName,_T("GENERIC") );
		}
		else
			if ( !_tcscmp(GlobalModelName,_T("HP31X")))	{
					GlobalModelType=MODELTYPE_PNA_HP31X;
			}
		else
			if ( !_tcscmp(GlobalModelName,_T("PN6000"))) {
					GlobalModelType=MODELTYPE_PNA_PN6000;
			}
		else
			if ( !_tcscmp(GlobalModelName,_T("MIO"))) {
					GlobalModelType=MODELTYPE_PNA_MIO;
			}
		else
			_tcscpy(GlobalModelName,_T("UNKNOWN") );
	} else
		_tcscpy(GlobalModelName, _T("UNKNOWN") );
}


//
// Retrieve from the registry the previous set model type
// This value is defined in xcsoar.h , example> MODELTYPE_PNA_HP31X
// is equivalent to a value=10201 (defined in the header file)
//
void SetModelType() {

  TCHAR sTmp[100];
  TCHAR szRegistryInfoBoxModel[]= TEXT("AppInfoBoxModel");
  DWORD Temp=0;

  GetFromRegistry(szRegistryInfoBoxModel, &Temp);

  if ( SetModelName(Temp) != true ) {

    _stprintf(sTmp,_T("SetModelType ERROR! ModelName returned invalid value <%d> from Registry!\n"), Temp);
    StartupStore(sTmp);
    GlobalModelType=MODELTYPE_PNA_PNA;

  } else {

    GlobalModelType = Temp;
  }

  _stprintf(sTmp,_T("SetModelType: Name=<%s> Type=%d\n"),GlobalModelName, GlobalModelType);
  StartupStore(sTmp);
}

// Parse a MODELTYPE value and set the equivalent model name.
// If the modeltype is invalid or not yet handled, assume that
// the user changed it in the registry or in the profile, and
// correct the error returning false: this will force a Generic Type.

bool SetModelName(DWORD Temp) {
  switch (Temp) {
  case MODELTYPE_PNA_PNA:
    _tcscpy(GlobalModelName,_T("GENERIC"));
    return true;
    break;
  case MODELTYPE_PNA_HP31X:
    _tcscpy(GlobalModelName,_T("HP31X"));
    return true;
    break;
  case MODELTYPE_PNA_PN6000:
    _tcscpy(GlobalModelName,_T("PN6000"));
    return true;
  case MODELTYPE_PNA_MIO:
    _tcscpy(GlobalModelName,_T("MIO"));
    return true;
  case  MODELTYPE_PNA_MEDION_P5:
    _tcscpy(GlobalModelName,_T("MEDION P5"));
    return true;
  case MODELTYPE_PNA_NOKIA_500:
    _tcscpy(GlobalModelName,_T("NOKIA500"));
    return true;
  default:
    _tcscpy(GlobalModelName,_T("UNKNOWN"));
    return false;
  }

}

#endif


#if defined(PNA) || defined(FIVV)  // VENTA-ADDON gmfpathname & C.

/*
	Paolo Ventafridda 1 feb 08
	Get pathname & c. from GetModuleFilename (gmf)
	In case of problems, always return \ERRORxx\  as path name
	It will be displayed at startup and users will know that
	something is wrong reporting the error code to us.
	Approach not followed: It works but we don't know why
	Approach followed: It doesn't work and we DO know why

	These are temporary solutions to be improved
 */

#define MAXPATHBASENAME MAX_PATH

/*
 * gmfpathname returns the pathname of the current executed program, with leading and trailing slash
 * example:  \sdmmc\   \SD CARD\
 * In case of double slash, it is assumed currently as a single "\" .
 */
TCHAR * gmfpathname ()
{
  static TCHAR gmfpathname_buffer[MAXPATHBASENAME];
  TCHAR  *p;

  if (GetModuleFileName(NULL, gmfpathname_buffer, MAXPATHBASENAME) <= 0) {
//    StartupStore(TEXT("CRITIC- gmfpathname returned null GetModuleFileName\n")); // rob bughunt
    return(_T("\\ERROR_01\\") );
  }
  if (gmfpathname_buffer[0] != '\\' ) {
//   StartupStore(TEXT("CRITIC- gmfpathname starting without a leading backslash\n"));
    return(_T("\\ERROR_02\\"));
  }
  gmfpathname_buffer[MAXPATHBASENAME-1] = '\0';	// truncate for safety

  for (p=gmfpathname_buffer+1; *p != '\0'; p++)
    if ( *p == '\\' ) break;	// search for the very first "\"

  if ( *p == '\0') {
//    StartupStore(TEXT("CRITIC- gmfpathname no backslash found\n"));
    return(_T("\\ERROR_03\\"));
  }
  *++p = '\0';

  return (TCHAR *) gmfpathname_buffer;
}

/*
 * gmfbasename returns the filename of the current executed program, without leading path.
 * Example:  xcsoar.exe
 */
TCHAR * gmfbasename ()
{
  static TCHAR gmfbasename_buffer[MAXPATHBASENAME];
  TCHAR *p, *lp;

  if (GetModuleFileName(NULL, gmfbasename_buffer, MAXPATHBASENAME) <= 0) {
    StartupStore(TEXT("CRITIC- gmfbasename returned null GetModuleFileName\n"));
    return(_T("ERROR_04") );
  }
  if (gmfbasename_buffer[0] != '\\' ) {
    StartupStore(TEXT("CRITIC- gmfbasename starting without a leading backslash\n"));
    return(_T("ERROR_05"));
  }
  for (p=gmfbasename_buffer+1, lp=NULL; *p != '\0'; p++)
    {
      if ( *p == '\\' ) {
	lp=++p;
	continue;
      }
    }
  return  lp;
}

/*
 *	A little hack in the executable filename: if it contains an
 *	underscore, then the following chars up to the .exe is
 *	considered a modelname
 *  Returns 0 if failed, 1 if name found
 */
int GetGlobalModelName ()
{
  TCHAR modelname_buffer[MAXPATHBASENAME];
  TCHAR *p, *lp, *np;

  _tcscpy(GlobalModelName, _T(""));

  if (GetModuleFileName(NULL, modelname_buffer, MAXPATHBASENAME) <= 0) {
    StartupStore(TEXT("CRITIC- GetGlobalFileName returned NULL\n"));
    return 0;
  }
  if (modelname_buffer[0] != '\\' ) {
    StartupStore(TEXT("CRITIC- GetGlobalFileName starting without a leading backslash\n"));
    return 0;
  }
  for (p=modelname_buffer+1, lp=NULL; *p != '\0'; p++)
    {
      if ( *p == '\\' ) {
	lp=++p;
	continue;
      }
    } // assuming \sd\path\xcsoar_pna.exe  we are now at \xcsoar..

  for (p=lp, np=NULL; *p != '\0'; p++)
    {
      if (*p == '_' ) {
	np=++p;
	break;
      }
    } // assuming xcsoar_pna.exe we are now at _pna..

  if ( np == NULL ) {
    return 0;	// VENTA2-bugfix , null deleted
  }

  for ( p=np, lp=NULL; *p != '\0'; p++)
    {
      if (*p == '.' ) {
	lp=p;
	break;
      }
    } // we found the . in pna.exe

  if (lp == NULL) return 0; // VENTA2-bugfix null return
  *lp='\0'; // we cut .exe

  _tcscpy(GlobalModelName, np);

  return 1;  // we say ok

}

#endif   // PNA


#ifdef PNA
/* Paolo Ventafridda Apr 23th 2009 VENTA4
 * SetBacklight for PNA devices. There is no standard way of managing backlight on CE,
 * and every device may have different value names and settings. Microsoft did not set
 * a standard and thus we need a custom solution for each device.
 * But the approach is always the same: change a value and call an event.
 * We do this in XCSoar.cpp at the beginning, no need to make these settings configurable:
 * max brightness and no timeout if on power is the rule. Otherwise, do it manually..
 */
bool SetBacklight() // VENTA4
{
  HKEY    hKey;
  DWORD   Disp=0;
  HRESULT hRes;
  bool doevent=false;

  if (EnableAutoBacklight == false ) return false;

  hRes = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Backlight"), 0,  0, &hKey);
  if (hRes != ERROR_SUCCESS) return false;

  switch (GlobalModelType)
  {
	case MODELTYPE_PNA_HP31X:

		Disp=20; // max backlight
		// currently we ignore hres, if registry entries are spoiled out user is already in deep troubles
		hRes = RegSetValueEx(hKey, _T("BackLightCurrentACLevel"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("BackLightCurrentBatteryLevel"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("TotalLevels"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=0;
		hRes = RegSetValueEx(hKey, _T("UseExt"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		RegDeleteValue(hKey,_T("ACTimeout"));
		doevent=true;
		break;

	default:
		doevent=false;
		break;
  }

  RegCloseKey(hKey); if (doevent==false) return false;

  HANDLE BLEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("BacklightChangeEvent"));
  if ( SetEvent(BLEvent) == 0) doevent=false;
  	else CloseHandle(BLEvent);
  return doevent;
}

bool SetSoundVolume() // VENTA4
{

  if (EnableAutoSoundVolume == false ) return false;

/*
 * This does not work, dunno why
 *
  HKEY    hKey;
  DWORD   Disp=0;
  HRESULT hRes;

  hRes = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Volume"), 0,  0, &hKey);
  if (hRes != ERROR_SUCCESS) return false;
  switch (GlobalModelType)
  {
	case MODELTYPE_PNA_HP31X:
		Disp=0xFFFFFFFF; // max volume
		hRes = RegSetValueEx(hKey, _T("Volume"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=65538;
		hRes = RegSetValueEx(hKey, _T("Screen"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=0;
		hRes = RegSetValueEx(hKey, _T("Key"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=7;
		hRes = RegSetValueEx(hKey, _T("Mute"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0xF2, 0);
	        RegCloseKey(hKey);
		break;

	default:
		break;
  }
 */

  // should we enter critical section ?  probably...
  waveOutSetVolume(0, 0xffff); // this is working for all platforms

  return true;
}


#endif

#if defined(FIVV) || defined(PNA)
// VENTA2-ADDON fonts install
/*
 * Get the localpath, enter XCSoarData/Config, see if there are fonts to copy,
 * check that they have not already been copied in \Windows\Fonts,
 * and eventually copy everything in place.
 *
 * Returns: 0 if OK .
 * 1 - n other errors not really needed to handle. See below
 *
 * These are currently fonts used by PDA:
 *
	DejaVuSansCondensed2.ttf
	DejaVuSansCondensed-Bold2.ttf
	DejaVuSansCondensed-BoldOblique2.ttf
	DejaVuSansCondensed-Oblique2.ttf
 *
 *
 */
short InstallFonts() {

  TCHAR srcdir[MAX_PATH];
  TCHAR dstdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  TCHAR dstfile[MAX_PATH];

  _stprintf(srcdir,TEXT("%s%S\\Fonts"),gmfpathname(), XCSDATADIR );
  _stprintf(dstdir,TEXT("\\Windows\\Fonts"),gmfpathname() );


  if (  GetFileAttributes(srcdir) != FILE_ATTRIBUTE_DIRECTORY) return 1;
  if (  GetFileAttributes(dstdir) != FILE_ATTRIBUTE_DIRECTORY) return 2;

  _stprintf(srcfile,TEXT("%s\\DejaVuSansCondensed2.ttf"),srcdir);
  _stprintf(dstfile,TEXT("%s\\DejaVuSansCondensed2.ttf"),dstdir);
  //if (  GetFileAttributes(srcfile) != FILE_ATTRIBUTE_NORMAL) return 3;
  if (  GetFileAttributes(dstfile) != 0xffffffff ) return 4;
  if ( !CopyFile(srcfile, dstfile, TRUE)) return 5;

  // From now on we attempt to copy without overwriting
  _stprintf(srcfile,TEXT("%s\\DejaVuSansCondensed-Bold2.ttf"),srcdir);
  _stprintf(dstfile,TEXT("%s\\DejaVuSansCondensed-Bold2.ttf"),dstdir);
  CopyFile(srcfile,dstfile,TRUE);

  _stprintf(srcfile,TEXT("%s\\DejaVuSansCondensed-BoldOblique2.ttf"),srcdir);
  _stprintf(dstfile,TEXT("%s\\DejaVuSansCondensed-BoldOblique2.ttf"),dstdir);
  CopyFile(srcfile,dstfile,TRUE);

  _stprintf(srcfile,TEXT("%s\\DejaVuSansCondensed-Oblique2.ttf"),srcdir);
  _stprintf(dstfile,TEXT("%s\\DejaVuSansCondensed-Oblique2.ttf"),dstdir);
  CopyFile(srcfile,dstfile,TRUE);

  return 0;
}

/*
 * Check that XCSoarData exist where it should be
 * Return false if error, true if Ok
 */
bool CheckDataDir() {
	TCHAR srcdir[MAX_PATH];

	_stprintf(srcdir,TEXT("%s%S"),gmfpathname(), XCSDATADIR );
	if (  GetFileAttributes(srcdir) != FILE_ATTRIBUTE_DIRECTORY) return false;
	return true;
}

/*
 * Check for xcsoar-registry.prf  existance
 * Should really check if geometry has changed.. in 5.2.3!
 * Currently we disable it for HP31X which is the only PNA with different settings
 * for different geometries
 * 5.2.3 BOOL changed to bool
 * TODO: VENTA4 now that Rob fonts is used, should return false all the way
 */
bool CheckRegistryProfile() {
	TCHAR srcpath[MAX_PATH];
	if ( GlobalModelType == MODELTYPE_PNA_HP31X ) return false;
	_stprintf(srcpath,TEXT("%s%S\\%S"),gmfpathname(), XCSDATADIR , XCSPROFILE);
	if (  GetFileAttributes(srcpath) == 0xffffffff) return false;
	return true;
}
#endif


void SetProfileFiles(const TCHAR *ex);

void XCSoarGetOpts(LPTSTR CommandLine) {
  (void)CommandLine;
// SaveRegistryToFile(TEXT("iPAQ File Store\xcsoar-registry.prf"));

  TCHAR externProfileFile[MAX_PATH];
  externProfileFile[0] = 0;

#if (WINDOWSPC>0)
  SCREENWIDTH=640;
  SCREENHEIGHT=480;

#if defined(SCREENWIDTH_)
  SCREENWIDTH=SCREENWIDTH_;
#endif
#if defined(SCREENHEIGHT_)
  SCREENHEIGHT=SCREENHEIGHT_;
#endif

#else
  return; // don't do anything for PDA platforms
#endif

  TCHAR *MyCommandLine = GetCommandLine();

  if (MyCommandLine != NULL){
    TCHAR *pC, *pCe;

    pC = _tcsstr(MyCommandLine, TEXT("-profile="));
    if (pC != NULL){
      pC += strlen("-profile=");
      if (*pC == '"'){
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
      } else{
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
      }
      if (pCe != NULL && pCe-1 > pC){

        _tcsncpy(externProfileFile, pC, pCe-pC);
        externProfileFile[pCe-pC] = '\0';
      }
    }
#if (WINDOWSPC>0)
    pC = _tcsstr(MyCommandLine, TEXT("-800x480"));
    if (pC != NULL){
      SCREENWIDTH=800;
      SCREENHEIGHT=480;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x272"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=272;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x234"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=234;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-portrait"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=640;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-square"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=480;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-small"));
    if (pC != NULL){
      SCREENWIDTH/= 2;
      SCREENHEIGHT/= 2;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x240"));
    if (pC != NULL){
      SCREENWIDTH=320;
      SCREENHEIGHT=240;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x320"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=320;
    }

#endif
  }

  SetProfileFiles(externProfileFile);

}