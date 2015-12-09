#include "noborder.h"

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
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_ABOUT, NBD_CMI_ABOUT);
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, NBD_CMI_EXIT);
		
		SetForegroundWindow(hWnd); // else menu won't disappear when lose focus
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
		DestroyMenu(hMenu);
	}
}
