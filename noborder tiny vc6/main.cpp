/* noborder, the 'Borderless' mode switch, by Raymai97

Version 1.3.1 (2 May 2016) - bug fix

* FIXED: v1.3 wouldn't restore 'Always on Top' when 'DWM formula' was disabled
* OTHER: Always save config after changing options, and prompt if failed to save

*/

#include "noborder.h"

// For all cpp
HINSTANCE hInst;
NotifyIcon *notifyIcon;
bool canUseDWM;
bool excludeTaskbar;
AOT alwaysOnTopMode;
bool useDWM;

// Only this cpp
HANDLE hMutex;
HWND hWnd;
TCHAR cfgFilePath[MAX_PATH];

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow)
{
	// Init
	if (!InitWinAPIX())
	{
		MSGERR("FATAL: InitWinAPIX failed!");
	}
	hInst = hInstance;
	
	// Don't continue if noborder is already running
	hMutex = CreateMutex(NULL, true, NBD_MUTEX_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		bool gotIt = false;
		HWND hWnd = FindWindowEx(HWND_MESSAGE, NULL, NBD_DUMMY_MSG, NBD_DUMMY_MSG);
		if (hWnd) { gotIt = (SendMessage(hWnd, PREVINST_CALL, 0, 0) == PREVINST_CALL); }
		// If prev instance is old ver / Explorer not running...
		if (!gotIt) { MSGERR("noborder is already running!"); }
		return 0;
	}

	// Create message-only window
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.hInstance = hInst;
	wcex.lpszClassName = NBD_DUMMY_MSG;
	wcex.lpfnWndProc = WndProc;
	if (!RegisterClassEx(&wcex) ||
		!CreateWindow(NBD_DUMMY_MSG, NBD_DUMMY_MSG, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hInst, 0))
	{
		MSGERR("FATAL: MsgWindow creation failed!"); return 1;
	}

	// Init Notify Icon
	notifyIcon = new NotifyIcon(hInst, NBD_DUMMY_NI, NOTIFYICON_ID);
	if (notifyIcon->HwndInitFailed()) { MSGERR("FATAL: NotifyIcon creation failed!"); return 1; }
	notifyIcon->SetTip(NBD_TRAYICON_TIP);
	notifyIcon->SetIcon((HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_NOBORDER), IMAGE_ICON, 16, 16, LR_SHARED));
	notifyIcon->OnMenuItemSelected = MenuItemSelectedProc;
	notifyIcon->OnMenuCreating = MenuCreatingProc;

	// Install the low-level keyboard hooks
	HHOOK hhk = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
	if (!hhk) { MSGERR("FATAL: SetWindowsHookEx WH_KEYBOARD_LL failed!"); return 1; }

	// Find out cfgFilePath and load it
	GetModuleFileName(GetModuleHandle(NULL), cfgFilePath, MAX_PATH);
	for (size_t i = _tcslen(cfgFilePath); i--> 0;)
	{
		if (cfgFilePath[i] == '\\') { cfgFilePath[i + 1] = '\0'; break; }
	}
	_tcscat(cfgFilePath, NBD_CONFIG_FILENAME);
	LoadConfig();

	// Check OS & Init Core.cpp
	DWORD osver = GetVersion();
	BYTE major = LOBYTE(LOWORD(osver));
	BYTE minor = HIBYTE(LOWORD(osver));
	canUseDWM = (major >= 6);
	if (canUseDWM && !InitDwmAPI())
	{
		MSGERR("FATAL: InitDwmAPI failed!");
		PostQuitMessage(1);
	}
	CoreInit();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoreClosing();
	SaveConfig();
	UnhookWindowsHookEx(hhk);
	delete notifyIcon;
	ReleaseMutex(hMutex);
	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == PREVINST_CALL && notifyIcon->ShowBalloon(
		_T("You can find me here."), _T("noborder is already running!"), NIIF_INFO))
	{
		// tell 'new instance' that 'ShowBalloon' is OK
		return PREVINST_CALL; 
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
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
	return (fEatKeystroke ? TRUE : CallNextHookEx(NULL, nCode, wParam, lParam));
}


void MenuCreatingProc(UINT niId, HMENU hMenu)
{
	InsertMenu(hMenu, -1, MF_BYPOSITION | MF_GRAYED, 0, NBD_APP_TITLE);
	InsertMenu(hMenu, -1, MF_SEPARATOR, 0, NULL);
	HMENU hAOTMenu = CreatePopupMenu();
	if (hAOTMenu)
	{
		InsertMenu(hAOTMenu, -1, MF_BYPOSITION, SWM_AOT_AUTO, NBD_CMI_AOT_AUTO);
		InsertMenu(hAOTMenu, -1, MF_BYPOSITION, SWM_AOT_ALWAYS, NBD_CMI_AOT_ALWAYS);
		InsertMenu(hAOTMenu, -1, MF_BYPOSITION, SWM_AOT_NEVER, NBD_CMI_AOT_NEVER);
		InsertMenu(hMenu, -1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hAOTMenu, NBD_CMI_AOT);
	}
	InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXCLUDE_TASKBAR, NBD_CMI_EXCLUDE_TASKBAR);
	if (canUseDWM) { InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_USE_DWM, NBD_CMI_USE_DWM); }
	InsertMenu(hMenu, -1, MF_SEPARATOR, 0, NULL);
	InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_ABOUT, NBD_CMI_ABOUT);
	InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, NBD_CMI_EXIT);

	if (excludeTaskbar) { CheckMenuItem(hMenu, SWM_EXCLUDE_TASKBAR, MF_BYCOMMAND | MF_CHECKED); }
	switch (alwaysOnTopMode)
	{
	case AOT_AUTO:
		CheckMenuItem(hMenu, SWM_AOT_AUTO, MF_BYCOMMAND | MF_CHECKED);
		break;
	case AOT_ALWAYS:
		CheckMenuItem(hMenu, SWM_AOT_ALWAYS, MF_BYCOMMAND | MF_CHECKED);
		break;
	case AOT_NEVER:
		CheckMenuItem(hMenu, SWM_AOT_NEVER, MF_BYCOMMAND | MF_CHECKED);
		break;
	}
	if (useDWM) { CheckMenuItem(hMenu, SWM_USE_DWM, MF_BYCOMMAND | MF_CHECKED); }
}

void MenuItemSelectedProc(WORD id, WORD event)
{
	if (id == SWM_ABOUT)
	{
		MessageBox(NULL, NBD_APP_DESC, NBD_APP_TITLE, MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
	}
	else if (id == SWM_EXIT) { PostQuitMessage(0); }
	else
	{
		if (id == SWM_AOT_AUTO) { alwaysOnTopMode = AOT_AUTO; }
		else if (id == SWM_AOT_ALWAYS) { alwaysOnTopMode = AOT_ALWAYS; }
		else if (id == SWM_AOT_NEVER) { alwaysOnTopMode = AOT_NEVER; }
		else if (id == SWM_EXCLUDE_TASKBAR) { excludeTaskbar = !excludeTaskbar; }
		else if (id == SWM_USE_DWM) { useDWM = !useDWM; }
		if (!SaveConfig())
		{
			TCHAR msg[100] = _T("Failed to write data into ");
			_tcscat(msg, NBD_CONFIG_FILENAME);
			notifyIcon->ShowBalloon(msg, _T("Failed to save config!"), NIIF_ERROR);
		}
	}
}

bool LoadConfig()
{
	HANDLE hFile = CreateFile(cfgFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		DWORD read;
		ReadFile(hFile, &excludeTaskbar, 1, &read, NULL);
		ReadFile(hFile, &alwaysOnTopMode, 1, &read, NULL);
		ReadFile(hFile, &useDWM, 1, &read, NULL);
		CloseHandle(hFile);
		return (read > 0);
	}
	return false;
}

bool SaveConfig()
{
	HANDLE hFile = CreateFile(cfgFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		DWORD written;
		WriteFile(hFile, &excludeTaskbar, 1, &written, NULL);
		WriteFile(hFile, &alwaysOnTopMode, 1, &written, NULL);
		WriteFile(hFile, &useDWM, 1, &written, NULL);
		CloseHandle(hFile);
		return (written > 0);
	}
	return false;
}
