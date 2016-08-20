#include "Hooks-ScriptEditor.h"
#include "Hooks-CompilerErrorDetours.h"

#pragma warning(push)
/*#pragma optimize("", off)*/
#pragma warning(disable: 4005 4748)

namespace cse
{
	namespace hooks
	{
		const UInt32						kMaxScriptDataSize = 0x8000;

		_DefineHookHdlr(MainWindowEntryPoint, 0x0041A5F6);
		_DefineHookHdlr(ScriptableFormEntryPoint, 0x004A16AD);
		_DefineHookHdlr(ScriptEffectItemEntryPoint, 0x00566387);
		_DefinePatchHdlrWithBuffer(ToggleScriptCompilingOriginalData, 0x00503450, 8, 0x6A, 0xFF, 0x68, 0x68, 0x13, 0x8C, 0, 0x64);
		_DefinePatchHdlrWithBuffer(ToggleScriptCompilingNewData, 0x00503450, 8, 0xB8, 1, 0, 0, 0, 0xC2, 8, 0);
		_DefineHookHdlr(MaxScriptSizeOverrideScriptBufferCtor, 0x004FFECB);
		_DefineHookHdlr(MaxScriptSizeOverrideParseScriptLine, 0x005031C6);
		_DefineHookHdlr(InitializeScriptLineBufferLFLineEnds, 0x0050006A);
		_DefineHookHdlr(ScriptCompileCheckSyntaxInvalidRef, 0x00500C6C);
		_DefineHookHdlr(ScriptCompilerWriteByteCodeCheckSetExprParentheses, 0x00502EC6);
		_DefineHookHdlr(ScriptCompilerParseIFExpression, 0x00503005);

		void PatchScriptEditorHooks(void)
		{
			_MemHdlr(ScriptableFormEntryPoint).WriteJump();
			_MemHdlr(ScriptEffectItemEntryPoint).WriteJump();
			_MemHdlr(MainWindowEntryPoint).WriteJump();
			_MemHdlr(MaxScriptSizeOverrideScriptBufferCtor).WriteJump();
			_MemHdlr(MaxScriptSizeOverrideParseScriptLine).WriteJump();
			_MemHdlr(InitializeScriptLineBufferLFLineEnds).WriteJump();
			_MemHdlr(ScriptCompileCheckSyntaxInvalidRef).WriteJump();
			_MemHdlr(ScriptCompilerWriteByteCodeCheckSetExprParentheses).WriteJump();
			_MemHdlr(ScriptCompilerParseIFExpression).WriteJump();

			PatchCompilerErrorDetours();
		}

		void __stdcall InstantiateScriptEditor(HWND ParentWindow)
		{
			Script* AuxScript = nullptr;
			if (ParentWindow)
				AuxScript = (Script*)TESComboBox::GetSelectedItemData(GetDlgItem(ParentWindow, kAssetFileButton_Script));

			TESDialog::ShowScriptEditorDialog(AuxScript);
		}

		#define _hhName		MainWindowEntryPoint
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0041A610);
			__asm
			{
				push	0
				call	InstantiateScriptEditor
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		ScriptableFormEntryPoint
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004A16C5);
			__asm
			{
				push	eax
				call	InstantiateScriptEditor
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		ScriptEffectItemEntryPoint
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0056639F);
			__asm
			{
				push	eax
				call	InstantiateScriptEditor
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		MaxScriptSizeOverrideScriptBufferCtor
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004FFEE8);
			_hhSetVar(Call, 0x00401DA0);
			__asm
			{
				push	kMaxScriptDataSize
				mov     ecx, 0x00A09E90
				mov     [esi + 0x1C], ebx
				mov     [esi + 4], ebx
				mov     [esi + 0x24], ebx
				call    _hhGetVar(Call)
				push    kMaxScriptDataSize
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		MaxScriptSizeOverrideParseScriptLine
		_hhBegin()
		{
			_hhSetVar(Retn, 0x005031D9);
			__asm
			{
				mov     eax, [edi + 0x24]
				mov     ecx, [esi + 0x40C]
				lea     edx, [eax + ecx + 0x0A]
				cmp     edx, kMaxScriptDataSize
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		InitializeScriptLineBufferLFLineEnds
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0050006F);
			_hhSetVar(JumpCR, 0x005000D1);
			_hhSetVar(JumpLF, 0x005000D5);
			__asm
			{
				cmp     bl, 0x0D	// check for '\r'
				jz      JUMPCRLF
				cmp		bl, 0x0A	// check for '\n'
				jz		JUMPLF
				jmp		_hhGetVar(Retn)
			JUMPCRLF:
				jmp		_hhGetVar(JumpCR)
			JUMPLF:
				jmp		_hhGetVar(JumpLF)
			}
		}

		#define _hhName		ScriptCompileCheckSyntaxInvalidRef
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00500C76);
			__asm
			{
				test	ecx, ecx
				jz		NOREF

				mov		eax, [ecx]
				mov		edx, [eax + 0xC8]
				call	edx
			EXIT:
				jmp		_hhGetVar(Retn)
			NOREF:
				mov		eax, [esi]
				jmp		EXIT
			}
		}

		#define _hhName		ScriptCompilerWriteByteCodeCheckSetExprParentheses
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00502ECC);
			_hhSetVar(Error, 0x00502FB2);
			__asm
			{
				jz		MISMATCH

				jmp		_hhGetVar(Retn)
			MISMATCH:
				jmp		_hhGetVar(Error)
			}
		}

		void __stdcall DoScriptCompilerParseIFExpressionHook(ScriptBuffer* Buffer)
		{
			ScriptCompileResultBuffer = 0;
			TESScriptCompiler::ShowMessage(Buffer, "Invalid condition expression.");
		}

		#define _hhName		ScriptCompilerParseIFExpression
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0050300B);
			_hhSetVar(Error, 0x00503283);
			__asm
			{
				jz		BADEXPR

				jmp		_hhGetVar(Retn)
			BADEXPR:
				pushad
				push	edi
				call	DoScriptCompilerParseIFExpressionHook
				popad

				mov		al, 1				// fix-up the result so that compilation continues
				jmp		_hhGetVar(Error)
			}
		}
	}
}

#pragma warning(pop)
/*#pragma optimize("", on)*/