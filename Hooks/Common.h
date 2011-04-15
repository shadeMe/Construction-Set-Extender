#pragma once
#include "../[Common]/CLIWrapper.h"
#include "../[Common]/HandshakeStructs.h"
#include "../WindowManager.h"
#include "../resource.h"
#include "../HallofFame.h"
#include "../CSInterop.h"
#include "../WindowManager.h"
#include "../ExtenderInternals.h"
#include "../Exports.h"
#include "obse/GameData.h"

extern FARPROC		g_WindowHandleCallAddr;

class TESForm;
class TESObjectCELL;
class TESWorldSpace;
struct FormData;
struct UseListCellItemData;
class INISetting;

using namespace	MemoryHandler;

void __stdcall GetWindowTextAddress(void);
void __stdcall CreateDialogParamAddress(void);
void __stdcall EndDialogAddress(void);
void __stdcall EnableWindowAddress(void);
void __stdcall GetWindowLongAddress(void);
void __stdcall CreateWindowExAddress(void);
void __stdcall SetWindowTextAddress(void);
void __stdcall SendDlgItemMessageAddress(void);
void __stdcall SendMessageAddress(void);
void __stdcall TrackPopupMenuAddress(void);

UInt32 __stdcall IsControlKeyDown(void);

#define _DeclareMemHdlr(Name, Comment)							extern MemHdlr		k##Name;	\
																void Name##Hook(void)
#define _DeclareNopHdlr(Name, Comment)							extern NopHdlr		k##Name;

#define _DefineHookHdlr(Name, Addr)								MemHdlr	k##Name		(##Addr, Name##Hook, 0, 0)
#define _DefineHookHdlrWithBuffer(Name, Addr)					MemHdlr	k##Name		(##Addr, Name##Hook, MakeUInt8Array
#define _DefinePatchHdlr(Name, Addr)							MemHdlr	k##Name		(##Addr, (UInt32)0, 0, 0)
#define _DefinePatchHdlrWithBuffer(Name, Addr)					MemHdlr	k##Name		(##Addr, (UInt32)0, MakeUInt8Array
#define _DefineJumpHdlr(Name, Addr, JumpAddr)					MemHdlr	k##Name		(##Addr, JumpAddr, 0, 0)
#define _DefineNopHdlr(Name, Addr, Size)						NopHdlr	k##Name		(##Addr, Size)

#define _MemoryHandler(Name)									k##Name

#define _BeginHookHdlrFn(Name)									void __declspec(naked) Name##Hook(void)
#define _DeclareHookHdlrFnVariable(Name, Variable, Address)		static UInt32 k##Name##Hook##Variable##Addr = Address

#define _HookHdlrFnVariable(Name, Variable)						k##Name##Hook##Variable##Addr