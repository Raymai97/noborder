#include "DwmFormula.h"
#ifndef THROW
#define THROW(caller, callee)	throw std::exception(\
	"at " #callee "()\n" \
	"at " #caller "()\n")
#endif

using namespace DwmFormula;
using Noborder::PosSize;

DwmWrapper const dwm;

// ... DwmWindow ... impl Noborder::ITargetSetAble

DwmWindow::DwmWindow(std::wstring const & className) :
	m_isSet(false),
	m_topMost(false),
	m_doCheckTarget(false),
	m_metTargetChild(false),
	m_className(className),
	m_hwnd(nullptr),
	m_hwndTarget(nullptr),
	m_dwExStyleTarget(0),
	m_hthumb(nullptr) {
}

DwmWindow::~DwmWindow() {
	try { this->Unset(); }
	catch (...) {}
	if (m_hwnd) {
		DestroyWindow(m_hwnd);
		UnregisterClassW(m_className.c_str(), nullptr);
	}
}

bool DwmFormula::DwmWindow::IsSet() const {
	return m_isSet;
}

DwmWindow & DwmWindow::Set(
	HWND const hwndTarget,
	Noborder::PosSize const & psNbd,
	bool const topMost)
{
	if (this->IsSet()) { return *this; }
	if (!dwm.IsSupported()) { throw Error::DwmNotSupported; }
	if (!dwm.IsEnabled()) { throw Error::DwmNotEnabled; }
	auto targetExStyle = Noborder::GetWndExStyle(hwndTarget);
	if (targetExStyle & WS_EX_LAYERED) { throw Error::TargetIsLayeredWindow; }
	m_topMost = topMost;
	m_hwndTarget = hwndTarget;
	m_dwExStyleTarget = targetExStyle;

	// Create dwmWindow if not created yet
	if (!m_hwnd) {
		WNDCLASSEXW wcex = { 0 };
		wcex.cbSize = sizeof(wcex);
		wcex.lpszClassName = m_className.c_str();
		wcex.lpfnWndProc = DwmWindow::WndProc;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
		if (!RegisterClassExW(&wcex)) {
			THROW(DwmWindow::Set, RegisterClassExW);
		}
		m_hwnd = CreateWindowExW(WS_EX_TOOLWINDOW,
			m_className.c_str(), m_className.c_str(), WS_POPUP,
			0, 0, 0, 0, nullptr, nullptr, nullptr, this);
		if (!m_hwnd) {
			THROW(DwmWindow::Set, CreateWindowExW);
		}
	}
	// Make dwmWindow looks like nobordered target
	Noborder::SetWndPosSize(m_hwnd, psNbd, topMost);
	Noborder::BringWndToTop(m_hwnd, topMost);

	// Target may be child of a window, so mimic this behavior as well
	SetWindowLongPtr(m_hwnd, GWLP_HWNDPARENT,
		reinterpret_cast<LONG_PTR>(GetParent(m_hwndTarget)));
	
	// Make target invisible...
	Noborder::SetWndExStyle(hwndTarget, targetExStyle | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwndTarget, 0, 0, LWA_ALPHA);
	SetForegroundWindow(hwndTarget); // but activated

	// DWM Magic!
	dwm.RegisterThumbnail(m_hwnd, hwndTarget, &m_hthumb);
	SIZE srcSize = Noborder::GetClientSize(hwndTarget);
	SIZE destSize = { psNbd.cx, psNbd.cy };
	dwm.UpdateThumbnail(m_hthumb, srcSize, destSize);

	// Start CheckTargetProc
	m_isSet = m_doCheckTarget = true;
	DWORD tid = 0;
	CreateThread(nullptr, 0, DwmWindow::CheckTargetProc, this, 0, &tid);
	return *this;
}

DwmWindow & DwmWindow::Unset() {
	if (this->IsSet()) {
		// Stop CheckTargetProc
		m_doCheckTarget = false;

		// Hide dwmWindow (don't destroy!)
		ShowWindow(m_hwnd, SW_HIDE);

		// Stop DWM magic (fail doesn't matter)
		try { dwm.UnregisterThumbnail(m_hthumb); }
		catch (...) {}
		m_hthumb = nullptr;

		// If target still exist
		if (IsWindow(m_hwndTarget)) {
			// Restore target
			Noborder::SetWndExStyle(m_hwndTarget, m_dwExStyleTarget);
			if (!m_metTargetChild) {
				SetForegroundWindow(m_hwndTarget);
			}
		}
		m_hwndTarget = nullptr;
		m_metTargetChild = false;
		m_isSet = false;
	}
	return *this;
}

// ... DwmWindow ... Private ...

LRESULT CALLBACK DwmWindow::WndProc(
	HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
	auto ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (msg == WM_NCCREATE) {
		auto cs = reinterpret_cast<LPCREATESTRUCT>(l);
		SetWindowLongPtr(hwnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
	}
	else if (ptr) {
		auto self = reinterpret_cast<DwmWindow*>(ptr);
		if (self->IsSet()) {
			switch (msg) {
			case WM_ACTIVATE:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
				Noborder::BringWndToTop(self->m_hwnd, self->m_topMost);
				SetForegroundWindow(self->m_hwndTarget);
				SendMessage(self->m_hwndTarget, WM_ACTIVATEAPP, 1, 0);
				break;
			case WM_DWMCOMPOSITIONCHANGED:
				self->Unset();
				break;
			}
		}
	}
	return DefWindowProc(hwnd, msg, w, l);
}

DWORD DwmWindow::CheckTargetProc(LPVOID ptr) {
	printf("CheckTargetProc started... \n");
	auto self = reinterpret_cast<DwmWindow*>(ptr);
	HWND hwndCurr = nullptr;
	HWND hwndPrev = nullptr;
	// 'we' = DwmWindow and target
	auto weWereActivated = true;
	while (self->IsSet() && self->m_doCheckTarget) {
		// If target no longer exists
		if (!IsWindowVisible(self->m_hwndTarget)) {
			self->Unset();
		}
		// Else if activated window changed
		else if ((hwndCurr = GetForegroundWindow()) != hwndPrev) {
			printf("  activated window changed \n");
			// If target minimized, DwmWindow.hide(), else .show()
			// without affecting 'activate state'
			ShowWindow(self->m_hwnd,
				IsIconic(self->m_hwndTarget) ? SW_HIDE : SW_SHOWNOACTIVATE);
			// Visibility alone is not enough, z-order matters too
			// Bring DwmWindow to top as needed
			auto weAreActivated =
				hwndCurr == self->m_hwnd ||
				hwndCurr == self->m_hwndTarget;
			if (weAreActivated && !weWereActivated) {
				weWereActivated = true;
				printf("  weAreActivated && !weWereActivated so \n");
				printf("  BringWndToTop(self->m_hwnd... \n");
				Noborder::BringWndToTop(self->m_hwnd, self->m_topMost);
			}
			else if (weWereActivated && !weAreActivated) {
				weWereActivated = false;
				if (GetParent(hwndCurr) == self->m_hwndTarget) {
					self->m_metTargetChild = true;
					self->Unset();
				}
			}
			hwndPrev = hwndCurr;
		}
		Sleep(100);
	}
	printf("... CheckTargetProc \n");
	return 0;
}

// ... DwmWrapper ...

DwmWrapper::DwmWrapper() :
	m_hmod(nullptr), m_fnICE(nullptr), m_fnRT(nullptr),
	m_fnUT(nullptr), m_fnUTP(nullptr)
{
	m_hmod = LoadLibraryW(L"dwmapi.dll");
	m_fnICE = GetProcAddress(m_hmod, "DwmIsCompositionEnabled");
	m_fnRT = GetProcAddress(m_hmod, "DwmRegisterThumbnail");
	m_fnUT = GetProcAddress(m_hmod, "DwmUnregisterThumbnail");
	m_fnUTP = GetProcAddress(m_hmod, "DwmUpdateThumbnailProperties");
}

DwmWrapper::~DwmWrapper() {
	FreeLibrary(m_hmod);
}

bool DwmWrapper::IsSupported() const {
	return m_hmod && m_fnICE && m_fnRT && m_fnUT && m_fnUTP;
}

bool DwmWrapper::IsEnabled() const {
	if (!m_fnICE) { return false; }
	typedef HRESULT(__stdcall *fnICE_t)(BOOL *pfEnabled);
	auto DwmIsCompositionEnabled = reinterpret_cast<fnICE_t>(m_fnICE);
	BOOL val = FALSE;
	auto hr = DwmIsCompositionEnabled(&val);
	if (FAILED(hr)) {
		throw DwmException(hr, "DwmIsCompositionEnabled");
	}
	return val != FALSE;
}

void DwmWrapper::RegisterThumbnail(
	HWND const hwndDest,
	HWND const hwndSrc,
	PHTHUMBNAIL const phThumb) const
{
	if (!m_fnRT) { return; }
	typedef HRESULT(__stdcall *fnRT_t)(
		HWND hwndDest, HWND hwndSrc, PHTHUMBNAIL phThumb);
	auto DwmRegisterThumbnail = reinterpret_cast<fnRT_t>(m_fnRT);
	auto hr = DwmRegisterThumbnail(hwndDest, hwndSrc, phThumb);
	if (FAILED(hr)) {
		throw DwmException(hr, "DwmRegisterThumbnail");
	}
}

void DwmWrapper::UnregisterThumbnail(
	HTHUMBNAIL const hThumb) const
{
	if (!m_fnUT) { return; }
	typedef HRESULT(__stdcall *fnUT_t)(HTHUMBNAIL hThumb);
	auto DwmUnregisterThumbnail = reinterpret_cast<fnUT_t>(m_fnUT);
	auto hr = DwmUnregisterThumbnail(hThumb);
	if (FAILED(hr)) {
		throw DwmException(hr, "DwmUnregisterThumbnail");
	}
}

void DwmWrapper::UpdateThumbnail(
	HTHUMBNAIL const hThumb,
	DWM_THUMBNAIL_PROPERTIES const *pThumbProp) const
{
	if (!m_fnUTP) { return; }
	typedef HRESULT(__stdcall *fnUTP_t)(
		HTHUMBNAIL hThumb,
		const DWM_THUMBNAIL_PROPERTIES* ptnProperties);
	auto DwmUpdateThumbnailProperties =
		reinterpret_cast<fnUTP_t>(m_fnUTP);
	auto hr = DwmUpdateThumbnailProperties(hThumb, pThumbProp);
	if (FAILED(hr)) {
		throw DwmException(hr, "DwmUpdateThumbnailProperties");
	}
}

void DwmWrapper::UpdateThumbnail(
	HTHUMBNAIL const hThumb,
	SIZE const & srcSize,
	SIZE const & destSize) const
{
	DWM_THUMBNAIL_PROPERTIES p = { 0 };
	p.dwFlags = DWM_TNP::Visible | DWM_TNP::SourceClientAreaOnly |
		DWM_TNP::RectDestination | DWM_TNP::RectSource;
	p.fSourceClientAreaOnly = true; // if false, DWM scale ugly!
	p.fVisible = true;
	// NOTE: Set rcSource explicitly to avoid weird problem
	p.rcSource = { 0, 0, srcSize.cx, srcSize.cy };
	p.rcDestination = { 0, 0, destSize.cx, destSize.cy };
	dwm.UpdateThumbnail(hThumb, &p);
}

// ... DwmException ...

DwmException::DwmException(
	HRESULT const hr,
	std::string const & callee) :
	m_hr(hr), m_callee(callee) {
}

HRESULT DwmException::GetHr() const {
	return m_hr;
}

std::string const & DwmException::GetCalleeName() const {
	return m_callee;
}

bool DwmFormula::IsDwmSupported() {
	return dwm.IsSupported();
}
