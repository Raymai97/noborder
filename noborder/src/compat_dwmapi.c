#include "compat_dwmapi.h"

static FARPROC s_DwmIsCompositionEnabled;
static FARPROC s_DwmRegisterThumbnail;
static FARPROC s_DwmUnregisterThumbnail;
static FARPROC s_DwmUpdateThumbnailProperties;

EXTERN_C void InitCompat_dwmapi(
	HMODULE hMod_dwmapi)
{
#define OptProcAddr(hMod, name)  (!hMod ? 0 : GetProcAddress(hMod, name))
	s_DwmIsCompositionEnabled = OptProcAddr(hMod_dwmapi, "DwmIsCompositionEnabled");
	s_DwmRegisterThumbnail = OptProcAddr(hMod_dwmapi, "DwmRegisterThumbnail");
	s_DwmUnregisterThumbnail = OptProcAddr(hMod_dwmapi, "DwmUnregisterThumbnail");
	s_DwmUpdateThumbnailProperties = OptProcAddr(hMod_dwmapi, "DwmUpdateThumbnailProperties");
#undef OptProcAddr
}

EXTERN_C HRESULT Compat_DwmIsCompositionEnabled(
	BOOL *pfEnabled)
{
	typedef HRESULT(WINAPI *fn_t)(BOOL *);
	fn_t pfn = (fn_t)s_DwmIsCompositionEnabled;
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
	fn_t pfn = (fn_t)s_DwmRegisterThumbnail;
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
	fn_t pfn = (fn_t)s_DwmUnregisterThumbnail;
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
	fn_t pfn = (fn_t)s_DwmUpdateThumbnailProperties;
	if (pfn)
	{
		return pfn(hThumbnailId, ptnProperties);
	}
	return E_NOTIMPL;
}
