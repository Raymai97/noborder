#include "Program.h"

using Noborder::Target;
typedef Target::AlwaysOnTopMode		AotMode;
using DwmFormula::DwmWindow;
typedef DwmWindow::Error	DwmWndError;

Target target;
DwmWindow dwmWindow(nbdDwmWindowClass);
NotifyIcon nbdNotifyIcon(NBD_NOTIFYICON, nbdNotifyIconClass);

auto nbdAotMode = AotMode::Auto;
auto nbdExcludeTaskbar = false;
auto nbdUseDwm = false;
auto nbdUseAltBack = true;
auto nbdUseWinBack = false;
wchar_t nbdCfgFilePath[MAX_PATH] = { 0 };

inline std::wstring WideFromUtf8(std::string const & utf8) {
	int cchWide = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
	LPWSTR szWide = new WCHAR[cchWide];
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, szWide, cchWide);
	auto wide = std::wstring(szWide);
	delete[] szWide;
	return wide;
}

inline void MsgErr(LPCWSTR szMsg, LPCWSTR szTitle = L"noborder Error!") {
	MessageBoxW(nullptr, szMsg, szTitle, MB_OK | MB_ICONSTOP | MB_TOPMOST);
}

inline void MsgErr2(LPCWSTR szMsg, LPCSTR szSite) {
	std::wstringstream ss;
	ss << szMsg << std::endl << WideFromUtf8(szSite);
	MsgErr(ss.str().c_str());
}

inline void BalloonOrMsg(
	std::wstring const & msg,
	std::wstring const & title,
	NotifyIcon::BalloonIcon const & icon)
{
	try {
		nbdNotifyIcon.ShowBalloon(msg, title, icon);
	}
	catch (std::runtime_error const &) {
		typedef NotifyIcon::BalloonIcon Icon;
		auto uIcon =
			icon == Icon::Info ? MB_ICONINFORMATION :
			icon == Icon::Warning ? MB_ICONWARNING :
			icon == Icon::Error ? MB_ICONERROR : 0;
		MessageBoxW(nullptr, msg.c_str(), title.c_str(), uIcon);
	}
}

#ifdef _DEBUG
int main() {
	puts("--- Noborder Debug ---");
	return WinMain(GetModuleHandleW(nullptr), 0, 0, 0);
}
#endif

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
	// Don't continue if prev instance exists
	// PS: Mutex releases automatically when process terminates
	CreateMutexW(nullptr, TRUE, nbdMutexName);
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
		wchar_t szMyExePath[MAX_PATH] = { 0 };
		GetModuleFileNameW(hInst, szMyExePath, MAX_PATH);
		auto p = wcsrchr(szMyExePath, '\\');
		if (p) {
			*p = '\0';
			swprintf_s(nbdCfgFilePath, L"%s\\%s", szMyExePath, nbdCfgFileName);
		}
		else {
			MsgErr(L"FATAL : Unexpected result from GetModuleFileNameW.");
			return nbdFatalExitCode;
		}
	}

	// Install the low-level keyboard hook
	HHOOK hhk = SetWindowsHookExW(WH_KEYBOARD_LL, LLKeybrdProc, hInst, 0);
	if (!hhk) {
		MsgErr(L"FATAL : Failed to set low-level keyboard hook.");
		return nbdFatalExitCode;
	}

	// Init Notify Icon
	try {
		auto MyIcon = [hInst]() -> HICON {
			auto himg = LoadImageW(hInst,
				MAKEINTRESOURCEW(IDI_NOBORDER),
				IMAGE_ICON, 16, 16, LR_SHARED);
			return static_cast<HICON>(himg);
		};
		nbdNotifyIcon.Init()
			.SetTip(nbdAppTitle)
			.SetIcon(MyIcon())
			.SetEventHandler(OnNotifyIconEvent)
			.SetVisible(true);
	}
	catch (std::runtime_error const & ex) {
		MsgErr2(L"FATAL : Failed to init Notify Icon.", ex.what());
		PostQuitMessage(nbdFatalExitCode);
	}

	LoadConfig();
	if (!DwmFormula::IsDwmSupported()) { nbdUseDwm = false; }
	MSG msg = { 0 };
	while (GetMessageW(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	SaveConfig();
	UnhookWindowsHookEx(hhk);
	return static_cast<int>(msg.wParam);
}

void ToggleNoborder() {
	// Balloon stays for a few secs. If user clicks on Notify Icon,
	// and toggle noborder soon enough, the balloon with old info
	// will still be there, which seems awkward, so...
	try { nbdNotifyIcon.HideBalloon(); }
	catch (...) {}
	// If user noborders a window while another window is nobordered,
	// it will restore the old one and 'noborder' the new one.
	try {
		auto shouldSet = true;
		HWND hwndCurr = GetForegroundWindow();
		if (target.IsNobordered()) {
			HWND hwndOld = target.GetHwnd();
			target.Unset();
			if (hwndOld == hwndCurr) {
				shouldSet = false;
			}
		}
		if (shouldSet) {
			target.Set(hwndCurr,
				nbdAotMode,
				nbdExcludeTaskbar,
				nbdUseDwm ? &dwmWindow : nullptr);
		}	
	}
	catch (DwmWndError const & dwmErr) {
		if (dwmErr == DwmWndError::DwmNotSupported) {
			BalloonOrMsg(
				L"DWM is not supported on OS prior to Windows Vista.",
				L"DWM is not supported!",
				NotifyIcon::BalloonIcon::Error);
		}
		else if (dwmErr == DwmWndError::DwmNotEnabled) {
			BalloonOrMsg(
				L"Vista/Win7 users may enable it by using Aero theme.",
				L"DWM is not enabled!",
				NotifyIcon::BalloonIcon::Error);
		}
		else if (dwmErr == DwmWndError::TargetIsLayeredWindow) {
			BalloonOrMsg(
				L"DWM formula doesn't work on Layered window.",
				L"Unsupported!",
				NotifyIcon::BalloonIcon::Error);
		}
	}
	catch (DwmFormula::DwmException const & ex) {
		MsgErr2(L"FATAL : Unexpected DwmExpection in ToggleNoborder().", ex.what());
		PostQuitMessage(nbdFatalExitCode);
	}
	catch (std::runtime_error const & ex) {
		MsgErr2(L"FATAL : Unexpected error in ToggleNoborder().", ex.what());
		PostQuitMessage(nbdFatalExitCode);
	}
}

bool LoadConfig() {
	auto hfile = CreateFileW(nbdCfgFilePath, GENERIC_READ, 0,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hfile) {
		DWORD read = 0;
		ReadFile(hfile, &nbdExcludeTaskbar, 1, &read, nullptr);
		ReadFile(hfile, &nbdAotMode, 1, &read, nullptr);
		ReadFile(hfile, &nbdUseDwm, 1, &read, nullptr);
		ReadFile(hfile, &nbdUseAltBack, 1, &read, nullptr);
		ReadFile(hfile, &nbdUseWinBack, 1, &read, nullptr);
		// If not using other hotkey, fallback to Alt+Backspace
		if (!nbdUseWinBack) { nbdUseAltBack = true; }
		CloseHandle(hfile);
		return (read > 0);
	}
	return false;
}

bool SaveConfig() {
	auto hfile = CreateFileW(nbdCfgFilePath, GENERIC_WRITE, 0,
		nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hfile) {
		DWORD written = 0;
		WriteFile(hfile, &nbdExcludeTaskbar, 1, &written, nullptr);
		WriteFile(hfile, &nbdAotMode, 1, &written, nullptr);
		WriteFile(hfile, &nbdUseDwm, 1, &written, nullptr);
		WriteFile(hfile, &nbdUseAltBack, 1, &written, nullptr);
		WriteFile(hfile, &nbdUseWinBack, 1, &written, nullptr);
		CloseHandle(hfile);
		return (written > 0);
	}
	return false;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	if (msg == nbdPrevInstCall) {
		try {
			nbdNotifyIcon.ShowBalloon(
				L"You can find me here.",
				L"noborder is already running!",
				NotifyIcon::BalloonIcon::Info);
			// tell 'new instance' that 'ShowBalloon' is OK
			return nbdPrevInstCall;
		}
		catch (std::runtime_error const & ex) {
			MsgErr2(L"Failed to show 'already running' balloon.", ex.what());
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
	else if (msg == WM_LBUTTONUP) {
		using namespace std;
		auto GetWndTitle = [](HWND hwnd) -> wstring {
			wchar_t szWndTitle[MAX_PATH] = { 0 };
			GetWindowTextW(hwnd, szWndTitle, MAX_PATH);
			return wstring(szWndTitle);
		};		
		wstringstream ss;
		if (target.IsNobordered()) {
			auto hwnd = target.GetHwnd();
			ss << L"A window has been nobordered." << endl;
			ss << L"Window title: " << GetWndTitle(hwnd);
		}
		else {
			ss << "Nothing has been 'nobordered' yet." << endl;
			ss << "Have a nice day!";
		}
		nbdNotifyIcon.ShowBalloon(ss.str().c_str(), nbdAppTitle,
			NotifyIcon::BalloonIcon::Info);
	}
}

void OnNbdPopupMenuItemClick(DWORD id) {
	if (id == CMI_EXCL_TASKBAR) { nbdExcludeTaskbar = !nbdExcludeTaskbar; }
	else if (id == CMI_USE_DWM) { nbdUseDwm = !nbdUseDwm; }
	else if (id == CMI_AOT_AUTO) { nbdAotMode = AotMode::Auto; }
	else if (id == CMI_AOT_ALWAYS) { nbdAotMode = AotMode::Always; }
	else if (id == CMI_AOT_NEVER) { nbdAotMode = AotMode::Never; }
	else if (id == CMI_HOTKEY_ALT_BACK) {
		if (nbdUseWinBack) { nbdUseAltBack = !nbdUseAltBack; }
	}
	else if (id == CMI_HOTKEY_WIN_BACK) {
		if (nbdUseAltBack) { nbdUseWinBack = !nbdUseWinBack; }
	}
	else {
		if (id == CMI_ABOUT) {
			MessageBoxW(nullptr, nbdAppInfo, nbdAppTitle, MB_ICONINFORMATION);
		}
		else if (id == CMI_EXIT) { PostQuitMessage(0); }
		return;
	}
	if (!SaveConfig()) {
		BalloonOrMsg(
			std::wstring(L"Failed to write data into ") + nbdCfgFileName,
			L"Failed to save config!",
			NotifyIcon::BalloonIcon::Error);
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
			&& nbdUseDwm ? MFS_CHECKED :

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
	if (DwmFormula::IsDwmSupported()) {
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
