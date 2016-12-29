#include "Program.h"

using Noborder::Target;
typedef Target::AlwaysOnTopMode AotMode;

Target target;
NotifyIcon * nbdNotifyIcon = nullptr;

auto nbdCanUseDwm = true;
auto nbdAotMode = AotMode::Auto;
auto nbdExcludeTaskbar = false;
auto nbdUseDwmFormula = false;
auto nbdUseAltBack = true;
auto nbdUseWinBack = false;
TCHAR nbdCfgFilePath[MAX_PATH] = { 0 };

inline void MsgErr(LPCWSTR szMsg) {
	MessageBox(nullptr, szMsg, L"noborder Error!",
		MB_OK | MB_ICONSTOP | MB_TOPMOST);
}

inline void MsgErrWithSite(LPCWSTR szMsg, LPCSTR szSite) {
	int cchWideLog = MultiByteToWideChar(CP_UTF8, 0, szSite, -1, nullptr, 0);
	auto wszLog = new WCHAR[cchWideLog];
	MultiByteToWideChar(CP_UTF8, 0, szSite, -1, wszLog, cchWideLog);
	std::wstringstream ss;
	ss << szMsg << std::endl << std::endl << szSite;
	MsgErr(ss.str().c_str());
}

int APIENTRY _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
	// Don't continue if prev instance exists
	auto mutexHandle = CreateMutexW(nullptr, true, nbdMutexName);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		bool gotIt =
			nbdPrevInstCall == SendMessageW(
				FindWindowW(nbdMsgWindowClass, nbdMsgWindowClass),
				nbdPrevInstCall, 0, 0);
		// If prev instance is old ver / Explorer not running...
		if (!gotIt) { MsgErr(L"noborder is already running!"); }
		return 0;
	}

	// Create a message-only window
	WNDCLASSEXW wcexMsg = { 0 };
	wcexMsg.cbSize = sizeof(wcexMsg);
	wcexMsg.lpfnWndProc = WndProc;
	wcexMsg.lpszClassName = nbdMsgWindowClass;
	if (!RegisterClassExW(&wcexMsg)) {
		MsgErr(L"FATAL : Failed to register class of MsgWindow.");
		return nbdFatalExitCode;
	}
	HWND hwndMsg = CreateWindowExW(0, nbdMsgWindowClass, nbdMsgWindowClass,
		0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
	if (!hwndMsg) {
		MsgErr(L"FATAL : Failed to create MsgWindow.");
		return nbdFatalExitCode;
	}

	// Determine szCfgFilePath
	{
		TCHAR szMyExePath[MAX_PATH] = { 0 };
		GetModuleFileNameW(GetModuleHandleW(nullptr), szMyExePath, MAX_PATH);
		auto p = _tcsrchr(szMyExePath, '\\');
		if (!p) {
			MsgErr(L"FATAL : Unexpected result from GetModuleFileNameW.");
			return nbdFatalExitCode;
		}
		*p = '\0';
		_stprintf_s(nbdCfgFilePath, L"%s\\%s", szMyExePath, nbdCfgFileName);
	}

	// TODO: Load config

	// Install the low-level keyboard hook
	HHOOK hhk = SetWindowsHookExW(WH_KEYBOARD_LL, LLKeybrdProc, nullptr, 0);
	if (!hhk) {
		MsgErr(L"FATAL : Failed to set low-level keyboard hook.");
		return nbdFatalExitCode;
	}

	// Init Notify Icon
	try {
		(*(nbdNotifyIcon = new NotifyIcon(NBD_NOTIFYICON, nbdNotifyIconClass)))
			.SetTip(nbdAppTitle)
			.SetEventHandler(OnNotifyIconEvent)
			.SetVisible(true);
	}
	catch (std::exception const & ex) {
		MsgErrWithSite(L"FATAL : Failed to init Notify Icon.", ex.what());
		return nbdFatalExitCode;
	}

	MSG msg = { 0 };
	while (GetMessageW(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	delete nbdNotifyIcon;
	UnhookWindowsHookEx(hhk);
	// TODO: Save config
	ReleaseMutex(mutexHandle);
	return static_cast<int>(msg.wParam);
}

void ToggleNoborder() {
	try {
		HWND hwndOld = target.GetHwnd();
		HWND hwndCurr = GetForegroundWindow();
		if (target.IsNobordered()) { target.Unset(); }
		if (hwndCurr != hwndOld) {
			target.Set(hwndCurr,
				nbdAotMode,
				nbdExcludeTaskbar,
				nbdUseDwmFormula
			);
		}
	}
	catch (std::exception const & ex) {
		MsgErrWithSite(
			L"FATAL : Unexpected error at ToggleNoborder().", ex.what());
		PostQuitMessage(nbdFatalExitCode);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	if (msg == nbdPrevInstCall) {
		try {
			nbdNotifyIcon->ShowBalloon(
				L"You can find me here.",
				L"noborder is already running!",
				NotifyIcon::BalloonIcon::Info);
			// tell 'new instance' that 'ShowBalloon' is OK
			return nbdPrevInstCall;
		}
		catch (std::exception const & ex) {
			MsgErrWithSite(
				L"Failed to show 'already running' balloon.",
				ex.what());
		}
	}
	return DefWindowProc(hwnd, msg, w, l);
}

LRESULT CALLBACK LLKeybrdProc(int code, WPARAM w, LPARAM l) {
	bool handled = false;
	if (code == HC_ACTION && (w == WM_KEYDOWN || w == WM_SYSKEYDOWN)) {
		auto p = reinterpret_cast<KBDLLHOOKSTRUCT*>(l);
		auto holdingAlt = GetAsyncKeyState(VK_MENU) < 0;
		auto holdingWin = GetAsyncKeyState(VK_LWIN) < 0 ||
			GetAsyncKeyState(VK_RWIN) < 0;
		auto holdingAltBack = holdingAlt && p->vkCode == VK_BACK;
		auto holdingWinBack = holdingWin && p->vkCode == VK_BACK;

		if (nbdUseAltBack && holdingAltBack ||
			nbdUseWinBack && holdingWinBack) {
			ToggleNoborder();
			handled = true;
		}
	}
	return handled ? TRUE : CallNextHookEx(nullptr, code, w, l);
}

void OnNotifyIconEvent(NotifyIcon & self, UINT msg) {
	if (msg == WM_RBUTTONUP) {
		HMENU hmenu = CreateNbdPopupMenu();
		POINT pt = { 0 };
		GetCursorPos(&pt);
		// Make sure menu close when lose focus
		SetForegroundWindow(self.GetHwnd());
		int id = TrackPopupMenu(hmenu,
			TPM_RIGHTBUTTON | TPM_RETURNCMD,
			pt.x, pt.y, 0, self.GetHwnd(), nullptr);
		DestroyMenu(hmenu);
		OnNbdPopupMenuItemClick(id);
	}
}

void OnNbdPopupMenuItemClick(DWORD id) {
	if (id == CMI_EXIT) { PostQuitMessage(0); }
	else if (id == CMI_ABOUT) {
		MessageBoxW(nullptr, nbdAppInfo, nbdAppTitle, MB_ICONINFORMATION);
	}
	else if (id == CMI_AOT_AUTO) { nbdAotMode = AotMode::Auto; }
	else if (id == CMI_AOT_ALWAYS) { nbdAotMode = AotMode::Always; }
	else if (id == CMI_AOT_NEVER) { nbdAotMode = AotMode::Never; }
	else if (id == CMI_EXCL_TASKBAR) { nbdExcludeTaskbar = !nbdExcludeTaskbar; }
	else if (id == CMI_USE_DWM) { nbdUseDwmFormula = !nbdUseDwmFormula; }
	else if (id == CMI_HOTKEY_ALT_BACK) {
		if (nbdUseWinBack) { nbdUseAltBack = !nbdUseAltBack; }
	}
	else if (id == CMI_HOTKEY_WIN_BACK) {
		if (nbdUseAltBack) { nbdUseWinBack = !nbdUseWinBack; }
	}
}

HMENU CreateNbdPopupMenu() {
	auto MifOf = [](DWORD id) -> DWORD {
		return
			id == CMI_AOT_ALWAYS
			&& (nbdAotMode == AotMode::Always) ||
			id == CMI_AOT_AUTO
			&& (nbdAotMode == AotMode::Auto) ||
			id == CMI_AOT_NEVER
			&& (nbdAotMode == AotMode::Never) ? MFS_CHECKED :

			id == CMI_EXCL_TASKBAR
			&& nbdExcludeTaskbar ||
			id == CMI_USE_DWM
			&& nbdUseDwmFormula ? MFS_CHECKED :

			id == CMI_HOTKEY_ALT_BACK
			&& nbdUseAltBack ||
			id == CMI_HOTKEY_WIN_BACK
			&& nbdUseWinBack ? MFS_CHECKED : MFS_DEFAULT;
	};
	HMENU hmenu = CreatePopupMenu();
	AppendMenuW(hmenu, MFS_DISABLED, 0, nbdAppTitle);
	AppendMenuW(hmenu, MFT_SEPARATOR, 0, nullptr);
	{
		HMENU hmenuAot = CreatePopupMenu();
		AppendMenuW(hmenuAot, MifOf(CMI_AOT_AUTO), CMI_AOT_AUTO, L"Auto");
		AppendMenuW(hmenuAot, MifOf(CMI_AOT_ALWAYS), CMI_AOT_ALWAYS, L"Always");
		AppendMenuW(hmenuAot, MifOf(CMI_AOT_NEVER), CMI_AOT_NEVER, L"Never");
		AppendMenuW(hmenu, MF_POPUP,
			reinterpret_cast<UINT_PTR>(hmenuAot), L"Always on Top");
	}
	AppendMenuW(hmenu, MifOf(CMI_EXCL_TASKBAR), CMI_EXCL_TASKBAR, L"Exclude Taskbar");
	if (nbdCanUseDwm) {
		AppendMenuW(hmenu, MifOf(CMI_USE_DWM), CMI_USE_DWM, L"Use DWM formula");
	}
	AppendMenuW(hmenu, MFT_SEPARATOR, 0, nullptr);
	{
		HMENU hmenuHotkey = CreatePopupMenu();
		AppendMenuW(hmenuHotkey, MifOf(CMI_HOTKEY_ALT_BACK), CMI_HOTKEY_ALT_BACK,
			L"Alt + Backspace");
		AppendMenuW(hmenuHotkey, MifOf(CMI_HOTKEY_WIN_BACK), CMI_HOTKEY_WIN_BACK,
			L"Win + Backspace");
		AppendMenuW(hmenu, MF_POPUP,
			reinterpret_cast<UINT_PTR>(hmenuHotkey), L"Hotkey");
	}
	AppendMenuW(hmenu, MFT_SEPARATOR, 0, nullptr);
	AppendMenuW(hmenu, MifOf(CMI_ABOUT), CMI_ABOUT, L"About");
	AppendMenuW(hmenu, MifOf(CMI_EXIT), CMI_EXIT, L"Exit");
	return hmenu;
}
