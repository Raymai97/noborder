#include "noborder.h"

// For all cpp
HINSTANCE x_hInst;
NotifyIcon *x_pNotifyIcon;
bool x_canUseDwm;
Cfg x_cfg;
FARPROC x_lpfnPhyToLogPtForPerMonitorDPI;
FARPROC x_lpfnChangeWindowMessageFilter;

// Only this cpp
static TCHAR cfgFilePath[MAX_PATH];
static NbdCore nbdCore;

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	// Init
	x_hInst = hInstance;
	x_cfg.wantUseAltBksp = true;
	HMODULE hMod_user32 = GetModuleHandle(_T("user32"));
	if (hMod_user32)
	{
		x_lpfnPhyToLogPtForPerMonitorDPI = GetProcAddress(hMod_user32,
			"PhysicalToLogicalPointForPerMonitorDPI");
		x_lpfnChangeWindowMessageFilter = GetProcAddress(hMod_user32,
			"ChangeWindowMessageFilter");
	}
	x_compat_dwmapi_hMod = LoadLibrary(TEXT("dwmapi"));

	// Prevent UIPI from intercepting message we send to prev instance.
	Compat_ChangeWindowMessageFilter(PREVINST_CALL, 1); // MSGFLT_ADD

	// Don't continue if noborder is already running
	if (HasExistingInstance())
	{
		return 0;
	}

	// Create message-only window
	if (!CreateNbdMsgWindow())
	{
		return 1;
	}

	// Init Notify Icon
	x_pNotifyIcon = new NotifyIcon(x_hInst, WCN_NotifyIconWindow, NOTIFYICON_ID);
	if (x_pNotifyIcon->HwndInitFailed()) { MSGERR("FATAL: NotifyIcon creation failed!"); return 1; }
	x_pNotifyIcon->SetTip(NBD_TRAYICON_TIP);
	x_pNotifyIcon->SetIcon((HICON)LoadImage(x_hInst, MAKEINTRESOURCE(IDI_NOBORDER), IMAGE_ICON, 16, 16, LR_SHARED));
	x_pNotifyIcon->OnMenuItemSelected = MenuItemSelectedProc;
	x_pNotifyIcon->OnMenuCreating = MenuCreatingProc;

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
	x_canUseDwm = x_compat_dwmapi_hMod != 0;
	nbdCore.Init();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	nbdCore.Uninit();
	SaveConfig();
	UnhookWindowsHookEx(hhk);
	delete x_pNotifyIcon;
	return (int)msg.wParam;
}

bool HasExistingInstance(void)
{
	// No need to keep track of the returned Mutex in this case as
	// Windows will free the Mutex automatically when process exit.
	CreateMutex(nullptr, true, NBD_MUTEX_NAME);
	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		return false;
	}
	HWND hWnd = FindWindowEx(HWND_MESSAGE, 0, WCN_NbdMsgWindow, WCN_NbdMsgWindow);
	LRESULT lRes = 0;
	if (hWnd)
	{
		SendMessageTimeout(hWnd, PREVINST_CALL, 0, 0,
			SMTO_BLOCK, 2000, (PDWORD_PTR)&lRes);
	}
	// If prev instance is old ver / Explorer not running...
	if (lRes != PREVINST_CALL)
	{
		MSGERR("noborder is already running!");
	}
	return true;
}

bool CreateNbdMsgWindow(void)
{
	bool ok = false;
	WNDCLASS wc = { 0 };
	wc.hInstance = x_hInst;
	wc.lpszClassName = WCN_NbdMsgWindow;
	wc.lpfnWndProc = NbdMsgWndProc;
	ok = !!RegisterClass(&wc);
	if (ok)
	{
		ok = !!CreateWindow(WCN_NbdMsgWindow, WCN_NbdMsgWindow, 0,
			0, 0, 0, 0, HWND_MESSAGE, 0, x_hInst, 0);
	}
	if (!ok)
	{
		MSGERR("FATAL: MsgWindow creation failed!");
	}
	return ok;
}

LRESULT CALLBACK NbdMsgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == PREVINST_CALL)
	{
		BOOL ok = x_pNotifyIcon->ShowBalloon(
			_T("It is in the Taskbar Notification Area."),
			_T("noborder is already running!"), NIIF_INFO);
		return ok ? PREVINST_CALL : 0;
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
			if (p->vkCode == VK_BACK)
			{
				if (x_cfg.wantUseAltBksp)
				{
					fEatKeystroke |= GetAsyncKeyState(VK_MENU) < 0;
				}
				if (x_cfg.wantUseWinBksp)
				{
					fEatKeystroke |= GetAsyncKeyState(VK_LWIN) < 0;
					fEatKeystroke |= GetAsyncKeyState(VK_RWIN) < 0;
				}
			}
			if (fEatKeystroke) nbdCore.ToggleNoborder();
			break;
		}
	}
	return (fEatKeystroke ? TRUE : CallNextHookEx(nullptr, nCode, wParam, lParam));
}


void MenuCreatingProc(HMENU hMenu)
{
	InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION | MF_GRAYED, 0, NBD_APP_TITLE);
	InsertMenu(hMenu, (UINT)-1, MF_SEPARATOR, 0, nullptr);
	HMENU hAOTMenu = CreatePopupMenu();
	if (hAOTMenu)
	{
		InsertMenu(hAOTMenu, (UINT)-1, MF_BYPOSITION, IDM_OnTopMode_Auto, NBD_CMI_AOT_AUTO);
		InsertMenu(hAOTMenu, (UINT)-1, MF_BYPOSITION, IDM_OnTopMode_Always, NBD_CMI_AOT_ALWAYS);
		InsertMenu(hAOTMenu, (UINT)-1, MF_BYPOSITION, IDM_OnTopMode_Never, NBD_CMI_AOT_NEVER);
		InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hAOTMenu, NBD_CMI_AOT);
	}
	InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION, IDM_ExcludeTaskbar, NBD_CMI_EXCLUDE_TASKBAR);
	if (x_canUseDwm) { InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION, IDM_UseDwmFormula, NBD_CMI_USE_DWM); }
	InsertMenu(hMenu, (UINT)-1, MF_SEPARATOR, 0, nullptr);
	HMENU hHotkeyMenu = CreatePopupMenu();
	if (hHotkeyMenu)
	{
		InsertMenu(hHotkeyMenu, (UINT)-1, MF_BYPOSITION, IDM_UseAltBksp, NBD_CMI_USE_ALT_BKSP);
		InsertMenu(hHotkeyMenu, (UINT)-1, MF_BYPOSITION, IDM_UseWinBksp, NBD_CMI_USE_WIN_BKSP);
		InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hHotkeyMenu, NBD_CMI_HOTKEY);
	}
	InsertMenu(hMenu, (UINT)-1, MF_SEPARATOR, 0, nullptr);
	InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION, IDM_About, NBD_CMI_ABOUT);
	InsertMenu(hMenu, (UINT)-1, MF_BYPOSITION, IDM_Exit, NBD_CMI_EXIT);

	if (x_cfg.wantExcludeTaskbar)
	{
		CheckMenuItem(hMenu, IDM_ExcludeTaskbar, MF_BYCOMMAND | MF_CHECKED);
	}
	switch (x_cfg.onTopMode)
	{
	case OnTopMode_Auto:
		CheckMenuItem(hMenu, IDM_OnTopMode_Auto, MF_BYCOMMAND | MF_CHECKED);
		break;
	case OnTopMode_Always:
		CheckMenuItem(hMenu, IDM_OnTopMode_Always, MF_BYCOMMAND | MF_CHECKED);
		break;
	case OnTopMode_Never:
		CheckMenuItem(hMenu, IDM_OnTopMode_Never, MF_BYCOMMAND | MF_CHECKED);
		break;
	}
	if (x_cfg.wantUseDwmFormula)
	{
		CheckMenuItem(hMenu, IDM_UseDwmFormula, MF_BYCOMMAND | MF_CHECKED);
	}
	if (x_cfg.wantUseAltBksp)
	{
		CheckMenuItem(hMenu, IDM_UseAltBksp, MF_BYCOMMAND | MF_CHECKED);
	}
	if (x_cfg.wantUseWinBksp)
	{
		CheckMenuItem(hMenu, IDM_UseWinBksp, MF_BYCOMMAND | MF_CHECKED);
	}
}

void MenuItemSelectedProc(WORD id)
{
	if (id == IDM_About)
	{
		MessageBox(nullptr, NBD_APP_DESC, NBD_APP_TITLE, MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
	}
	else if (id == IDM_Exit) { PostQuitMessage(0); }
	else
	{
		if (id == IDM_OnTopMode_Auto) { x_cfg.onTopMode = OnTopMode_Auto; }
		else if (id == IDM_OnTopMode_Always) { x_cfg.onTopMode = OnTopMode_Always; }
		else if (id == IDM_OnTopMode_Never) { x_cfg.onTopMode = OnTopMode_Never; }
		else if (id == IDM_ExcludeTaskbar) { flip(x_cfg.wantExcludeTaskbar); }
		else if (id == IDM_UseDwmFormula) { flip(x_cfg.wantUseDwmFormula); }
		else if (id == IDM_UseAltBksp) { flip(x_cfg.wantUseAltBksp); }
		else if (id == IDM_UseWinBksp) { flip(x_cfg.wantUseWinBksp); }
		if (!SaveConfig())
		{
			TCHAR msg[100] = _T("Failed to write data into ");
			_tcscat(msg, NBD_CONFIG_FILENAME);
			x_pNotifyIcon->ShowBalloon(msg, _T("Failed to save config!"), NIIF_ERROR);
		}
	}
}

bool LoadConfig()
{
	HANDLE hFile = CreateFile(cfgFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile)
	{
		BYTE buf[32] = { 0 };
		DWORD read;
		bool ok = ReadFile(hFile, buf, sizeof(buf), &read, nullptr) && (read > 3);
		if (ok)
		{
			x_cfg.wantExcludeTaskbar = buf[0] != 0;
			switch (buf[1])
			{
			case OnTopMode_Always: x_cfg.onTopMode = OnTopMode_Always; break;
			case OnTopMode_Never: x_cfg.onTopMode = OnTopMode_Never; break;
			default: x_cfg.onTopMode = OnTopMode_Auto; break;
			}
			x_cfg.wantUseDwmFormula = buf[2] != 0;
			if (read >= 5) // new cfg since v1.4
			{
				x_cfg.wantUseAltBksp = buf[3] != 0;
				x_cfg.wantUseWinBksp = buf[4] != 0;
			}
			if (read >= 9) // new cfg since v1.6
			{
				x_cfg.letterboxColor.a = buf[5];
				x_cfg.letterboxColor.r = buf[6];
				x_cfg.letterboxColor.g = buf[7];
				x_cfg.letterboxColor.b = buf[8];
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
		BYTE buf[32] = { 0 };
		DWORD written;
		buf[0] = x_cfg.wantExcludeTaskbar;
		buf[1] = (BYTE)(x_cfg.onTopMode);
		buf[2] = x_cfg.wantUseDwmFormula;
		buf[3] = x_cfg.wantUseAltBksp;
		buf[4] = x_cfg.wantUseWinBksp;
		buf[5] = x_cfg.letterboxColor.a;
		buf[6] = x_cfg.letterboxColor.r;
		buf[7] = x_cfg.letterboxColor.g;
		buf[8] = x_cfg.letterboxColor.b;
		bool ok = WriteFile(hFile, buf, 9, &written, nullptr) && (written == 9);
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

EXTERN_C BOOL Compat_ChangeWindowMessageFilter(UINT message, DWORD dwFlag)
{
	typedef BOOL(WINAPI *fn_t)(UINT, DWORD);
	fn_t fn = (fn_t)x_lpfnChangeWindowMessageFilter;
	return fn ? fn(message, dwFlag) : FALSE;
}
