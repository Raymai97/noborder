#include "NotifyIcon.h"

// Target OS: Windows 2000 ++

#define LEN_TIP		128
#define LEN_BTITLE	64
#define LEN_BTEXT	256
#define NI_MSG		WM_APP

NotifyIcon::NotifyIcon(HINSTANCE hInst, LPCTSTR className, UINT id)
{
	this->hInst = hInst;
	this->className = className;
	this->id = id;
	// Register class & Create window
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = NotifyIcon::WndProc;
	wcex.hInstance = this->hInst;
	wcex.lpszClassName = this->className;
	RegisterClassEx(&wcex);
	// Don't use HWND_MESSAGE here, becuz I need to use lpParam
	HWND hWndParent = nullptr;
	HMENU hMenu = nullptr;
	LPVOID lpParam = this; // so WndProc can retrieve 'this' later
	this->hWnd = CreateWindow(
		className,
		className,
		WS_POPUP,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		hWndParent,
		hMenu,
		hInst,
		lpParam);
	// Init var
	this->added = false;
	this->visible = true;
	this->hIcon = nullptr;
	this->szTip = new TCHAR[LEN_TIP];
	this->szBalloonTitle = new TCHAR[LEN_BTITLE]; 
	this->szBalloonText = new TCHAR[LEN_BTEXT];
	this->niifBalloon = NIIF_INFO;
	this->wmTaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));
}

NotifyIcon::~NotifyIcon()
{
	this->Update(false);
	DestroyWindow(this->hWnd);
	UnregisterClass(this->className, this->hInst);
}

bool NotifyIcon::HwndInitFailed()
{
	return (this->hWnd == nullptr);
}

BOOL NotifyIcon::SetTip(LPCTSTR szTip)
{
	lstrcpyn(this->szTip, szTip, LEN_TIP);
	return Update(this->visible);
}

BOOL NotifyIcon::SetIcon(HICON hIcon)
{
	this->hIcon = hIcon;
	return Update(this->visible);
}

BOOL NotifyIcon::SetVisible(bool visible)
{
	return this->Update(this->visible = visible);
}

BOOL NotifyIcon::HideBalloon()
{
	if (this->added)
	{
		NOTIFYICONDATA d;
		ZeroMemory(&d, sizeof(d));
		d.cbSize = sizeof(d);
		d.hWnd = this->hWnd;
		d.uID = this->id;
		d.uFlags = NIF_INFO;
		lstrcpyn(d.szInfo, _T(""), 256);
		return Shell_NotifyIcon(NIM_MODIFY, &d);
	}
	return FALSE;
}

BOOL NotifyIcon::ShowBalloon(LPCTSTR szText, LPCTSTR szTitle, DWORD niif)
{
	lstrcpyn(this->szBalloonText, szText, LEN_BTEXT);
	lstrcpyn(this->szBalloonTitle, szTitle, LEN_BTITLE);
	this->niifBalloon = niif;
	return this->ShowBalloon();
}

BOOL NotifyIcon::ShowBalloon()
{
	if (this->added)
	{
		NOTIFYICONDATA d;
		ZeroMemory(&d, sizeof(d));
		d.cbSize = sizeof(d);
		d.hWnd = this->hWnd;
		d.uID = this->id;
		d.uFlags = NIF_INFO;
		lstrcpyn(d.szInfo, this->szBalloonText, LEN_BTEXT);
		lstrcpyn(d.szInfoTitle, this->szBalloonTitle, LEN_BTITLE);
		d.dwInfoFlags = this->niifBalloon;
		return Shell_NotifyIcon(NIM_MODIFY, &d);
	}
	return FALSE;
}

BOOL NotifyIcon::Update(bool showInTray)
{
	BOOL ok;
	NOTIFYICONDATA d;
	ZeroMemory(&d, sizeof(d));
	d.cbSize = sizeof(d);
	d.hWnd = this->hWnd;
	d.uID = id;
	d.uFlags = NIF_MESSAGE | NIF_ICON;
	d.uCallbackMessage = NI_MSG;
	d.hIcon = this->hIcon;
	if (lstrlen(this->szTip))
	{
		d.uFlags |= NIF_TIP;
		lstrcpyn(d.szTip, this->szTip, LEN_TIP);
	}
	if (showInTray)
	{
		if (this->added)
		{
			ok =Shell_NotifyIcon(NIM_MODIFY, &d);
		}
		else
		{
			ok = Shell_NotifyIcon(NIM_ADD, &d);
			if (ok) { this->added = true; }
		}
	}
	else if (added)
	{
		ok = Shell_NotifyIcon(NIM_DELETE, &d);
		if (ok) { this->added = false; }
	}
	return ok;
}

void NotifyIcon::ShowMenu(UINT niId)
{
	POINT m;
	GetCursorPos(&m);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		if (OnMenuCreating) { OnMenuCreating(niId, hMenu); }
		SetForegroundWindow(this->hWnd); // else menu won't disappear when lose focus
		TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, m.x, m.y, 0, this->hWnd, nullptr);
		DestroyMenu(hMenu);
	}
}

LRESULT CALLBACK NotifyIcon::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	NotifyIcon *me = (NotifyIcon*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (msg == WM_NCCREATE)
	{
		// Retrieve 'this' from CreateWindow's lpParam
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		me = (NotifyIcon*)(lpcs->lpCreateParams);
		// Save it here, next time we will get 'this'
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)me);
	}
	else if (msg == NI_MSG)
	{
		if (lParam == WM_RBUTTONUP)
		{
			me->ShowMenu((UINT)wParam);
		}
	}
	else if (msg == WM_COMMAND)
	{
		if (me->OnMenuItemSelected)
		{
			me->OnMenuItemSelected(LOWORD(wParam), HIWORD(wParam));
		}
	}
	else if (msg == me->wmTaskbarCreated)
	{
		me->added = false;
		me->Update(me->visible);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
