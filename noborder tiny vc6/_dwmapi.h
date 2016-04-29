#pragma once
#include <Windows.h>
#include <tchar.h>

#define WM_DWMCOMPOSITIONCHANGED		0x031E
#define DWM_TNP_RECTDESTINATION			0x00000001
#define DWM_TNP_RECTSOURCE				0x00000002
#define DWM_TNP_VISIBLE					0x00000008
#define DWM_TNP_SOURCECLIENTAREAONLY	0x00000010

typedef HANDLE HTHUMBNAIL;
typedef HTHUMBNAIL* PHTHUMBNAIL;

typedef struct _DWM_THUMBNAIL_PROPERTIES
{
    DWORD dwFlags;
    RECT rcDestination;
    RECT rcSource;
    BYTE opacity;
    BOOL fVisible;
    BOOL fSourceClientAreaOnly;
} DWM_THUMBNAIL_PROPERTIES, *PDWM_THUMBNAIL_PROPERTIES;

typedef HRESULT (__stdcall *_DWM_IsCompoEnabled)(BOOL *pfEnabled);
typedef HRESULT (__stdcall *_DWM_RegThumb)(HWND hwndDestination, HWND hwndSource, PHTHUMBNAIL phThumbnailId);
typedef HRESULT (__stdcall *_DWM_UnregThumb)(HTHUMBNAIL hThumbnailId);
typedef HRESULT (__stdcall *_DWM_UpdateThumbProp)(HTHUMBNAIL hThumbnailId, const DWM_THUMBNAIL_PROPERTIES* ptnProperties);

bool InitDwmAPI();
extern _DWM_IsCompoEnabled DwmIsCompositionEnabled;
extern _DWM_RegThumb DwmRegisterThumbnail;
extern _DWM_UnregThumb DwmUnregisterThumbnail;
extern _DWM_UpdateThumbProp DwmUpdateThumbnailProperties;
