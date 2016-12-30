#pragma once
#include <Windows.h>
#include <tchar.h>
#include <exception>
#include <string>
#include "Noborder.h"

namespace DwmFormula {
	
	typedef HANDLE HTHUMBNAIL;
	typedef HTHUMBNAIL* PHTHUMBNAIL;

	struct DWM_THUMBNAIL_PROPERTIES {
		DWORD dwFlags;
		RECT rcDestination;
		RECT rcSource;
		BYTE opacity;
		BOOL fVisible;
		BOOL fSourceClientAreaOnly;
	};

	namespace DWM_TNP {
		enum {
			RectDestination = 0x1,
			RectSource = 0x2,
			Visible = 0x8,
			SourceClientAreaOnly = 0x10
		};
	}

	class DwmWindow; // impl Noborder::ISetAble
	class DwmWrapper; // need DwmException
	class DwmException;

	bool IsDwmSupported();
}

class DwmFormula::DwmWindow : public Noborder::ITargetSetAble {
	// m_doCheckTarget keeps CheckTargetProc thread running
	// m_metTargetChild means Unset() is caused by target's pop-up

	bool		m_isSet, m_topMost;
	bool		m_doCheckTarget, m_metTargetChild;
	std::wstring const	m_className;
	HWND		m_hwnd, m_hwndTarget;
	DWORD		m_dwExStyleTarget;
	HTHUMBNAIL	m_hthumb;

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
		PHTHUMBNAIL const phThumb) const;
	void UnregisterThumbnail(HTHUMBNAIL const) const;
	void UpdateThumbnail(
		HTHUMBNAIL const,
		DWM_THUMBNAIL_PROPERTIES const *) const;
	void UpdateThumbnail(
		HTHUMBNAIL const,
		SIZE const & srcSize,
		SIZE const & destSize) const;

};

class DwmFormula::DwmException : std::exception {
	HRESULT m_hr;
	std::string m_callee;
public:
	DwmException(HRESULT const, std::string const &);

	HRESULT GetHr() const;
	std::string const & GetCalleeName() const;
};
