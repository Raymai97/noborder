#include "NotifyIcon.h"
#ifndef THROW
#define THROW(caller, callee)	throw std::exception(\
	"at " #callee "()\n" \
	"at " #caller "()\n")
#endif

#define NI_CALLBACK_MSG		(WM_USER)

NotifyIcon::NotifyIcon(UINT const id, std::wstring const & className) :
	m_id(id),
	m_className(className),
	m_taskbarCreatedMsg(RegisterWindowMessageW(L"TaskbarCreated")),
	m_hwnd(nullptr),
	m_hicon(nullptr),
	m_isVisible(false),
	m_eventHandler(nullptr) {
}

NotifyIcon::~NotifyIcon() {
	if (m_hwnd) {
		m_isVisible = false;
		try { this->Update(); }
		catch (...) {}
		DestroyWindow(m_hwnd);
		UnregisterClassW(m_className.c_str(), nullptr);
	}
}

LRESULT NotifyIcon::WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	auto ptr = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (msg == WM_NCCREATE) {
		auto cs = reinterpret_cast<CREATESTRUCT*>(l);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
	}
	else if (ptr) {
		auto self = reinterpret_cast<NotifyIcon*>(ptr);
		if (msg == NI_CALLBACK_MSG) {
			/* wparam = id, lparam = wm_xxx */
			if (self->m_eventHandler) { self->m_eventHandler(*self, l); }
		}
		else if (msg == self->m_taskbarCreatedMsg) {
			self->Update(true);
		}
	}
	return DefWindowProcW(hwnd, msg, w, l);
}

BOOL NotifyIcon::MyShell_NotifyIcon(
	NOTIFYICONDATA & ni, bool const & forceAdd) const
{
	// If this assertion failed, you forgot to call Init()
	assert(m_hwnd != nullptr);
	ni.cbSize = sizeof(ni);
	ni.hWnd = m_hwnd;
	ni.uID = m_id;
	ni.uCallbackMessage = NI_CALLBACK_MSG;
	ni.uFlags |= NIF_MESSAGE;
	// If visible, modify or add, else delete.
	static bool hasAdded = false;
	BOOL ok = Shell_NotifyIconW(
		m_isVisible ?
		(hasAdded && !forceAdd ? NIM_MODIFY : NIM_ADD) :
		NIM_DELETE, &ni);
	if (ok) { hasAdded = (m_isVisible); }
	return ok;
}

void NotifyIcon::Update(bool const & forceAdd) const {
	NOTIFYICONDATA ni = { 0 };
	ni.uFlags = NIF_TIP | NIF_ICON;
	_tcscpy_s(ni.szTip, m_tipText.c_str());
	ni.hIcon = m_hicon;
	if (!MyShell_NotifyIcon(ni, forceAdd)) {
		THROW(NotifyIcon::Update, MyShell_NotifyIcon);
	}
}

void NotifyIcon::UpdateBalloon(
	std::wstring const & text,
	std::wstring const & title,
	DWORD const & infoFlag) const
{
	NOTIFYICONDATA ni = { 0 };
	ni.uFlags = NIF_INFO;
	_tcscpy_s(ni.szInfo, text.c_str());
	_tcscpy_s(ni.szInfoTitle, title.c_str());
	ni.dwInfoFlags = infoFlag;
	if (!MyShell_NotifyIcon(ni, false)) {
		THROW(NotifyIcon::UpdateBalloon, MyShell_NotifyIcon);
	}
}

NotifyIcon& NotifyIcon::Init() {
	// Init window to listen for WM_xxx
	if (!m_hwnd) {
		WNDCLASSEXW wcex = { 0 };
		wcex.cbSize = sizeof(wcex);
		wcex.lpszClassName = m_className.c_str();
		wcex.lpfnWndProc = WndProc;
		if (!RegisterClassExW(&wcex)) {
			THROW(NotifyIcon, RegisterClassExW);
		}
		m_hwnd = CreateWindowExW(0,
			m_className.c_str(), m_className.c_str(),
			WS_OVERLAPPED, 0, 0, 0, 0, nullptr, nullptr, nullptr,
			reinterpret_cast<LPVOID>(this));
		if (!m_hwnd) {
			THROW(NotifyIcon, CreateWindowExW);
		}
	}
	return *this;
}

HWND NotifyIcon::GetHwnd() const {
	return m_hwnd;
}

bool NotifyIcon::IsVisible() const {	
	return m_isVisible;
}

NotifyIcon & NotifyIcon::SetVisible(bool const & val) {
	m_isVisible = val;
	this->Update();
	return *this;
}

bool NotifyIcon::HasIcon() const {
	return (m_hicon != nullptr);
}

NotifyIcon & NotifyIcon::SetIcon(HICON const & val) {
	m_hicon = val;
	if (m_isVisible) { this->Update(); }
	return *this;
}

std::wstring const & NotifyIcon::GetTip() const {
	return m_tipText;
}

NotifyIcon & NotifyIcon::SetTip(std::wstring const & val) {
	m_tipText = val;
	if (m_isVisible) { this->Update(); }
	return *this;
}


NotifyIcon & NotifyIcon::SetEventHandler(EventHandler const & val) {
	m_eventHandler = val;
	return *this;
}

NotifyIcon & NotifyIcon::ShowBalloon(
	std::wstring const & text,
	std::wstring const & title,
	BalloonIcon const & icon)
{
	DWORD niif =
		icon == BalloonIcon::Info ? NIIF_INFO :
		icon == BalloonIcon::Warning ? NIIF_WARNING :
		icon == BalloonIcon::Error ? NIIF_ERROR : NIIF_NONE;
	this->UpdateBalloon(text, title, niif);
	return *this;
}

NotifyIcon & NotifyIcon::HideBalloon() {
	this->UpdateBalloon(L"", L"", NIIF_NONE);
	return *this;
}
