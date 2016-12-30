#include "Noborder.h"
#ifndef THROW
#define THROW(caller, callee)	throw std::exception(\
	"at " #callee "()\n" \
	"at " #caller "()\n")
#endif

using namespace Noborder;

// ... Target ... need PosSize, DwmWindow

Target::Target() :
	m_isUsingDwm(false),
	m_isSet(false),
	m_hwnd(nullptr),
	m_dwExStyle(0),
	m_dwStyle(0),
	m_rcWnd({ 0 }),
	m_dwmWnd(new DwmWindow()) {
}

Target::~Target() {
	try { this->Unset(); }
	catch (...) {}
}

bool Target::IsNobordered() const {
	return m_isSet;
}

bool Target::IsUsingDwm() const {
	return m_isUsingDwm;
}

HWND Target::GetHwnd() const {
	return m_hwnd;
}

Target & Target::Set(
	HWND const hwndTarget,
	AlwaysOnTopMode const & aotMode,
	bool const & excludeTaskbar,
	bool const & useDwm)
{
	if (m_isSet) { return *this; }
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
	if (useDwm) {
		m_dwmWnd->Set(hwndTarget, psNbd, topMost);
		m_isUsingDwm = true;
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
	if (m_isSet) {
		if (m_isUsingDwm) {
			m_dwmWnd->Unset();
			m_isUsingDwm = false;
		}
		else if (IsWindow(m_hwnd)) {
			bool topMost = (m_dwExStyle & WS_EX_TOPMOST) != 0;
			SetWndStyle(m_hwnd, m_dwStyle);
			SetWndExStyle(m_hwnd, m_dwExStyle);
			SetWndPosSize(m_hwnd, PosSize(m_rcWnd), topMost);
		}
		m_hwnd = nullptr;
		m_isSet = false;
	}
	return *this;
}

// ... Target ... Static Helper Functions ...

RECT Target::GetMonitorRect(
	HWND const & hwnd,
	bool const & doExcludeTaskbar)
{
	HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = { 0 };
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfoW(hmon, &mi)) {
		THROW(Noborder::GetMonitorRect, GetMonitorInfoW);
	}
	return doExcludeTaskbar ? mi.rcWork : mi.rcMonitor;
}

SIZE Target::GetClientSize(HWND const & hwnd) {
	RECT rc = { 0 };
	if (!GetClientRect(hwnd, &rc)) {
		THROW(Noborder::GetClientSize, GetClientRect);
	}
	SIZE size = { 0 };
	size.cx = rc.right - rc.left;
	size.cy = rc.bottom - rc.top;
	return size;
}

DWORD Target::GetWndStyle(HWND const & hwnd) {
	SetLastError(0);
	DWORD val = GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (GetLastError()) {
		THROW(Noborder::GetWndStyle, SetWindowLongPtrW);
	}
	return val;
}

DWORD Target::GetWndExStyle(HWND const & hwnd) {
	SetLastError(0);
	DWORD val = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
	if (GetLastError()) {
		THROW(Noborder::GetWndExStyle, SetWindowLongPtrW);
	}
	return val;
}

void Target::SetWndStyle(
	HWND const & hwnd,
	DWORD const & val)
{
	SetLastError(0);
	SetWindowLongPtrW(hwnd, GWL_STYLE, val);
	if (GetLastError()) {
		THROW(Noborder::SetWndStyle, SetWindowLongPtrW);
	}
}

void Target::SetWndExStyle(
	HWND const & hwnd,
	DWORD const & val)
{
	SetLastError(0);
	SetWindowLongPtrW(hwnd, GWL_EXSTYLE, val);
	if (GetLastError()) {
		THROW(Noborder::SetWndExStyle, SetWindowLongPtrW);
	}
}

RECT Target::GetWndRect(HWND const & hwnd) {
	RECT rc = { 0 };
	if (!GetWindowRect(hwnd, &rc)) {
		THROW(Noborder::GetWndRect, GetWindowRect);
	}
	return rc;
}

void Target::SetWndPosSize(
	HWND const & hwnd,
	PosSize const & ps,
	bool const & topMost)
{
	BOOL ok = SetWindowPos(hwnd,
		topMost ? HWND_TOPMOST : HWND_NOTOPMOST,
		ps.x, ps.y, ps.cx, ps.cy, SWP_SHOWWINDOW);
	if (!ok) {
		THROW(Noborder::SetWndPosSize, SetWindowPos);
	}
}

// ... PosSize ...

PosSize::PosSize(LONG x, LONG y, LONG cx, LONG cy) :
	x(x), y(y), cx(cx), cy(cy) {
}

PosSize::PosSize(RECT const & rc) :
	x(rc.left), y(rc.top),
	cx(rc.right - rc.left),
	cy(rc.bottom - rc.top) {
}

RECT PosSize::ToRECT() {
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

// ... DwmWindow ... need PosSize

DwmWindow::DwmWindow() :
	m_isSet(false),
	m_hwnd(nullptr),
	m_rcWnd({0}) {
}

DwmWindow::~DwmWindow() {
	try { this->Unset(); }
	catch (...) {}
}

bool DwmWindow::IsSet() const {
	return m_isSet;
}

HWND DwmWindow::GetHwnd() const {
	return m_hwnd;
}

DwmWindow & DwmWindow::Set(
	HWND const hwndTarget,
	PosSize const & ps,
	bool topMost)
{
	if (m_isSet) { return *this; }
	// TODO: Set DwmWindow
	throw Error::DwmNotSupported;
	m_isSet = true;
	return *this;
}

DwmWindow & DwmWindow::Unset() {
	if (m_isSet) {
		// TODO: Unset DwmWindow
		m_isSet = false;
	}
	return *this;
}
