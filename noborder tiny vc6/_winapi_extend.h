#pragma once
#include <Windows.h>
#include <tchar.h>

bool InitWinAPIX();

// General
typedef long LONG_PTR;

// Layered window

#define WS_EX_LAYERED	0x00080000
#define LWA_ALPHA		0x02

typedef HRESULT (__stdcall *_X_SetLayeredWinAttr)(
    HWND hwnd,
    COLORREF crKey,
    BYTE bAlpha,
    DWORD dwFlags);

extern _X_SetLayeredWinAttr SetLayeredWindowAttributes;

// Save class instance to HWND

#define GetWindowLongPtrW   GetWindowLongW
#define GetWindowLongPtrA   GetWindowLongA
#define SetWindowLongPtrW   SetWindowLongW
#define SetWindowLongPtrA   SetWindowLongA
#define GWLP_USERDATA		GWL_USERDATA

#ifdef UNICODE
	#define GetWindowLongPtr  GetWindowLongPtrW
	#define SetWindowLongPtr  SetWindowLongPtrW
#else
	#define GetWindowLongPtr  GetWindowLongPtrA
	#define SetWindowLongPtr  SetWindowLongPtrA
#endif

// NotifyIcon

#define NIF_INFO	0x10
#define NIIF_INFO	0x01
#define NIIF_ERROR	0x03

typedef struct {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    CHAR szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    CHAR szInfo[256];
    union {
        UINT  uTimeout;
        UINT  uVersion;
    } DUMMYUNIONNAME;
    CHAR   szInfoTitle[64];
    DWORD dwInfoFlags;
} _X_NOTIFYICONDATAA, *_X_PNOTIFYICONDATAA;
typedef struct {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    WCHAR  szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    WCHAR  szInfo[256];
    union {
        UINT  uTimeout;
        UINT  uVersion;
    } DUMMYUNIONNAME;
    WCHAR  szInfoTitle[64];
    DWORD dwInfoFlags;
} _X_NOTIFYICONDATAW, *_X_PNOTIFYICONDATAW;

#undef Shell_NotifyIcon
#ifdef UNICODE
	typedef _X_NOTIFYICONDATAW _X_NOTIFYICONDATA;
	typedef _X_PNOTIFYICONDATAW _X_PNOTIFYICONDATA;
	#define Shell_NotifyIcon(x,y) Shell_NotifyIconW(x,(PNOTIFYICONDATA)(y))
#else
	typedef _X_NOTIFYICONDATAA _X_NOTIFYICONDATA;
	typedef _X_PNOTIFYICONDATAA _X_PNOTIFYICONDATA;
	#define Shell_NotifyIcon(x,y) Shell_NotifyIconA(x,(PNOTIFYICONDATA)(y))
#endif


