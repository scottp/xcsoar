/*  wcecompat: Windows CE C Runtime Library "compatibility" library.
 *
 *  Copyright (C) 2001-2002 Essemer Pty Ltd.  All rights reserved.
 *  http://www.essemer.com.au/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ts_string.h"

#include <string.h>
#include <stdlib.h>

static bool
check_wchar_align(const void *p)
{
  return ((long)p & (sizeof(WCHAR) - 1)) == 0;
}

void ascii2unicode(const char* ascii, WCHAR* unicode)
{
  if (strlen(ascii)==0) {
    unicode[0]=0;
    unicode[1]=0;
    return;
  }

  if (check_wchar_align(unicode))
	{	// word-aligned
		while (*ascii != '\0')
			*unicode++ = *ascii++;
		*unicode = '\0';
	}
	else
	{	// not word-aligned
		while (*ascii != '\0')
		{
			*(char*)unicode = *ascii++;
			*(((char*)unicode)+1) = 0;
			unicode++;
		}
		*(char*)unicode = 0;
		*(((char*)unicode)+1) = 0;
	}
}

#ifdef _UNICODE

void unicode2ascii(const WCHAR* unicode, char* ascii)
{
  if (wcslen(unicode)==0) {
    ascii[0] = 0;
    return;
  }

  if (check_wchar_align(unicode))
	{	// word-aligned
		while (*unicode != '\0')
			*ascii++ = (char)*unicode++;
		*ascii = '\0';
	}
	else
	{	// not word-aligned
		while (*(char*)unicode != 0 || *(((char*)unicode)+1) != 0)
			*ascii++ = *(char*)unicode++;
		*ascii = '\0';
	}
}

#endif /* _UNICODE */

void ascii2unicode(const char* ascii, WCHAR* unicode, int maxChars)
{
  if (strlen(ascii)==0) {
    unicode[0]=0;
    unicode[1]=0;
    return;
  }

  if (check_wchar_align(unicode))
	{	// word-aligned
    int i;
		for (i=0; ascii[i] != 0 && i<maxChars; i++)
			unicode[i] = ascii[i];
		unicode[i] = 0;
	}
	else
	{	// not word-aligned
    int i;
		for (i=0; ascii[i] != 0 && i<maxChars; i++)
		{
			*(char*)&unicode[i] = ascii[i];
			*(((char*)&unicode[i])+1) = 0;
			unicode++;
		}
		*(char*)&unicode[i] = 0;
		*(((char*)&unicode[i])+1) = 0;
	}
}

#ifdef _UNICODE

void unicode2ascii(const WCHAR* unicode, char* ascii, int maxChars)
{
  if (wcslen(unicode)==0) {
    ascii[0] = 0;
    return;
  }

  if (check_wchar_align(unicode))
	{	// word-aligned
    int i;
		for (i=0; unicode[i] != 0 && i<maxChars; i++)
			ascii[i] = (char)unicode[i];
		ascii[i] = 0;
	}
	else
	{	// not word-aligned
    int i;
		for (i=0; (*(char*)&unicode[i] != 0 || *(((char*)&unicode[i])+1) != 0) && i<maxChars; i++)
			ascii[i] = *(char*)&unicode[i];
		ascii[i] = 0;
	}
}

#endif /* _UNICODE */

#ifndef _UNICODE
void unicode2ascii(const char *unicode, char *ascii, int maxChars)
{
  strncpy(ascii, unicode, maxChars - 1);
  ascii[maxChars - 1] = 0;
}

void unicode2ascii(const char *unicode, char *ascii)
{
  strcpy(ascii, unicode);
}

void ascii2unicode(const char *ascii, char *unicode)
{
  strcpy(unicode, ascii);
}
#endif /* _UNICODE */


//
// ascii/unicode typesafe versions of strcat
//

#ifdef _UNICODE

char* ts_strcat(char* dest, const unsigned short* src)
{
	char* p = dest;
	while (*p != '\0')
		p++;
	unicode2ascii((const wchar_t *)src, p);
	return dest;
}

unsigned short* ts_strcat(unsigned short* dest, const char* src)
{
	unsigned short* p = dest;
	while (*p != '\0')
		p++;
	ascii2unicode(src, (wchar_t *)p);
	return dest;
}

#endif /* _UNICODE */


//
// ascii/unicode typesafe versions of strdup
//

#ifdef _UNICODE

char* ts_strdup_unicode_to_ascii(const unsigned short* str)
{
	char* result = (char*)malloc(wcslen((const wchar_t *)str)+1);
	if (result == NULL)
		return NULL;
	unicode2ascii((const wchar_t *)str, result);
	return result;
}

unsigned short* ts_strdup_ascii_to_unicode(const char* str)
{
	unsigned short* result = (unsigned short*)malloc((strlen(str)+1)*2);
	if (result == NULL)
		return NULL;
	ascii2unicode(str, (wchar_t *)result);
	return result;
}

#endif /* _UNICODE */
