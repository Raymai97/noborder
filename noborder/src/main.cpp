#include "noborder.h"

// For all cpp
HINSTANCE hInst;
NotifyIcon *notifyIcon;
bool canUseDWM;
bool excludeTaskbar;
AOT alwaysOnTopMode;
bool useDWM;
FARPROC x_lpfnPhyToLogPtForPerMonitorDPI;

// Only this cpp
static TCHAR cfgFilePath[MAX_PATH];
static bool useAltBksp = true;
static bool useWinBksp;

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow)
{
	HANDLE hMutex = 0;
	// Init
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	hInst = hInstance;
	
	// Don't continue if noborder is already running
	hMutex = CreateMutex(nullptr, true, NBD_MUTEX_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		bool gotIt = false;
		HWND hWnd = FindWindowEx(HWND_MESSAGE, nullptr, NBD_DUMMY_MSG, NBD_DUMMY_MSG);
		if (hWnd) { gotIt = (SendMessage(hWnd, PREVINST_CALL, 0, 0) == PREVINST_CALL); }
		// If prev instance is old ver / Explorer not running...
		if (!gotIt) { MSGERR("noborder is already running!"); }
		return 0;
	}
	if (!hMutex)
	{
		MSGERR("CreateMutex failed.");
		return 99;
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
	HHOOK hhk = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
	if (!hhk) { MSGERR("FATAL: SetWindowsHookEx WH_KEYBOARD_LL failed!"); return 1; }

	// Find out cfgFilePath and load it
	GetModuleFileName(GetModuleHandle(nullptr), cfgFilePath, MAX_PATH);
	for (size_t i = _tcslen(cfgFilePath); i--> 0;)
	{
		if (cfgFilePath[i] == '\\') { cfgFilePath[i + 1] = '\0'; break; }
	}
	_tcscat(cfgFilePath, NBD_CONFIG_FILENAME);
	LoadConfig();

	// Check OS & Init Core.cpp
	HMODULE hMod_user32 = GetModuleHandle(_T("user32"));
	if (hMod_user32)
	{
		x_lpfnPhyToLogPtForPerMonitorDPI = GetProcAddress(hMod_user32,
			"PhysicalToLogicalPointForPerMonitorDPI");
	}
	x_compat_dwmapi_hMod = LoadLibrary(TEXT("dwmapi"));
	canUseDWM = x_compat_dwmapi_hMod != 0;
	CoreInit();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
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
			if (p->vkCode == TOGGLE_KEY)
			{
				if (useAltBksp)
				{
					fEatKeystroke |= GetAsyncKeyState(VK_MENU) < 0;
				}
				if (useWinBksp)
				{
					fEatKeystroke |= GetAsyncKeyState(VK_LWIN) < 0;
					fEatKeystroke |= GetAsyncKeyState(VK_RWIN) < 0;
				}
			}
			if (fEatKeystroke) ToggleNoborder();
			break;
		}
	}
	return (fEatKeystroke ? TRUE : CallNextHookEx(nullptr, nCode, wParam, lParam));
}


void MenuCreatingProc(UINT niId, HMENU hMenu)
{
	UNREFERENCED_PARAMETER(niId);
	InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION | MF_GRAYED, 0, NBD_APP_TITLE);
	InsertMenu(hMenu, (UINT)-1, MF_SEPARATOR, 0, nullptr);
	HMENU hAOTMenu = CreatePopupMenu();
	if (hAOTMenu)
	{
		InsertMenu(hAOTMenu, (UINT)-1, MF_BYPOSITION, SWM_AOT_AUTO, NBD_CMI_AOT_AUTO);
		InsertMenu(hAOTMenu, (UINT)-1, MF_BYPOSITION, SWM_AOT_ALWAYS, NBD_CMI_AOT_ALWAYS);
		InsertMenu(hAOTMenu, (UINT)-1, MF_BYPOSITION, SWM_AOT_NEVER, NBD_CMI_AOT_NEVER);
		InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hAOTMenu, NBD_CMI_AOT);
	}
	InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION, SWM_EXCLUDE_TASKBAR, NBD_CMI_EXCLUDE_TASKBAR);
	if (canUseDWM) { InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION, SWM_USE_DWM, NBD_CMI_USE_DWM); }
	InsertMenu(hMenu, (UINT)-1, MF_SEPARATOR, 0, nullptr);
	HMENU hHotkeyMenu = CreatePopupMenu();
	if (hHotkeyMenu)
	{
		InsertMenu(hHotkeyMenu, (UINT)-1, MF_BYPOSITION, SWM_USE_ALT_BKSP, NBD_CMI_USE_ALT_BKSP);
		InsertMenu(hHotkeyMenu, (UINT)-1, MF_BYPOSITION, SWM_USE_WIN_BKSP, NBD_CMI_USE_WIN_BKSP);
		InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hHotkeyMenu, NBD_CMI_HOTKEY);
	}
	InsertMenu(hMenu, (UINT)-1, MF_SEPARATOR, 0, nullptr);
	InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION, SWM_ABOUT, NBD_CMI_ABOUT);
	InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION, SWM_EXIT, NBD_CMI_EXIT);

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
	if (useAltBksp) { CheckMenuItem(hMenu, SWM_USE_ALT_BKSP, MF_BYCOMMAND | MF_CHECKED); }
	if (useWinBksp) { CheckMenuItem(hMenu, SWM_USE_WIN_BKSP, MF_BYCOMMAND | MF_CHECKED); }
}

void MenuItemSelectedProc(WORD id, WORD event)
{
	UNREFERENCED_PARAMETER(event);
	if (id == SWM_ABOUT)
	{
		MessageBox(nullptr, NBD_APP_DESC, NBD_APP_TITLE, MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
	}
	else if (id == SWM_EXIT) { PostQuitMessage(0); }
	else
	{
		if (id == SWM_AOT_AUTO) { alwaysOnTopMode = AOT_AUTO; }
		else if (id == SWM_AOT_ALWAYS) { alwaysOnTopMode = AOT_ALWAYS; }
		else if (id == SWM_AOT_NEVER) { alwaysOnTopMode = AOT_NEVER; }
		else if (id == SWM_EXCLUDE_TASKBAR) { excludeTaskbar = !excludeTaskbar; }
		else if (id == SWM_USE_DWM) { useDWM = !useDWM; }
		else if (id == SWM_USE_ALT_BKSP) { useAltBksp = !useAltBksp; }
		else if (id == SWM_USE_WIN_BKSP) { useWinBksp = !useWinBksp; }
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
	HANDLE hFile = CreateFile(cfgFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile)
	{
		BYTE buf[8] = { 0 };
		DWORD read;
		bool ok = ReadFile(hFile, buf, sizeof(buf), &read, nullptr) && (read > 3);
		if (ok)
		{
			excludeTaskbar = buf[0] != 0;
			switch (buf[1])
			{
			case AOT_ALWAYS: alwaysOnTopMode = AOT_ALWAYS; break;
			case AOT_NEVER: alwaysOnTopMode = AOT_NEVER; break;
			default: alwaysOnTopMode = AOT_AUTO; break;
			}
			useDWM = buf[2] != 0;
			if (read >= 5) // new cfg since v1.4
			{
				useAltBksp = buf[3] != 0;
				useWinBksp = buf[4] != 0;
			}
		}
		CloseHandle(hFile);
		return ok;
	}
	return false;
}

bool SaveConfig()
{
	HANDLE hFile = CreateFile(cfgFilePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile)
	{
		BYTE buf[8] = { 0 };
		DWORD written;
		buf[0] = excludeTaskbar;
		buf[1] = (BYTE)alwaysOnTopMode;
		buf[2] = useDWM;
		buf[3] = useAltBksp;
		buf[4] = useWinBksp;
		bool ok = WriteFile(hFile, buf, 5, &written, nullptr) && (written == 5);
		CloseHandle(hFile);
		return ok;
	}
	return false;
}

EXTERN_C HRESULT Compat_PhyToLogPtForPerMonitorDPI(HWND hWnd, LPPOINT lpPoint)
{
	typedef HRESULT(WINAPI *fn_t)(HWND, LPPOINT);
	fn_t fn = (fn_t)x_lpfnPhyToLogPtForPerMonitorDPI;
	return fn ? fn(hWnd, lpPoint) : E_NOTIMPL;
}
