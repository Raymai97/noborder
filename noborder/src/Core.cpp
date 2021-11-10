#include "noborder.h"

Target target;
DwmWindow *dwmWindow;
HMONITOR hMonitor;
RECT displayRect;

void CoreInit()
{
	if (x_canUseDwm)	{ dwmWindow = new DwmWindow(); }
	else { x_cfg.wantUseDwmFormula = false; }
}

void CoreClosing()
{
	if (target.nobordered) { UndoNoborder(&target); }
	if (dwmWindow) { delete dwmWindow; }
}

void ToggleNoborder()
{
	HWND hWndOfTarget = GetForegroundWindow();
	if (target.nobordered)
	{
		UndoNoborder(&target);
		if (target.hWnd != hWndOfTarget)
		{
			target.hWnd = hWndOfTarget;
			DoNoborder(&target);
		}
	}
	else
	{
		target.hWnd = hWndOfTarget;
		DoNoborder(&target);
	}
}

void DoNoborder(Target *t)
{
	// Get info about window size and client size
	WINDOWINFO wInfo;
	wInfo.cbSize = sizeof(WINDOWINFO);
	if (!GetWindowInfo(t->hWnd, &wInfo)) { return; }

	// Adjust client size if got menu bar
	MENUBARINFO mbInfo;
	mbInfo.cbSize = sizeof(mbInfo);
	if (GetMenuBarInfo(t->hWnd, OBJID_MENU, 0, &mbInfo))
	{
		wInfo.rcClient.top -= (mbInfo.rcBar.bottom - mbInfo.rcBar.top);
	}

	t->nobordered = true;
	t->psWin = PosSize(wInfo.rcWindow);
	t->psCli = PosSize(wInfo.rcClient);
	t->psNbd = NoborderPosSize(t->hWnd, t->psCli);
	t->style = wInfo.dwStyle;
	// DON'T USE wInfo.dwExStyle, it gives wrong value!
	t->exStyle = GetWindowLong(t->hWnd, GWL_EXSTYLE);
	t->isUsingDwm = x_cfg.wantUseDwmFormula;
	bool onTop = (
		x_cfg.onTopMode == OnTopMode_Always ? true :
		x_cfg.onTopMode == OnTopMode_Never ? false :
		x_cfg.wantExcludeTaskbar ? false : true);
	if (x_cfg.wantUseDwmFormula)
	{
		dwmWindow->Start(t, onTop);
	}
	else
	{
		SetWindowLong(t->hWnd, GWL_STYLE, t->style & ~KEY_STYLE);
		SetWindowPos(
			t->hWnd,
			onTop ? HWND_TOPMOST : HWND_NOTOPMOST,
			t->psNbd.X,
			t->psNbd.Y,
			t->psNbd.Width,
			t->psNbd.Height,
			SWP_SHOWWINDOW);
	}
}

void UndoNoborder(Target *t)
{
	t->nobordered = false;
	if (t->isUsingDwm)
	{
		dwmWindow->Stop();
	}
	else
	{
		SetWindowLong(t->hWnd, GWL_STYLE, t->style);
		SetWindowPos(
			t->hWnd,
			HASFLAG(t->exStyle, WS_EX_TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST,
			t->psWin.X,
			t->psWin.Y,
			t->psWin.Width,
			t->psWin.Height,
			SWP_SHOWWINDOW);
	}
}

PosSize NoborderPosSize(HWND hWnd, const PosSize psClient)
{
	// Compute aspect ratio based on ClientSize
	PosSize ps = psClient;
	ps.X = 0; ps.Y = 0;
	double ratio = (double)ps.Width / ps.Height;

	// Find RECT of display containing the window
	hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, 0);

	// Compute PosSize of 'nobordered' window
	PosSize s = PosSize(displayRect);
	if (s.Width > s.Height && !(s.Height * ratio > s.Width))
	{
		ps.Width = (int)(s.Height * ratio);
		ps.Height = s.Height;
		ps.X = (s.Width - ps.Width) / 2;
	}
	else
	{
		ps.Width = s.Width;
		ps.Height = int(s.Width / ratio);
		ps.Y = (s.Height - ps.Height) / 2;
	}
	ps.X += s.X;
	ps.Y += s.Y;
	return ps;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR h, HDC hdc, LPRECT lprc, LPARAM dwData)
{
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(lprc);
	UNREFERENCED_PARAMETER(dwData);
	if (h == hMonitor)
	{
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(h, &info);
		displayRect = (x_cfg.wantExcludeTaskbar ? info.rcWork : info.rcMonitor);
	}
	return true;
}

void BringToTop(HWND hWnd, bool topMost)
{
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	if (!topMost)
	{
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}
