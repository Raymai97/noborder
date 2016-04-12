/* noborder
   Switch a running Win32 program to 'Borderless Fullscreen'.
   Press ALT+BACKSPACE to toggle.

   Author: Raymai97
     Date: 5/12/2015
*/

#include "noborder.h"

// For all cpp
HINSTANCE hInst;
HWND hWnd;
bool excludeTaskbar;
AOT alwaysOnTopMode;

// Only this cpp
HANDLE hMutex;
UINT msgTaskbarCreated;
TCHAR myExeDir[MAX_PATH];

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR lpCmdLine,
	_In_ int nCmdShow)
{
	// Init
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	hInst = hInstance;
	hWnd = CreateDummyWindow();
	if (!hWnd) return FALSE;
	
	// Don't continue if noborder is already running
	hMutex = CreateMutex(nullptr, true, NBD_MUTEX_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(nullptr, NBD_ERROR_ALREADY_RUNNING, nullptr, MB_OK | MB_ICONSTOP);
		return 0;
	}

	// Init Tray Icon
	AddNotifyIcon();
	msgTaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

	// Install the low-level keyboard & mouse hooks
	HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);

	// Find out myExeDir and Load Config
	GetModuleFileName(GetModuleHandle(NULL), myExeDir, MAX_PATH);
	for (size_t i = _tcslen(myExeDir); i--> 0;)
	{
		if (myExeDir[i] == '\\') { myExeDir[i + 1] = '\0'; break; }
	}
	LoadConfig();

	// Main message loop
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	SaveConfig();
	UnhookWindowsHookEx(hhkLowLevelKybd);
	RemoveNotifyIcon();
	ReleaseMutex(hMutex);

	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case SWM_TRAYMSG:
	{
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextMenu(hWnd);
			break;
		}
	}
	case WM_COMMAND:
	{
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case SWM_AOT_AUTO: alwaysOnTopMode = AOT_AUTO; break;
		case SWM_AOT_ALWAYS: alwaysOnTopMode = AOT_ALWAYS; break;
		case SWM_AOT_NEVER:	 alwaysOnTopMode = AOT_NEVER; break;
		case SWM_EXCLUDE_TASKBAR: excludeTaskbar = !excludeTaskbar; break;
		case SWM_ABOUT:
			MessageBox(hWnd, NBD_APP_DESC, NBD_APP_TITLE, MB_OK | MB_ICONINFORMATION);
			break;
		case SWM_EXIT:
			PostQuitMessage(0);
			break;
		}
	}
	default:
		if (message == msgTaskbarCreated) { AddNotifyIcon(); }
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	BOOL fEatKeystroke = FALSE;

	if (nCode == HC_ACTION)
	{
		switch (wParam)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
			fEatKeystroke = ( (p->vkCode == TOGGLE_KEY) & (GetAsyncKeyState(TOGGLE_MOD) < 0) );
			if (fEatKeystroke) ToggleNoborder();
			break;
		}
	}

	return (fEatKeystroke ? TRUE : CallNextHookEx(nullptr, nCode, wParam, lParam));
}

void LoadConfig()
{
	TCHAR filePath[MAX_PATH];
	_tcscpy(filePath, myExeDir);
	_tcscat(filePath, NBD_CONFIG_FILENAME);
	HANDLE hFile = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		DWORD byteCount;
		ReadFile(hFile, &excludeTaskbar, 1, &byteCount, NULL);
		ReadFile(hFile, &alwaysOnTopMode, 1, &byteCount, NULL);
		CloseHandle(hFile);
	}
}

void SaveConfig()
{
	TCHAR filePath[MAX_PATH];
	_tcscpy(filePath, myExeDir);
	_tcscat(filePath, NBD_CONFIG_FILENAME);
	HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		DWORD byteCount;
		WriteFile(hFile, &excludeTaskbar, 1, &byteCount, NULL);
		WriteFile(hFile, &alwaysOnTopMode, 1, &byteCount, NULL);
		CloseHandle(hFile);
	}
}

