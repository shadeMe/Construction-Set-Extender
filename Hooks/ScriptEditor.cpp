#include "ScriptEditor.h"
#include "CompilerErrorDetours.h"

namespace Hooks
{
	const void*							g_ExpressionBuffer = new char[0x500];
	UInt32								g_MaxScriptDataSize = 0x8000;

	_DefineHookHdlr(MainWindowEntryPoint, 0x0041A5F6);
	_DefineHookHdlr(ScriptableFormEntryPoint, 0x004A16AD);
	_DefineHookHdlr(ScriptEffectItemEntryPoint, 0x00566387);
	_DefineHookHdlr(LoadRelease, 0x0040D090);
	_DefinePatchHdlrWithBuffer(ToggleScriptCompilingOriginalData, 0x00503450, 8, 0x6A, 0xFF, 0x68, 0x68, 0x13, 0x8C, 0, 0x64);
	_DefinePatchHdlrWithBuffer(ToggleScriptCompilingNewData, 0x00503450, 8, 0xB8, 1, 0, 0, 0, 0xC2, 8, 0);
	_DefineHookHdlr(MaxScriptSizeOverrideScriptBufferCtor, 0x004FFECB);
	_DefineHookHdlr(MaxScriptSizeOverrideParseScriptLine, 0x005031C6);

	void PatchScriptEditorHooks(void)
	{
		_MemHdlr(LoadRelease).WriteJump();

		_MemHdlr(ScriptableFormEntryPoint).WriteJump();
		_MemHdlr(ScriptEffectItemEntryPoint).WriteJump();
		_MemHdlr(MainWindowEntryPoint).WriteJump();

		_MemHdlr(MaxScriptSizeOverrideScriptBufferCtor).WriteJump();
		_MemHdlr(MaxScriptSizeOverrideParseScriptLine).WriteJump();

		PatchCompilerErrorDetours();
	}

	void __stdcall InstantiateTabContainer(HWND ScriptListComboBox)
	{
		Script* AuxScript = NULL;
		if (ScriptListComboBox)
			AuxScript = (Script*)TESComboBox::GetSelectedItemData(GetDlgItem(ScriptListComboBox, 1226));

		tagRECT ScriptEditorLoc;
		TESDialog::GetPositionFromINI("Script Edit", &ScriptEditorLoc);

		ComponentDLLInterface::ScriptData* Data = NULL;
		if (AuxScript)
		{
			Data = new ComponentDLLInterface::ScriptData(AuxScript);
		}

		CLIWrapper::Interfaces::SE->InstantiateEditor(Data, ScriptEditorLoc.left, ScriptEditorLoc.top, ScriptEditorLoc.right, ScriptEditorLoc.bottom);
	}

	#define _hhName		MainWindowEntryPoint
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0041A610);
		__asm
		{
			push	0
			call	InstantiateTabContainer
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		ScriptableFormEntryPoint
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004A16C5);
		__asm
		{
			push	eax
			call	InstantiateTabContainer
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		ScriptEffectItemEntryPoint
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0056639F);
		__asm
		{
			push	eax
			call	InstantiateTabContainer
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoLoadReleaseHook(void)
	{
		CLIWrapper::Interfaces::SE->CloseAllOpenEditors();
	}

	#define _hhName		LoadRelease
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0040D096);
		__asm
		{
			call	EndDialogAddress
			call	[g_WindowHandleCallAddr]

			pushad
			call	DoLoadReleaseHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		MaxScriptSizeOverrideScriptBufferCtor
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FFEE8);
		_hhSetVar(Call, 0x00401DA0);
		__asm
		{
			push	g_MaxScriptDataSize
			mov     ecx, 0x00A09E90
			mov     [esi + 0x1C], ebx
			mov     [esi + 4], ebx
			mov     [esi + 0x24], ebx
			call    [_hhGetVar(Call)]
			push    g_MaxScriptDataSize
			jmp		[_hhGetVar(Retn)]
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
			cmp     edx, g_MaxScriptDataSize
			jmp		[_hhGetVar(Retn)]
		}
	}
}