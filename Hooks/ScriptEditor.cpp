#include "ScriptEditor.h"
#include "CompilerErrorDetours.h"

struct CommandTableData;
struct FormData;
struct ScriptData;
struct OBSEInterface;
class Script;
class TESForm;
struct ScriptVarIndexData;

HWND								g_ScriptEditorBuffer = NULL;			// handle to the editor dialog being processed by the WndProc
Script*								g_EditorInitScript	=	NULL;			// must point to valid Script object to be used. needs to be reset right after dialog instantiation
Script*								g_SetEditorTextCache = NULL;			// stores the script object from the last call of f_Script::SetEditorText
UInt32								g_WParamBuffer		=	0;				// WParam processed by the WndProc
ScriptData*							g_ScriptDataPackage = new ScriptData();
const char*							g_RecompileAllScriptsStr = "Are you sure you want to recompile every script in the active plugin?";
Script*								g_ScriptListResult = NULL;				// used by our script list hook, to set the selected script form
const void*							g_ExpressionBuffer = new char[0x400];
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
_DefineHookHdlr(MessagingCallbackClose, 0x004FED69);
_DefineHookHdlr(MessagingCallbackSave, 0x004FE63D);
_DefineHookHdlr(ScriptListOpen, 0x004FEE1D);
_DefineHookHdlr(ScriptListDelete, 0x004FF133);
_DefineHookHdlr(SaveDialogBox, 0x004FE56D);
_DefineHookHdlr(LogRecompileResults, 0x004FF07E);
_DefinePatchHdlr(RecompileScriptsMessageBoxString, 0x004FEF3F);
_DefinePatchHdlr(SaveDialogBoxType, 0x004FE558);
_DefinePatchHdlrWithBuffer(ToggleScriptCompilingOriginalData, 0x00503450)(8, 0x6A, 0xFF, 0x68, 0x68, 0x13, 0x8C, 0, 0x64), 8);
_DefinePatchHdlrWithBuffer(ToggleScriptCompilingNewData, 0x00503450)(8, 0xB8, 1, 0, 0, 0, 0xC2, 8, 0), 8);
_DefineHookHdlr(MaxScriptSizeOverrideScriptBufferCtor, 0x004FFECB);
_DefineHookHdlr(MaxScriptSizeOverrideParseScriptLine, 0x005031C6);


void PatchScriptEditorHooks(void)
{
	_MemoryHandler(LoadRelease).WriteJump();

	_MemoryHandler(ScriptableFormEntryPoint).WriteJump();	
	_MemoryHandler(ScriptEffectItemEntryPoint).WriteJump();	
	_MemoryHandler(MainWindowEntryPoint).WriteJump();	

	_MemoryHandler(EditorWindowProc).WriteJump();
	_MemoryHandler(EditorWindowWParam).WriteJump();

	_MemoryHandler(RecompileScripts).WriteJump();
	_MemoryHandler(EditorInitScript).WriteJump();
	_MemoryHandler(EditorInitGetAuxScript).WriteJump();
	_MemoryHandler(EditorInitWindowPos).WriteJump();

	_MemoryHandler(MessagingCallbackNewScript).WriteJump();
	_MemoryHandler(MessagingCallbackPreviousScript).WriteJump();
	_MemoryHandler(MessagingCallbackOpenNextScript).WriteJump();
	_MemoryHandler(MessagingCallbackClose).WriteJump();
	_MemoryHandler(MessagingCallbackSave).WriteJump();

	_MemoryHandler(ScriptListOpen).WriteJump();
	_MemoryHandler(ScriptListDelete).WriteJump();	

	_MemoryHandler(SaveDialogBox).WriteJump();
	_MemoryHandler(LogRecompileResults).WriteJump();
		
	_MemoryHandler(RecompileScriptsMessageBoxString).WriteUInt32((UInt32)g_RecompileAllScriptsStr);
	_MemoryHandler(SaveDialogBoxType).WriteUInt8(3);

	_MemoryHandler(MaxScriptSizeOverrideScriptBufferCtor).WriteJump();
	_MemoryHandler(MaxScriptSizeOverrideParseScriptLine).WriteJump();

	PatchCompilerErrorDetours();
}


void FillScriptDataPackage(Script* ScriptForm)
{
	g_ScriptDataPackage->EditorID = ScriptForm->editorData.editorID.m_data;
	g_ScriptDataPackage->Text = ScriptForm->text;
	g_ScriptDataPackage->TypeID = kFormType_Script;

	if (ScriptForm->IsObjectScript()) {
														g_ScriptDataPackage->Type = 0;
		if (ScriptForm->info.dataLength >= 15) {
			UInt8* Data = (UInt8*)ScriptForm->data;
			if (*(Data + 8) == 7)						g_ScriptDataPackage->Type = 9;			// function script
		}
	}
	else if (ScriptForm->IsQuestScript())				g_ScriptDataPackage->Type = 1;
	else 												g_ScriptDataPackage->Type = 2;

	g_ScriptDataPackage->ByteCode = ScriptForm->data;
	g_ScriptDataPackage->Length = ScriptForm->info.dataLength;
	g_ScriptDataPackage->FormID = ScriptForm->refID;

	g_ScriptDataPackage->Flags = ScriptForm->flags;
}

void __stdcall InstantiateTabContainer(void)
{
	if (g_EditorAuxHWND)		g_EditorAuxScript = (Script*)GetComboBoxItemData(GetDlgItem(g_EditorAuxHWND, 1226));
	else						g_EditorAuxScript = NULL;

	tagRECT ScriptEditorLoc;
	GetPositionFromINI("Script Edit", &ScriptEditorLoc);
	CLIWrapper::ScriptEditor::AllocateNewEditor(ScriptEditorLoc.left, ScriptEditorLoc.top, ScriptEditorLoc.right, ScriptEditorLoc.bottom);
	g_EditorAuxHWND = NULL;
}
	
_BeginHookHdlrFn(MainWindowEntryPoint)
{
	_DeclareHookHdlrFnVariable(MainWindowEntryPoint, Retn, 0x0041A610);	
    __asm
    {
		call	InstantiateTabContainer
		jmp		[_HookHdlrFnVariable(MainWindowEntryPoint, Retn)]
    }
}


_BeginHookHdlrFn(ScriptableFormEntryPoint)			
{
	_DeclareHookHdlrFnVariable(ScriptableFormEntryPoint, Retn, 0x004A16C5);	
    __asm
    {
		mov		g_EditorAuxHWND, eax
		call	InstantiateTabContainer
		jmp		[_HookHdlrFnVariable(ScriptableFormEntryPoint, Retn)]
    }
} 

_BeginHookHdlrFn(ScriptEffectItemEntryPoint)			
{
	_DeclareHookHdlrFnVariable(ScriptEffectItemEntryPoint, Retn, 0x0056639F);	
    __asm
    {
		mov		g_EditorAuxHWND, eax
		call	InstantiateTabContainer
		jmp		[_HookHdlrFnVariable(ScriptEffectItemEntryPoint, Retn)]
    }
} 



void __stdcall DoLoadReleaseHook(void)
{
	EditorAllocator::GetSingleton()->DestroyVanillaDialogs();
	EditorAllocator::GetSingleton()->DeleteAllTrackedEditors();
	SendPingBack(8);
}

_BeginHookHdlrFn(LoadRelease)
{
	_DeclareHookHdlrFnVariable(LoadRelease, Retn, 0x0040D096);	
    __asm
    {
		call	EndDialogAddress	
		call	[g_WindowHandleCallAddr]				// EndDialog

		pushad	
		call	DoLoadReleaseHook
		popad

		jmp		[_HookHdlrFnVariable(LoadRelease, Retn)]
    }
} 



_BeginHookHdlrFn(EditorWindowProc)
{
	_DeclareHookHdlrFnVariable(EditorWindowProc, Retn, 0x004FE7B2);	
    __asm
    {
		mov		[g_ScriptEditorBuffer], edi
		pushad
		call	GetWindowLongAddress
		popad

		call	[g_WindowHandleCallAddr]				// GetWindowLongA
		jmp		[_HookHdlrFnVariable(EditorWindowProc, Retn)]
    }
} 

_BeginHookHdlrFn(EditorWindowWParam)
{
	_DeclareHookHdlrFnVariable(EditorWindowWParam, Retn, 0x004FED08);	
    __asm
    {
		mov		[g_WParamBuffer], eax
		jg		JMPG
	JMPG:
		jmp		[_HookHdlrFnVariable(EditorWindowWParam, Retn)]
    }
} 

bool __stdcall DoRecompileScriptsHook(TESForm* Form)
{
	if (Form->flags & TESForm::kFormFlags_FromActiveFile)
		return false;										// don't skip, script is from the active plugin
	else
		return true;										// skip
}

_BeginHookHdlrFn(RecompileScripts)
{
	static bool bShouldSkip = false;
	_DeclareHookHdlrFnVariable(RecompileScripts, Retn, 0x004FEFF1);
	_DeclareHookHdlrFnVariable(RecompileScripts, Call, 0x0047A4E0);				// sub_47A4E0
	_DeclareHookHdlrFnVariable(RecompileScripts, Skip, 0x004FF102);
    __asm
    {
		pushad
		push	eax
		call	DoRecompileScriptsHook
		mov		[bShouldSkip], al
		popad

		call	[_HookHdlrFnVariable(RecompileScripts, Call)]
		mov		ebx, eax
		cmp		[bShouldSkip], 1
		jz		SKIP
		jmp		[_HookHdlrFnVariable(RecompileScripts, Retn)]	
	SKIP:
		jmp		[_HookHdlrFnVariable(RecompileScripts, Skip)]
    }
}

bool __stdcall DoEditorInitScriptHook(void)
{
	if (g_EditorInitScript) {						// custom init script
		g_SetEditorTextCache = g_EditorInitScript;
		return true;		
	} else
		return false;
}

_BeginHookHdlrFn(EditorInitScript)
{
	_DeclareHookHdlrFnVariable(EditorInitScript, AuxRetn, 0x004FEB28);	
	_DeclareHookHdlrFnVariable(EditorInitScript, CSERetn, 0x004FEB51);	
	_DeclareHookHdlrFnVariable(EditorInitScript, Call, 0x004FC1F0);
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
		jnz		CSE

		popad
		jmp		[_HookHdlrFnVariable(EditorInitScript, CSERetn)]		// instantiation through the main window
	AUX:
		jmp		[_HookHdlrFnVariable(EditorInitScript, AuxRetn)]
	CSE:
		popad
		mov		ecx, g_EditorInitScript
		mov     [esi + 0x10], ecx					// copy our script to vanilla editor's userdata struct
		push	edi									// HWND ScriptEditor
		call	[_HookHdlrFnVariable(EditorInitScript, Call)]
		jmp		[_HookHdlrFnVariable(EditorInitScript, CSERetn)]
	}
}

_BeginHookHdlrFn(EditorInitGetAuxScript)
{
	_DeclareHookHdlrFnVariable(EditorInitGetAuxScript, Retn, 0x004FEB4E);	
	__asm
	{
		mov		g_SetEditorTextCache, ecx
		mov     eax, [edx + 0x114]     

		jmp		[_HookHdlrFnVariable(EditorInitGetAuxScript, Retn)]
	}
}


_BeginHookHdlrFn(EditorInitWindowPos)
{
	_DeclareHookHdlrFnVariable(EditorInitWindowPos, Retn, 0x004FEBB0);
	__asm
	{
		push	4
		push	0
		push	0
		push	-100
		push	-100
		jmp		[_HookHdlrFnVariable(EditorInitWindowPos, Retn)]
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

_BeginHookHdlrFn(MessagingCallbackNewScript)
{
	_DeclareHookHdlrFnVariable(MessagingCallbackNewScript, Retn, 0x004FEE0E);	
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
		jmp		[_HookHdlrFnVariable(MessagingCallbackNewScript, Retn)]
	}
}

_BeginHookHdlrFn(MessagingCallbackOpenNextScript)
{			
	_DeclareHookHdlrFnVariable(MessagingCallbackOpenNextScript, Retn, 0x004FEE75);	
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
		jmp		[_HookHdlrFnVariable(MessagingCallbackOpenNextScript, Retn)]
	}
}

_BeginHookHdlrFn(MessagingCallbackPreviousScript)
{
	_DeclareHookHdlrFnVariable(MessagingCallbackPreviousScript, Retn, 0x004FF2D2);	
	__asm
	{
		mov		g_SetEditorTextCache, ecx
		call	eax

		pushad
		push	2
		call	SendPingBack		
		popad

		mov     byte ptr [esi + 0x14], 0
		jmp		[_HookHdlrFnVariable(MessagingCallbackPreviousScript, Retn)]
	}
}

_BeginHookHdlrFn(MessagingCallbackClose)
{
	_DeclareHookHdlrFnVariable(MessagingCallbackClose, Retn, 0x004FED6F);	
	__asm
	{
		call	EndDialogAddress
		call	[g_WindowHandleCallAddr]

		pushad
		push	7
		call	SendPingBack

		push	edi
		call	DoMessagingCallbackCloseHookRelease
		popad
		jmp		[_HookHdlrFnVariable(MessagingCallbackClose, Retn)]
	}
}

_BeginHookHdlrFn(MessagingCallbackSave)			
{
	static Script* ScriptToBeCompiled = NULL;
	_DeclareHookHdlrFnVariable(MessagingCallbackSave, Retn, 0x004FE642);	
	_DeclareHookHdlrFnVariable(MessagingCallbackSave, Call, 0x00503450);					// f_PrecompileSub()
    __asm
    {
		mov		ScriptToBeCompiled, esi
		call	[_HookHdlrFnVariable(MessagingCallbackSave, Call)]
		test	al, al
		jnz		PASS								// on a successful compile
		jmp		[_HookHdlrFnVariable(MessagingCallbackSave, Retn)]
	PASS:
		cmp		[g_WParamBuffer], 0x9CDA			// if in a save callback
		jz		GETDATA
		jmp		[_HookHdlrFnVariable(MessagingCallbackSave, Retn)]
	GETDATA:
		pushad
		mov		ecx, ScriptToBeCompiled
		mov		g_SetEditorTextCache, ecx
		push	4
		call	SendPingBack
		popad
		jmp		[_HookHdlrFnVariable(MessagingCallbackSave, Retn)]
    }
} 

_BeginHookHdlrFn(ScriptListOpen)
{
	_DeclareHookHdlrFnVariable(ScriptListOpen, Retn, 0x004FEE33);	
	__asm
	{
		mov		eax, g_ScriptListResult
		jmp		[_HookHdlrFnVariable(ScriptListOpen, Retn)]
	}
}

_BeginHookHdlrFn(ScriptListDelete)
{
	_DeclareHookHdlrFnVariable(ScriptListDelete, Retn, 0x004FF14B);	
	__asm
	{
		mov		ebx, g_ScriptListResult
		jmp		[_HookHdlrFnVariable(ScriptListDelete, Retn)]
	}
}


_BeginHookHdlrFn(SaveDialogBox)
{
	_DeclareHookHdlrFnVariable(SaveDialogBox, YesRetn, 0x004FE59D);	
	_DeclareHookHdlrFnVariable(SaveDialogBox, NoRetn, 0x004FE572);	
	_DeclareHookHdlrFnVariable(SaveDialogBox, CancelRetn, 0x004FE731);	
	__asm
	{
		cmp		eax, 7
		jz		NO
		cmp		eax, 2
		jz		CANCEL
		
		jmp		[_HookHdlrFnVariable(SaveDialogBox, YesRetn)]
	NO:
		jmp		[_HookHdlrFnVariable(SaveDialogBox, NoRetn)]
	CANCEL:
		mov		al, 0
		jmp		[_HookHdlrFnVariable(SaveDialogBox, CancelRetn)]
	}
}

void __stdcall DoLogRecompileResultsHook(Script* Arg)
{
	DebugPrint(Console::e_CS, "Script '%s' failed to recompile due to compile errors.", Arg->editorData.editorID.m_data);
}

_BeginHookHdlrFn(LogRecompileResults)
{
	_DeclareHookHdlrFnVariable(LogRecompileResults, Retn, 0x004FF083);
	_DeclareHookHdlrFnVariable(LogRecompileResults, Call, 0x00503450);
	__asm
	{
		call	[_HookHdlrFnVariable(LogRecompileResults, Call)]
		test	al, al
		jz		LOG
		jmp		EXIT
	LOG:
		push	edi
		call	DoLogRecompileResultsHook
	EXIT:
		jmp		[_HookHdlrFnVariable(LogRecompileResults, Retn)]
	}
}

void ToggleScriptCompiling(bool Enable)
{
	if (!Enable)
		_MemoryHandler(ToggleScriptCompilingNewData).WriteBuffer();
	else
		_MemoryHandler(ToggleScriptCompilingOriginalData).WriteBuffer();
}

_BeginHookHdlrFn(MaxScriptSizeOverrideScriptBufferCtor)
{
	_DeclareHookHdlrFnVariable(MaxScriptSizeOverrideScriptBufferCtor, Retn, 0x004FFEE8);
	_DeclareHookHdlrFnVariable(MaxScriptSizeOverrideScriptBufferCtor, Call, 0x00401DA0);
	__asm
	{
		push	g_MaxScriptDataSize
		mov     ecx, 0x00A09E90
		mov     [esi + 0x1C], ebx
		mov     [esi + 4], ebx
		mov     [esi + 0x24], ebx
		call    [_HookHdlrFnVariable(MaxScriptSizeOverrideScriptBufferCtor, Call)]
		push    g_MaxScriptDataSize
		jmp		[_HookHdlrFnVariable(MaxScriptSizeOverrideScriptBufferCtor, Retn)]
	}
}

_BeginHookHdlrFn(MaxScriptSizeOverrideParseScriptLine)
{
	_DeclareHookHdlrFnVariable(MaxScriptSizeOverrideParseScriptLine, Retn, 0x005031D9);
	__asm
	{
		mov     eax, [edi+24h]
		mov     ecx, [esi+40Ch]
		lea     edx, [eax+ecx+0Ah]
		cmp     edx, g_MaxScriptDataSize
		jmp		[_HookHdlrFnVariable(MaxScriptSizeOverrideParseScriptLine, Retn)]
	}
}

