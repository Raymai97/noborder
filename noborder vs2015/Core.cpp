#include "noborder.h"

/* ... about the concept of this program

The 'targets' vector never delete its element.

Let's say th11.exe is running, and "nobordered" (mean its hWnd is stored in 'targets').
If it is terminated, its hWnd is no longer valid.
Instead of removing it from 'targets', we just reset its 'nobordered' to false.

In another words, we reuse the already-existed TARGET in 'targets', thus have no need to delete.

*/

std::vector<TARGET*> targets;
HMONITOR hMonitor;
RECT screenRect;

void ToggleNoborder()
{
	HWND hWndOfTarget = GetForegroundWindow();
	TARGET * old = NULL;
	HWND hWndInsertAfter;

	for (TARGET * t : targets)
	{
		// If a hWnd is no longer valid, its 'nobordered' should be false.
		if (IsWindow(t->hWnd) == false)
		{
			t->nobordered = false;
		}
		// Reuse the TARGET in 'targets', if it exists.
		if (t->hWnd == hWndOfTarget)
		{
			old = t;
			break;
		}
	}
	if (old == NULL) // No same hWnd existed in 'targets', gonna push a new one to it.
	{
		old = new TARGET();
		old->hWnd = hWndOfTarget;
		targets.push_back(old);
	}
	// If it's not 'nobordered', we 'noborder' it
	if (old->nobordered == false)
	{
		old->nobordered = true;
		// save the original window pos, size, style...
		old->style = GetWindowLong(hWndOfTarget, GWL_STYLE);
		old->exstyle = GetWindowLong(hWndOfTarget, GWL_EXSTYLE);
		GetWindowRect(hWndOfTarget, &old->rect);
		// maintain aspect ratio based on ClientSize
		RECT cr; GetClientRect(old->hWnd, &cr);
		int x = 0;
		int y = 0;
		int width = cr.right - cr.left;
		int height = cr.bottom - cr.top;
		// if simply 'int/int', decimal place would be eaten :(
		double ratio = (double)width / height;
		// find out RECT of screen containing the window
		hMonitor = MonitorFromWindow(old->hWnd, MONITOR_DEFAULTTOPRIMARY);
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, NULL);
		SIZE s = { screenRect.right - screenRect.left,
			screenRect.bottom - screenRect.top };
		if (s.cx > s.cy && !(s.cy * ratio > s.cx))
		{
			width = s.cy * ratio;
			height = s.cy;
			x = (s.cx - width) / 2;
		}
		else
		{
			width = s.cx;
			height = s.cx / ratio;
			y = (s.cy - height) / 2;
		}
		x += screenRect.left;
		y += screenRect.top;
		SetWindowLong(old->hWnd, GWL_STYLE, old->style & ~KEY_STYLE);
		hWndInsertAfter = (
			alwaysOnTopMode == AOT_ALWAYS ? HWND_TOPMOST : (
			alwaysOnTopMode == AOT_NEVER ? HWND_NOTOPMOST : 
			excludeTaskbar ? HWND_NOTOPMOST : HWND_TOPMOST));
		SetWindowPos(old->hWnd, hWndInsertAfter, x, y, width, height, SWP_SHOWWINDOW);
	}
	else // restore the original window size and position
	{
		old->nobordered = false;
		int x = old->rect.left;
		int y = old->rect.top;
		int width = old->rect.right - x;
		int height = old->rect.bottom - y;
		SetWindowLong(old->hWnd, GWL_STYLE, old->style);
		hWndInsertAfter = (old->exstyle & WS_EX_TOPMOST) ? HWND_TOPMOST : HWND_NOTOPMOST;
		SetWindowPos(old->hWnd, hWndInsertAfter, x, y, width, height, SWP_SHOWWINDOW);
	}
}

BOOL CALLBACK MonitorEnumProc(HMONITOR h, HDC hdc, LPRECT lprc, LPARAM dwData)
{
	if (h == hMonitor)
	{
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(h, &info);
		screenRect = (excludeTaskbar ? info.rcWork : info.rcMonitor);
	}
	return true;
}

/* Old code : Doesn't support multiple monitors

SIZE getDesktopSize()
{
	HWND hwnd = GetDesktopWindow();
	RECT rect;
	GetWindowRect(hwnd, &rect);
	SIZE size;
	size.cx = rect.right;
	size.cy = rect.bottom;
	return size;
} */
