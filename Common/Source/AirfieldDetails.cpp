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

#include "AirfieldDetails.h"
#include "UtilsText.hpp"
#include "SettingsTask.hpp"
#include "Language.hpp"
#include "Registry.hpp"
#include "LocalPath.hpp"
#include "LogFile.hpp"
#include "WayPoint.hpp"
#include "Interface.hpp"

#include <zzip/lib.h>
#include "wcecompat/ts_string.h"
#include <stdlib.h>

ZZIP_FILE* zAirfieldDetails = NULL;

static TCHAR  szAirfieldDetailsFile[MAX_PATH] = TEXT("\0");

static void
OpenAirfieldDetails()
{
  char zfilename[MAX_PATH] = "\0";

  zAirfieldDetails = NULL;

  GetRegistryString(szRegistryAirfieldFile, szAirfieldDetailsFile, MAX_PATH);

  if (_tcslen(szAirfieldDetailsFile)>0) {
    ExpandLocalPath(szAirfieldDetailsFile);
    unicode2ascii(szAirfieldDetailsFile, zfilename, MAX_PATH);
    SetRegistryString(szRegistryAirfieldFile, TEXT("\0"));
  } else {
    static TCHAR szMapFile[MAX_PATH] = TEXT("\0");
    static TCHAR szFile[MAX_PATH] = TEXT("\0");
    GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
    if (_tcslen(szMapFile)>0) {
      ExpandLocalPath(szMapFile);
      _tcscpy(szFile,szMapFile);
      _tcscat(szFile,TEXT("/airfields.txt"));
      unicode2ascii(szFile, zfilename, MAX_PATH);
    } else {
      zfilename[0]= 0;
    }
  }
  if (strlen(zfilename)>0) {
    zAirfieldDetails = zzip_fopen(zfilename,"rb");
  }
}

static void
CloseAirfieldDetails()
{
  if (zAirfieldDetails == NULL) {
    return;
  }
  // file was OK, so save the registry
  ContractLocalPath(szAirfieldDetailsFile);
  SetRegistryString(szRegistryAirfieldFile, szAirfieldDetailsFile);

  zzip_fclose(zAirfieldDetails);
  zAirfieldDetails = NULL;
}




/*
 * VENTA3: Home and Preferred landing points for BestAlternate
 * Paolo Ventafridda
 */


class WaypointNameLookup: public WaypointVisitor {
public:
  WaypointNameLookup(const TCHAR* _Name, const TCHAR* _Details):
    Name(_Name), Details(_Details)
    {};

  void waypoint_landable(WAYPOINT &waypoint, WPCALC &wpcalc, const unsigned i)
    {
      waypoint_airport(waypoint, wpcalc, i);
    }
  void waypoint_airport(WAYPOINT &waypoint, WPCALC &wpcalc, const unsigned i)
    {

      // TODO: detect and warn on multiple matches!

      TCHAR UName[100];
      TCHAR NameA[100];
      TCHAR NameB[100];
      TCHAR NameC[100];
      TCHAR NameD[100];
      TCHAR TmpName[100];

      _tcscpy(UName, waypoint.Name);

      CharUpper(UName); // WP name
      // VENTA3 fix: If airfields name
      // was not uppercase it was not recon

      _stprintf(NameA,TEXT("%s A/F"),Name);
      _stprintf(NameB,TEXT("%s AF"),Name);
      _stprintf(NameC,TEXT("%s A/D"),Name);
      _stprintf(NameD,TEXT("%s AD"),Name);

      bool isHome=false;
      bool isPreferred=false;

      _stprintf(TmpName,TEXT("%s=HOME"),UName);
      if ( (_tcscmp(Name, TmpName)==0) )  isHome=true;
      _stprintf(TmpName,TEXT("%s=PREF"),UName);
      if ( (_tcscmp(Name, TmpName)==0) )  isPreferred=true;
      _stprintf(TmpName,TEXT("%s=PREFERRED"),UName);
      if ( (_tcscmp(Name, TmpName)==0) )  isPreferred=true;

      if ( isHome==true ) {
        wpcalc.Preferred = true;
        XCSoarInterface::SetSettingsComputer().HomeWaypoint = i;
      }
      if ( isPreferred==true ) {
        wpcalc.Preferred = true;
      }

      if ((_tcscmp(UName, Name)==0)
          ||(_tcscmp(UName, NameA)==0)
          ||(_tcscmp(UName, NameB)==0)
          ||(_tcscmp(UName, NameC)==0)
          ||(_tcscmp(UName, NameD)==0)
          || isHome || isPreferred )
      {
        // found

        if (_tcslen(Details) >0 ) { // VENTA3 avoid setting empty details
          if (waypoint.Details) {
            free(waypoint.Details);
          }
          waypoint.Details = (TCHAR*)malloc((_tcslen(Details)+1)*sizeof(TCHAR));
          _tcscpy(waypoint.Details, Details);
        }
        return;
      }
    }
private:
  const TCHAR* Name;
  const TCHAR* Details;
};


static void
LookupAirfieldDetail(TCHAR *Name, const TCHAR *Details)
{
  CharUpper(Name);  // AIR name
  WaypointNameLookup wnl(Name, Details);
  WaypointScan::scan_forward(wnl);
}


#define DETAILS_LENGTH 5000

/*
 * VENTA3 fix: if empty lines, do not set details for the waypoint
 *        fix: remove CR from text appearing as a spurious char in waypoint details
 */
static void
ParseAirfieldDetails()
{
  if(zAirfieldDetails == NULL)
    return;

  TCHAR TempString[READLINE_LENGTH+1];
  TCHAR CleanString[READLINE_LENGTH+1];
  TCHAR Details[DETAILS_LENGTH+1];
  TCHAR Name[201];

  Details[0]= 0;
  Name[0]= 0;
  TempString[0]=0;
  CleanString[0]=0;

  bool inDetails = false;
  bool hasDetails = false; // VENTA3
  int i, n;
  unsigned j;
  int k=0;

  while(ReadString(zAirfieldDetails,READLINE_LENGTH,TempString))
    {
      if(TempString[0]=='[') { // Look for start

	if (inDetails) {
	  LookupAirfieldDetail(Name, Details);
	  Details[0]= 0;
	  Name[0]= 0;
	  hasDetails=false;
	}

	// extract name
	for (i=1; i<200; i++) {
	  if (TempString[i]==']') {
	    break;
	  }
	  Name[i-1]= TempString[i];
	}
	Name[i-1]= 0;

	inDetails = true;

        if (k % 20 == 0) {
	  XCSoarInterface::StepProgressDialog();
        }
        k++;

      } else {
	// VENTA3: append text to details string
	for (j=0; j<_tcslen(TempString); j++ ) {
	  if ( TempString[j] > 0x20 ) {
	    hasDetails = true;
	    break;
	  }
	}
	// first hasDetails set true for rest of details
	if (hasDetails==true) {

	  // Remove carriage returns
	  for (j=0, n=0; j<_tcslen(TempString); j++) {
	    if ( TempString[j] == 0x0d ) continue;
	    CleanString[n++]=TempString[j];
	  }
	  CleanString[n]='\0';

	  if (_tcslen(Details)+_tcslen(CleanString)+3<DETAILS_LENGTH) {
	    _tcscat(Details,CleanString);
	    _tcscat(Details,TEXT("\r\n"));
	  }
	}
      }
    }

  if (inDetails) {
    LookupAirfieldDetail(Name, Details);
  }

}


void ReadAirfieldFile() {

  StartupStore(TEXT("ReadAirfieldFile\n"));

  XCSoarInterface::CreateProgressDialog(gettext(TEXT("Loading Airfield Details File...")));

  {
    OpenAirfieldDetails();
    ParseAirfieldDetails();
    CloseAirfieldDetails();
  }

}

