#include "_dwmapi.h"

_DWM_IsCompoEnabled DwmIsCompositionEnabled;
_DWM_RegThumb DwmRegisterThumbnail;
_DWM_UnregThumb DwmUnregisterThumbnail;
_DWM_UpdateThumbProp DwmUpdateThumbnailProperties;

bool InitDwmAPI()
{
	HMODULE h = LoadLibrary(_T("dwmapi.dll"));
	DwmIsCompositionEnabled = (_DWM_IsCompoEnabled)GetProcAddress(h, "DwmIsCompositionEnabled");
	DwmRegisterThumbnail = (_DWM_RegThumb)GetProcAddress(h, "DwmRegisterThumbnail");
	DwmUnregisterThumbnail = (_DWM_UnregThumb)GetProcAddress(h, "DwmUnregisterThumbnail");
	DwmUpdateThumbnailProperties = (_DWM_UpdateThumbProp)GetProcAddress(h, "DwmUpdateThumbnailProperties");
	return (h &&
		DwmIsCompositionEnabled &&
		DwmRegisterThumbnail &&
		DwmUnregisterThumbnail &&
		DwmUpdateThumbnailProperties);
}

