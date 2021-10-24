#pragma once
#include <Windows.h>

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED  (0x031E)
#endif

typedef HANDLE      HTHUMBNAIL;
typedef HTHUMBNAIL *PHTHUMBNAIL;
#define DWM_TNP_RECTDESTINATION                  0x00000001
#define DWM_TNP_RECTSOURCE                       0x00000002
#define DWM_TNP_OPACITY                          0x00000004
#define DWM_TNP_VISIBLE                          0x00000008
#define DWM_TNP_SOURCECLIENTAREAONLY             0x00000010

typedef struct _DWM_THUMBNAIL_PROPERTIES
{
    DWORD dwFlags;
    RECT rcDestination;
    RECT rcSource;
    BYTE opacity;
    BOOL fVisible;
    BOOL fSourceClientAreaOnly;
} DWM_THUMBNAIL_PROPERTIES, *PDWM_THUMBNAIL_PROPERTIES;

EXTERN_C HMODULE x_compat_dwmapi_hMod;

EXTERN_C HRESULT Compat_DwmIsCompositionEnabled(
    BOOL *pfEnabled);

EXTERN_C HRESULT Compat_DwmRegisterThumbnail(
    HWND hwndDestination,
    HWND hwndSource,
    PHTHUMBNAIL phThumbnailId);

EXTERN_C HRESULT Compat_DwmUnregisterThumbnail(
    HTHUMBNAIL hThumbnailId);

EXTERN_C HRESULT Compat_DwmUpdateThumbnailProperties(
    HTHUMBNAIL hThumbnailId,
    DWM_THUMBNAIL_PROPERTIES *ptnProperties);
