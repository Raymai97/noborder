#include "noborder.h"
#define TIMER_ID__REPOSITION_CURSOR_OVERLAY  (100)

static void MyTweakTargetForCursorOverlay(Target *t, bool wantUndo);
static void MySetCursorPosToCenterOfTarget(Target const *t);

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
	if (!cursorOverlay.CreateWnd())
	{
		MSGERR("FATAL: CursorOverlay CreateWnd failed!");
		PostQuitMessage(1);
	}
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
	// Make target becomes a Layered Window
	SetWindowLong(this->target->hWnd, GWL_EXSTYLE, this->target->exStyle | WS_EX_LAYERED);
	if (this->target->isUsingDwmFormulaWithMouse)
	{
		// For CursorOverlay to work properly, DwmWindow must be Never On Top.
		BringToTop(this->hWnd, FALSE);
		// Make target visually invisible but receives mouse-event
		SetLayeredWindowAttributes(this->target->hWnd, 0, 1, LWA_ALPHA);
		MyTweakTargetForCursorOverlay(this->target, false);
		BringToTop(this->target->hWnd, FALSE);
		MySetCursorPosToCenterOfTarget(this->target);
		// Start CursorOverlay
		cursorOverlay.PreShow();
		SetTimer(this->hWnd, TIMER_ID__REPOSITION_CURSOR_OVERLAY, 10, 0);
	}
	else
	{
		BringToTop(this->hWnd, this->topMost);
		// Make target completely invisible and not receiving any mouse-event
		SetLayeredWindowAttributes(this->target->hWnd, 0, 0, LWA_ALPHA);
		SetForegroundWindow(this->target->hWnd); // but activated
	}
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
	// Stop CursorOverlay if started
	KillTimer(hWnd, TIMER_ID__REPOSITION_CURSOR_OVERLAY);
	this->cursorOverlay.Hide();
	// Hide dwmWindow
	SetWindowPos(this->hWnd, HWND_TOP, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, SWP_HIDEWINDOW);
	if (this->target)
	{
		// Stop DWM Magic
		Compat_DwmUnregisterThumbnail(this->hThumb);
		// Restore target
		if (this->target->isUsingDwmFormulaWithMouse)
		{
			MyTweakTargetForCursorOverlay(this->target, true);
		}
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
	// If target's window style has been modified in MyTweakTargetForCursorOverlay()
	// and the window is not a popup window...
	if (this->target->isUsingDwmFormulaWithMouse && (this->target->style & WS_POPUP) == 0)
	{
		// DWM API will expect rcSource to include non-client area which is nonsense IMHO
		// so the easiest workaround now is NOT to specify it.
		props.dwFlags &= ~DWM_TNP_RECTSOURCE;
	}
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
	DwmWindow * const me = (DwmWindow*)param;
	bool wasActivated = true;
	HWND curr = nullptr, last = nullptr;
	for (; me->ing && me->target; Sleep(100))
	{
		if (!IsWindowVisible(me->target->hWnd))
		{
			me->dontFocus = true;
			me->Stop();
			break;
		}
		if (IsIconic(me->target->hWnd))
		{
			ShowWindow(me->hWnd, SW_HIDE);
			continue;
		}
		if ((curr = GetForegroundWindow()) != last)
		{
			// If 'target' has top-level child, stop DWM formula.
			if (GetParent(curr) == me->target->hWnd)
			{
				me->dontFocus = true;
				me->Stop();
				break;
			}
			if (wasActivated && !(curr == me->hWnd || curr == me->target->hWnd))
			{
				wasActivated = false;
			}
			else if (!wasActivated && (curr == me->hWnd || curr == me->target->hWnd))
			{
				wasActivated = true;
				if (me->target->isUsingDwmFormulaWithMouse)
				{
					MySetCursorPosToCenterOfTarget(me->target);
				}
				else
				{
					BringToTop(me->hWnd, me->topMost);
				}
				BringToTop(me->target->hWnd, FALSE);
			}
			last = curr;
		}
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
		else if (msg == WM_TIMER)
		{
			if (wParam == TIMER_ID__REPOSITION_CURSOR_OVERLAY)
			{
				me->RepositionCursorOverlay();
			}
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void DwmWindow::RepositionCursorOverlay()
{
	POINT pt = { 0 };
	GetCursorPos(&pt);
	// Using DWM Formula with mouse is a very hacky and complicated thing.
	// To prevent surprise or confusion, we should do "DWM shrink" when
	// - target is not current active window (to let user operate on other window)
	// - mouse cursor is not on target (to prevent "I thought I clicked on target")
	if (GetForegroundWindow() != this->target->hWnd ||
		WindowFromPoint(pt) != this->target->hWnd)
	{
		if (this->target->hadDwmShrinkTemporarily == false)
		{
			this->cursorOverlay.Hide();
			ShowWindow(this->hWnd, SW_HIDE);
			SetLayeredWindowAttributes(this->target->hWnd, 0, 255, LWA_ALPHA);
			this->target->hadDwmShrinkTemporarily = true;
		}
		return;
	}
	if (this->target->hadDwmShrinkTemporarily)
	{
		BringToTop(this->hWnd, FALSE);
		BringToTop(this->target->hWnd, FALSE);
		SetLayeredWindowAttributes(this->target->hWnd, 0, 1, LWA_ALPHA);
		this->target->hadDwmShrinkTemporarily = false;
	}
	// --- End of DWM shrink
	double const scaleX = this->target->cursorOverlay.scaleX;
	pt.x -= this->target->psNbdFreeStretch.X;
	pt.x = (LONG)((pt.x * scaleX) + this->target->psNbd.X - 1);
	double const scaleY = this->target->cursorOverlay.scaleY;
	pt.y -= this->target->psNbdFreeStretch.Y;
	pt.y = (LONG)((pt.y * scaleY) + this->target->psNbd.Y - 1);
	this->cursorOverlay.ShowAt(pt.x, pt.y);
}

static void MyTweakTargetForCursorOverlay(Target *t, bool wantUndo)
{
	if (wantUndo)
	{
		SetWindowPos(t->hWnd, 0, t->psWin.X, t->psWin.Y,
			t->psWin.Width, t->psWin.Height,
			SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowLong(t->hWnd, GWL_STYLE, t->style);
		return;
	}
	// These tweaks are here to
	// - make CursorOverlay position accurate
	// - prevent unwanted mouse event on target's window's title bar
	SetWindowLong(t->hWnd, GWL_STYLE, t->style & ~(WS_CAPTION | WS_THICKFRAME));
	SetWindowPos(t->hWnd, 0, t->psNbdFreeStretch.X, t->psNbdFreeStretch.Y,
		t->psCli.Width, t->psCli.Height,
		SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOACTIVATE);
	// Precache scale factor
	t->cursorOverlay.scaleX = ((double)t->psNbd.Width) / ((double)t->psCli.Width);
	t->cursorOverlay.scaleY = ((double)t->psNbd.Height) / ((double)t->psCli.Height);
}

static void MySetCursorPosToCenterOfTarget(Target const *t)
{
	SetCursorPos(t->psNbdFreeStretch.X + (t->psCli.Width / 2),
		t->psNbdFreeStretch.Y + (t->psCli.Height / 2));
}
