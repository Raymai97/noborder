#include "CursorOverlay.hpp"

#define MY_CLASSNAME  TEXT("noborder.CursorOverlay")

CursorOverlay::CursorOverlay() :
	m_hwnd(0)
{
}

CursorOverlay::~CursorOverlay()
{
	if (m_hwnd)
	{
		DestroyWindow(m_hwnd);
	}
}

static LRESULT CALLBACK My_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL fOverrid = 0;
	if (!fOverrid)
	{
		lRes = DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return lRes;
}

BOOL CursorOverlay::CreateWnd()
{
	WNDCLASS wc = { 0 };
	wc.lpszClassName = MY_CLASSNAME;
	wc.lpfnWndProc = My_WndProc;
	::RegisterClass(&wc);
	m_hwnd = ::CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT,
		MY_CLASSNAME, NULL,
		WS_POPUP,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		0, 0, 0, this);
	return !!m_hwnd;
}

static BOOL CALLBACK MyRenderProc(
	HDC hdcBmp,
	DWORD *pBmpPixels,
	LONG cxBmp,
	LONG cyBmp,
	void *pUser)
{
	UNREFERENCED_PARAMETER(pBmpPixels);
	UNREFERENCED_PARAMETER(cxBmp);
	UNREFERENCED_PARAMETER(cyBmp);
	UNREFERENCED_PARAMETER(pUser);
	HCURSOR hCur = 0;
	TCHAR szPath[MAX_PATH] = { 0 }, *pSlash = 0;
	GetModuleFileName(0, szPath, MAX_PATH);
	for (TCHAR *p = szPath; *p; ++p)
	{
		if (*p == '\\') pSlash = p;
	}
	if (pSlash)
	{
		lstrcpy(&pSlash[1], TEXT("CursorOverlay.cur"));
	}
	hCur = LoadCursorFromFile(szPath);
	if (!hCur)
	{
		hCur = LoadCursor(0, IDC_ARROW);
	}
	DrawIcon(hdcBmp, 0, 0, hCur);
	return TRUE;
}

void CursorOverlay::Hide()
{
	ShowWindow(m_hwnd, SW_HIDE);
}

void CursorOverlay::PreShow()
{
	SetWindowPos(m_hwnd, 0, 0, 0,
		GetSystemMetrics(SM_CXCURSOR),
		GetSystemMetrics(SM_CYCURSOR),
		SWP_NOSENDCHANGING | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	Boon_FullUpdateLayeredWindow32bpp(m_hwnd, MyRenderProc, this);
}

void CursorOverlay::ShowAt(int x, int y)
{
	SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, 0, 0,
		SWP_NOSENDCHANGING | SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSIZE);
}
