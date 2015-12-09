#include "noborder.h"

/* ... about the concept of this program

The 'targets' vector never delete its element.

Let's say th11.exe is running, and "nobordered" (mean its hWnd is stored in 'targets').
If it is terminated, its hWnd is no longer valid.
Instead of removing it from 'targets', we just reset its 'nobordered' to false.

In another words, we reuse the already-existed TARGET in 'targets', thus have no need to delete.

*/

void ToggleNoborder()
{
	HWND hWndOfTarget = GetForegroundWindow();
	TARGET * old = NULL;

	//for (TARGET * t : targets)
	for (std::vector<TARGET*>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		TARGET * t = *it;
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
		// save the original window position and size
		old->style = GetWindowLong(hWndOfTarget, GWL_STYLE);
		GetWindowRect(hWndOfTarget, &old->rect);
		// maintain aspect ratio based on ClientSize
		RECT cr; GetClientRect(old->hWnd, &cr);
		int x = 0;
		int y = 0;
		int width = cr.right - cr.left;
		int height = cr.bottom - cr.top;
		// must assign 'width(int)' to 'ratio(double)' first, else decimal place would be eaten :(
		double ratio = (ratio = width) / height;
		SIZE s = getDesktopSize();
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
		SetWindowLong(old->hWnd, GWL_STYLE, old->style & ~KEY_STYLE);
		SetWindowPos(old->hWnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW);
	}
	else // restore the original window size and position
	{
		old->nobordered = false;
		int x = old->rect.left;
		int y = old->rect.top;
		int width = old->rect.right - x;
		int height = old->rect.bottom - y;
		SetWindowLong(old->hWnd, GWL_STYLE, old->style);
		SetWindowPos(old->hWnd, HWND_NOTOPMOST, x, y, width, height, SWP_SHOWWINDOW);
	}
}


SIZE getDesktopSize()
{
	HWND hwnd = GetDesktopWindow();
	RECT rect;
	GetWindowRect(hwnd, &rect);
	SIZE size;
	size.cx = rect.right;
	size.cy = rect.bottom;
	return size;
}

