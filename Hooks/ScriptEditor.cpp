#include "ScriptEditor.h"
#include "CompilerErrorDetours.h"
#include "..\ScriptEditorAllocator.h"

struct CommandTableData;
struct FormData;
struct ScriptData;
struct OBSEInterface;
class Script;
class TESForm;
struct ScriptVarIndexData;

namespace Hooks
{
	HWND								g_ScriptEditorBuffer = NULL;			// handle to the editor dialog last processed by the WndProc
	Script*								g_EditorInitScript	=	NULL;			// must point to valid Script object to be used. needs to be reset right after dialog instantiation
	Script*								g_SetEditorTextCache = NULL;			// stores the script object from the last call of f_Script::SetEditorText
	UInt32								g_WParamBuffer		=	0;				// WParam last processed by the WndProc
	ScriptData*							g_ScriptDataInteropPackage = new ScriptData();
	const char*							g_RecompileAllScriptsStr = "Are you sure you want to recompile every script in the active plugin?";
	Script*								g_ScriptListResult = NULL;				// used by our script list hook, to set the selected script form
	const void*							g_ExpressionBuffer = new char[0x500];
	Script*								g_EditorAuxScript = NULL;
	HWND								g_EditorAuxHWND = NULL;
	UInt32								g_MaxScriptDataSize = 0x8000;

	_DefineHookHdlr(MainWindowEntryPoint, 0x0041A5F6);
	_DefineHookHdlr(ScriptableFormEntryPoint, 0x004A16AD);
	_DefineHookHdlr(ScriptEffectItemEntryPoint, 0x00566387);
	_DefineHookHdlr(LoadRelease, 0x0040D090);
	_DefineHookHdlr(EditorWindowProc, 0x004FE7AC);
	_DefineHookHdlr(EditorWindowWParam, 0x004FEC46);
	_DefineHookHdlr(RecompileScripts, 0x004FEFEA);
	_DefineHookHdlr(EditorInitScript, 0x004FEB1F);
	_DefineHookHdlr(EditorInitGetAuxScript, 0x004FEB48);
	_DefineHookHdlr(EditorInitWindowPos, 0x004FEB9A);
	_DefineHookHdlr(MessagingCallbackNewScript, 0x004FEDFD);
	_DefineHookHdlr(MessagingCallbackOpenNextScript, 0x004FEE6F);
	_DefineHookHdlr(MessagingCallbackPreviousScript, 0x004FEEDF);
	_DefineHookHdlr(MessagingCallbackClose, 0x004FED5E);
	_DefineHookHdlr(MessagingCallbackSave, 0x004FE63D);
	_DefineHookHdlr(ScriptListOpen, 0x004FEE1D);
	_DefineHookHdlr(ScriptListDelete, 0x004FF133);
	_DefineHookHdlr(SaveDialogBox, 0x004FE56D);
	_DefineHookHdlr(LogRecompileResults, 0x004FF07E);
	_DefinePatchHdlr(RecompileScriptsMessageBoxString, 0x004FEF3F);
	_DefinePatchHdlr(SaveDialogBoxType, 0x004FE558);
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

		_MemHdlr(EditorWindowProc).WriteJump();
		_MemHdlr(EditorWindowWParam).WriteJump();

		_MemHdlr(RecompileScripts).WriteJump();
		_MemHdlr(EditorInitScript).WriteJump();
		_MemHdlr(EditorInitGetAuxScript).WriteJump();
		_MemHdlr(EditorInitWindowPos).WriteJump();

		_MemHdlr(MessagingCallbackNewScript).WriteJump();
		_MemHdlr(MessagingCallbackPreviousScript).WriteJump();
		_MemHdlr(MessagingCallbackOpenNextScript).WriteJump();
		_MemHdlr(MessagingCallbackClose).WriteJump();
		_MemHdlr(MessagingCallbackSave).WriteJump();

		_MemHdlr(ScriptListOpen).WriteJump();
		_MemHdlr(ScriptListDelete).WriteJump();

		_MemHdlr(SaveDialogBox).WriteJump();
		_MemHdlr(LogRecompileResults).WriteJump();

		_MemHdlr(RecompileScriptsMessageBoxString).WriteUInt32((UInt32)g_RecompileAllScriptsStr);
		_MemHdlr(SaveDialogBoxType).WriteUInt8(3);

		_MemHdlr(MaxScriptSizeOverrideScriptBufferCtor).WriteJump();
		_MemHdlr(MaxScriptSizeOverrideParseScriptLine).WriteJump();

		PatchCompilerErrorDetours();
	}

	void FillScriptDataPackage(Script* ScriptForm)
	{
		g_ScriptDataInteropPackage->EditorID = ScriptForm->editorID.c_str();
		g_ScriptDataInteropPackage->Text = ScriptForm->text;
		g_ScriptDataInteropPackage->TypeID = TESForm::kFormType_Script;

		if (ScriptForm->IsObjectScript())
		{
															g_ScriptDataInteropPackage->Type = 0;
			if (ScriptForm->info.dataLength >= 15)
			{
				UInt8* Data = (UInt8*)ScriptForm->data;
				if (*(Data + 8) == 7)						g_ScriptDataInteropPackage->Type = 9;			// function script
			}
		}
		else if (ScriptForm->IsQuestScript())				g_ScriptDataInteropPackage->Type = 1;
		else 												g_ScriptDataInteropPackage->Type = 2;

		g_ScriptDataInteropPackage->ByteCode = ScriptForm->data;
		g_ScriptDataInteropPackage->Length = ScriptForm->info.dataLength;
		g_ScriptDataInteropPackage->FormID = ScriptForm->formID;

		g_ScriptDataInteropPackage->Flags = ScriptForm->formFlags;
	}

	void __stdcall InstantiateTabContainer(void)
	{
		if (g_EditorAuxHWND)		g_EditorAuxScript = (Script*)TESDialog_GetComboBoxSelectedItemData(GetDlgItem(g_EditorAuxHWND, 1226));
		else						g_EditorAuxScript = NULL;

		tagRECT ScriptEditorLoc;
		TESDialog_GetPositionFromINI("Script Edit", &ScriptEditorLoc);
		CLIWrapper::ScriptEditor::AllocateNewEditor(ScriptEditorLoc.left, ScriptEditorLoc.top, ScriptEditorLoc.right, ScriptEditorLoc.bottom);
		g_EditorAuxHWND = NULL;
	}

	#define _hhName		MainWindowEntryPoint
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0041A610);
		__asm
		{
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
			mov		g_EditorAuxHWND, eax
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
			mov		g_EditorAuxHWND, eax
			call	InstantiateTabContainer
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoLoadReleaseHook(void)
	{
		ScriptEditorAllocator::GetSingleton()->DestroyVanillaDialogs();
		ScriptEditorAllocator::GetSingleton()->DeleteAllTrackedEditors();
		SendPingBack(8);
	}

	#define _hhName		LoadRelease
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0040D096);
		__asm
		{
			call	EndDialogAddress
			call	[g_WindowHandleCallAddr]				// EndDialog

			pushad
			call	DoLoadReleaseHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		EditorWindowProc
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FE7B2);
		__asm
		{
			mov		[g_ScriptEditorBuffer], edi
			pushad
			call	GetWindowLongAddress
			popad

			call	[g_WindowHandleCallAddr]				// GetWindowLongA
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		EditorWindowWParam
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FED08);
		__asm
		{
			mov		[g_WParamBuffer], eax
			jg		JMPG
		JMPG:
			jmp		[_hhGetVar(Retn)]
		}
	}

	bool __stdcall DoRecompileScriptsHook(TESForm* Form)
	{
		if (Form->formFlags & TESForm::kFormFlags_FromActiveFile)
			return false;										// don't skip, script is from the active plugin
		else
			return true;										// skip
	}

	#define _hhName		RecompileScripts
	_hhBegin()
	{
		static bool bShouldSkip = false;
		_hhSetVar(Retn, 0x004FEFF1);
		_hhSetVar(Call, 0x0047A4E0);				// sub_47A4E0
		_hhSetVar(Skip, 0x004FF102);
		__asm
		{
			pushad
			push	eax
			call	DoRecompileScriptsHook
			mov		[bShouldSkip], al
			popad

			call	[_hhGetVar(Call)]
			mov		ebx, eax
			cmp		[bShouldSkip], 1
			jz		SKIP
			jmp		[_hhGetVar(Retn)]
		SKIP:
			jmp		[_hhGetVar(Skip)]
		}
	}

	bool __stdcall DoEditorInitScriptHook(void)
	{
		if (g_EditorInitScript) 
		{								// custom init script
			g_SetEditorTextCache = g_EditorInitScript;
			return true;
		}
		else
			return false;
	}

	#define _hhName		EditorInitScript
	_hhBegin()
	{
		_hhSetVar(AuxRetn, 0x004FEB28);
		_hhSetVar(CSERetn, 0x004FEB51);
		_hhSetVar(Call, 0x004FC1F0);
		__asm
		{
			call	SendMessageAddress
			call	[g_WindowHandleCallAddr]

			mov		eax, [esi + 0x24]					// HWND cbScriptList
			test	eax, eax
			jnz		AUX									// instantiation through aux entry point

			pushad
			call	DoEditorInitScriptHook
			test	eax, eax
			jnz		CUSTOM

			popad
			jmp		[_hhGetVar(CSERetn)]				// instantiation through the main window
		AUX:
			jmp		[_hhGetVar(AuxRetn)]
		CUSTOM:
			popad
			mov		ecx, g_EditorInitScript
			mov     [esi + 0x10], ecx					// copy our script to vanilla editor's userdata struct
			push	edi									// HWND ScriptEditor
			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(CSERetn)]
		}
	}

	#define _hhName		EditorInitGetAuxScript
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FEB4E);
		__asm
		{
			mov		g_SetEditorTextCache, ecx
			mov     eax, [edx + 0x114]

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		EditorInitWindowPos
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FEBB0);
		__asm
		{
			push	4
			push	0
			push	0
			push	-100
			push	-100
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall SendPingBack(UInt16 Message)
	{
		UInt32 TrackedIndex = 0;
		switch (Message)
		{
		case 8:											// pass 0 as the allocated inedx
		case 9:
		case 10:
			break;
		default:
			TrackedIndex = EDAL->GetTrackedIndex(g_ScriptEditorBuffer);
			break;
		}

		CLIWrapper::ScriptEditor::SendMessagePingback(TrackedIndex, Message);
	}

	void __stdcall DoMessagingCallbackCloseHookRelease(HWND Editor)
	{
		EDAL->DeleteTrackedEditor(EDAL->GetTrackedIndex(Editor));
	}

	#define _hhName		MessagingCallbackNewScript
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FEE0E);
		__asm
		{
			mov		g_SetEditorTextCache, ecx			// cache Script*
			call	eax

			pushad
			push	0
			call	SendPingBack
			popad

			push	0
			push	1
			push	0x0B9
			push    0x48E
			push	edi
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		MessagingCallbackOpenNextScript
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FEE75);
		__asm											// ugh!
		{
			cmp		[g_WParamBuffer], 0x9D13			// Delete
			jz		DELEC

			mov		g_SetEditorTextCache, ecx
			call	edx

			pushad
			cmp		[g_WParamBuffer], 0x9D39			// Next
			jz		NEXTC
			jmp		OPENC
		NEXTC:
			push	3
			jmp		PINGB
		OPENC:
			push	1
		PINGB:
			call	SendPingBack
			popad
			jmp		EXIT
		DELEC:
			call	edx
		EXIT:
			mov     byte ptr [esi + 0x14], 0
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		MessagingCallbackPreviousScript
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FF2D2);
		__asm
		{
			mov		g_SetEditorTextCache, ecx
			call	eax

			pushad
			push	2
			call	SendPingBack
			popad

			mov     byte ptr [esi + 0x14], 0
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		MessagingCallbackClose
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FED63);
		__asm
		{
			pushad
			push	7
			call	SendPingBack
			popad

			call	TESDialog_WritePositionToINI

			pushad
			push	edi
			call	DoMessagingCallbackCloseHookRelease
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		MessagingCallbackSave
	_hhBegin()
	{
		static Script* ScriptToBeCompiled = NULL;
		_hhSetVar(Retn, 0x004FE642);
		_hhSetVar(Call, 0x00503450);					// f_PrecompileSub()
		__asm
		{
			mov		ScriptToBeCompiled, esi
			call	[_hhGetVar(Call)]
			test	al, al
			jnz		PASS								// on a successful compile
			jmp		[_hhGetVar(Retn)]
		PASS:
			cmp		[g_WParamBuffer], 0x9CDA			// if in a save callback
			jz		GETDATA
			jmp		[_hhGetVar(Retn)]
		GETDATA:
			pushad
			mov		ecx, ScriptToBeCompiled
			mov		g_SetEditorTextCache, ecx
			push	4
			call	SendPingBack
			popad
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		ScriptListOpen
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FEE33);
		__asm
		{
			mov		eax, g_ScriptListResult
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		ScriptListDelete
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FF14B);
		__asm
		{
			mov		ebx, g_ScriptListResult
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		SaveDialogBox
	_hhBegin()
	{
		_hhSetVar(YesRetn, 0x004FE59D);
		_hhSetVar(NoRetn, 0x004FE572);
		_hhSetVar(CancelRetn, 0x004FE731);
		__asm
		{
			cmp		eax, 7
			jz		NO
			cmp		eax, 2
			jz		CANCEL

			jmp		[_hhGetVar(YesRetn)]
		NO:
			jmp		[_hhGetVar(NoRetn)]
		CANCEL:
			mov		al, 0
			jmp		[_hhGetVar(CancelRetn)]
		}
	}

	void __stdcall DoLogRecompileResultsHook(Script* Arg)
	{
		DebugPrint(Console::e_CS, "Script '%s' failed to recompile due to compile errors.", Arg->editorID.c_str());
	}

	#define _hhName		LogRecompileResults
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FF083);
		_hhSetVar(Call, 0x00503450);
		__asm
		{
			call	[_hhGetVar(Call)]
			test	al, al
			jz		LOG
			jmp		EXIT
		LOG:
			push	edi
			call	DoLogRecompileResultsHook
		EXIT:
			jmp		[_hhGetVar(Retn)]
		}
	}

	void ToggleScriptCompiling(bool Enable)
	{
		if (!Enable)
			_MemHdlr(ToggleScriptCompilingNewData).WriteBuffer();
		else
			_MemHdlr(ToggleScriptCompilingOriginalData).WriteBuffer();
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

