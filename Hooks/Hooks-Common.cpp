#include "Hooks-Common.h"

namespace ConstructionSetExtender
{
	namespace Hooks
	{
		FARPROC								IATProcBuffer = NULL;			// used to call WndMgmt functions

		void __stdcall IATCacheGetWindowTextAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "GetWindowTextA");
		}
		void __stdcall IATCacheCreateDialogParamAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "CreateDialogParamA");
		}
		void __stdcall IATCacheEndDialogAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "EndDialog");
		}
		void __stdcall IATCacheEnableWindowAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "EnableWindow");
		}
		void __stdcall IATCacheGetWindowLongAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "GetWindowLongA");
		}
		void __stdcall IATCacheCreateWindowExAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "CreateWindowExA");
		}
		void __stdcall IATCacheSetWindowTextAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "SetWindowTextA");
		}
		void __stdcall IATCacheSendDlgItemMessageAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "SendDlgItemMessageA");
		}
		void __stdcall IATCacheSendMessageAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "SendMessageA");
		}
		void __stdcall IATCacheTrackPopupMenuAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "TrackPopupMenu");
		}
		void __stdcall IATCacheShowWindowAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "ShowWindow");
		}
		void __stdcall IATCacheMessageBoxAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "MessageBoxA");
		}
		void __stdcall IATCacheGetClientRectAddress(void)
		{
			HMODULE hMod = GetModuleHandle("USER32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "GetClientRect");
		}

		UInt32 __stdcall IsControlKeyDown(void)
		{
			return GetAsyncKeyState(VK_CONTROL);
		}

		void __stdcall IATCacheInterlockedDecrementAddress( void )
		{
			HMODULE hMod = GetModuleHandle("KERNEL32.DLL");
			IATProcBuffer = GetProcAddress(hMod, "InterlockedDecrement");
		}
	}
}
