#include "_winapi_extend.h"

_X_SetLayeredWinAttr SetLayeredWindowAttributes;

bool InitWinAPIX()
{
	HMODULE h = LoadLibrary(_T("user32.dll"));
	SetLayeredWindowAttributes = (_X_SetLayeredWinAttr)GetProcAddress(h, "SetLayeredWindowAttributes");
	return (h &&
		SetLayeredWindowAttributes);
}