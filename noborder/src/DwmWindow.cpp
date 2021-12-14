#include "noborder.h"

DwmWindow::DwmWindow() :
	hWnd(0), hThumb(0), target(0),
	ing(false), topMost(false), dontFocus(false)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hInstance = x_hInst;
	wcex.lpszClassName = WCN_NbdDwmWindow;
	wcex.lpfnWndProc = DwmWindow::WndProc;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClassEx(&wcex)) { MSGERR("FATAL: DwmWindow RegisterClassEx failed!"); PostQuitMessage(1); }
	HWND hWndParent = nullptr;
	HMENU hMenu = nullptr;
	LPVOID lpParam = this; // so WndProc can retrieve 'this' later
	this->hWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		WCN_NbdDwmWindow,
		WCN_NbdDwmWindow,
		WS_POPUP,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		hWndParent,
		hMenu,
		x_hInst,
		lpParam);
	if (this->hWnd == nullptr)	{ MSGERR("FATAL: DwmWindow InitHwnd failed!"); PostQuitMessage(1); }
}

DwmWindow::~DwmWindow()
{
	DestroyWindow(this->hWnd);
	UnregisterClass(WCN_NbdDwmWindow, x_hInst);
}

void DwmWindow::Start(Target *pTarget, bool isTopMost)
{
	BOOL dwmIsOK;
	Compat_DwmIsCompositionEnabled(&dwmIsOK);
	if (!dwmIsOK)
	{
		x_pNotifyIcon->ShowBalloon(
			_T("Vista/Win7 users may enable it by using Aero theme."),
			_T("DWM is not enabled!"),
			NIIF_ERROR);
		pTarget->nobordered = false;
		return;
	}
	if (HASFLAG(pTarget->exStyle, WS_EX_LAYERED))
	{
		x_pNotifyIcon->ShowBalloon(
			_T("DWM formula doesn't work on Layered window."),
			_T("Unsupported!"),
			NIIF_ERROR);
		pTarget->nobordered = false;
		return;
	}
	this->target = pTarget;
	this->topMost = isTopMost;
	this->dontFocus = false;
	// Make dwmWindow looks like nobordered target...
	PosSize const &ps = (x_cfg.letterboxColor.a > 0)
		? this->target->psNbdFreeStretch
		: this->target->psNbd;
	SetWindowPos(this->hWnd, HWND_TOP, ps.X, ps.Y, ps.Width, ps.Height, SWP_SHOWWINDOW);
	BringToTop(this->hWnd, this->topMost);
	// Make target invisible...
	SetWindowLong(this->target->hWnd, GWL_EXSTYLE, this->target->exStyle | WS_EX_LAYERED);
	SetLayeredWindowAttributes(this->target->hWnd, 0, 0, LWA_ALPHA);
	SetForegroundWindow(this->target->hWnd); // but activated
	// DWM Magic!
	Compat_DwmRegisterThumbnail(this->hWnd, this->target->hWnd, &(this->hThumb));
	UpdateThumb();
	// Start CheckTargetProc
	this->ing = true;
	DWORD tid;
	CreateThread(nullptr, 0, DwmWindow::CheckTargetProc, this, 0, &tid);
}

void DwmWindow::Stop()
{
	// Stop CheckTargetProc
	this->ing = false;
	// Hide dwmWindow
	SetWindowPos(this->hWnd, HWND_TOP, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, SWP_HIDEWINDOW);
	if (this->target)
	{
		// Stop DWM Magic
		Compat_DwmUnregisterThumbnail(this->hThumb);
		// Restore target
		SetWindowLong(this->target->hWnd, GWL_EXSTYLE, this->target->exStyle);
		if (this->dontFocus == false)
		{
			SetForegroundWindow(this->target->hWnd);
		}
		this->target->nobordered = false; // in case Core.cpp didn't (e.g. when DWM restarted)
		this->target = nullptr;
	}
}

void DwmWindow::UpdateThumb()
{
	// NOTE: Set rcSource explicitly to avoid weird problem
	PosSize psSrc = this->target->psCli;
	DWM_THUMBNAIL_PROPERTIES props;
	props.dwFlags = DWM_TNP_VISIBLE | DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE;
	props.fSourceClientAreaOnly = true; // if false, DWM scale ugly!
	props.fVisible = true;
	if (x_user32__PhyToLogPtForPerMonitorDPI)
	{
		POINT posPhy, posLog;
		posPhy.x = this->target->psWin.X + 1; posLog.x = posPhy.x;
		posPhy.y = this->target->psWin.Y + 1; posLog.y = posPhy.y;
		HRESULT hr = Compat_PhyToLogPtForPerMonitorDPI(this->target->hWnd, &posLog);
		if (SUCCEEDED(hr))
		{
			psSrc.Width = MulDiv(psSrc.Width, posLog.x, posPhy.x);
			psSrc.Height = MulDiv(psSrc.Height, posLog.y, posPhy.y);
		}
	}
	SetRect(&props.rcSource, 0, 0, psSrc.Width, psSrc.Height);
	if (x_cfg.letterboxColor.a > 0)
	{
		props.rcDestination = this->target->psNbd.ToRECT();
	}
	else
	{
		PosSize const &ps = this->target->psNbd;
		SetRect(&props.rcDestination, 0, 0, ps.Width, ps.Height);
	}
	Compat_DwmUpdateThumbnailProperties(this->hThumb, &props);
}

DWORD WINAPI DwmWindow::CheckTargetProc(LPVOID param)
{
	DwmWindow *me = (DwmWindow*)param;
	bool wasActivated = true;
	HWND curr = nullptr, last = nullptr;
	while (me->ing && me->target)
	{
		if (!IsWindowVisible(me->target->hWnd))
		{
			me->Stop();
		}
		else if ((curr = GetForegroundWindow()) != last)
		{
			ShowWindow(me->hWnd, IsIconic(me->target->hWnd) ? SW_HIDE : SW_SHOWNOACTIVATE);
			if (wasActivated && !(curr == me->hWnd || curr == me->target->hWnd))
			{
				wasActivated = false;
				if (GetParent(curr) == me->target->hWnd)
				{
					me->dontFocus = true;
					me->Stop();
				}
			}
			else if (!wasActivated && (curr == me->hWnd || curr == me->target->hWnd))
			{
				wasActivated = true;
				BringToTop(me->hWnd, me->topMost);
			}
			last = curr;
		}
		Sleep(100);
	}
	return 0;
}

LRESULT CALLBACK DwmWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DwmWindow *me = nullptr;
	LONG_PTR winlong = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (winlong) { me = (DwmWindow*)winlong; }
	if (msg == WM_NCCREATE)
	{
		// Retrieve 'this' from CreateWindowEx's lpParam
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		me = (DwmWindow*)(lpcs->lpCreateParams);
		// Save it here, next time we will get 'this'
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)me);
	}
	else if (me && me->target)
	{
		if (msg == WM_ACTIVATE || msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN)
		{
			BringToTop(me->hWnd, me->topMost);
			SetForegroundWindow(me->target->hWnd);
			SendMessage(me->target->hWnd, WM_ACTIVATEAPP, 1, 0);
		}
		else if (msg == WM_DWMCOMPOSITIONCHANGED)
		{
			me->Stop();
		}
		else if (msg == WM_ERASEBKGND)
		{
			ColorARGB const &c = x_cfg.letterboxColor;
			if (c.a > 0)
			{
				HDC const hdc = (HDC)wParam;
				HBRUSH hbr = CreateSolidBrush(RGB(c.r, c.g, c.b));
				if (hbr)
				{
					RECT rc;
					GetClientRect(hWnd, &rc);
					FillRect(hdc, &rc, hbr);
					DeleteObject(hbr);
				}
			}
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
