#include "SEHooks.h"
#include "ExtenderInternals.h"
#include "Common/CLIWrapper.h"
#include "Exports.h"
#include "Common/HandShakeStructs.h"


static HWND							g_ScriptEditorBuffer = NULL;			// handle to the editor dialog being processed by the WndProc
FARPROC								g_WindowHandleCallAddr = NULL;			// used to call WndMgmt functions

Script*								g_EditorInitScript	=	NULL;			// must point to valid Script object to be used. needs to be reset right after dialog instantiation
Script*								g_SetEditorTextCache = NULL;			// stores the script object from the last call of f_Script::SetEditorText

static UInt32						g_WParamBuffer		=	0;				// WParam processed by the WndProc


ScriptData*							g_ScriptDataPackage = new ScriptData();
std::string							g_ActivePluginName  =	"";
static const char*					g_RecompileAllScriptsStr = "Are you sure you want to recompile every script in the active plugin?";

Script*								g_ScriptListResult = NULL;				// used by our script list hook, to set the selected script form

static const void*					g_ExpressionBuffer = new char[0x400];

ScriptVarIndexData*					g_ScriptVarIndices = new ScriptVarIndexData();
Script*								g_EditorAuxScript = NULL;
HWND								g_EditorAuxHWND = NULL;


bool PatchSEHooks()
{
	PLACE_HOOK(LoadRelease);

	PLACE_HOOK(ScriptableFormEntryPoint);	
	PLACE_HOOK(ScriptEffectItemEntryPoint);	
	PLACE_HOOK(MainWindowEntryPoint);	

	PLACE_HOOK(EditorWindowProc);
	PLACE_HOOK(EditorWindowWParam);

	PLACE_HOOK(RecompileScripts);
	PLACE_HOOK(EditorInitScript);
	PLACE_HOOK(EditorInitGetAuxScript);
	PLACE_HOOK(EditorInitWindowPos);
	

	PLACE_HOOK(MessagingCallbackNewScript);
	PLACE_HOOK(MessagingCallbackPreviousScript);
	PLACE_HOOK(MessagingCallbackOpenNextScript);
	PLACE_HOOK(MessagingCallbackClose);
	PLACE_HOOK(MessagingCallbackSave);

	PLACE_HOOK(GetPluginNameSave);
	PLACE_HOOK(GetPluginNameLoad);

	PLACE_HOOK(ScriptListOpen);
	PLACE_HOOK(ScriptListDelete);	

	PLACE_HOOK(ExpressionParserSwapBufferA);
	PLACE_HOOK(ExpressionParserSwapBufferB);

	PLACE_HOOK(SaveDialogBox);
	
	SafeWrite32(kRecompileScriptsMessageBoxStringPatchAddr, (UInt32)g_RecompileAllScriptsStr);
	SafeWrite8(kSaveDialogBoxTypePatchAddr, 3);
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



// User32.dll calls

void __stdcall GetWindowTextAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "GetWindowTextA");
}
void __stdcall CreateDialogParamAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "CreateDialogParamA");
}
void __stdcall EndDialogAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "EndDialog");
}
void __stdcall EnableWindowAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "EnableWindow");
}
void __stdcall GetWindowLongAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "GetWindowLongA");
}
void __stdcall CreateWindowExAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "CreateWindowExA");
}
void __stdcall SetWindowTextAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "SetWindowTextA");
}
void __stdcall SendDlgItemMessageAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "SendDlgItemMessageA");
}
void __stdcall SendMessageAddress(void)
{
	HMODULE hMod = GetModuleHandle("USER32.DLL");
	g_WindowHandleCallAddr = GetProcAddress(hMod, "SendMessageA");
}


// Hooks


void DispatchInteropMessage(void)
{
	static bool InterOpMessageDispatched = false;

	if (!InterOpMessageDispatched) {
		_D_PRINT("Plugin interop initialized; Message dispatched\n");
		g_msgIntfc->Dispatch(g_pluginHandle, 'CSEL', NULL, 0, NULL);
		InterOpMessageDispatched = true;
	}
}

void __stdcall InstantiateTabContainer(void)
{
	DispatchInteropMessage();

	if (g_EditorAuxHWND)		g_EditorAuxScript = (Script*)GetComboBoxItemData(GetDlgItem(g_EditorAuxHWND, 1226));
	else						g_EditorAuxScript = NULL;

	tagRECT ScriptEditorLoc;
	GetPositionFromINI("Script Edit", &ScriptEditorLoc);
	CLIWrapper::SE_AllocateNewEditor(ScriptEditorLoc.left, ScriptEditorLoc.top, ScriptEditorLoc.right, ScriptEditorLoc.bottom);
	g_EditorAuxHWND = NULL;
}

void __declspec(naked) MainWindowEntryPointHook(void)
{
    __asm
    {
		call	InstantiateTabContainer
		jmp		[kMainWindowEntryPointRetnAddr]
    }
}


void __declspec(naked) ScriptableFormEntryPointHook(void)			
{
    __asm
    {
		mov		g_EditorAuxHWND, eax
		call	InstantiateTabContainer
		jmp		[kScriptableFormEntryPointRetnAddr]
    }
} 

void __declspec(naked) ScriptEffectItemEntryPointHook(void)			
{
    __asm
    {
		mov		g_EditorAuxHWND, eax
		call	InstantiateTabContainer
		jmp		[kScriptEffectItemEntryPointRetnAddr]
    }
} 



void __stdcall DoLoadReleaseHook(void)
{
	EditorAllocator::GetSingleton()->DeleteAllTrackedEditors();
	SendPingBack(8);
}

void __declspec(naked) LoadReleaseHook(void)
{
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
    __asm
    {
		mov		[g_WParamBuffer], eax
		jg		JMPG
	JMPG:
		jmp		[kEditorWindowWParamRetnAddr]
    }
} 



void __stdcall DoGetPluginNameHook(const char* WindowTitle)
{
	g_ActivePluginName = WindowTitle;
	UInt32 PadStart = g_ActivePluginName.find("[") + 1, PadEnd  = g_ActivePluginName.rfind("]");
	if (PadStart == std::string::npos || PadEnd == std::string::npos)	g_ActivePluginName = "";
	else																g_ActivePluginName = g_ActivePluginName.substr(PadStart, PadEnd - PadStart);
}

void __declspec(naked) GetPluginNameSaveHook(void)
{
    __asm
    {
		pushad
		call	SetWindowTextAddress
		push	ebx
		call	DoGetPluginNameHook
		popad

		call	[g_WindowHandleCallAddr]				// SetWindowTextA
		pushad
		push	10
		call	SendPingBack
		popad
		jmp		[kGetPluginNameSaveRetnAddr]
    }
}

void __declspec(naked) GetPluginNameLoadHook(void)
{
    __asm
    {
		pushad
		call	SetWindowTextAddress
		push	ebx
		call	DoGetPluginNameHook
		popad

		call	[g_WindowHandleCallAddr]				// SetWindowTextA
		pushad
		push	9
		call	SendPingBack
		popad
		jmp		[kGetPluginNameLoadRetnAddr]
    }
}

bool __stdcall DoRecompileScriptsHook(TESForm* Form)
{
	Script* ScriptForm = CS_CAST(Form, TESForm, Script);

	if (g_ActivePluginName != "" && CLIWrapper::SE_IsActivePluginScriptRecord(ScriptForm->editorData.editorID.m_data))		// the 0x00000002 flag isn't reliable as it seems to mark the first (script) record as modified
		return false;										// don't skip, script is from the active plugin
	else
		return true;										// skip
}

void __declspec(naked) RecompileScriptsHook(void)
{
	static bool bShouldSkip = false;

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
	__asm
	{
		mov		g_SetEditorTextCache, ecx
		mov     eax, [edx + 0x114]     

		jmp		[kEditorInitGetAuxScriptRetnAddr]
	}
}


void __declspec(naked) EditorInitWindowPosHook(void)
{
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
	 
	CLIWrapper::SE_SendMessagePingback(TrackedIndex, Message);
}

void __stdcall DoMessagingCallbackCloseHookRelease(void)
{
	EDAL->DeleteTrackedEditor(EDAL->GetTrackedIndex(g_ScriptEditorBuffer));
}

void __declspec(naked) MessagingCallbackNewScriptHook(void)
{
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
{			// ugh!
	__asm
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
	__asm
	{
		call	EndDialogAddress
		call	[g_WindowHandleCallAddr]

		pushad
		push	7
		call	SendPingBack
		popad
		jmp		[kMessagingCallbackCloseRetnAddr]
	}
}

void __declspec(naked) MessagingCallbackSaveHook(void)			
{
	static Script* ScriptToBeCompiled = NULL;

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
	__asm
	{
		mov		eax, g_ScriptListResult
		jmp		[kScriptListOpenRetnAddr]
	}
}

void __declspec(naked) ScriptListDeleteHook(void)
{
	__asm
	{
		mov		ebx, g_ScriptListResult
		jmp		[kScriptListDeleteRetnAddr]
	}
}

void __declspec(naked) ExpressionParserSwapBufferAHook(void)
{
	__asm
	{
		mov		edx, g_ExpressionBuffer
		push	edx
		jmp		[kExpressionParserSwapBufferARetnAddr]
	}
}

void __declspec(naked) ExpressionParserSwapBufferBHook(void)
{
	__asm
	{
		mov		edx, g_ExpressionBuffer
		mov		byte ptr [esi], 0x20
		jmp		[kExpressionParserSwapBufferBRetnAddr]
	}
}

void __declspec(naked) SaveDialogBoxHook(void)
{
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
