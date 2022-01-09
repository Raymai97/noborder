#pragma once
#include <Windows.h>
#include "boon_layeredwindow.h"

class CursorOverlay
{
	HWND m_hwnd;
public:
	CursorOverlay();

	~CursorOverlay();

	BOOL CreateWnd();

	void Hide();

	void PreShow();

	void ShowAt(int x, int y);
};
