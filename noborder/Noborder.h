#pragma once
#include <Windows.h>
#include <stdexcept>

namespace Noborder {

	DWORD const KEY_STYLE = (WS_CAPTION | WS_THICKFRAME);
	DWORD const KEY_EXSTYLE = (WS_EX_DLGMODALFRAME |
		WS_EX_STATICEDGE | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);
	
	class Target; // need PosSize, ITargetSetAble
	class PosSize;
	class ITargetSetAble; // need PosSize

	RECT GetMonitorRect(HWND const,
		bool const & doExcludeTaskbar);
	SIZE GetClientSize(HWND const);
	DWORD GetWndStyle(HWND const);
	DWORD GetWndExStyle(HWND const);
	void SetWndStyle(HWND const, DWORD const);
	void SetWndExStyle(HWND const, DWORD const);
	RECT GetWndRect(HWND const);
	void SetWndPosSize(HWND const,
		PosSize const &,
		bool const & topMost);
	void BringWndToTop(HWND const,
		bool const & topMost);
}

class Noborder::Target {
	bool	m_isSet;
	HWND	m_hwnd;
	DWORD	m_dwStyle, m_dwExStyle;
	RECT	m_rcWnd;
	ITargetSetAble *m_setAble;

public:
	Target();
	~Target();

	enum class AlwaysOnTopMode {
		Auto, Always, Never
	};

	bool IsNobordered() const;
	HWND GetHwnd() const;
	Target & Set(
		HWND const hwndTarget,
		AlwaysOnTopMode const &,
		bool const & excludeTaskbar,
		ITargetSetAble * = nullptr);
	Target & Unset();

};

class Noborder::PosSize {
public:
	LONG x, y, cx, cy;

	PosSize(LONG const x = 0, LONG const y = 0,
		LONG const  cx = 0, LONG const  cy = 0);
	PosSize(RECT const & rc);
	RECT ToRECT() const;
	void MaxCenterIn(PosSize const & psMax);
};

class Noborder::ITargetSetAble {
public:
	virtual bool IsSet() const = 0;
	virtual ITargetSetAble & Set(
		HWND const hwndTarget,
		PosSize const &,
		bool const topMost) = 0;
	virtual ITargetSetAble & Unset() = 0;
};

