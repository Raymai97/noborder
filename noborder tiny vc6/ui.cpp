#include "noborder.h"

NOTIFYICONDATA ni;

void AddNotifyIcon()
{
	ZeroMemory(&ni, sizeof(NOTIFYICONDATA));
	ni.cbSize = sizeof(NOTIFYICONDATA);
	ni.uID = TRAYICON_ID;
	ni.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	ni.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_NOBORDER), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ni.hWnd = hWnd;
	ni.uCallbackMessage = SWM_TRAYMSG;
	lstrcpyn(ni.szTip, NBD_TRAYICON_TEXT, sizeof(ni.szTip) / sizeof(TCHAR));

	Shell_NotifyIcon(NIM_ADD, &ni);

	if (ni.hIcon && DestroyIcon(ni.hIcon)) ni.hIcon = NULL;
}

void RemoveNotifyIcon()
{
	Shell_NotifyIcon(NIM_DELETE, &ni);
}

HWND CreateDummyWindow()
{
	// Register the window class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = NBD_DUMMY;
	wcex.hIconSm = NULL;

	if (!RegisterClassEx(&wcex)) return FALSE;

	// CreateWindow & ShowWindow & UpdateWindow
	DWORD dwStyle = WS_POPUPWINDOW;
	int x = CW_USEDEFAULT;
	int y = 0;
	int nWidth = CW_USEDEFAULT;
	int nHeight = 0;
	HWND hWndParent = NULL;
	HMENU hMenu = NULL;
	HWND lpParam = NULL;
	return CreateWindow(NBD_DUMMY, NBD_DUMMY, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInst, lpParam);
}

void ShowContextMenu(HWND hWnd)
{
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		InsertMenu(hMenu, -1, MF_BYPOSITION | MF_GRAYED, 0, NBD_APP_TITLE);
		InsertMenu(hMenu, -1, MF_SEPARATOR, 0, NULL);
		HMENU hAOTMenu = CreatePopupMenu();
		if (hAOTMenu)
		{
			InsertMenu(hAOTMenu, -1, MF_BYPOSITION, SWM_AOT_AUTO, NBD_CMI_AOT_AUTO);
			InsertMenu(hAOTMenu, -1, MF_BYPOSITION, SWM_AOT_ALWAYS, NBD_CMI_AOT_ALWAYS);
			InsertMenu(hAOTMenu, -1, MF_BYPOSITION, SWM_AOT_NEVER, NBD_CMI_AOT_NEVER);
			InsertMenu(hMenu, -1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hAOTMenu, NBD_CMI_AOT);
		}
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXCLUDE_TASKBAR, NBD_CMI_EXCLUDE_TASKBAR);
		InsertMenu(hMenu, -1, MF_SEPARATOR, 0, NULL);
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_ABOUT, NBD_CMI_ABOUT);
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, NBD_CMI_EXIT);

		if (excludeTaskbar)
		{
			CheckMenuItem(hMenu, SWM_EXCLUDE_TASKBAR, MF_BYCOMMAND | MF_CHECKED);
		}
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
		
		SetForegroundWindow(hWnd); // else menu won't disappear when lose focus
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
		DestroyMenu(hMenu);
	}
}
