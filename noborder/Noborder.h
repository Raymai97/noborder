#pragma once
#include <Windows.h>
#include <tchar.h>
#include <exception>

namespace Noborder {

	DWORD const KEY_STYLE = (WS_CAPTION | WS_THICKFRAME);
	DWORD const KEY_EXSTYLE = (WS_EX_DLGMODALFRAME |
		WS_EX_STATICEDGE | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);
	
	class Target;
	class PosSize;
	
	RECT GetMonitorRect(HWND const &,
		bool const & doExcludeTaskbar);
	SIZE GetClientSize(HWND const &);
	DWORD GetWndStyle(HWND const &);
	DWORD GetWndExStyle(HWND const &);
	void SetWndStyle(HWND const &, DWORD const &);
	void SetWndExStyle(HWND const &, DWORD const &);
	RECT GetWndRect(HWND const &);
	void SetWndPosSize(HWND const &,
		PosSize const &,
		bool const & topMost);
}

class Noborder::Target {
	bool	m_isNbded, m_isUsingDwm;
	HWND	m_hwnd;
	DWORD	m_dwStyle, m_dwExStyle;
	RECT	m_rcWnd;

public:
	Target();
	~Target();

	enum class AlwaysOnTopMode {
		Auto, Always, Never
	};

	bool IsNobordered() const;
	bool IsUsingDwm() const;
	HWND GetHwnd() const;
	Target & Set(
		HWND const hwndTarget,
		AlwaysOnTopMode const &,
		bool const & excludeTaskbar,
		bool const & useDwm);
	Target & Unset();

};

class Noborder::PosSize {
public:
	LONG x, y, cx, cy;

	PosSize(LONG x = 0, LONG y = 0, LONG cx = 0, LONG cy = 0);
	PosSize(RECT const & rc);
	RECT ToRECT();
	void MaxCenterIn(PosSize const & psMax);
};
