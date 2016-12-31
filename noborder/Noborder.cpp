#include "Noborder.h"
#ifndef THROW
#define THROW(caller, callee)	throw std::runtime_error(\
	"at " #callee "()\n" \
	"at " #caller "()\n")
#endif

using namespace Noborder;

// ... Target ... need PosSize, ITargetSetAble

Target::Target() :
	m_isSet(false),
	m_hwnd(nullptr),
	m_dwExStyle(0),
	m_dwStyle(0),
	m_rcWnd({ 0 }),
	m_setAble(nullptr) {
}

Target::~Target() {
	try { this->Unset(); }
	catch (...) {}
}

bool Target::IsNobordered() const {
	if (m_setAble) { return m_setAble->IsSet(); }
	return m_isSet;
}

HWND Target::GetHwnd() const {
	return m_hwnd;
}

Target & Target::Set(
	HWND const hwndTarget,
	AlwaysOnTopMode const & aotMode,
	bool const & excludeTaskbar,
	ITargetSetAble * setAble)
{
	if (this->IsNobordered()) { return *this; }
	// Obtain info for restoring window later
	m_dwExStyle = GetWndExStyle(hwndTarget);
	m_dwStyle = GetWndStyle(hwndTarget);
	m_rcWnd = GetWndRect(hwndTarget);
	// Determine nobordered PosSize
	PosSize psNbd;
	SIZE cliSize = GetClientSize(hwndTarget);
	psNbd.cx = cliSize.cx;
	psNbd.cy = cliSize.cy;
	RECT rcMax = GetMonitorRect(hwndTarget, excludeTaskbar);
	psNbd.MaxCenterIn(PosSize(rcMax));
	// Make window noborder
	bool topMost =
		(aotMode == AlwaysOnTopMode::Always) ||
		(aotMode == AlwaysOnTopMode::Auto && !excludeTaskbar);
	if (setAble != nullptr) {
		setAble->Set(hwndTarget, psNbd, topMost);
		m_setAble = setAble;
	}
	else {
		SetWndExStyle(hwndTarget, m_dwExStyle & ~KEY_EXSTYLE);
		SetWndStyle(hwndTarget, m_dwStyle & ~KEY_STYLE);
		SetWndPosSize(hwndTarget, psNbd, topMost);	
	}
	m_hwnd = hwndTarget;
	m_isSet = true;
	return *this;
}

Target & Target::Unset() {
	if (this->IsNobordered()) {
		if (m_setAble != nullptr) {
			m_setAble->Unset();
			m_setAble = nullptr;
		}
		else if (IsWindow(m_hwnd)) {
			bool topMost = (m_dwExStyle & WS_EX_TOPMOST) != 0;
			SetWndStyle(m_hwnd, m_dwStyle);
			SetWndExStyle(m_hwnd, m_dwExStyle);
			SetWndPosSize(m_hwnd, PosSize(m_rcWnd), topMost);
		}
	}
	m_hwnd = nullptr;
	m_isSet = false;
	return *this;
}

// ... PosSize ...

PosSize::PosSize(LONG const x, LONG const y,
	LONG const cx, LONG const cy) :
	x(x), y(y), cx(cx), cy(cy) {
}

PosSize::PosSize(RECT const & rc) :
	x(rc.left), y(rc.top),
	cx(rc.right - rc.left),
	cy(rc.bottom - rc.top) {
}

RECT PosSize::ToRECT() const {
	RECT rc = { 0 };
	rc.left = this->x;
	rc.right = this->x + this->cx;
	rc.top = this->y;
	rc.bottom = this->y + this->cy;
	return rc;
}

void PosSize::MaxCenterIn(PosSize const & psMax) {
	/* Maximize 'this' in psMax while preserving aspect ratio */
	LONG new_cx = 0, new_cy = 0;
	new_cx = psMax.cx;
	new_cy = static_cast<LONG>
		(this->cy * (1.0 * psMax.cx / this->cx));
	if (new_cy > psMax.cy) {
		new_cy = psMax.cy;
		new_cx = static_cast<LONG>
			(this->cx * (1.0 * psMax.cy / this->cy));
	}
	this->cx = new_cx;
	this->cy = new_cy;
	/* Center 'this' in psMax */
	this->x = (psMax.cx - this->cx) / 2;
	this->y = (psMax.cy - this->cy) / 2;
}


// ... Noborder Helper Functions ...

RECT Noborder::GetMonitorRect(
	HWND const hwnd,
	bool const & doExcludeTaskbar) {
	HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = { 0 };
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfoW(hmon, &mi)) {
		THROW(Noborder::GetMonitorRect, GetMonitorInfoW);
	}
	return doExcludeTaskbar ? mi.rcWork : mi.rcMonitor;
}

SIZE Noborder::GetClientSize(HWND const hwnd) {
	RECT rc = { 0 };
	if (!GetClientRect(hwnd, &rc)) {
		THROW(Noborder::GetClientSize, GetClientRect);
	}
	SIZE size = { 0 };
	size.cx = rc.right - rc.left;
	size.cy = rc.bottom - rc.top;
	// Adjust client size if got menu bar
	MENUBARINFO mbInfo;
	mbInfo.cbSize = sizeof(mbInfo);
	if (GetMenuBarInfo(hwnd, OBJID_MENU, 0, &mbInfo)) {
		size.cy += PosSize(mbInfo.rcBar).cy;
	}
	return size;
}

DWORD Noborder::GetWndStyle(HWND const hwnd) {
	SetLastError(0);
	auto val = static_cast<DWORD>(GetWindowLongPtrW(hwnd, GWL_STYLE));
	if (GetLastError()) {
		THROW(Noborder::GetWndStyle, GetWindowLongPtrW);
	}
	return val;
}

DWORD Noborder::GetWndExStyle(HWND const hwnd) {
	SetLastError(0);
	auto val = static_cast<DWORD>(GetWindowLongPtrW(hwnd, GWL_EXSTYLE));
	if (GetLastError()) {
		THROW(Noborder::GetWndExStyle, GetWindowLongPtrW);
	}
	return val;
}

void Noborder::SetWndStyle(
	HWND const hwnd,
	DWORD const val)
{
	SetLastError(0);
	SetWindowLongPtrW(hwnd, GWL_STYLE, val);
	if (GetLastError()) {
		THROW(Noborder::SetWndStyle, SetWindowLongPtrW);
	}
}

void Noborder::SetWndExStyle(
	HWND const hwnd,
	DWORD const val)
{
	SetLastError(0);
	SetWindowLongPtrW(hwnd, GWL_EXSTYLE, val);
	if (GetLastError()) {
		THROW(Noborder::SetWndExStyle, SetWindowLongPtrW);
	}
}

RECT Noborder::GetWndRect(HWND const hwnd) {
	RECT rc = { 0 };
	if (!GetWindowRect(hwnd, &rc)) {
		THROW(Noborder::GetWndRect, GetWindowRect);
	}
	return rc;
}

void Noborder::SetWndPosSize(
	HWND const hwnd,
	PosSize const & ps,
	bool const & topMost)
{
	BOOL ok = SetWindowPos(hwnd,
		topMost ? HWND_TOPMOST : HWND_NOTOPMOST,
		ps.x, ps.y, ps.cx, ps.cy, SWP_NOACTIVATE | SWP_SHOWWINDOW);
	if (!ok) {
		THROW(Noborder::SetWndPosSize, SetWindowPos);
	}
}

void Noborder::BringWndToTop(
	HWND const hwnd,
	bool const & topMost)
{
	// To make sure 'Bring To Top' works 100%, we need to set TopMost first,
	// and disable it if we don't need it.
	BOOL ok = SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	if (ok && !topMost) {
		ok = SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
	if (!ok) {
		THROW(Noborder::BringWndToTop, SetWindowPos);
	}
}

