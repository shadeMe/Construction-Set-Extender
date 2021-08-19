#include "Hooks-CompilerErrorDetours.h"
#include "[Common]/CLIWrapper.h"

#pragma warning(push)
#pragma warning(disable: 4005 4748)

namespace cse
{
	namespace hooks
	{
		UInt32 ScriptCompileResultBuffer = 0;	// saves the result of a compile operation so as to allow it to go on unhindered

		_DefineNopHdlr(RidScriptErrorMessageBox, 0x004FFFEC, 20);
		_DefineNopHdlr(RidUnknownFunctionCodeMessage, 0x0050310C, 5);
		_DefineHookHdlr(RerouteScriptErrors, 0x004FFF9C);
		_DefineHookHdlr(CompilerPrologReset, 0x00503330);
		_DefineHookHdlr(CompilerEpilogCheck, 0x0050341F);
		_DefineHookHdlr(ParseScriptLineOverride, 0x00503401);
		_DefineHookHdlr(CheckLineLengthLineCount, 0x0050013B);
		_DefineHookHdlr(ResultScriptErrorNotification, 0x005035EE);
		_DefineHookHdlr(MaxScriptSizeExceeded, 0x005031DB);
		_DefineHookHdlr(PrintCompilerErrorToConsoleOverride, 0x00500001 + 5);	// NOTE: since OBSE actually fixes a bug at this location by moving the entire block of code starting at that address
																				// by 5 bytes (the extra bytes eats into the alignment bytes immediately following the function, so there is no other displacement to RVAs that follow)
																				// so, we need to account for this change in our hook

		// ERROR HANDLERS
																		//  f_ScriptBuffer__ConstructLineBuffers
		DefineCompilerErrorOverrideHook(0x00502781, 0x00502791, 0xC)
		DefineCompilerErrorOverrideHook(0x00502813, 0x005027AD, 0xC)
		DefineCompilerErrorOverrideHook(0x005027D3, 0x00502824, 0xC)
		DefineCompilerErrorOverrideHook(0x005028B5, 0x00502889, 0x8)
																		// f_ScriptCompiler__CheckSyntax
		DefineCompilerErrorOverrideHook(0x00500B44, 0x00500B4C, 0x8)	// Mismatching quotes check - this one needs to return immediately , will CTD otherwise
		DefineCompilerErrorOverrideHook(0x00500B5D, 0x00500A7E, 0x8)
		DefineCompilerErrorOverrideHook(0x00500B76, 0x00500A8B, 0x8)
		DefineCompilerErrorOverrideHook(0x00500B8C, 0x00500AAB, 0xC)
		DefineCompilerErrorOverrideHook(0x00500BBE, 0x00500B11, 0x8)

		DefineCompilerErrorOverrideHook(0x00500BA5, 0x00500B11, 0x8)
		DefineCompilerErrorOverrideHook(0x00500C09, 0x00500C18, 0xC)
		DefineCompilerErrorOverrideHook(0x00500C81, 0x00500CB6, 0xC)
		DefineCompilerErrorOverrideHook(0x00500CA7, 0x00500CB6, 0x8)
																		// f_ScriptBuffer__ConstructRefVariables
		DefineCompilerErrorOverrideHook(0x00500669, 0x00500676, 0xC)
		DefineCompilerErrorOverrideHook(0x0050068F, 0x0050069E, 0xC)
																		// f_ScriptCompiler__CheckScriptBlockStructure
		DefineCompilerErrorOverrideHook(0x00500262, 0x0050024F, 0x8)
		DefineCompilerErrorOverrideHook(0x0050027D, 0x0050024F, 0x8)
		DefineCompilerErrorOverrideHook(0x00500298, 0x0050024F, 0x8)
																		// f_ScriptBuffer__CheckReferencedObjects
		DefineCompilerErrorOverrideHook(0x005001DC, 0x005001C9, 0xC)

		void PatchCompilerErrorDetours()
		{
			_MemHdlr(RidScriptErrorMessageBox).WriteNop();
			_MemHdlr(RidUnknownFunctionCodeMessage).WriteNop();
			_MemHdlr(RerouteScriptErrors).WriteJump();
			_MemHdlr(CompilerPrologReset).WriteJump();
			_MemHdlr(CompilerEpilogCheck).WriteJump();
			_MemHdlr(ParseScriptLineOverride).WriteJump();
			_MemHdlr(CheckLineLengthLineCount).WriteJump();
			_MemHdlr(ResultScriptErrorNotification).WriteJump();
			_MemHdlr(MaxScriptSizeExceeded).WriteJump();
			_MemHdlr(PrintCompilerErrorToConsoleOverride).WriteJump();

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

		void __stdcall DoRerouteScriptErrorsHook(UInt32 Line, const char* Message)
		{
			TESScriptCompiler::CompilerMessage CurrentMessage(Line, Message);

			// flag the entire operation as a failure iff the message is a vanilla/OBSE compiler error
			if (CurrentMessage.IsError())
				ScriptCompileResultBuffer = 0;

			// don't handle when compiling result scripts or recompiling
			if (!TESScriptCompiler::PreventErrorDetours)
				TESScriptCompiler::LastCompilationMessages.emplace_back(std::move(CurrentMessage));
		}

		#define _hhName		RerouteScriptErrors
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004FFFA5);
			__asm
			{
				mov     [esp + 0x18], ebx
				mov     [esp + 0x1C], bx

				lea     edx, [esp + 0x20]
				pushad
				push	edx
				push	[esi + 0x1C]
				call	DoRerouteScriptErrorsHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		CompilerEpilogCheck
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00503424);
			_hhSetVar(Call, 0x00500190);
			__asm
			{
				call	_hhGetVar(Call)
				mov		eax, ScriptCompileResultBuffer

				jmp		_hhGetVar(Retn)
			}
		}
		UInt32	MaxScriptSizeExceeded = 0;

		#define _hhName		CompilerPrologReset
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00503336);
			__asm
			{
				mov		ScriptCompileResultBuffer, 1
				mov		MaxScriptSizeExceeded, 0
				pushad
			}
			TESScriptCompiler::LastCompilationMessages.clear();
			__asm
			{
				popad
				push    ebx
				push    ebp
				mov     ebp, [esp + 0xC]

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		ParseScriptLineOverride
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0050340A);
			_hhSetVar(Call, 0x005028D0);
			_hhSetVar(Exit, 0x005033BE);
			__asm
			{
				call	_hhGetVar(Call)
				test	al, al
				jz		FAIL

				jmp		_hhGetVar(Retn)
			FAIL:
				mov		ScriptCompileResultBuffer, 0
				mov		eax, MaxScriptSizeExceeded
				test	eax, eax
				jnz		EXIT

				jmp		_hhGetVar(Retn)
			EXIT:
				jmp		_hhGetVar(Exit)
			}
		}

		#define _hhName		CheckLineLengthLineCount
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00500143);
			__asm
			{
				mov		eax, [esp + 0x18]
				add		[eax + 0x1C], 1

				add     dword ptr [esi], 1
				push    0x200

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoResultScriptErrorNotificationHook(void)
		{
			BGSEEUI->MsgBoxE(nullptr,
							 MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND|MB_OK,
							 "Result script compilation failed. Check the console for error messages.");
		}

		#define _hhName		ResultScriptErrorNotification
		_hhBegin()
		{
			_hhSetVar(Retn, 0x005035F3);
			_hhSetVar(Call, 0x00503330);
			__asm
			{
				mov		TESScriptCompiler::PreventErrorDetours, 1
				call	_hhGetVar(Call)
				mov		TESScriptCompiler::PreventErrorDetours, 0
				test	al, al
				jz		FAIL

				jmp		_hhGetVar(Retn)
			FAIL:
				pushad
				call	DoResultScriptErrorNotificationHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		MaxScriptSizeExceeded
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00502A7C);
			__asm
			{
				mov		MaxScriptSizeExceeded, 1
				push	0x0094AD6C
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		PrintCompilerErrorToConsoleOverride
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00500006 + 5);
			_hhSetVar(Call, 0x00403490);
			__asm
			{
				test	TESScriptCompiler::PrintErrorsToConsole, 1
				jz		SKIP
				call	_hhGetVar(Call)
			SKIP:
				jmp		_hhGetVar(Retn)
			}
		}
	}
}

#pragma warning(pop)