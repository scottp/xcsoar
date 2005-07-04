/**************************************************************************

	XCSoarLaunch

	main.c

	(C) Copyright 2001 By Tomoaki Nakashima. All right reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <Todaycmn.h>
#include <Commctrl.h>
#include <commdlg.h>
#include <aygshell.h>

#include "resource-launch.h"


/**************************************************************************
	Define
**************************************************************************/

#define WINDOW_TITLE			TEXT("XCSoarLaunch")
#define MAIN_WND_CLASS			TEXT("XCSoarLaunchWndClass")

#define REG_PATH				TEXT("Software\\XCSoarLaunch")

#define FILE_EXPLORER			TEXT("fexplore.exe")

#define ID_ICON_TIMER			1

#define BUF_SIZE				256


/**************************************************************************
	Global Variables
**************************************************************************/

static HINSTANCE hInst;
static HWND hToolTip;

static TCHAR backbmpfile[BUF_SIZE];
static int stretch_backbmp = 0;
static int IconSizeX = 64;
static int IconSizeY = 32;
static int HMargin = 2;
static int VMargin = 2;
static int WinLeftMargin = 0;
static int WinTopMargin = 0;
static int WinRightMargin = 13;
static int WinBottomMargin = 5;
static int ShowIconSec = 1;


static BOOL Refresh;

typedef struct _FILELIST {
	TCHAR Name[BUF_SIZE];
	TCHAR FileName[BUF_SIZE];
	TCHAR CommandLine[BUF_SIZE];
	int Index;
  HBITMAP bitmap;
} FILELIST;

static FILELIST FileList[2];
static int FileListCnt=2;
static int SelItem = -1;



static void CreateFileList() {
  lstrcpy(FileList[0].Name, TEXT("XCSoar"));
  lstrcpy(FileList[0].FileName, TEXT("\\Program Files\\XCSoar\\XCSoar.exe"));
  lstrcpy(FileList[0].CommandLine, TEXT("\\Program Files\\XCSoar\\XCSoar.exe"));

  if (FileList[0].bitmap==NULL) {
    FileList[0].bitmap = LoadBitmap(hInst,
                                    MAKEINTRESOURCE(IDB_XCSOAR));
  }
  FileList[0].Index = 0;

  lstrcpy(FileList[1].Name, TEXT("XCSoar Sim"));
  lstrcpy(FileList[1].FileName, TEXT("\\Program Files\\XCSoar\\XCSoarSimulator.exe"));
  lstrcpy(FileList[1].CommandLine, TEXT("\\Program Files\\XCSoar\\XCSoarSimulator.exe"));
  FileList[1].Index = 1;

  if (FileList[1].bitmap==NULL) {
    FileList[1].bitmap = LoadBitmap(hInst,
                                    MAKEINTRESOURCE(IDB_XCSOARSIMULATOR));
  }


}


/******************************************************************************

	DllMain

	DllMain

******************************************************************************/

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD fdwReason, PVOID pvReserved)
{
	hInst = hModule;
	return TRUE;
}



/******************************************************************************

	ToolTipProc

	�c�[���`�b�v

******************************************************************************/

static BOOL CALLBACK ToolTipProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT rect;
	SIZE sz;
	HDC hdc;
	static TCHAR buf[BUF_SIZE];

	switch(uMsg)
	{
	case WM_INITDIALOG:
		break;

	case WM_LBUTTONDOWN:
		ShowWindow(hDlg, SW_HIDE);
		break;

	case WM_SETTEXT:
		if(lParam == 0){
			break;
		}
		lstrcpy(buf, (TCHAR *)lParam);

		hdc = GetDC(hDlg);
		GetTextExtentPoint32(hdc, buf, lstrlen(buf), &sz);
		ReleaseDC(hDlg, hdc);

		SetWindowPos(hDlg, 0, 0, 0, sz.cx + 6, sz.cy + 6, SWP_NOMOVE | SWP_NOZORDER | SWP_HIDEWINDOW | SWP_NOACTIVATE);
		break;

	case WM_PAINT:
                /*
		hdc = BeginPaint(hDlg, &ps);

		GetClientRect(hDlg,(LPRECT)&rect);
		FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
		ExtTextOut(hdc, 2, 2, ETO_OPAQUE, NULL, buf, lstrlen(buf), NULL);

		EndPaint(hDlg, &ps);
                */
		break;

	default:
		return FALSE;
	}
	return TRUE;
}



/******************************************************************************

	OnPaint

	�A�C�R���̕`��

******************************************************************************/

void OnPaint(HWND hWnd, HDC hdc, PAINTSTRUCT *ps)
{
  //#ifdef _WCE_PPC2002
	TODAYDRAWWATERMARKINFO dwi;
        //#endif

	HDC drawdc, tempdc;
	HBITMAP hDrawBitMap;
	HBITMAP hRetDrawBmp;
	HBITMAP hRetBmp;
	RECT rect;
	RECT selrect;
	BITMAP bmp;
	int x, y;
	int i;
	HBRUSH hBrush;

	GetClientRect(hWnd, (LPRECT)&rect);

	drawdc = CreateCompatibleDC(hdc);
	tempdc = CreateCompatibleDC(hdc);
	hDrawBitMap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
	hRetDrawBmp = SelectObject(drawdc, hDrawBitMap);

        //#ifdef _WCE_PPC2002

          dwi.hdc = drawdc;
          GetClientRect(hWnd, &dwi.rc);
          dwi.hwnd = hWnd;
          SendMessage(GetParent(hWnd), TODAYM_DRAWWATERMARK, 0, (LPARAM)&dwi);

        /*
#else
	FillRect(drawdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
#endif
        */

	x = WinLeftMargin;
	y = WinTopMargin;

	for(i = 0; i < FileListCnt; i++){

          if(SelItem == i){
            SetRect(&selrect, x, y, x + IconSizeX + (HMargin * 2),
                    y + IconSizeY + (VMargin * 2));
            hBrush = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
            FillRect(drawdc, &selrect, hBrush);
            DeleteObject(hBrush);
          }

          SelectObject(tempdc, FileList[i].bitmap);
          TransparentBlt(drawdc,
                         x+HMargin, y+VMargin,
                         IconSizeX, IconSizeY,
                         tempdc, 0, 0, IconSizeX, IconSizeY, RGB(0, 0, 255));

          x+= IconSizeX+HMargin*2;

	}

	BitBlt(hdc, ps->rcPaint.left, ps->rcPaint.top, ps->rcPaint.right, ps->rcPaint.bottom,
		drawdc, ps->rcPaint.left, ps->rcPaint.top, SRCCOPY);


	SelectObject(drawdc, hRetDrawBmp);
	DeleteObject(hDrawBitMap);
	DeleteDC(drawdc);
	DeleteDC(tempdc);
}


/******************************************************************************

	Point2Item

	�ʒu����A�C�e�����擾

******************************************************************************/

static int Point2Item(int px, int py)
{
	RECT rect;
	POINT pt;
	int x, y;
	int i;

	pt.x = px;
	pt.y = py;
	for(x = WinLeftMargin, y = WinTopMargin, i = 0; i < FileListCnt; i++){
		if((x + IconSizeX + (HMargin * 2)) > GetSystemMetrics(SM_CXSCREEN) - WinRightMargin){
			x = WinLeftMargin;
			y += IconSizeY + (VMargin * 2);
		}
		SetRect(&rect, x, y, x + IconSizeX + (HMargin * 2), y + IconSizeY + (VMargin * 2));
		if(PtInRect(&rect, pt) == TRUE){
			return i;
		}
		x += IconSizeX + (HMargin * 2);
	}
	return -1;
}


/******************************************************************************

	ShellOpen

	�t�@�C�������s

******************************************************************************/

static BOOL ShellOpen(TCHAR *FileName, TCHAR *CommandLine)
{
	SHELLEXECUTEINFO sei;
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if((hFindFile = FindFirstFile(FileName, &FindData)) != INVALID_HANDLE_VALUE){
		FindClose(hFindFile);
		if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			//Folder open
			CommandLine = FileName;
			FileName = FILE_EXPLORER;
		}
	}
	if(lstrcmp(FileName, TEXT("\\")) == 0){
		CommandLine = FileName;
		FileName = FILE_EXPLORER;
	}

	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(sei);
	sei.fMask = 0;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	sei.lpFile = FileName;
	if(*CommandLine != TEXT('\0')){
		sei.lpParameters = CommandLine;
	}
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = hInst;
	return ShellExecuteEx(&sei);
}


/******************************************************************************

	WndProc

	���C���E�B���h�E�v���V�[�W��

******************************************************************************/

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	SHRGINFO rg;
	int x, y;
	int i;

	switch(msg)
	{
	case WM_CREATE:
		if(hToolTip == NULL){
			hToolTip = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_TOOLTIP), NULL, ToolTipProc);
		}
		break;

	case WM_DESTROY:
		DestroyWindow(hToolTip);
		hToolTip = NULL;

		FileListCnt = 0;
		return 0;

	case WM_TODAYCUSTOM_CLEARCACHE:
		break;

	case WM_TODAYCUSTOM_QUERYREFRESHCACHE:
		if(Refresh){
			Refresh = FALSE;

                        // compute screen extents
			for(x = WinLeftMargin, y = WinTopMargin, i = 0; i < FileListCnt; i++){
				if((x + IconSizeX + (HMargin * 2)) > GetSystemMetrics(SM_CXSCREEN) - WinRightMargin){
					x = WinLeftMargin;
					y += IconSizeY + (VMargin * 2);
				}
				x += IconSizeX + (HMargin * 2);
			}
			y += IconSizeY + (VMargin * 2) + WinBottomMargin;
			((TODAYLISTITEM *)(wParam))->cyp = y;

			SetTimer(hWnd, ID_ICON_TIMER, ShowIconSec * 1000, NULL);
			return TRUE;
		}
		return FALSE;

	case WM_LBUTTONDOWN:
		SelItem = Point2Item(LOWORD(lParam), HIWORD(lParam));
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);

		rg.cbSize = sizeof(SHRGINFO);
		rg.hwndClient = hWnd;
		rg.ptDown.x = LOWORD(lParam);
		rg.ptDown.y = HIWORD(lParam);
#ifdef _WCE_PPC2002
		rg.dwFlags = SHRG_RETURNCMD | SHRG_NOANIMATION;
#else
		rg.dwFlags = SHRG_RETURNCMD;
#endif

		if(SelItem != -1 && SHRecognizeGesture(&rg) == GN_CONTEXTMENU){
			RECT rect;
			RECT tip_rect;

			SendMessage(hToolTip, WM_SETTEXT, 0, (LPARAM)(FileList + SelItem)->Name);
			GetWindowRect(hWnd, &rect);
			GetWindowRect(hToolTip, &tip_rect);
			tip_rect.left = rect.left + LOWORD(lParam) - (tip_rect.right - tip_rect.left) - 10;
			if(tip_rect.left < 0) tip_rect.left = 0;
			tip_rect.top = rect.top + HIWORD(lParam) - (tip_rect.bottom - tip_rect.top) - 10;
			if(tip_rect.top < 0) tip_rect.top = 0;

			SetWindowPos(hToolTip, HWND_TOPMOST,
				tip_rect.left, tip_rect.top,
				0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		SetCapture(hWnd);
		break;

	case WM_LBUTTONUP:
		ShowWindow(hToolTip, SW_HIDE);
		ReleaseCapture();
		i = Point2Item(LOWORD(lParam), HIWORD(lParam));
		if(i != -1 && i == SelItem){
			ShellOpen((FileList + i)->FileName, (FileList + i)->CommandLine);
		}
		SelItem = -1;
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		OnPaint(hWnd, hdc, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_TIMER:
		switch(wParam)
		{
		case ID_ICON_TIMER:

                  // JMW maybe break here?

                  KillTimer(hWnd, ID_ICON_TIMER);
                  InvalidateRect(hWnd, NULL, FALSE);
                  UpdateWindow(hWnd);
                  break;
		}
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}


/******************************************************************************

	InitInstance

	�E�B���h�E�̍쐬

******************************************************************************/


static HWND InitInstance(HWND pWnd, TODAYLISTITEM *ptli)
{
	WNDCLASS wc;

	hInst = ptli->hinstDLL;

	CreateFileList();

	//�E�B���h�E�N���X�̓o�^
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hCursor = 0;
	wc.lpszMenuName = 0;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = ptli->hinstDLL;
	wc.hIcon = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = MAIN_WND_CLASS;
	UnregisterClass(MAIN_WND_CLASS, ptli->hinstDLL);
	RegisterClass(&wc);

	Refresh = TRUE;

	//�E�B���h�E�̍쐬
	return CreateWindow(MAIN_WND_CLASS, WINDOW_TITLE, WS_CHILD | WS_VISIBLE, CW_DEFAULT, CW_DEFAULT, 0, 0,
		pWnd, NULL, ptli->hinstDLL, NULL);
}


/******************************************************************************

	InitializeCustomItem

	������

******************************************************************************/

HWND APIENTRY InitializeCustomItem(TODAYLISTITEM *ptli, HWND pWnd)
{
	if(ptli->fEnabled == 0){
		return NULL;
	}
	return InitInstance(pWnd, ptli);
}


/******************************************************************************

	CustomItemOptionsDlgProc

	�ݒ���

******************************************************************************/

BOOL APIENTRY CustomItemOptionsDlgProc(HWND hDlg, UINT uMsg, UINT wParam, LONG lParam)
{
	SHINITDLGINFO shidi;
	LVCOLUMN lvc;
	LV_ITEM lvi;
	int ItemIndex;
	int i;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		//PocketPC�p������
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
		shidi.hDlg = hDlg;
		SHInitDialog(&shidi);

		SetWindowText(hDlg, TEXT("XCSoarLaunch"));

		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{


		case IDC_BUTTON_UNINSTALL:
			if(MessageBox(hDlg, TEXT("Delete today information?"), TEXT("Uninstall"), MB_ICONSTOP | MB_YESNO) == IDYES){
				RegDeleteKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Today\\Items\\XCSoarLaunch"));
				MessageBox(hDlg, TEXT("Information was deleted. Please uninstall."), TEXT("Info"), MB_OK | MB_ICONINFORMATION);
			}
			EndDialog(hDlg, IDOK);
			break;

		case IDOK:
			EndDialog(hDlg, IDOK);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/* End of source */