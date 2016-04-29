#pragma once
#include <Windows.h>
#include <shellapi.h>
#include <tchar.h>
#include "_winapi_extend.h"

class NotifyIcon
{
private:
	HINSTANCE hInst;
	LPCTSTR className;
	UINT id;
	HWND hWnd;
	UINT wmTaskbarCreated;
	// these vars change at runtime
	bool added, visible;
	HICON hIcon;
	LPTSTR szTip;
	LPTSTR szBalloonTitle, szBalloonText;
	UINT niifBalloon;
	BOOL Update(bool showInTray);
	void ShowMenu(UINT niId);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	NotifyIcon(HINSTANCE hInst, LPCTSTR className, UINT niId);
	~NotifyIcon();
	bool HwndInitFailed();
	BOOL SetTip(LPCTSTR szTip); 
	BOOL SetIcon(HICON hIcon);
	BOOL SetVisible(bool enable);
	BOOL HideBalloon();
	BOOL ShowBalloon();
	BOOL ShowBalloon(LPCTSTR szMsg, LPCTSTR szTitle, DWORD niif);
	void(*OnMenuCreating)(UINT niId, HMENU hMenu);
	void(*OnMenuItemSelected)(WORD itemId, WORD event);
};
