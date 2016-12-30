#pragma once
#include <Windows.h>
#include <tchar.h>
#include <exception>
#include <memory>

namespace Noborder {

	DWORD const KEY_STYLE = (WS_CAPTION | WS_THICKFRAME);
	DWORD const KEY_EXSTYLE = (WS_EX_DLGMODALFRAME |
		WS_EX_STATICEDGE | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);
	
	class Target; // need PosSize, DwmWindow
	class PosSize;
	class DwmWindow; // need PosSize	
}

class Noborder::Target {
	bool	m_isSet, m_isUsingDwm;
	HWND	m_hwnd;
	DWORD	m_dwStyle, m_dwExStyle;
	RECT	m_rcWnd;
	std::unique_ptr<DwmWindow> m_dwmWnd;

	static RECT GetMonitorRect(HWND const &,
		bool const & doExcludeTaskbar);
	static SIZE GetClientSize(HWND const &);
	static DWORD GetWndStyle(HWND const &);
	static DWORD GetWndExStyle(HWND const &);
	static void SetWndStyle(HWND const &, DWORD const &);
	static void SetWndExStyle(HWND const &, DWORD const &);
	static RECT GetWndRect(HWND const &);
	static void SetWndPosSize(HWND const &,
		PosSize const &,
		bool const & topMost);

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

class Noborder::DwmWindow {
	bool m_isSet;
	HWND m_hwnd;
	RECT m_rcWnd;

public:
	DwmWindow();
	~DwmWindow();

	enum class Error {
		Ok,
		DwmNotSupported,
		DwmNotEnabled,
		TargetIsLayeredWindow
	};

	bool IsSet() const;
	HWND GetHwnd() const;
	DwmWindow & Set(
		HWND const hwndTarget,
		PosSize const &,
		bool topMost);
	DwmWindow & Unset();
};
