// this module does NOTHING!
// it just simulate the aygSH interface to keep the compiler happy

// some macros to keep the compiler happy

#define SHCMBF_EMPTYBAR   0  //???
#define SHCMBF_HIDDEN     0  //???

#define SHIDIM_FLAGS      0

#define SHIDIF_DONEBUTTON       0x0001
// puts the OK button on the navigation bar
#define SHIDIF_SIZEDLG         0x0002
// sizes the dialog box, noting the current position of the input panel
#define SHIDIF_SIZEDLGFULLSCREEN   0x0004
// sizes the dialog box FULL screen, regardless of the position of the input panel
#define SHIDIF_SIPDOWN         0x0008
// puts the input panel down
#define SHIDIF_FULLSCREENNOMENUBAR   0x0010
// sizes the dialog box FULL screen. Does not leave room at the bottom for a menu bar


// missing library function dummy's

#define SHFullScreen(x, y)                    (1==1)
#define SHSetAppKeyWndAssoc(x, y)             (1==1)
#define SHHandleWMSettingChange(x, y, z, v)   (1==1)
#define SHHandleWMActivate(x, y, z, v, w)     (1==1)
#define SHCreateMenuBar(x)                    (1==1)
#define SHInitDialog(x)                       ((void)0)
#define SHSipPreference(hDlg,SIP_FORCEDOWN)   ((void)0)

// some typedefs to keep the compiler happy

typedef struct tagSHINITDIALOG{
  DWORD dwMask;
  HWND hDlg;
  DWORD dwFlags;
} SHINITDLGINFO, *PSHINITDLGINFO;

typedef struct {
  DWORD cbSize;
  HWND hwndLastFocus;
  UINT fSipUp :1;
  UINT fSipOnDeactivation :1;
  UINT fActive :1;
  UINT fReserved :29;
} SHACTIVATEINFO, *PSHACTIVATEINFO;

typedef struct tagSHMENUBARINFO{
  DWORD cbSize;
  HWND hwndParent;
  DWORD dwFlags;
  UINT nToolBarId;
  HINSTANCE hInstRes;
  int nBmpId;
  int cBmpImages;
  HWND hwndMB;
} SHMENUBARINFO, *PSHMENUBARINFO;

