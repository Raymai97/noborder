#pragma once
// Enable NT5 Win32 API
#define WINVER			0x0500
#define _WIN32_WINNT	0x0500
// VC6 SDK is too old... IKR
#define message(ignore)

#include <Windows.h>
#include <shellapi.h>
#include <tchar.h>
#include "_dwmapi.h"
#include "_winapi_extend.h"
#include "NotifyIcon.h"
#include "resource.h"


// Define, Const, Class...
#define MSGERR(x)		MessageBox(NULL, _T(x), _T("noborder Error!"), MB_OK | MB_ICONSTOP | MB_TOPMOST)
#define NOTIFYICON_ID	1
#define PREVINST_CALL	WM_APP + 97
#define SWM_TRAYMSG		WM_APP
#define SWM_ABOUT		WM_APP + 1
#define SWM_EXIT		WM_APP + 2
#define SWM_EXCLUDE_TASKBAR		WM_APP + 3
#define SWM_AOT_AUTO	WM_APP + 4
#define SWM_AOT_ALWAYS	WM_APP + 5
#define SWM_AOT_NEVER	WM_APP + 6
#define SWM_USE_DWM		WM_APP + 7

typedef enum
{
	AOT_AUTO,
	AOT_ALWAYS,
	AOT_NEVER
} AOT;

static const TCHAR *NBD_APP_TITLE = _T("noborder v1.3");
static const TCHAR *NBD_APP_DESC =
	_T("Ver tiny 1.3 . by Raymai97 . in VC6.0 from VS2015\n\n")
	_T("A tool to switch current window into 'Borderless' mode.\n")
	_T("Press ALT+BACKSPACE to toggle.");
static const TCHAR *NBD_MUTEX_NAME = _T("NOBORDER MUTEX");
static const TCHAR *NBD_DUMMY_MSG = _T("noborder MsgWindow");
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
static const TCHAR *NBD_CMI_ABOUT = _T("About");
static const TCHAR *NBD_CMI_EXIT = _T("Exit");

static const DWORD KEY_STYLE = (WS_CAPTION | WS_THICKFRAME);
static const DWORD TOGGLE_MOD = VK_MENU;
static const DWORD TOGGLE_KEY = VK_BACK;

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void MenuCreatingProc(UINT niId, HMENU hMenu);
void MenuItemSelectedProc(WORD id, WORD event);
void LoadConfig();
void SaveConfig();
// Core.cpp
void CoreInit();
void CoreClosing();
void ToggleNoborder();
void DoNoborder(Target *t);
void UndoNoborder(Target *t);
PosSize NoborderPosSize(HWND hWnd, const PosSize psClient);
BOOL CALLBACK MonitorEnumProc(HMONITOR h, HDC hdc, LPRECT lprc, LPARAM dwData);
void BringToTop(HWND hWnd, bool topMost);

// Global variables
extern HINSTANCE hInst;
extern NotifyIcon *notifyIcon;
extern bool canUseDWM;
extern bool excludeTaskbar;
extern AOT alwaysOnTopMode;
extern bool useDWM;
