#pragma once
#include <Windows.h>
#include <shellapi.h>
#include <tchar.h>
#include "compat_dwmapi.h"
#include "NotifyIcon.h"
#include "resource.h"

FORCEINLINE void flip(bool &b)
{
	b = !b;
}

// Define, Const, Class...
#define HASFLAG(x, f)	((x & f) == f)
#define MSGERR(x)		MessageBox(nullptr, _T(x), _T("noborder Error!"), MB_OK | MB_ICONSTOP | MB_TOPMOST)
#define NOTIFYICON_ID	1
#define PREVINST_CALL	WM_APP + 97

// --- NotifyIcon Context Menu IDs ---

#define IDM_About             101
#define IDM_Exit              102
#define IDM_ExcludeTaskbar    103
#define IDM_OnTopMode_Auto        104
#define IDM_OnTopMode_Always      105
#define IDM_OnTopMode_Never       106
#define IDM_UseDwmFormula     107
#define IDM_UseAltBksp        108
#define IDM_UseWinBksp        109

typedef enum
{
	OnTopMode_Auto,
	OnTopMode_Always,
	OnTopMode_Never
} OnTopMode;

//
// --- Windows Class Name ---
//

#define WCN_NbdMsgWindow  _T("noborder MsgWindow")


static const TCHAR *NBD_APP_TITLE = _T("noborder v1.6.0");
static const TCHAR *NBD_APP_DESC =
	_T("Version 1.6.0 . by raymai97\n\n")
	_T("Use hotkey to switch foreground window to 'Borderless' and vice versa.\n")
	_T("By default, the hotkey is set to ALT+BACKSPACE.");
static const TCHAR *NBD_MUTEX_NAME = _T("NOBORDER MUTEX");
static const TCHAR *NBD_DUMMY_NI = _T("noborder NotifyIcon");
static const TCHAR *NBD_DUMMY_DWMWINDOW = _T("noborder DwmWindow");
static const TCHAR *NBD_CONFIG_FILENAME = _T("noborder.config");

static const TCHAR *NBD_TRAYICON_TIP = NBD_APP_TITLE;
static const TCHAR *NBD_CMI_AOT = _T("Always on Top");
static const TCHAR *NBD_CMI_AOT_AUTO = _T("Auto");
static const TCHAR *NBD_CMI_AOT_ALWAYS = _T("Always");
static const TCHAR *NBD_CMI_AOT_NEVER = _T("Never");
static const TCHAR *NBD_CMI_EXCLUDE_TASKBAR = _T("Exclude Taskbar");
static const TCHAR *NBD_CMI_USE_DWM = _T("Use DWM formula");
static const TCHAR *NBD_CMI_HOTKEY = _T("Hotkey");
static const TCHAR *NBD_CMI_USE_ALT_BKSP = _T("Use Alt+Backspace");
static const TCHAR *NBD_CMI_USE_WIN_BKSP = _T("Use Win+Backspace");
static const TCHAR *NBD_CMI_ABOUT = _T("About");
static const TCHAR *NBD_CMI_EXIT = _T("Exit");


class PosSize
{
public:
	int X;
	int Y;
	int Width;
	int Height;

	PosSize()
	{
		this->X = 0;
		this->Y = 0;
		this->Width = 0;
		this->Height = 0;
	}

	PosSize(int x, int y, int width, int height)
	{
		this->X = x;
		this->Y = y;
		this->Width = width;
		this->Height = height;
	}

	PosSize(const RECT rc)
	{
		this->X = rc.left;
		this->Width = rc.right - rc.left;
		this->Y = rc.top;
		this->Height = rc.bottom - rc.top;
	}

	RECT ToRECT()
	{
		RECT rc;
		rc.left = this->X;
		rc.right = this->X + this->Width;
		rc.top = this->Y;
		rc.bottom = this->Y + this->Height;
		return rc;
	}

};

class Target
{
public:
	HWND hWnd;
	LONG style;
	LONG exStyle;
	PosSize psWin;
	PosSize psNbd;
	PosSize psCli;
	bool nobordered;
	bool isUsingDwm;

	Target()
	{
		nobordered = false;
		// don't have to init other members, they're considered
		// Write-Only if 'nobordered' is false
	}
};

class DwmWindow
{
private:
	HWND hWnd;
	HTHUMBNAIL hThumb;
	Target *target;
	// 'ing' controls CheckTargetProc while-loop
	// 'dontFocus' is True if 'Stop()' is caused by target's pop-up
	bool ing, topMost, dontFocus;
	void UpdateThumb();
	static DWORD WINAPI CheckTargetProc(LPVOID param);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	DwmWindow();
	~DwmWindow();
	void Start(Target *target, bool topMost);
	void Stop();
};

// Forward declarations of function
bool HasExistingInstance(void);
bool CreateNbdMsgWindow(void);
LRESULT CALLBACK NbdMsgWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void MenuCreatingProc(HMENU hMenu);
void MenuItemSelectedProc(WORD id);
bool LoadConfig();
bool SaveConfig();
// Core.cpp
void CoreInit();
void CoreClosing();
void ToggleNoborder();
void DoNoborder(Target *t);
void UndoNoborder(Target *t);
PosSize NoborderPosSize(HWND hWnd, const PosSize psClient);
void BringToTop(HWND hWnd, bool topMost);

typedef struct Cfg
{
	bool wantExcludeTaskbar;
	OnTopMode onTopMode;
	bool wantUseDwmFormula;
	bool wantUseAltBksp;
	bool wantUseWinBksp;
} Cfg;

// Global variables
extern HINSTANCE x_hInst;
extern NotifyIcon *x_pNotifyIcon;
extern bool x_canUseDwm;
extern Cfg x_cfg;
EXTERN_C FARPROC x_lpfnPhyToLogPtForPerMonitorDPI;

EXTERN_C HRESULT Compat_PhyToLogPtForPerMonitorDPI(HWND hWnd, LPPOINT lpPoint);
