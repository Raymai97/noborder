#include "compat_dwmapi.h"

HMODULE x_compat_dwmapi_hMod;

EXTERN_C HRESULT Compat_DwmIsCompositionEnabled(
	BOOL *pfEnabled)
{
	typedef HRESULT(WINAPI *fn_t)(BOOL *);
	fn_t pfn = 0;
	if (x_compat_dwmapi_hMod)
	{
		pfn = (fn_t)GetProcAddress(x_compat_dwmapi_hMod, "DwmIsCompositionEnabled");
	}
	if (pfn)
	{
		return pfn(pfEnabled);
	}
	return E_NOTIMPL;
}

EXTERN_C HRESULT Compat_DwmRegisterThumbnail(
	HWND hwndDestination,
	HWND hwndSource,
	PHTHUMBNAIL phThumbnailId)
{
	typedef HRESULT(WINAPI *fn_t)(HWND, HWND, PHTHUMBNAIL);
	fn_t pfn = 0;
	if (x_compat_dwmapi_hMod)
	{
		pfn = (fn_t)GetProcAddress(x_compat_dwmapi_hMod, "DwmRegisterThumbnail");
	}
	if (pfn)
	{
		return pfn(hwndDestination, hwndSource, phThumbnailId);
	}
	return E_NOTIMPL;
}

EXTERN_C HRESULT Compat_DwmUnregisterThumbnail(
	HTHUMBNAIL hThumbnailId)
{
	typedef HRESULT(WINAPI *fn_t)(HTHUMBNAIL);
	fn_t pfn = 0;
	if (x_compat_dwmapi_hMod)
	{
		pfn = (fn_t)GetProcAddress(x_compat_dwmapi_hMod, "DwmUnregisterThumbnail");
	}
	if (pfn)
	{
		return pfn(hThumbnailId);
	}
	return E_NOTIMPL;
}

EXTERN_C HRESULT Compat_DwmUpdateThumbnailProperties(
	HTHUMBNAIL hThumbnailId,
	DWM_THUMBNAIL_PROPERTIES *ptnProperties)
{
	typedef HRESULT(WINAPI *fn_t)(HTHUMBNAIL, DWM_THUMBNAIL_PROPERTIES *);
	fn_t pfn = 0;
	if (x_compat_dwmapi_hMod)
	{
		pfn = (fn_t)GetProcAddress(x_compat_dwmapi_hMod, "DwmUpdateThumbnailProperties");
	}
	if (pfn)
	{
		return pfn(hThumbnailId, ptnProperties);
	}
	return E_NOTIMPL;
}
