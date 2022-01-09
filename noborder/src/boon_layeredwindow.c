#include "boon_layeredwindow.h"

EXTERN_C BOOL
Boon_FullUpdateLayeredWindow32bpp(
    HWND hWnd,
    Boon_LayeredWindow32bppBmpRenderProc pfnRenderProc,
    void *pUser)
{
    BOOL ok = FALSE;
    RECT rcWnd = { 0 };
    LONG cxBmp = 0;
    LONG cyBmp = 0;
    HDC hdc0 = 0;
    HDC mem_hdc = 0;
    BITMAPINFO mem_bi = { 0 };
    HBITMAP mem_hBmp = 0;
    DWORD *mem_pBmpPixels = 0;
    HGDIOBJ mem_old_hBmp = 0;
    SIZE ulwSize = { 0 };
    POINT ulwSrcPt = { 0 };
    POINT ulwDstPt = { 0 };
    BLENDFUNCTION ulwBF = { 0 };
    ok = GetWindowRect(hWnd, &rcWnd);
    if (!ok) goto eof;
    cxBmp = rcWnd.right - rcWnd.left;
    cyBmp = rcWnd.bottom - rcWnd.top;;
    hdc0 = GetDC(0);
    ok = !!hdc0;
    if (!ok) goto eof;
    mem_hdc = CreateCompatibleDC(hdc0);
    ok = !!mem_hdc;
    if (!ok) goto eof;
    mem_bi.bmiHeader.biSize = sizeof(mem_bi.bmiHeader);
    mem_bi.bmiHeader.biBitCount = 32;
    mem_bi.bmiHeader.biPlanes = 1;
    mem_bi.bmiHeader.biWidth = cxBmp;
    mem_bi.bmiHeader.biHeight = -cyBmp;
    mem_hBmp = CreateDIBSection(
        hdc0, &mem_bi, DIB_RGB_COLORS, (void **)&mem_pBmpPixels, 0, 0
    );
    ok = !!mem_hBmp && !!mem_pBmpPixels;
    if (!ok) goto eof;
    mem_old_hBmp = SelectObject(mem_hdc, mem_hBmp);
    ok = pfnRenderProc(mem_hdc, mem_pBmpPixels, cxBmp, cyBmp, pUser);
    if (!ok) goto eof;
    ulwSize.cx = cxBmp;
    ulwSize.cy = cyBmp;
    ulwDstPt.x = rcWnd.left;
    ulwDstPt.y = rcWnd.top;
    ulwBF.AlphaFormat = AC_SRC_ALPHA;
    ulwBF.BlendOp = AC_SRC_OVER;
    ulwBF.SourceConstantAlpha = 255; // to use per-pixel alpha
    ok = UpdateLayeredWindow(hWnd, hdc0, &ulwDstPt, &ulwSize,
        mem_hdc, &ulwSrcPt, 0, &ulwBF, ULW_ALPHA);
eof:
    if (mem_old_hBmp) SelectObject(mem_hdc, mem_old_hBmp);
    if (mem_hBmp) DeleteObject(mem_hBmp);
    if (hdc0) ReleaseDC(0, hdc0);
    return ok;
}
