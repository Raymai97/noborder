#pragma once
#include <Windows.h>
#include <string>
#include <sstream>
#include <exception>
#include "NotifyIcon.h"
#include "Noborder.h"

auto const nbdAppTitle = L"noborder v1.5";
auto const nbdAppInfo =
	L"noborder v1.5 . by Raymai97 . in MSVC2015\n\n"
	L"A tool to switch current window into 'Borderless' mode.\n"
	L"By default, press ALT+BACKSPACE to toggle.";
auto const nbdMutexName = L"NOBORDER MUTEX";
auto const nbdNotifyIconClass = L"noborder NotifyIcon";
auto const nbdMsgWindowClass = L"noborder MsgWindow";
auto const nbdDwmWindowClass = L"noborder DwmWindow";
auto const nbdCfgFileName = L"noborder.config";
auto const nbdPrevInstCall = (WM_APP + 97);
auto const nbdFatalExitCode = 99;

enum MYID {
	MYID_FIRST = WM_USER,
	NBD_NOTIFYICON,
	CMI_AOT_AUTO,
	CMI_AOT_ALWAYS,
	CMI_AOT_NEVER,
	CMI_EXCL_TASKBAR,
	CMI_USE_DWM,
	CMI_HOTKEY_ALT_BACK,
	CMI_HOTKEY_WIN_BACK,
	CMI_ABOUT,
	CMI_EXIT
};

void ToggleNoborder();
bool LoadConfig();
bool SaveConfig();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK LLKeybrdProc(int, WPARAM, LPARAM);
void OnNotifyIconEvent(NotifyIcon & self, UINT msg);
void OnNbdPopupMenuItemClick(DWORD id);
HMENU CreateNbdPopupMenu();
