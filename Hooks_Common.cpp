#include "Hooks_Common.h"

FARPROC								g_WindowHandleCallAddr = NULL;			// used to call WndMgmt functions

void __stdcall GetWindowTextAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "GetWindowTextA");
}
void __stdcall CreateDialogParamAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "CreateDialogParamA");
}
void __stdcall EndDialogAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "EndDialog");
}
void __stdcall EnableWindowAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "EnableWindow");
}
void __stdcall GetWindowLongAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "GetWindowLongA");
}
void __stdcall CreateWindowExAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "CreateWindowExA");
}
void __stdcall SetWindowTextAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "SetWindowTextA");
}
void __stdcall SendDlgItemMessageAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "SendDlgItemMessageA");
}
void __stdcall SendMessageAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "SendMessageA");
}
void __stdcall TrackPopupMenuAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "TrackPopupMenu");
}