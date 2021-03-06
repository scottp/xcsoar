#ifndef MAPERROR_H
#define MAPERROR_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MS_NOERR 0 /* general error codes */
#define MS_IOERR 1
#define MS_MEMERR 2
#define MS_TYPEERR 3
#define MS_SYMERR 4
#define MS_REGEXERR 5
#define MS_TTFERR 6
#define MS_DBFERR 7
#define MS_GDERR 8
#define MS_IDENTERR 9
#define MS_EOFERR 10
#define MS_PROJERR 11
#define MS_MISCERR 12
#define MS_CGIERR 13
#define MS_WEBERR 14
#define MS_IMGERR 15
#define MS_HASHERR 16
#define MS_JOINERR 17
#define MS_NOTFOUND 18 /* empty search results */
#define MS_SHPERR 19
#define MS_PARSEERR 20
#define MS_SDEERR 21
#define MS_OGRERR 22
#define MS_QUERYERR 23
#define MS_WMSERR 24      /* WMS server error */
#define MS_WMSCONNERR 25  /* WMS connectiontype error */
#define MS_ORACLESPATIALERR 26
#define MS_WFSERR 27      /* WFS server error */
#define MS_WFSCONNERR 28  /* WFS connectiontype error */
#define MS_MAPCONTEXTERR 29 /* Map Context error */
#define MS_HTTPERR 30

#define MS_NUMERRORCODES 31

#define MESSAGELENGTH 2048
#define ROUTINELENGTH 64

typedef struct error_obj {
  int code;
  char routine[ROUTINELENGTH];
  char message[MESSAGELENGTH];
#ifndef SWIG
  struct error_obj *next;
#endif
} errorObj;

/*
** Global variables
*/
/* extern errorObj ms_error; */

/*
** Function prototypes
*/
errorObj *msGetErrorObj(void);
void msResetErrorList();
char *msGetVersion(void);
char *msGetErrorString(char *delimiter);

#ifndef SWIG
void msSetError(int code, const char *message, const char *routine, ...);
void msWriteError(FILE *stream);
char *msGetErrorCodeString(int code);

struct map_obj;
void msWriteErrorImage(struct map_obj *map, char *filename, int blank);

void msDebug( const char * pszFormat, ... );
void msWebDebug( const char * pszFormat, ... );
#endif

#ifdef __cplusplus
}
#endif

#endif /* MAPERROR_H */
