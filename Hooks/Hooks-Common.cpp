#include "Hooks-Common.h"

namespace ConstructionSetExtender
{
	namespace Hooks
	{
		FARPROC								g_TempIATProcBuffer = NULL;			// used to call WndMgmt functions

		void __stdcall IATCacheGetWindowTextAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "GetWindowTextA");
		}
		void __stdcall IATCacheCreateDialogParamAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "CreateDialogParamA");
		}
		void __stdcall IATCacheEndDialogAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "EndDialog");
		}
		void __stdcall IATCacheEnableWindowAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "EnableWindow");
		}
		void __stdcall IATCacheGetWindowLongAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "GetWindowLongA");
		}
		void __stdcall IATCacheCreateWindowExAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "CreateWindowExA");
		}
		void __stdcall IATCacheSetWindowTextAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "SetWindowTextA");
		}
		void __stdcall IATCacheSendDlgItemMessageAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "SendDlgItemMessageA");
		}
		void __stdcall IATCacheSendMessageAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "SendMessageA");
		}
		void __stdcall IATCacheTrackPopupMenuAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "TrackPopupMenu");
		}
		void __stdcall IATCacheShowWindowAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "ShowWindow");
		}
		void __stdcall IATCacheMessageBoxAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "MessageBoxA");
		}
		void __stdcall IATCacheGetClientRectAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "GetClientRect");
		}

		UInt32 __stdcall IsControlKeyDown(void)
		{
			return GetAsyncKeyState(VK_CONTROL);
		}

		void __stdcall IATCacheInterlockedDecrementAddress( void )
		{
			HMODULE hMod = GetModuleHandle("KERNEL32.DLL");
			g_TempIATProcBuffer = GetProcAddress(hMod, "InterlockedDecrement");
		}
	}
}
