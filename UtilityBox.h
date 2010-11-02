#pragma once

#include <string>
#include <fstream>
#include <map>
#include "windows.h"
#include "windowsx.h"
#include "atltypes.h"
#include "commctrl.h"
#include "richedit.h"
#include "shlobj.h"

void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]
void LogWinAPIErrorMessage(DWORD ErrorID);

#define PLACE_HOOK(name)									WriteRelJump(k##name##HookAddr, (UInt32)##name##Hook)

class Console
{
	static Console*				Singleton;

	Console();

	HWND						WindowHandle;
	HWND						EditHandle;
	bool						DisplayState;
	std::string					MessageBuffer;
	std::fstream				DebugLog;
	UInt32						IndentLevel;
public:
	static Console*				GetSingleton();

	enum MessageSource
	{
		e_CSE = 0,
		e_CS,
		e_BE,
		e_UL,
		e_SE,
		e_BSA
	};

	void						InitializeConsole();
	void						InitializeLog(const char* AppPath);
	bool						IsHidden() { return DisplayState == 0; }
	bool						IsConsoleInitalized() { return WindowHandle != 0; }
	bool						IsLogInitalized() { return DebugLog.is_open(); }
	bool						ToggleDisplayState();
	void						LoadINISettings();
	void						SaveINISettings();
	HWND						GetWindowHandle() { return WindowHandle; }		

	void						LogMessage(UInt8 Source, const char* Format, va_list Args);
	UInt32						Indent();
	UInt32						Exdent();
	void						ExdentAll() { IndentLevel = 0; }
	void						Clear();
};

#define CONSOLE					Console::GetSingleton()

void DebugPrint(const char* fmt, ...);
void DebugPrint(UInt8 source, const char* fmt, ...);

struct NopData
{
	UInt32				Address;
	UInt8				Size;
};

void DoNop(const NopData* Data);

void CSEDumpClass(void * theClassPtr, UInt32 nIntsToDump = 512);





// __thisCall handlers (non-virtual)
template <typename Tthis>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(); } u = {addr};
    return ((T*)_this->*u.m)();
}
template <typename Tthis, typename T1>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1); } u = {addr};
    return ((T*)_this->*u.m)(arg1);
}
template <typename Tthis, typename T1, typename T2>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1, T2 arg2)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2); } u = {addr};
    return ((T*)_this->*u.m)(arg1, arg2);
}
template <typename Tthis, typename T1, typename T2, typename T3>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1, T2 arg2, T3 arg3)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3); } u = {addr};
    return ((T*)_this->*u.m)(arg1, arg2, arg3);
}
template <typename Tthis, typename T1, typename T2, typename T3, typename T4>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3, T4); } u = {addr};
    return ((T*)_this->*u.m)(arg1, arg2, arg3, arg4);
}
template <typename Tthis, typename T1, typename T2, typename T3, typename T4, typename T5>
__forceinline UInt32 thisCall(UInt32 addr, Tthis _this, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
	if (!addr) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3, T4, T5); } u = {addr};
    return ((T*)_this->*u.m)(arg1, arg2, arg3, arg6, arg5);
}

// __thisCall handlers (virtual)
template <typename Tthis>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)();
}
template <typename Tthis, typename T1>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1);
}
template <typename Tthis, typename T1, typename T2>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1, T2 arg2)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1, arg2);
}
template <typename Tthis, typename T1, typename T2, typename T3>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1, T2 arg2, T3 arg3)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1, arg2, arg3);
}
template <typename Tthis, typename T1, typename T2, typename T3, typename T4>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3, T4); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1, arg2, arg3, arg4);
}
template <typename Tthis, typename T1, typename T2, typename T3, typename T4, typename T5>
__forceinline UInt32 thisVirtualCall(UInt32 vtbl, UInt32 offset, Tthis _this, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
	if (!vtbl) return 0; 
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1, T2, T3, T4, T5); } u = {*(UInt32*)(vtbl + offset)};
    return ((T*)_this->*u.m)(arg1, arg2, arg3, arg6, arg5);
}
