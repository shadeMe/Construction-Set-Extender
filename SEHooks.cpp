#include "SEHooks.h"
#include "ExtenderInternals.h"
#include "[Common]/CLIWrapper.h"
#include "Exports.h"
#include "[Common]/HandShakeStructs.h"
#include "CompilerErrorDetours.h"

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



MemHdlr								kMainWindowEntryPoint				(0x0041A5F6, MainWindowEntryPointHook, 0, 0);
MemHdlr								kScriptableFormEntryPoint			(0x004A16AD, ScriptableFormEntryPointHook, 0, 0);
MemHdlr								kScriptEffectItemEntryPoint			(0x00566387, ScriptEffectItemEntryPointHook, 0, 0);
MemHdlr								kLoadRelease						(0x0040D090, LoadReleaseHook, 0, 0);
MemHdlr								kEditorWindowProc					(0x004FE7AC, EditorWindowProcHook, 0, 0);
MemHdlr								kEditorWindowWParam					(0x004FEC46, EditorWindowWParamHook, 0, 0);
MemHdlr								kRecompileScripts					(0x004FEFEA, RecompileScriptsHook, 0, 0);
MemHdlr								kEditorInitScript					(0x004FEB1F, EditorInitScriptHook, 0, 0);
MemHdlr								kEditorInitGetAuxScript				(0x004FEB48, EditorInitGetAuxScriptHook, 0, 0);
MemHdlr								kEditorInitWindowPos				(0x004FEB9A, EditorInitWindowPosHook, 0, 0);
MemHdlr								kMessagingCallbackNewScript			(0x004FEDFD, MessagingCallbackNewScriptHook, 0, 0);
MemHdlr								kMessagingCallbackOpenNextScript	(0x004FEE6F, MessagingCallbackOpenNextScriptHook, 0, 0);	
MemHdlr								kMessagingCallbackPreviousScript	(0x004FEEDF, MessagingCallbackPreviousScriptHook, 0, 0);
MemHdlr								kMessagingCallbackClose				(0x004FED69, MessagingCallbackCloseHook, 0, 0);
MemHdlr								kMessagingCallbackSave				(0x004FE63D, MessagingCallbackSaveHook, 0, 0);
MemHdlr								kScriptListOpen						(0x004FEE1D, ScriptListOpenHook, 0, 0);
MemHdlr								kScriptListDelete					(0x004FF133, ScriptListDeleteHook, 0, 0);
MemHdlr								kSaveDialogBox						(0x004FE56D, SaveDialogBoxHook, 0, 0);
MemHdlr								kLogRecompileResults				(0x004FF07E, LogRecompileResultsHook, 0, 0);
MemHdlr								kRecompileScriptsMessageBoxString	(0x004FEF3F, (UInt32)0, 0, 0);
MemHdlr								kSaveDialogBoxType					(0x004FE558, (UInt32)0, 0, 0);
MemHdlr								kToggleScriptCompilingOriginalData	(0x00503450, (UInt32)0, MakeUInt8Array(8, 0x6A, 0xFF, 0x68, 0x68, 0x13, 0x8C, 0, 0x64), 8);
MemHdlr								kToggleScriptCompilingNewData		(0x00503450, (UInt32)0, MakeUInt8Array(8, 0xB8, 1, 0, 0, 0, 0xC2, 8, 0), 8);
MemHdlr								kMaxScriptSizeOverrideScriptBufferCtor
																		(0x004FFECB, MaxScriptSizeOverrideScriptBufferCtorHook, 0, 0);
MemHdlr								kMaxScriptSizeOverridesParseScriptLine
																		(0x005031C6, MaxScriptSizeOverrideParseScriptLineHook, 0, 0);


bool PatchSEHooks()
{
	kLoadRelease.WriteJump();

	kScriptableFormEntryPoint.WriteJump();	
	kScriptEffectItemEntryPoint.WriteJump();	
	kMainWindowEntryPoint.WriteJump();	

	kEditorWindowProc.WriteJump();
	kEditorWindowWParam.WriteJump();

	kRecompileScripts.WriteJump();
	kEditorInitScript.WriteJump();
	kEditorInitGetAuxScript.WriteJump();
	kEditorInitWindowPos.WriteJump();

	kMessagingCallbackNewScript.WriteJump();
	kMessagingCallbackPreviousScript.WriteJump();
	kMessagingCallbackOpenNextScript.WriteJump();
	kMessagingCallbackClose.WriteJump();
	kMessagingCallbackSave.WriteJump();

	kScriptListOpen.WriteJump();
	kScriptListDelete.WriteJump();	

	kSaveDialogBox.WriteJump();
	kLogRecompileResults.WriteJump();
	
	kRecompileScriptsMessageBoxString.WriteUInt32((UInt32)g_RecompileAllScriptsStr);
	kSaveDialogBoxType.WriteUInt8(3);

	kMaxScriptSizeOverrideScriptBufferCtor.WriteJump();
	kMaxScriptSizeOverridesParseScriptLine.WriteJump();

	PatchCompilerErrorDetours();
	return true;
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
	
void __declspec(naked) MainWindowEntryPointHook(void)
{
	static const UInt32 kMainWindowEntryPointRetnAddr =	0x0041A610;	
    __asm
    {
		call	InstantiateTabContainer
		jmp		[kMainWindowEntryPointRetnAddr]
    }
}


void __declspec(naked) ScriptableFormEntryPointHook(void)			
{
	static const UInt32	kScriptableFormEntryPointRetnAddr =	0x004A16C5;	
    __asm
    {
		mov		g_EditorAuxHWND, eax
		call	InstantiateTabContainer
		jmp		[kScriptableFormEntryPointRetnAddr]
    }
} 

void __declspec(naked) ScriptEffectItemEntryPointHook(void)			
{
	static const UInt32			kScriptEffectItemEntryPointRetnAddr =	0x0056639F;	
    __asm
    {
		mov		g_EditorAuxHWND, eax
		call	InstantiateTabContainer
		jmp		[kScriptEffectItemEntryPointRetnAddr]
    }
} 



void __stdcall DoLoadReleaseHook(void)
{
	EditorAllocator::GetSingleton()->DestroyVanillaDialogs();
	EditorAllocator::GetSingleton()->DeleteAllTrackedEditors();
	SendPingBack(8);
}

void __declspec(naked) LoadReleaseHook(void)
{
	static const UInt32			kLoadReleaseRetnAddr =	0x0040D096;	
    __asm
    {
		call	EndDialogAddress	
		call	[g_WindowHandleCallAddr]				// EndDialog

		pushad	
		call	DoLoadReleaseHook
		popad

		jmp		[kLoadReleaseRetnAddr]
    }
} 



void __declspec(naked) EditorWindowProcHook(void)
{
	static const UInt32			kEditorWindowProcRetnAddr =	0x004FE7B2;	
    __asm
    {
		mov		[g_ScriptEditorBuffer], edi
		pushad
		call	GetWindowLongAddress
		popad

		call	[g_WindowHandleCallAddr]				// GetWindowLongA
		jmp		[kEditorWindowProcRetnAddr]
    }
} 

void __declspec(naked) EditorWindowWParamHook(void)
{
	static const UInt32			kEditorWindowWParamRetnAddr =	0x004FED08;	
    __asm
    {
		mov		[g_WParamBuffer], eax
		jg		JMPG
	JMPG:
		jmp		[kEditorWindowWParamRetnAddr]
    }
} 

bool __stdcall DoRecompileScriptsHook(TESForm* Form)
{
	if (Form->flags & TESForm::kFormFlags_FromActiveFile)
		return false;										// don't skip, script is from the active plugin
	else
		return true;										// skip
}

void __declspec(naked) RecompileScriptsHook(void)
{
	static bool bShouldSkip = false;
	static const UInt32 kRecompileScriptsRetnAddr = 0x004FEFF1;
	static const UInt32 kRecompileScriptsCallAddr = 0x0047A4E0;				// sub_47A4E0
	static const UInt32 kRecompileScriptsSkipAddr = 0x004FF102;
    __asm
    {
		pushad
		push	eax
		call	DoRecompileScriptsHook
		mov		[bShouldSkip], al
		popad

		call	[kRecompileScriptsCallAddr]
		mov		ebx, eax
		cmp		[bShouldSkip], 1
		jz		SKIP
		jmp		[kRecompileScriptsRetnAddr]	
	SKIP:
		jmp		[kRecompileScriptsSkipAddr]
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

void __declspec(naked) EditorInitScriptHook(void)
{
	static const UInt32			kEditorInitScriptAuxRetnAddr =	0x004FEB28;	
	static const UInt32			kEditorInitScriptCSERetnAddr =	0x004FEB51;	
	static const UInt32			kScriptSetEditorTextCallAddr = 0x004FC1F0;
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
		jmp		[kEditorInitScriptCSERetnAddr]		// instantiation through the main window
	AUX:
		jmp		[kEditorInitScriptAuxRetnAddr]
	CSE:
		popad
		mov		ecx, g_EditorInitScript
		mov     [esi + 0x10], ecx					// copy our script to vanilla editor's userdata struct
		push	edi									// HWND ScriptEditor
		call	[kScriptSetEditorTextCallAddr]
		jmp		[kEditorInitScriptCSERetnAddr]
	}
}

void __declspec(naked) EditorInitGetAuxScriptHook(void)
{
	static const UInt32			kEditorInitGetAuxScriptRetnAddr =	0x004FEB4E;	
	__asm
	{
		mov		g_SetEditorTextCache, ecx
		mov     eax, [edx + 0x114]     

		jmp		[kEditorInitGetAuxScriptRetnAddr]
	}
}


void __declspec(naked) EditorInitWindowPosHook(void)
{
	static const UInt32			kEditorInitWindowPosRetnAddr =	0x004FEBB0;
	__asm
	{
		push	4
		push	0
		push	0
		push	-100
		push	-100
		jmp		[kEditorInitWindowPosRetnAddr]
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

void __declspec(naked) MessagingCallbackNewScriptHook(void)
{
	static const UInt32			kMessagingCallbackNewScriptRetnAddr =	0x004FEE0E;	
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
		jmp		[kMessagingCallbackNewScriptRetnAddr]
	}
}

void __declspec(naked) MessagingCallbackOpenNextScriptHook(void)
{			
	static const UInt32			kMessagingCallbackOpenNextScriptRetnAddr =	0x004FEE75;	
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
		jmp		[kMessagingCallbackOpenNextScriptRetnAddr]
	}
}

void __declspec(naked) MessagingCallbackPreviousScriptHook(void)
{
	static const UInt32			kMessagingCallbackPreviousScriptRetnAddr =	0x004FF2D2;	
	__asm
	{
		mov		g_SetEditorTextCache, ecx
		call	eax

		pushad
		push	2
		call	SendPingBack		
		popad

		mov     byte ptr [esi + 0x14], 0
		jmp		[kMessagingCallbackPreviousScriptRetnAddr]
	}
}

void __declspec(naked) MessagingCallbackCloseHook(void)
{
	static const UInt32			kMessagingCallbackCloseRetnAddr =	0x004FED6F;	
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
		jmp		[kMessagingCallbackCloseRetnAddr]
	}
}

void __declspec(naked) MessagingCallbackSaveHook(void)			
{
	static Script* ScriptToBeCompiled = NULL;
	static const UInt32			kMessagingCallbackSaveRetnAddr	=	0x004FE642;	
	static const UInt32			kMessagingCallbackSaveCallAddr	=	0x00503450;					// f_PrecompileSub()
    __asm
    {
		mov		ScriptToBeCompiled, esi
		call	[kMessagingCallbackSaveCallAddr]
		test	al, al
		jnz		PASS								// on a successful compile
		jmp		[kMessagingCallbackSaveRetnAddr]
	PASS:
		cmp		[g_WParamBuffer], 0x9CDA			// if in a save callback
		jz		GETDATA
		jmp		[kMessagingCallbackSaveRetnAddr]
	GETDATA:
		pushad
		mov		ecx, ScriptToBeCompiled
		mov		g_SetEditorTextCache, ecx
		push	4
		call	SendPingBack
		popad
		jmp		[kMessagingCallbackSaveRetnAddr]
    }
} 

void __declspec(naked) ScriptListOpenHook(void)
{
	static const UInt32			kScriptListOpenRetnAddr	=	0x004FEE33;	
	__asm
	{
		mov		eax, g_ScriptListResult
		jmp		[kScriptListOpenRetnAddr]
	}
}

void __declspec(naked) ScriptListDeleteHook(void)
{
	static const UInt32			kScriptListDeleteRetnAddr	=	0x004FF14B;	
	__asm
	{
		mov		ebx, g_ScriptListResult
		jmp		[kScriptListDeleteRetnAddr]
	}
}


void __declspec(naked) SaveDialogBoxHook(void)
{
	static const UInt32			kSaveDialogBoxYesRetnAddr	=	0x004FE59D;	
	static const UInt32			kSaveDialogBoxNoRetnAddr	=	0x004FE572;	
	static const UInt32			kSaveDialogBoxCancelRetnAddr	=	0x004FE731;	
	static const UInt32			kSaveDialogBoxTypePatchAddr	=	0x004FE558;	
	__asm
	{
		cmp		eax, 7
		jz		NO
		cmp		eax, 2
		jz		CANCEL
		
		jmp		[kSaveDialogBoxYesRetnAddr]
	NO:
		jmp		[kSaveDialogBoxNoRetnAddr]
	CANCEL:
		mov		al, 0
		jmp		[kSaveDialogBoxCancelRetnAddr]
	}
}

void __stdcall DoLogRecompileResultsHook(Script* Arg)
{
	DebugPrint(Console::e_CS, "Script '%s' failed to recompile due to compile errors.", Arg->editorData.editorID.m_data);
}

void __declspec(naked) LogRecompileResultsHook(void)
{
	static const UInt32			kLogRecompileResultsRetnAddr = 0x004FF083;
	static const UInt32			kLogRecompileResultsCallAddr = 0x00503450;
	__asm
	{
		call	[kLogRecompileResultsCallAddr]
		test	al, al
		jz		LOG
		jmp		EXIT
	LOG:
		push	edi
		call	DoLogRecompileResultsHook
	EXIT:
		jmp		[kLogRecompileResultsRetnAddr]
	}
}

void ToggleScriptCompiling(bool Enable)
{
	if (!Enable)
		kToggleScriptCompilingNewData.WriteBuffer();
	else
		kToggleScriptCompilingOriginalData.WriteBuffer();
}

void __declspec(naked) MaxScriptSizeOverrideScriptBufferCtorHook(void)
{
	static const UInt32			kMaxScriptSizeOverrideScriptBufferCtorRetnAddr = 0x004FFEE8;
	static const UInt32			kMaxScriptSizeOverrideScriptBufferCtorCallAddr = 0x00401DA0;
	__asm
	{
		push	g_MaxScriptDataSize
		mov     ecx, 0x00A09E90
		mov     [esi + 0x1C], ebx
		mov     [esi + 4], ebx
		mov     [esi + 0x24], ebx
		call    kMaxScriptSizeOverrideScriptBufferCtorCallAddr
		push    g_MaxScriptDataSize
		jmp		kMaxScriptSizeOverrideScriptBufferCtorRetnAddr
	}
}

void __declspec(naked) MaxScriptSizeOverrideParseScriptLineHook(void)
{
	static const UInt32			kMaxScriptSizeOverrideParseScriptLineRetnAddr = 0x005031D9;
	__asm
	{
		mov     eax, [edi+24h]
		mov     ecx, [esi+40Ch]
		lea     edx, [eax+ecx+0Ah]
		cmp     edx, g_MaxScriptDataSize
		jmp		kMaxScriptSizeOverrideParseScriptLineRetnAddr
	}
}

