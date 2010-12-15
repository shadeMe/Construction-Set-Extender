#pragma once

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include "windows.h"
#include "windowsx.h"
#include "atltypes.h"
#include "commctrl.h"
#include "richedit.h"
#include "shlobj.h"
#include "obse/obse_common/SafeWrite.h"

struct BaseExtraList;

#define CS_CAST(obj, from, to)								(to *)Oblivion_DynamicCast((void*)(obj), 0, RTTI_ ## from, RTTI_ ## to, 0)

void DebugPrint(const char* fmt, ...);
void DebugPrint(UInt8 source, const char* fmt, ...);
void CSEDumpClass(void * theClassPtr, UInt32 nIntsToDump = 512);
void WaitUntilDebuggerAttached();
UInt8*	MakeUInt8Array(UInt32 Size, ...);
void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]
void LogWinAPIErrorMessage(DWORD ErrorID);
void DumpExtraDataList(BaseExtraList* List);

namespace MemoryHandler
{
	class Handler_Nop
	{
		UInt32				m_Address;
		UInt32				m_Size;
	public:
		Handler_Nop(UInt32 PatchAddr, UInt32 Size) : m_Address(PatchAddr), m_Size(Size) {}

		void				WriteNop();
	};
	typedef Handler_Nop NopHdlr;

	class Handler_Ace
	{
		UInt32				m_AddressA;
		UInt32				m_AddressB;
		UInt8*				m_Buffer;
		UInt32				m_BufferSize;
	public:
		Handler_Ace(UInt32 HookAddr, UInt32 JumpAddr, UInt8* Buffer, UInt32 BufferSize) : m_AddressA(HookAddr), m_AddressB(JumpAddr), m_Buffer(Buffer), m_BufferSize(BufferSize) {}
		Handler_Ace(UInt32 HookAddr, void* JumpAddr, UInt8* Buffer, UInt32 BufferSize) : m_AddressA(HookAddr), m_AddressB((UInt32)JumpAddr), m_Buffer(Buffer), m_BufferSize(BufferSize) {}

		void				WriteJump();
		void				WriteCall();

		void				WriteBuffer();
		void				WriteUInt32(UInt32 Data);
		void				WriteUInt16(UInt16 Data);
		void				WriteUInt8(UInt8 Data);
	};
	typedef Handler_Ace MemHdlr;
}


#pragma region Executable Code Handlers
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
#pragma endregion
