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

#include "TeamCodeCalculation.h"

#include <math.h>
#include <string.h>

void ConvertHeadingToTeamCode(double heading, TCHAR *code);
void NumberToTeamCode(double value, TCHAR *code, int minCiffers);
double GetBearing(const TCHAR *code );
double GetRange(const TCHAR *code );
int GetValueFromTeamCode(const TCHAR *code, int maxCount);

#define TEAMCODE_COMBINAIONS 1296
#define DEG_TO_RAD .0174532925199432958

void GetTeamCode(TCHAR *code, double bearing, double range)
{
	memset(code, 0, sizeof(TCHAR) * 10);

		// trim to seeyou
	//double trim = cos(bearing*DEG_TO_RAD)*range/8500;
		//bearing+= 1.2;

	 //   if (bearing > 360)
		//{
		//	bearing -= 360;
		//}



	ConvertHeadingToTeamCode(bearing, code);
	NumberToTeamCode(range/100.0, &code[2],0);
}

void ConvertHeadingToTeamCode(double heading, TCHAR *code)
{
	if (heading >= 360)
	{
		code[0] = '-';
		code[1] = '-';
		return;
	}

	double bamValue = (heading * TEAMCODE_COMBINAIONS) / 360.0;
	NumberToTeamCode(bamValue, code, 2);
}

void NumberToTeamCode(double value, TCHAR *code, int minCiffers)
{

	int maxCif = 0;
	int curCif = 0;

	if (minCiffers > 0)
	{
		maxCif = minCiffers - 1;
		curCif = maxCif;
	}

	double rest = value;
	while(rest > 0 || curCif >= 0 )
	{
		int cifVal = (int)pow(36.0, curCif);
		int partSize = (int)(rest / cifVal);
		int partVal = partSize * cifVal;
		int txtPos = maxCif - curCif;

		if (partSize < 10)
		{
			rest -= partVal;
			code[txtPos] = (unsigned char)('0' + partSize);
			curCif--;
		}
		else if (partSize < 36)
		{
			rest -= partVal;
			code[txtPos] = (unsigned char)('A' + partSize - 10);
			curCif--;
		}
		else
		{
			curCif++;
			maxCif = curCif;
		}

		if (rest < 1) rest = 0;
	}
}

double GetBearing(const TCHAR *code )
{
	int val = GetValueFromTeamCode(code, 2);

	double bearing = (val * 360.0/TEAMCODE_COMBINAIONS);
	 bearing -= 0;
	if (bearing < 0)
	{
		bearing += 360;
	}

	return bearing;
}

double GetRange(const TCHAR *code )
{
	int val = GetValueFromTeamCode(&code[2], 3);
	return val*100.0;
}

double GetTeammateBearingFromRef(const TCHAR *code )
{
	return GetBearing(code);
}

double GetTeammateRangeFromRef(const TCHAR *code )
{
	return GetRange(code);
}

int GetValueFromTeamCode(const TCHAR *code, int maxCount)
{
	int val = 0;
	int charPos = 0;
	while (code[charPos] != 0 && charPos < maxCount)
	{
		int cifferVal = 0;
		if (code[charPos] >= '0' && code[charPos] <= '9')
		{
			cifferVal = (int) (code[charPos] - '0');
		}
		else if (code[charPos] >= 'A' && code[charPos] <= 'Z')
		{
			cifferVal = (int) (code[charPos]+ - 'A')+10;
		}

		val = val * 36;
		val += (int)cifferVal;

		charPos++;
	}
	return val;
}

void CalcTeamMatePos(double ownBear, double ownDist, double mateBear, double mateDist, double *bearToMate, double *distToMate)
{
	// define constants
	double PI = 3.14159265358979;
	double toRad = PI / 180.0;
	double toDeg = 180.0 / PI;

	// convert bearings to radians
	ownBear = ownBear * toRad;
	mateBear = mateBear * toRad;

	// Calculate range
	double Xs = ownDist*sin(ownBear) - mateDist*sin(mateBear);
	double Ys = ownDist*cos(ownBear) - mateDist*cos(mateBear);
	double range = sqrt((Xs*Xs)+(Ys*Ys));
	*distToMate = range;

	// Calculate bearing
	double bearing;

	if (Xs != 0)
	{
		bearing = (atan(Ys/Xs) * toDeg);
		if (Xs < 0) bearing =  bearing + 180;
		*bearToMate = 90.0 - bearing;
		if (*bearToMate < 0) *bearToMate += 360;
	}
	else
	{
		if (Ys >= 0)
		{
			*bearToMate = 180;
		}
		else
		{
			*bearToMate = 0;
		}
	}
}

void CalcTeammateBearingRange(double ownBear, double ownDist, const TCHAR *TeamMateCode,  double *bearToMate, double *distToMate)
{
	double calcBearing = GetBearing(TeamMateCode)/*+ 180*/;
	double calcRange = GetRange(TeamMateCode);


	//if (calcBearing > 360)
	//{
	//	calcBearing -= 360;
	//}


	CalcTeamMatePos(ownBear, ownDist, calcBearing, calcRange, bearToMate, distToMate);
}

