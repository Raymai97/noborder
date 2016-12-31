#pragma once
#include <Windows.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include "Noborder.h"

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED        0x031E
#endif

namespace DwmFormula {
	// Self-defined to avoid side-effect with/without 'dwmapi.h'
	namespace DWM {
		typedef HANDLE HTHUMBNAIL;
		typedef HTHUMBNAIL* PHTHUMBNAIL;

		struct THUMBNAIL_PROPERTIES {
			DWORD dwFlags;
			RECT rcDestination;
			RECT rcSource;
			BYTE opacity;
			BOOL fVisible;
			BOOL fSourceClientAreaOnly;
		};

		namespace TNP {
			enum {
				RectDestination = 0x1,
				RectSource = 0x2,
				Visible = 0x8,
				SourceClientAreaOnly = 0x10
			};
		}
	}

	class DwmWindow; // impl Noborder::ISetAble, need Noborder, DwmWrapper
	class DwmWrapper; // need DwmException
	class DwmException;

	bool IsDwmSupported();	

}

class DwmFormula::DwmWindow : public Noborder::ITargetSetAble {
	// m_doCheckTarget keeps CheckTargetProc thread running
	// m_metTargetChild means Unset() is caused by target's pop-up

	bool	m_isSet, m_topMost;
	bool	m_doCheckTarget, m_metTargetChild;
	HWND	m_hwnd, m_hwndTarget;
	DWORD	m_dwExStyleTarget;
	DWM::HTHUMBNAIL		m_hthumb;
	std::wstring const	m_className;

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static DWORD WINAPI CheckTargetProc(LPVOID);

public:
	DwmWindow(std::wstring const & className);
	~DwmWindow();

	enum class Error {
		Ok,
		DwmNotSupported,
		DwmNotEnabled,
		TargetIsLayeredWindow
	};

	virtual bool IsSet() const override;
	virtual DwmWindow & Set(
		HWND const hwndTarget,
		Noborder::PosSize const &,
		bool const topMost) override;
	virtual DwmWindow & Unset() override;

};

class DwmFormula::DwmWrapper {
	HMODULE m_hmod;
	FARPROC m_fnICE, m_fnRT, m_fnUT, m_fnUTP;
public:
	DwmWrapper();
	~DwmWrapper();

	bool IsSupported() const;
	bool IsEnabled() const;
	void RegisterThumbnail(
		HWND const hwndDest,
		HWND const hwndSrc,
		DWM::PHTHUMBNAIL const phThumb) const;
	void UnregisterThumbnail(
		DWM::HTHUMBNAIL const) const;
	void UpdateThumbnail(
		DWM::HTHUMBNAIL const,
		DWM::THUMBNAIL_PROPERTIES const *) const;
	void UpdateThumbnail(
		DWM::HTHUMBNAIL const,
		SIZE const & srcSize,
		SIZE const & destSize) const;

};

class DwmFormula::DwmException : public std::runtime_error {
	HRESULT m_hr;
	std::string m_callee;
	std::string m_humanErrorMsg;

public:
	DwmException(HRESULT const, std::string const &);

	HRESULT GetHr() const;
	std::string const & GetCalleeName() const;

	char const *what() const noexcept override;
};
