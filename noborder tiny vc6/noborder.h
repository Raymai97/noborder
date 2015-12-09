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

static const TCHAR * NBD_APP_TITLE = _T("noborder v1.0");
static const TCHAR * NBD_MUTEX_NAME = _T("NOBORDER MUTEX");
static const TCHAR * NBD_DUMMY = _T("NOBORDER DUMMY");
static const TCHAR * NBD_ERROR_ALREADY_RUNNING = _T("Error! noborder is already running!");
static const TCHAR * NBD_TRAYICON_TEXT = NBD_APP_TITLE;
static const TCHAR * NBD_CMI_ABOUT = _T("About");
static const TCHAR * NBD_CMI_EXIT = _T("Exit");
static const TCHAR * NBD_APP_DESC = _T(
	"Ver tiny 1.0 . by Raymai97 . in VS6.0 from VS2015\n\n" \
	"Press ALT+BACKSPACE to switch current application into 'Borderless Fullscreen'. " \
	"Press again will return back to normal.");

static const DWORD KEY_STYLE = (WS_CAPTION | WS_THICKFRAME);
static const DWORD TOGGLE_MOD = VK_MENU;
static const DWORD TOGGLE_KEY = VK_BACK;

class TARGET
{
public:
	HWND hWnd;
	LONG style;
	RECT rect;
	bool nobordered;
	TARGET()
	{
		nobordered=false;
	}
};

// Forward declarations of function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
// UI.cpp
HWND CreateDummyWindow();
void ShowContextMenu(HWND hWnd);
// Core.cpp
void ToggleNoborder();
SIZE getDesktopSize();

// Global variables
static HANDLE hMutex;
static HINSTANCE hInst;
static HWND hWnd; 
static NOTIFYICONDATA ni;
static std::vector<TARGET*> targets;
