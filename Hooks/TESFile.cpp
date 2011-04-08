#include "TESFile.h"
#include "Misc.h"
#include "Dialog.h"

_DefineHookHdlr(SavePluginMasterEnum, 0x0047ECC6);
_DefineNopHdlr(CheckIsActivePluginAnESM, 0x0040B65E, 2);
_DefineNopHdlr(TESFormGetUnUsedFormID, 0x00486C08, 2);
_DefineHookHdlr(LoadPluginsProlog, 0x00485252);
_DefineHookHdlr(LoadPluginsEpilog, 0x004856B2);
_DefinePatchHdlr(DataDialogPluginDescription, 0x0040CAB6);
_DefinePatchHdlr(DataDialogPluginAuthor, 0x0040CAFE);
_DefineHookHdlr(SavePluginCommonDialog, 0x00446D51);
_DefineHookHdlr(QuickLoadPluginLoadHandlerPrologue, 0x0040D073);
_DefineHookHdlr(QuickLoadPluginLoadHandler, 0x004852E5);
_DefineJumpHdlr(MissingMasterOverride, 0x00484FC9, 0x00484E8E);
_DefinePatchHdlr(DataHandlerPostError, 0x004852F0);
_DefineHookHdlrWithBuffer(AutoLoadActivePluginOnStartup, 0x0041A26A)(6, 0x8B, 0x0D, 0x44, 0xB6, 0xA0, 0x0), 6);
_DefineHookHdlr(DataHandlerSaveFormToFile, 0x00479181);
_DefineHookHdlr(TESFileUpdateHeader, 0x004894D0);
_DefineHookHdlr(DataHandlerSavePluginEpilog, 0x0047F136);
_DefineHookHdlr(TESFileUpdateHeaderFlagBit, 0x00489570);
_DefineHookHdlr(TESObjectCELLSaveReferencesProlog, 0x00538860); 
_DefineHookHdlr(TESObjectCELLSaveReferencesEpilog, 0x005389DB);

void PatchTESFileHooks(void)
{
	_MemoryHandler(LoadPluginsProlog).WriteJump();
	_MemoryHandler(LoadPluginsEpilog).WriteJump();
	_MemoryHandler(SavePluginCommonDialog).WriteJump();
	_MemoryHandler(SavePluginMasterEnum).WriteJump();
	_MemoryHandler(QuickLoadPluginLoadHandlerPrologue).WriteJump();
	_MemoryHandler(QuickLoadPluginLoadHandler).WriteJump();
	_MemoryHandler(MissingMasterOverride).WriteJump();
	_MemoryHandler(DataHandlerPostError).WriteUInt8(0xEB);	
	_MemoryHandler(DataDialogPluginDescription).WriteUInt8(0xEB);
	_MemoryHandler(DataDialogPluginAuthor).WriteUInt8(0xEB);
	_MemoryHandler(CheckIsActivePluginAnESM).WriteNop();
	_MemoryHandler(TESFormGetUnUsedFormID).WriteNop();
	_MemoryHandler(DataHandlerSaveFormToFile).WriteJump();
	_MemoryHandler(TESFileUpdateHeader).WriteJump();
	_MemoryHandler(DataHandlerSavePluginEpilog).WriteJump();
	_MemoryHandler(TESFileUpdateHeaderFlagBit).WriteJump();
	_MemoryHandler(TESObjectCELLSaveReferencesProlog).WriteJump();
	_MemoryHandler(TESObjectCELLSaveReferencesEpilog).WriteJump();
}

bool __stdcall InitTESFileSaveDlg()
{
	return DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_TESFILE), *g_HWND_CSParent, (DLGPROC)TESFileDlgProc);
}

_BeginHookHdlrFn(SavePluginCommonDialog)
{
	_DeclareHookHdlrFnVariable(SavePluginCommonDialog, ESMRetn, 0x00446D58);
	_DeclareHookHdlrFnVariable(SavePluginCommonDialog, ESPRetn, 0x00446D69);
	_asm
	{
		pushad
		call	InitTESFileSaveDlg
		test	eax, eax
		jnz		ESM

		popad
		jmp		[_HookHdlrFnVariable(SavePluginCommonDialog, ESPRetn)]
	ESM:
		popad
		jmp		[_HookHdlrFnVariable(SavePluginCommonDialog, ESMRetn)]
	}
}


void __stdcall DoLoadPluginsPrologHook(void)
{
	ModEntry::Data* ActiveFile = (*g_dataHandler)->unk8B8.activeFile;

	if (ActiveFile && (ActiveFile->flags & ModEntry::Data::kFlag_IsMaster))
	{
		ToggleFlag(&ActiveFile->flags, ModEntry::Data::kFlag_IsMaster, 0);
	}

	sprintf_s(g_NumericIDWarningBuffer, 0x10, "%s", g_INIManager->GET_INI_STR("ShowNumericEditorIDWarning"));
	static const char* Zero = "0";
	g_INIManager->FetchSetting("ShowNumericEditorIDWarning")->SetValue(Zero);
}

_BeginHookHdlrFn(LoadPluginsProlog)
{
	_DeclareHookHdlrFnVariable(LoadPluginsProlog, Retn, 0x00485257);
	_DeclareHookHdlrFnVariable(LoadPluginsProlog, Call, 0x00431310);
	__asm
	{
		pushad
		call	DoLoadPluginsPrologHook
		popad

		call	[_HookHdlrFnVariable(LoadPluginsProlog, Call)]
		jmp		[_HookHdlrFnVariable(LoadPluginsProlog, Retn)]
	}
}

void __stdcall DoLoadPluginsEpilogHook(void)
{
	g_INIManager->FetchSetting("ShowNumericEditorIDWarning")->SetValue(g_NumericIDWarningBuffer);
}

_BeginHookHdlrFn(LoadPluginsEpilog)
{
	_DeclareHookHdlrFnVariable(LoadPluginsEpilog, Retn, 0x004856B7);
	_DeclareHookHdlrFnVariable(LoadPluginsEpilog, Call, 0x0047DA60);
	__asm
	{
		pushad
		call	DoLoadPluginsEpilogHook
		popad

		call	[_HookHdlrFnVariable(LoadPluginsEpilog, Call)]
		jmp		[_HookHdlrFnVariable(LoadPluginsEpilog, Retn)]		
	}
}


bool __stdcall DoSavePluginMasterEnumHook(ModEntry::Data* CurrentFile)
{
	if ((CurrentFile->flags & ModEntry::Data::kFlag_Loaded) == 0)
		return false;
	else
		return true;
}

_BeginHookHdlrFn(SavePluginMasterEnum)
{
	_DeclareHookHdlrFnVariable(SavePluginMasterEnum, RetnPass, 0x0047ECCF);
	_DeclareHookHdlrFnVariable(SavePluginMasterEnum, RetnFail, 0x0047ECEB);
	__asm
	{
		pushad
		push	ecx
		call	DoSavePluginMasterEnumHook
		test	al, al
		jz		SKIP

		popad
		jmp		[_HookHdlrFnVariable(SavePluginMasterEnum, RetnPass)]
	SKIP:
		popad
		jmp		[_HookHdlrFnVariable(SavePluginMasterEnum, RetnFail)]
	}
}

void __stdcall DoQuickLoadPluginLoadHandlerPrologueHook(HWND DataDlg)
{
	if (IsDlgButtonChecked(DataDlg, DATA_QUICKLOAD) == BST_CHECKED)
		g_QuickLoadToggle = true;
	else
		g_QuickLoadToggle = false;
}

_BeginHookHdlrFn(QuickLoadPluginLoadHandlerPrologue)
{
	_DeclareHookHdlrFnVariable(QuickLoadPluginLoadHandlerPrologue, Call, 0x0040CA30);
	_DeclareHookHdlrFnVariable(QuickLoadPluginLoadHandlerPrologue, Retn, 0x0040D078);
	__asm
	{
		pushad
		push	edi
		call	DoQuickLoadPluginLoadHandlerPrologueHook
		popad
		call	[_HookHdlrFnVariable(QuickLoadPluginLoadHandlerPrologue, Call)]
		jmp		[_HookHdlrFnVariable(QuickLoadPluginLoadHandlerPrologue, Retn)]
	}
}

bool __stdcall DoQuickLoadPluginLoadHandlerHook(ModEntry::Data* CurrentFile)
{
	return _stricmp(CurrentFile->name, (*g_dataHandler)->unk8B8.activeFile->name);
}

_BeginHookHdlrFn(QuickLoadPluginLoadHandler)
{
	_DeclareHookHdlrFnVariable(QuickLoadPluginLoadHandler, Call, 0x00484A60);		// f_DataHandler::LoadTESFile
	_DeclareHookHdlrFnVariable(QuickLoadPluginLoadHandler, Retn, 0x004852EE);
	_DeclareHookHdlrFnVariable(QuickLoadPluginLoadHandler, Skip, 0x004852F0);
	__asm
	{
		pushad
		mov		al, g_QuickLoadToggle
		test	al, al
		jz		CONTINUE
		push	edx
		call	DoQuickLoadPluginLoadHandlerHook
		test	eax, eax
		jnz		SKIP
	CONTINUE:
		popad

		push	ecx
		push	edx
		mov		ecx, edi
		call	[_HookHdlrFnVariable(QuickLoadPluginLoadHandler, Call)]
		jmp		[_HookHdlrFnVariable(QuickLoadPluginLoadHandler, Retn)]
	SKIP:
		popad
		jmp		[_HookHdlrFnVariable(QuickLoadPluginLoadHandler, Skip)]
	}
}

_BeginHookHdlrFn(AutoLoadActivePluginOnStartup)
{
	_DeclareHookHdlrFnVariable(AutoLoadActivePluginOnStartup, Retn, 0x0041A284);
	__asm
	{
		mov		eax, 1
		jmp		[_HookHdlrFnVariable(AutoLoadActivePluginOnStartup, Retn)]
	}
}


bool __stdcall DoTESFileUpdateHeaderHook(TESFile* Plugin)
{
	PrintToBuffer("%s%s", Plugin->filepath, Plugin->name);
	HANDLE TempFile = CreateFile(g_Buffer, GENERIC_READ|GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (TempFile == INVALID_HANDLE_VALUE)
	{
		LogWinAPIErrorMessage(GetLastError());
		PrintToBuffer("Couldn't open TESFile '%s' for read/write access.\n\nError logged to the console.", Plugin->name);
		MessageBox(NULL, g_Buffer, "CSE", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	else
	{
		CloseHandle(TempFile);
		return true;
	}
}

_BeginHookHdlrFn(TESFileUpdateHeader)
{
	_DeclareHookHdlrFnVariable(TESFileUpdateHeader, Retn, 0x004894D6);
	_DeclareHookHdlrFnVariable(TESFileUpdateHeader, Exit, 0x0048957B);
	__asm
	{
		pushad
		xor		eax, eax
		push	ecx
		call	DoTESFileUpdateHeaderHook
		test	al, al
		jz		EXIT

		popad
		jmp		[_HookHdlrFnVariable(TESFileUpdateHeader, Retn)]
	EXIT:
		popad
		jmp		[_HookHdlrFnVariable(TESFileUpdateHeader, Exit)]
	}
}



void __stdcall DoDataHandlerSavePluginEpilogHook(void)
{
	TESFile* ActiveFile = (*g_dataHandler)->unk8B8.activeFile;
	ToggleFlag(&ActiveFile->flags, ModEntry::Data::kFlag_IsMaster, 0);
}

_BeginHookHdlrFn(DataHandlerSavePluginEpilog)
{
	_DeclareHookHdlrFnVariable(DataHandlerSavePluginEpilog, Retn, 0x0047F13B);
	__asm
	{
		call	WriteToStatusBar

		pushad
		call	DoDataHandlerSavePluginEpilogHook
		popad

		jmp		[_HookHdlrFnVariable(DataHandlerSavePluginEpilog, Retn)]
	}
}

void __stdcall DoTESFileUpdateHeaderFlagBitHook(TESFile* Plugin)
{
	TESFile* ActiveFile = (*g_dataHandler)->unk8B8.activeFile;
	if (ActiveFile)
		ToggleFlag(&ActiveFile->flags, ModEntry::Data::kFlag_IsMaster, 0);
}

_BeginHookHdlrFn(TESFileUpdateHeaderFlagBit)
{
	_DeclareHookHdlrFnVariable(TESFileUpdateHeaderFlagBit, Retn, 0x0048957B);
	__asm
	{
		pushad
		push	esi
		call	DoTESFileUpdateHeaderFlagBitHook
		popad
		jmp		[_HookHdlrFnVariable(TESFileUpdateHeaderFlagBit, Retn)]
	}
}

bool __stdcall DoDataHandlerSaveFormToFileHook(TESForm* Form)
{
	ModEntry::Data* OverrideFile = (ModEntry::Data*)thisCall(kTESForm_GetOverrideFile, Form, -1);
	if (!OverrideFile || OverrideFile == (*g_dataHandler)->unk8B8.activeFile)
		return false;
	else
		return true;
}

_BeginHookHdlrFn(DataHandlerSaveFormToFile)
{
	_DeclareHookHdlrFnVariable(DataHandlerSaveFormToFile, Retn, 0x00479187);
	_DeclareHookHdlrFnVariable(DataHandlerSaveFormToFile, Jump, 0x0047919E);
	__asm
	{
		test	byte ptr [esi + 0x8], 1
		jz		FAIL
	EXIT:
		jmp		[_HookHdlrFnVariable(DataHandlerSaveFormToFile, Retn)]	// TESForm::SaveForm
	FAIL:
		pushad
		xor		eax, eax
		push	esi
		call	DoDataHandlerSaveFormToFileHook
		test	al, al
		jnz		FIX
		popad

		jmp		[_HookHdlrFnVariable(DataHandlerSaveFormToFile, Jump)]	// TESForm::SaveFormRecord
	FIX:
		popad
		jmp		EXIT
	}
}

bool __stdcall DoTESObjectCELLSaveReferencesPrologHook(TESObjectREFR* Reference, TESFile* SaveFile)
{
	TESFile* SourceFile = (ModEntry::Data*)thisCall(kTESForm_GetOverrideFile, Reference, 0);
	TESFile* ActiveFile = (ModEntry::Data*)thisCall(kTESForm_GetOverrideFile, Reference, -1);

	if (SourceFile == ActiveFile && ActiveFile == SaveFile)	
		return false;
	else
		return true;
}

_BeginHookHdlrFn(TESObjectCELLSaveReferencesProlog)
{
	_DeclareHookHdlrFnVariable(TESObjectCELLSaveReferencesProlog, Retn, 0x00538869);
	_DeclareHookHdlrFnVariable(TESObjectCELLSaveReferencesProlog, Jump, 0x0053886B);
	__asm
	{
		call	[kTESFile_GetIsESM]
		test	al, al
		jnz		PASS

		mov		eax, [esp + 0x44]		// TESFile* SaveFile
		pushad
		push	eax
		push	esi
		call	DoTESObjectCELLSaveReferencesPrologHook
		test	al, al
		jnz		FIX
		popad

		jmp		[_HookHdlrFnVariable(TESObjectCELLSaveReferencesProlog, Retn)]
	FIX:
		popad
	PASS:
		jmp		[_HookHdlrFnVariable(TESObjectCELLSaveReferencesProlog, Jump)]
	}
}

bool __stdcall DoTESObjectCELLSaveReferencesEpilogHook(TESObjectREFR* Reference, TESFile* SaveFile)
{
	if ((Reference->flags & TESForm::kFormFlags_Deleted))
	{
		TESFile* SourceFile = (ModEntry::Data*)thisCall(kTESForm_GetOverrideFile, Reference, 0);
		TESFile* ActiveFile = (ModEntry::Data*)thisCall(kTESForm_GetOverrideFile, Reference, -1);

		if ((SourceFile == ActiveFile && ActiveFile == SaveFile) ||
			(SourceFile == NULL && ActiveFile == NULL))
		{
			return false;
		}
		else if ((Reference->baseForm->flags & TESForm::kFormFlags_Deleted))
		{
			thisVirtualCall(*((UInt32*)Reference), 0x50, Reference, SaveFile);	// call SaveForm to dump an empty record
			return false;
		}
		else
			return true;
	}
	else
		return true;
}

_BeginHookHdlrFn(TESObjectCELLSaveReferencesEpilog)
{
	_DeclareHookHdlrFnVariable(TESObjectCELLSaveReferencesEpilog, Retn, 0x005389E1);
	__asm
	{
		pushad
		push	eax
		push	ecx
		call	DoTESObjectCELLSaveReferencesEpilogHook
		test	al, al
		jz		EXIT
		popad

		push	eax
		call	kTESForm_SaveFormRecord
		jmp		[_HookHdlrFnVariable(TESObjectCELLSaveReferencesEpilog, Retn)]
EXIT:
		popad
		jmp		[_HookHdlrFnVariable(TESObjectCELLSaveReferencesEpilog, Retn)]
	}
}

