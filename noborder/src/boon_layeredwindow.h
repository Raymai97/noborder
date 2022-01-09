#pragma once
#include <Windows.h>

typedef BOOL(CALLBACK *Boon_LayeredWindow32bppBmpRenderProc)(
    // Memory DC with 32bpp BMP selected
    HDC hdcBmp,
    // pBmpPixels[y * cxBmp + x] = some_32bit_ARGB_value;
    DWORD *pBmpPixels,
    LONG cxBmp,
    LONG cyBmp,
    void *pUser);

EXTERN_C BOOL
Boon_FullUpdateLayeredWindow32bpp(
    HWND hWnd,
    Boon_LayeredWindow32bppBmpRenderProc pfnRenderProc,
    void *pUser);
