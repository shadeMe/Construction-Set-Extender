#include "ExtenderInternals.h"
#include "CompilerErrorDetours.h"
#include "[Common]/CLIWrapper.h"
#include "..\ScriptEditorAllocator.h"

#pragma warning (disable : 4005)

namespace Hooks
{
	UInt32								g_CompileResultBuffer = 0;				// saves the result of a compile operation so as to allow it to go on unhindered
	UInt32								g_CompileCallerAddr = 0;				// location of the call to Script::Compile
	bool								g_RecompilingScripts = false;			// flag set during a recompile operation

	NopHdlr								kRidScriptErrorMessageBox			(0x004FFFEC, 20);
	MemHdlr								kRerouteScriptErrors				(0x004FFF9C, RerouteScriptErrorsHook, 0, 0);
	MemHdlr								kCompilerPrologReset				(0x00503330, CompilerPrologResetHook, 0, 0);
	MemHdlr								kCompilerEpilogCheck				(0x0050341F, CompilerEpilogCheckHook, 0, 0);
	MemHdlr								kParseScriptLineOverride			(0x00503401, ParseScriptLineOverride, 0, 0);
	MemHdlr								kCheckLineLengthLineCount			(0x0050013B, CheckLineLengthLineCountHook, 0, 0);
	NopHdlr								kRidUnknownFunctionCodeMessage		(0x0050310C, 5);

	void __stdcall DoRerouteScriptErrorsHook(UInt32 Line, const char* Message)
	{
		if (g_CompileCallerAddr &&
			g_CompileCallerAddr != 0x005035EE &&			// don't handle when compiling result scripts or recompiling
			g_RecompilingScripts == false)
		{
			CLIWrapper::ScriptEditor::PassScriptError(Line, Message, EDAL->GetLastContactedEditor());
		}
	}

	void __declspec(naked) RerouteScriptErrorsHook(void)
	{
		static const UInt32			kRerouteScriptErrorsRetnAddr = 0x004FFFA5;
		__asm
		{
			mov     [esp + 0x18], ebx
			mov     [esp + 0x1C], bx

			mov		g_CompileResultBuffer, 0
			lea     edx, [esp + 0x20]
			pushad
			push	edx
			push	[esi + 0x1C]
			call	DoRerouteScriptErrorsHook
			popad

			jmp		[kRerouteScriptErrorsRetnAddr]
		}
	}

	void __declspec(naked) CompilerEpilogCheckHook(void)
	{
		static const UInt32			kCompilerEpilogCheckRetnAddr = 0x00503424;
		static const UInt32			kCompilerEpilogCheckCallAddr = 0x00500190;
		__asm
		{
			call	kCompilerEpilogCheckCallAddr
			mov		eax, g_CompileResultBuffer

			jmp		[kCompilerEpilogCheckRetnAddr]
		}
	}

	void __declspec(naked) CompilerPrologResetHook(void)
	{
		static const UInt32			kCompilerPrologResetRetnAddr = 0x00503336;

		__asm
		{
			mov		g_CompileResultBuffer, 1
			mov		eax, [esp]
			sub		eax, 5
			mov		g_CompileCallerAddr, eax

			push    ebx
			push    ebp
			mov     ebp, [esp + 0xC]

			jmp		[kCompilerPrologResetRetnAddr]
		}
	}

	void __declspec(naked) ParseScriptLineOverride(void)
	{
		static const UInt32			kParseScriptLineOverrideCallAddr = 0x005028D0;
		static const UInt32			kParseScriptLineOverrideRetnAddr = 0x0050340A;

		__asm
		{
			call	kParseScriptLineOverrideCallAddr
			test	al, al
			jz		FAIL

			jmp		kParseScriptLineOverrideRetnAddr
		FAIL:
			mov		g_CompileResultBuffer, 0
			jmp		kParseScriptLineOverrideRetnAddr
		}
	}

	void __declspec(naked) CheckLineLengthLineCountHook(void)
	{
		static const UInt32			kCheckLineLengthLineCountHookRetnAddr = 0x00500143;

		__asm
		{
			mov		eax, [esp + 0x18]
			add		[eax + 0x1C], 1

			add     dword ptr [esi], 1
			push    0x200

			jmp		kCheckLineLengthLineCountHookRetnAddr
		}
	}

	// ERROR HANDLERS

																	//  f_ConstructLineBuffer
	DefineCompilerErrorOverrideHook(0x00502781, 0x00502791, 0xC)
	DefineCompilerErrorOverrideHook(0x00502813, 0x005027AD, 0xC)
	DefineCompilerErrorOverrideHook(0x005027D3, 0x00502824, 0xC)
	DefineCompilerErrorOverrideHook(0x005028B5, 0x00502889, 0x8)
																	// f_ParseScriptToken
	DefineCompilerErrorOverrideHook(0x00500B44, 0x00500A14, 0x8)
	DefineCompilerErrorOverrideHook(0x00500B5D, 0x00500A7E, 0x8)
	DefineCompilerErrorOverrideHook(0x00500B76, 0x00500A8B, 0x8)
	DefineCompilerErrorOverrideHook(0x00500B8C, 0x00500AAB, 0xC)
	DefineCompilerErrorOverrideHook(0x00500BBE, 0x00500B11, 0x8)

	DefineCompilerErrorOverrideHook(0x00500BA5, 0x00500B11, 0x8)
	DefineCompilerErrorOverrideHook(0x00500C09, 0x00500C18, 0xC)
	DefineCompilerErrorOverrideHook(0x00500C81, 0x00500CB6, 0xC)
	DefineCompilerErrorOverrideHook(0x00500CA7, 0x00500CB6, 0x8)
																	// sub_5004C0
	DefineCompilerErrorOverrideHook(0x00500669, 0x00500676, 0xC)
	DefineCompilerErrorOverrideHook(0x0050068F, 0x0050069E, 0xC)
																	// f_ValidateScriptBlocks
	DefineCompilerErrorOverrideHook(0x00500262, 0x0050024F, 0x8)
	DefineCompilerErrorOverrideHook(0x0050027D, 0x0050024F, 0x8)
	DefineCompilerErrorOverrideHook(0x00500298, 0x0050024F, 0x8)
																	// f_ScriptBuffer__CheckReferencedObjects
	DefineCompilerErrorOverrideHook(0x005001DC, 0x005001C9, 0xC)

	void PatchCompilerErrorDetours()
	{
		kRidScriptErrorMessageBox.WriteNop();
		kRerouteScriptErrors.WriteJump();
		kCompilerPrologReset.WriteJump();
		kCompilerEpilogCheck.WriteJump();
		kParseScriptLineOverride.WriteJump();
		kCheckLineLengthLineCount.WriteJump();
		kRidUnknownFunctionCodeMessage.WriteNop();

		GetErrorMemHdlr(0x00502781).WriteJump();
		GetErrorMemHdlr(0x00502813).WriteJump();
		GetErrorMemHdlr(0x005027D3).WriteJump();
		GetErrorMemHdlr(0x005028B5).WriteJump();

		GetErrorMemHdlr(0x00500B44).WriteJump();
		GetErrorMemHdlr(0x00500B5D).WriteJump();
		GetErrorMemHdlr(0x00500B76).WriteJump();
		GetErrorMemHdlr(0x00500B8C).WriteJump();
		GetErrorMemHdlr(0x00500BBE).WriteJump();

		GetErrorMemHdlr(0x00500BA5).WriteJump();
		GetErrorMemHdlr(0x00500C09).WriteJump();
		GetErrorMemHdlr(0x00500C81).WriteJump();
		GetErrorMemHdlr(0x00500CA7).WriteJump();

		GetErrorMemHdlr(0x00500669).WriteJump();
		GetErrorMemHdlr(0x0050068F).WriteJump();

		GetErrorMemHdlr(0x00500262).WriteJump();
		GetErrorMemHdlr(0x0050027D).WriteJump();
		GetErrorMemHdlr(0x00500298).WriteJump();

		GetErrorMemHdlr(0x005001DC).WriteJump();
	}
}