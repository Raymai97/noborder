#pragma once
#include <Windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <vector>
#include "resource.h"

// Define, Const, Class...
#define TRAYICON_ID		1
#define SWM_TRAYMSG		WM_APP
#define SWM_ABOUT		WM_APP + 1
#define SWM_EXIT		WM_APP + 2
#define SWM_EXCLUDE_TASKBAR		WM_APP + 3
#define SWM_AOT_AUTO		WM_APP + 4
#define SWM_AOT_ALWAYS		WM_APP + 5
#define SWM_AOT_NEVER		WM_APP + 6

typedef enum
{
	AOT_AUTO,
	AOT_ALWAYS,
	AOT_NEVER
} AOT;

static const TCHAR * NBD_APP_TITLE = _T("noborder v1.2");
static const TCHAR * NBD_APP_DESC = _T(
	"Version 1.2 . by Raymai97 . in VS2015\n\n" \
	"Press ALT+BACKSPACE to switch current application into 'Borderless Fullscreen'. " \
	"Press again will return back to normal.");
static const TCHAR * NBD_MUTEX_NAME = _T("NOBORDER MUTEX");
static const TCHAR * NBD_DUMMY = _T("NOBORDER DUMMY");
static const TCHAR * NBD_ERROR_ALREADY_RUNNING = _T("Error! noborder is already running!");
static const TCHAR * NBD_TRAYICON_TEXT = NBD_APP_TITLE;
static const TCHAR * NBD_CMI_ABOUT = _T("About");
static const TCHAR * NBD_CMI_EXIT = _T("Exit");
static const TCHAR * NBD_CMI_AOT = _T("Always on Top");
static const TCHAR * NBD_CMI_AOT_AUTO = _T("Auto");
static const TCHAR * NBD_CMI_AOT_ALWAYS = _T("Always");
static const TCHAR * NBD_CMI_AOT_NEVER = _T("Never");
static const TCHAR * NBD_CMI_EXCLUDE_TASKBAR = _T("Exclude Taskbar");
static const TCHAR * NBD_CONFIG_FILENAME = _T("noborder.config");

static const DWORD KEY_STYLE = (WS_CAPTION | WS_THICKFRAME);
static const DWORD TOGGLE_MOD = VK_MENU;
static const DWORD TOGGLE_KEY = VK_BACK;

class TARGET
{
public:
	HWND hWnd;
	LONG style;
	LONG exstyle;
	RECT rect;
	bool nobordered;

	TARGET()
	{
		nobordered = false;
	}
};

// Forward declarations of function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void LoadConfig();
void SaveConfig();
// UI.cpp
void AddNotifyIcon();
HWND CreateDummyWindow();
void RemoveNotifyIcon();
void ShowContextMenu(HWND hWnd);
// Core.cpp
void ToggleNoborder();
BOOL CALLBACK MonitorEnumProc(HMONITOR h, HDC hdc, LPRECT lprc, LPARAM dwData);

// Global variables
extern HINSTANCE hInst;
extern HWND hWnd;
extern bool excludeTaskbar;
extern AOT alwaysOnTopMode;

