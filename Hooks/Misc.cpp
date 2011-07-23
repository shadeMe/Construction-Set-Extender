#include "Misc.h"
#include "ScriptEditor.h"
#include "..\CSDialogs.h"
#include "..\ToolManager.h"
#include "..\WorkspaceManager.h"
#include "..\RenderWindowTextPainter.h"
#include "..\RenderSelectionGroupManager.h"
#include "..\ElapsedTimeCounter.h"
#include "..\CSEInterfaceManager.h"
#include "..\ChangeLogManager.h"
#include "..\Achievements.h"
#include "CSAS\ScriptRunner.h"

extern CommandTableData	g_CommandTableData;

namespace Hooks
{
	char g_NumericIDWarningBuffer[0x10] = {0};

	_DefineHookHdlrWithBuffer(CSInit, 0x00419260, 5, 0xE8, 0xEB, 0xC5, 0x2C, 0);
	_DefineHookHdlr(CSExit, 0x0041936E);
	_DefineNopHdlr(MissingTextureWarning, 0x0044F3AF, 14);
	_DefineHookHdlr(AssertOverride, 0x004B5670);
	_DefinePatchHdlr(TextureMipMapCheck, 0x0044F49B);
	_DefinePatchHdlr(UnnecessaryDialogEdits, 0x004EDFF7);
	_DefinePatchHdlr(UnnecessaryCellEdits, 0x005349A5);
	_DefineHookHdlr(PluginSave, 0x0041BBCD);
	_DefineHookHdlr(PluginLoad, 0x0041BEFA);
	_DefineHookHdlr(DataHandlerClearData, 0x0047AE76);
	_DefineJumpHdlr(TopicInfoCopyProlog, 0x004F0738, 0x004F07C4);
	_DefineHookHdlr(TopicInfoCopyEpilog, 0x004F1280);
	_DefineHookHdlr(NumericEditorID, 0x00497670);
	_DefineHookHdlr(DataHandlerConstructSpecialForms, 0x00481049);
	_DefineHookHdlr(ResultScriptSaveForm, 0x004FD258);
	_DefineHookHdlr(TESObjectREFRDoCopyFrom, 0x0054763D);
	_DefineHookHdlr(TESFormAddReference, 0x0049644B);
	_DefineHookHdlr(TESFormRemoveReference, 0x00496494);
	_DefineHookHdlr(TESFormClearReferenceList, 0x0049641E);
	_DefineHookHdlr(TESFormPopulateUseInfoList, 0x004964F2);
	_DefineHookHdlr(TESFormDelete, 0x00498712);
	_DefinePatchHdlr(TextureSizeCheck, 0x0044F444);
	_DefineHookHdlr(DataHandlerPlaceTESObjectLIGH, 0x005116C7);
	_DefineHookHdlr(TESWorldSpaceDestroyCellMapA, 0x00560753);
	_DefineHookHdlr(TESWorldSpaceDestroyCellMapB, 0x0079CA33);
	_DefineJumpHdlr(CSRegistryEntries, 0x00406820, 0x00406AF4);
	_DefineHookHdlr(AchievementAddTopic, 0x004F2ED4);
	_DefineHookHdlr(AchievementDeleteShadeMe, 0x004986B4);
	_DefineHookHdlr(AchievementModifyShadeMe, 0x00497BEA);
	_DefineHookHdlr(AchievementCloneHallOfFameForms, 0x00413E4F);
	_DefineHookHdlr(AchievementPluginDescription, 0x0040CD20);
	_DefineHookHdlr(AchievementBuildRoads, 0x00563CFF);
	_DefineHookHdlr(AchievementDialogResponseCreation, 0x004F2CC3);
	_DefineHookHdlr(TESDialogBuildSubwindowDiagnostics, 0x00404F2A);
	_DefineHookHdlr(ExtraTeleportInitItem, 0x00462702);

	void PatchMiscHooks(void)
	{
		_MemHdlr(CSInit).WriteJump();
		_MemHdlr(CSExit).WriteJump();
		_MemHdlr(PluginSave).WriteJump();
		_MemHdlr(PluginLoad).WriteJump();
		_MemHdlr(TextureMipMapCheck).WriteUInt8(0xEB);
		_MemHdlr(UnnecessaryCellEdits).WriteUInt8(0xEB);
		_MemHdlr(UnnecessaryDialogEdits).WriteUInt8(0xEB);
		_MemHdlr(AssertOverride).WriteJump();
		_MemHdlr(DataHandlerClearData).WriteJump();
		_MemHdlr(TopicInfoCopyProlog).WriteJump();
		_MemHdlr(TopicInfoCopyEpilog).WriteJump();
		_MemHdlr(NumericEditorID).WriteJump();
		_MemHdlr(DataHandlerConstructSpecialForms).WriteJump();
		_MemHdlr(ResultScriptSaveForm).WriteJump();
		_MemHdlr(TESObjectREFRDoCopyFrom).WriteJump();
		_MemHdlr(TESFormAddReference).WriteJump();
		_MemHdlr(TESFormRemoveReference).WriteJump();
		_MemHdlr(TESFormClearReferenceList).WriteJump();
		_MemHdlr(TESFormPopulateUseInfoList).WriteJump();
		_MemHdlr(TESFormDelete).WriteJump();
		_MemHdlr(TextureSizeCheck).WriteUInt8(0xEB);
		_MemHdlr(DataHandlerPlaceTESObjectLIGH).WriteJump();
		_MemHdlr(TESWorldSpaceDestroyCellMapA).WriteJump();
		_MemHdlr(TESWorldSpaceDestroyCellMapB).WriteJump();
		_MemHdlr(AchievementAddTopic).WriteJump();
		_MemHdlr(AchievementDeleteShadeMe).WriteJump();
		_MemHdlr(AchievementModifyShadeMe).WriteJump();
		_MemHdlr(AchievementCloneHallOfFameForms).WriteJump();
		_MemHdlr(AchievementPluginDescription).WriteJump();
		_MemHdlr(AchievementBuildRoads).WriteJump();
		_MemHdlr(AchievementDialogResponseCreation).WriteJump();
		_MemHdlr(TESDialogBuildSubwindowDiagnostics).WriteJump();
		_MemHdlr(ExtraTeleportInitItem).WriteJump();

		PatchMessageHandler();

		OSVERSIONINFO OSInfo;
		GetVersionEx(&OSInfo);
		if (OSInfo.dwMajorVersion >= 6)		// if running Windows Vista/7, fix the listview selection sound
			RegDeleteKey(HKEY_CURRENT_USER , "AppEvents\\Schemes\\Apps\\.Default\\CCSelect\\.Current");
	}

	void PatchEntryPointHooks(void)
	{
		_MemHdlr(CSRegistryEntries).WriteJump();
	}

	void PatchMessageHandler(void)
	{
		static UInt32 s_MessageHandlerVTBL = 0x00940760;

		if (!g_INIManager->FetchSetting("LogCSWarnings")->GetValueAsInteger())
			return;

		SafeWrite32(s_MessageHandlerVTBL + 0, (UInt32)&MessageHandlerOverride);
		SafeWrite32(s_MessageHandlerVTBL + 0x4, (UInt32)&MessageHandlerOverride);
		SafeWrite32(s_MessageHandlerVTBL + 0x8, (UInt32)&MessageHandlerOverride);
		SafeWrite32(s_MessageHandlerVTBL + 0x10, (UInt32)&MessageHandlerOverride);
		SafeWrite32(s_MessageHandlerVTBL + 0x14, (UInt32)&MessageHandlerOverride);
		SafeWrite32(s_MessageHandlerVTBL + 0x18, (UInt32)&MessageHandlerOverride);
		SafeWrite32(s_MessageHandlerVTBL + 0x1C, (UInt32)&MessageHandlerOverride);
		SafeWrite32(s_MessageHandlerVTBL + 0x20, (UInt32)&MessageHandlerOverride);
		SafeWrite32(s_MessageHandlerVTBL + 0x24, (UInt32)&MessageHandlerOverride);

														// patch spammy subroutines
		NopHdlr kDataHandlerAutoSave(0x0043083B, 5);
		NopHdlr	kAnimGroupNote(0x004CA21D, 5);
		NopHdlr kTangentSpaceCreation(0x0076989C, 5);
		NopHdlr	kHeightMapGenA(0x005E0D9D, 5), kHeightMapGenB(0x005E0DB6, 5);
		NopHdlr kModelLoadError(0x0046C215, 5);
		NopHdlr	kLoadTerrainLODQuad(0x005583F1, 5);
		NopHdlr	kFaceGenControlFreeformA(0x0044B2EA, 5), kFaceGenControlFreeFormB(0x0044B348, 5);
		NopHdlr kFaceGenControlStoringUndoA(0x004DD652, 5), kFaceGenControlStoringUndoB(0x004E8EC8, 5);

		SafeWrite8(0x00468597, 0xEB);					//		FileFinder::LogMessage
		kDataHandlerAutoSave.WriteNop();
		kAnimGroupNote.WriteNop();
		kTangentSpaceCreation.WriteNop();
		kHeightMapGenA.WriteNop();
		kHeightMapGenB.WriteNop();
		kModelLoadError.WriteNop();
		kLoadTerrainLODQuad.WriteNop();
		kFaceGenControlFreeformA.WriteNop();
		kFaceGenControlFreeFormB.WriteNop();
		kFaceGenControlStoringUndoA.WriteNop();
		kFaceGenControlStoringUndoB.WriteNop();
	}

	void __stdcall MessageHandlerOverride(const char* Message)
	{
		DebugPrint(Console::e_CS, "%s", Message);
	}

	void __stdcall DoCSExitHook(HWND MainWindow)
	{
		CONSOLE->Pad(2);
		TESDialog::WritePositionToINI(MainWindow, NULL);
		TESDialog::WritePositionToINI(*g_HWND_CellView, "Cell View");
		TESDialog::WritePositionToINI(*g_HWND_ObjectWindow, "Object Window");
		TESDialog::WritePositionToINI(*g_HWND_RenderWindow, "Render Window");
		DebugPrint("CS INI Settings Flushed");

		VersionControl::CHANGELOG->Deinitialize();
		DebugPrint("Change Log Manager Deinitialized");
		RENDERTEXT->Release();
		DebugPrint("Render Window Text Painter Released");
		CSIOM->Deinitialize();
		DebugPrint("CSInterop Manager Deinitialized");
		g_ToolManager.WriteToINI(g_INIPath.c_str());
		DebugPrint("Tool Manager Deinitialized");
		CSAutomationScript::DeitializeCSASEngine();
		DebugPrint("CSAS Deinitialized");

		DebugPrint("Deinitializing Console, Flushing CSE INI settings and Closing the CS!");
		CONSOLE->Deinitialize();
		g_INIManager->SaveSettingsToINI();
		g_INIManager->Deinitialize();

		ExitProcess(0);
	}

	#define _hhName	CSExit
	_hhBegin()
	{
		__asm
		{
			push    ebx
			call    DoCSExitHook
		}
	}

	static LPTOP_LEVEL_EXCEPTION_FILTER s_TopLevelExceptionFilter = NULL;

	bool CreateCSEMiniDump( _EXCEPTION_POINTERS *ExceptionInfo)
	{
		HANDLE DumpFile = CreateFile(PrintToBuffer("%sCSECrashDump.dmp", g_APPPath.c_str()),
									GENERIC_READ|GENERIC_WRITE,
									0,
									NULL,
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									NULL);

		if (DumpFile == INVALID_HANDLE_VALUE)
			return false;

		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId           = GetCurrentThreadId();
		mdei.ExceptionPointers  = ExceptionInfo;
		mdei.ClientPointers     = FALSE;

		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpNormal|MiniDumpWithIndirectlyReferencedMemory|MiniDumpScanMemory);

		BOOL rv = MiniDumpWriteDump(GetCurrentProcess(),
									GetCurrentProcessId(),
									DumpFile,
									mdt, (ExceptionInfo != 0) ? &mdei : 0, 0, 0 );

		if( !rv )
			return false;
		else
			DebugPrint("Minidump saved to %s", g_TextBuffer);

		CloseHandle(DumpFile);
		return true;
	}

	LONG WINAPI CSEUnhandledExceptionFilter(__in  struct _EXCEPTION_POINTERS *ExceptionInfo)
	{
		CONSOLE->Pad(2);
		DebugPrint("The CS crashed, dammit!");
		CONSOLE->Indent();
		Achievements::UnlockAchievement(Achievements::kAchievement_Saboteur);

		if (!CreateCSEMiniDump(ExceptionInfo))
		{
			DebugPrint("Couldn't create a memory dump!");
			LogWinAPIErrorMessage(GetLastError());
		}

		if (s_TopLevelExceptionFilter)
			return s_TopLevelExceptionFilter(ExceptionInfo);
		else
		{
			DebugPrint("Couldn't find a top level exception filter!");
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	void __stdcall DoCSInitHook()
	{
		if (!g_PluginPostLoad)					// prevents the hook from being called before the full init
			return;
		else if (!*g_HWND_CSParent || !*g_HWND_ObjectWindow || !*g_HWND_CellView || !*g_HWND_RenderWindow)
			return;

		_MemHdlr(CSInit).WriteBuffer();			// removed rightaway to keep it from hindering the subclassing that follows

		MersenneTwister::init_genrand(GetTickCount());

		DebugPrint("Initializing CSInterop Manager");
		CONSOLE->Indent();
		if (!CSIOM->Initialize())
			DebugPrint("CSInterop Manager failed to initialize successfully! LIP service will be unavailable during this session");
		CONSOLE->Exdent();

		DebugPrint("Initializing Achievements");
		CONSOLE->Indent();
		Achievements::Initialize();
		CONSOLE->Exdent();

		DebugPrint("Initializing Hall of Fame");
		CONSOLE->Indent();
		if (!TESForm::LookupByEditorID("TheShadeMeRef"))	// with the new injection method, DataHandler::ConstructSpecialForms gets called before the deferred init routine
			HallOfFame::Initialize(true);					// we make sure we don't reallocate the hall of fame forms by performing a sanity check
		CONSOLE->Exdent();

		DebugPrint("Initializing ScriptEditor");
		CONSOLE->Indent();

		IntelliSenseUpdateData* GMSTCollectionData = new IntelliSenseUpdateData();
		GMSTCollectionData->GMSTCount = g_GMSTCollection->GetGMSTCount();
		GMSTCollectionData->GMSTListHead = new GMSTData[GMSTCollectionData->GMSTCount];
		g_GMSTCollection->SerializeGMSTDataForHandShake(GMSTCollectionData->GMSTListHead);
		CLIWrapper::ScriptEditor::InitializeComponents(&g_CommandTableData, GMSTCollectionData);
		delete [] GMSTCollectionData->GMSTListHead;
		delete GMSTCollectionData;

		CONSOLE->Indent();
		for (std::map<std::string, std::string>::const_iterator Itr = g_URLMapBuffer.begin(); Itr != g_URLMapBuffer.end(); Itr++)
			CLIWrapper::ScriptEditor::AddToURLMap(Itr->first.c_str(), Itr->second.c_str());
		DebugPrint("IntelliSense: Bound %d developer URLs", g_URLMapBuffer.size());
		g_URLMapBuffer.clear();
		CONSOLE->Exdent();

		CONSOLE->Exdent();

		UInt32 CommandCount = 0;
		for (const CommandInfo* Itr = g_CommandTableData.CommandTableStart; Itr < g_CommandTableData.CommandTableEnd; ++Itr)
		{
			if (!_stricmp(Itr->longName, ""))
				continue;

			CommandCount++;
		}

		DebugPrint("Initializing Intellisense Database Update Thread");
		CLIWrapper::ScriptEditor::InitializeDatabaseUpdateTimer();

		DebugPrint("Initializing CSAS");
		CONSOLE->Indent();
		CSAutomationScript::InitializeCSASEngine();
		CONSOLE->Exdent();

		DebugPrint("Initializing Tools");
		CONSOLE->Indent();
		g_ToolManager.InitializeToolsMenu();
		g_ToolManager.ReadFromINI(g_INIPath.c_str());
		g_ToolManager.ReloadToolsMenu();
		CONSOLE->Exdent();

		DebugPrint("Initializing Workspace Manager");
		CONSOLE->Indent();
		g_WorkspaceManager.Initialize(g_APPPath.c_str());
		CONSOLE->Exdent();

		DebugPrint("Initializing Window Manager");
		CONSOLE->Indent();
		InitializeWindowManager();
		CONSOLE->Exdent();

		DebugPrint("Initializing Console");
		CONSOLE->Indent();
		CONSOLE->InitializeConsole();
		CONSOLE->LoadINISettings();
		CONSOLE->Exdent();

		DebugPrint("Initializing GMST Default Copy");
		CONSOLE->Indent();
		g_GMSTCollection->CreateDefaultCopy();
		CONSOLE->Exdent();

		DebugPrint("Initializing IdleAnim Tree");
		CONSOLE->Indent();
		TESIdleForm::InitializeIdleFormTreeRootNodes();
		CONSOLE->Exdent();

		DebugPrint("Initializing Archives");
		CONSOLE->Indent();
		ArchiveManager::LoadSkippedArchives((std::string(g_APPPath + "Data\\")).c_str());
		CONSOLE->Exdent();

		DebugPrint("Initializing Render Window Text Painter");
		CONSOLE->Indent();
		g_RenderWindowTimeManager.Update();
		RENDERTEXT->Initialize();
		CONSOLE->Exdent();

		DebugPrint("Initializing Change Log Manager");
		CONSOLE->Indent();
		VersionControl::CHANGELOG->Initialize();
		CONSOLE->Exdent();

		DebugPrint("Initializing CS Startup Manager");
		CONSOLE->Indent();
		CSStartupManager::LoadStartupWorkspace();
		CSStartupManager::LoadStartupPlugin();
		CSStartupManager::LoadStartupScript();
		CONSOLE->Exdent();

		CONSOLE->ExdentAll();
		DebugPrint("Construction Set Extender Initialized!");
		CONSOLE->Pad(2);

		Achievements::UnlockAchievement(Achievements::kAchievement_TheWiseOne);
		if (!g_INIManager->FetchSetting("LogCSWarnings")->GetValueAsInteger())
			Achievements::UnlockAchievement(Achievements::kAchievement_FlyingBlind);
		if (CommandCount >= Achievements::kMaxScriptCommandCount)
			Achievements::UnlockAchievement(Achievements::kAchievement_Commandant);

		s_TopLevelExceptionFilter = SetUnhandledExceptionFilter(CSEUnhandledExceptionFilter);
	}

	#define _hhName	CSInit
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00419265);
		_hhSetVar(Call, 0x006E5850);
		__asm
		{
			call	[_hhGetVar(Call)]
			call	DoCSInitHook
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoAssertOverrideHook(UInt32 EIP)
	{
		Achievements::UnlockAchievement(Achievements::kAchievement_WTF);

		if (!g_INIManager->FetchSetting("LogAssertions")->GetValueAsInteger())
			return;

		CONSOLE->Indent();
		DebugPrint("Assertion handled at 0x%08X", EIP);
		CONSOLE->Exdent();

		MessageBeep(MB_ICONHAND);
	}

	#define _hhName	AssertOverride
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004B575E);
		__asm
		{
			mov		eax, [esp]
			sub		eax, 5
			pushad
			push	eax
			call	DoAssertOverrideHook
			popad
			xor		eax, eax

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	PluginSave
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0041BBD3);
		__asm
		{
			call	SetWindowTextAddress
			call	[g_WindowHandleCallAddr]				// SetWindowTextA
			pushad
			push	10
			call	SendPingBack
			popad
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	PluginLoad
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0041BEFF);
		_hhSetVar(Call, 0x00430980);
		__asm
		{
			call	[_hhGetVar(Call)]

			pushad
			push	9
			call	SendPingBack
			call	FormEnumerationWrapper::ResetFormVisibility
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DestroyShadeMeRef(void)
	{
		TESForm* Ref = TESForm::LookupByEditorID("TheShadeMeRef");
		if (Ref)
			Ref->DeleteInstance();
	}
	void __stdcall ClearRenderSelectionGroupMap(void)
	{
		g_RenderSelectionGroupManager.Clear();
	}
	void __stdcall ClearGMSTCollection(void)
	{
		g_GMSTCollection->ResetCollection();
	}

	#define _hhName	DataHandlerClearData
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0047AE7B);
		__asm
		{
			lea     edi, [ebx+44h]
			mov     ecx, edi
			pushad
			call	DestroyShadeMeRef
			call	ClearRenderSelectionGroupMap
			call	ClearGMSTCollection
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	TopicInfoCopyEpilog
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004F1286);
		__asm
		{
			pushad
			mov		eax, [esi]
			mov		eax, [eax + 0x94]		// SetFromActiveFile
			push	1
			mov		ecx, esi
			call	eax
			popad

			mov     [esi + 0x30], bx
			mov     eax, [edi]
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoNumericEditorIDHook(TESForm* Form, const char* EditorID)
	{
		if (g_INIManager->FetchSetting("ShowNumericEditorIDWarning")->GetValueAsInteger() &&
			g_PluginPostLoad &&
			strlen(EditorID) > 0 &&
			isdigit((int)*EditorID) &&
			(Form->formFlags & TESForm::kFormFlags_Temporary) == 0)
		{
			PrintToBuffer("The editorID '%s' begins with an integer.\n\nWhile this is generally accepted by the engine, scripts referring this form might fail to run or compile as the script compiler can attempt to parse it as an integer.\n\nConsider starting the editorID with an alphabet.", EditorID);
			MessageBox(*g_HWND_CSParent, g_TextBuffer, "CSE", MB_OK|MB_ICONWARNING);
		}
	}

	#define _hhName	NumericEditorID
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00497676);
		__asm
		{
			mov		eax, [esp + 0x4]
			pushad
			push	eax
			push	ecx
			call	DoNumericEditorIDHook
			popad

			xor		eax, eax
			push	ebp
			mov		ebp, esp
			sub		esp, 0x10
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	DataHandlerConstructSpecialForms
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0048104E);
		_hhSetVar(Call, 0x00505070);
		__asm
		{
			pushad
			push	0
			call	HallOfFame::Initialize
			popad

			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	ResultScriptSaveForm
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004FD260);
		__asm
		{
			mov		eax, [ecx]
			mov		edx, [eax + 0x8]
			test	edx, edx
			jz		FAIL

			jmp		[_hhGetVar(Retn)]
		FAIL:
			mov		eax, 0x004FD271
			jmp		eax
		}
	}

	#define _hhName	TESObjectREFRDoCopyFrom
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00547668);
		_hhSetVar(Call1, 0x00460380);
		_hhSetVar(Call2, 0x0053F7A0);
		__asm
		{
			pushad
			push	ebx
			mov		ecx, ebp
			call	[_hhGetVar(Call1)]		// ExtraDataList_CopyList
			popad

			pushad
			mov		ecx, edi
			call	[_hhGetVar(Call2)]		// TESObjectREFR::RemoveExtraTeleport
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoTESFormAddReferenceHook(FormCrossReferenceListT* ReferenceList, TESForm* Form)
	{
		FormCrossReferenceData* Data = FormCrossReferenceData::FindDataInRefList(ReferenceList, Form);
		if (Data)
			Data->IncrementRefCount();
		else
		{
			FormCrossReferenceData* NewNode = FormCrossReferenceData::CreateInstance(Form);
			ReferenceList->AddAt(NewNode, eListEnd);
		}
	}

	#define _hhName	TESFormAddReference
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00496461);
		__asm
		{
			push	edi
			push	eax
			call	DoTESFormAddReferenceHook
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoTESFormRemoveReferenceHook(TESForm* Parent, tList<FormCrossReferenceData>* ReferenceList, TESForm* Form)
	{
		FormCrossReferenceData* Data = FormCrossReferenceData::FindDataInRefList(ReferenceList, Form);
		if (Data)
		{
			if (Data->DecrementRefCount() == 0)
			{
				ReferenceList->Remove(Data);
				Data->DeleteInstance();
			}

			if (ReferenceList->IsEmpty())
				Parent->CleanupCrossReferenceList();
		}
	}

	#define _hhName	TESFormRemoveReference
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004964AE);
		__asm
		{
			push	ebx
			push	esi
			push	edi
			call	DoTESFormRemoveReferenceHook
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoTESFormClearReferenceListHook(FormCrossReferenceListT* ReferenceList)
	{
		for (FormCrossReferenceListT::Iterator Itr = ReferenceList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
		{
			FormCrossReferenceData* Data = Itr.Get();
			Data->DeleteInstance();
		}
		ReferenceList->RemoveAll();
	}

	#define _hhName	TESFormClearReferenceList
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00496423);
		__asm
		{
			pushad
			push	ecx
			call	DoTESFormClearReferenceListHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	TESFormPopulateUseInfoList
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004964FB);
		_hhSetVar(Jump, 0x00496509);
		_hhSetVar(Call, 0x004FC950);
		__asm
		{
			call	[_hhGetVar(Call)]
			test	eax, eax
			jz		FAIL

			mov		eax, [eax]
			test	eax, eax
			jz		FAIL

			jmp		[_hhGetVar(Retn)]
		FAIL:
			jmp		[_hhGetVar(Jump)]
		}
	}

	#define _hhName	TESFormDelete
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00498717);
		_hhSetVar(Jump, 0x0049872D);
		_hhSetVar(Call, 0x004FC950);
		__asm
		{
			call	[_hhGetVar(Call)]
			cmp		eax, ebx
			jz		FAIL

			mov		eax, [eax]
			jmp		[_hhGetVar(Retn)]
		FAIL:
			jmp		[_hhGetVar(Jump)]
		}
	}

	#define _hhName	DataHandlerPlaceTESObjectLIGH
	_hhBegin()
	{
		_hhSetVar(Retn, 0x005116CF);
		_hhSetVar(Jump, 0x00511749);
		__asm
		{
			test	esi, esi
			jz		FIX
			mov     eax, [esi]
			mov     edx, [eax + 0x1A0]
			jmp		[_hhGetVar(Retn)]
		FIX:
			jmp		[_hhGetVar(Jump)]
		}
	}

	#define _hhName	TESWorldSpaceDestroyCellMapA
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00560703);
		__asm
		{
			mov		eax, [esi + eax * 4]
			cmp		eax, 0x10000
			jle		FIX
			jmp		[_hhGetVar(Retn)]
		FIX:
			xor		eax, eax
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	TESWorldSpaceDestroyCellMapB
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0079CA38);
		_hhSetVar(Jump, 0x0079CA5A);
		__asm
		{
			mov     edi, [ecx + ebx * 4]
			cmp		edi, 0x10000
			jle		FIX

			mov		edx, [edi]
			jmp		[_hhGetVar(Retn)]
		FIX:
			xor		edi, edi
			jmp		[_hhGetVar(Jump)]
		}
	}

	#define _hhName	AchievementAddTopic
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004F2ED9);
		_hhSetVar(Call, 0x004F5D20);
		__asm
		{
			pushad
			push	13		// Achievements::kAchievement_CardinalSin
			call	Achievements::UnlockAchievement
			popad

			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(Retn)]
		}
	}

	bool __stdcall CheckIsFormShadeMe(TESForm* Form)
	{
		TESForm* shadeMe = TESForm::LookupByEditorID("shadeMe");
		if (Form == shadeMe)
			return true;
		else
			return false;
	}

	#define _hhName	AchievementDeleteShadeMe
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004986B9);
		_hhSetVar(Call, 0x00401EA0);
		__asm
		{
			pushad
			push	esi
			call	CheckIsFormShadeMe
			test	al, al
			jz		EXIT

			push	6		// Achievements::kAchievement_AntiChrist
			call	Achievements::UnlockAchievement
		EXIT:
			popad

			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	AchievementModifyShadeMe
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00497BEF);
		__asm
		{
			pushad
			push	esi
			call	CheckIsFormShadeMe
			test	al, al
			jz		EXIT

			push	4		// Achievements::kAchievement_Heretic
			call	Achievements::UnlockAchievement
		EXIT:
			popad

			mov     eax, [esi + 0x8]
			mov     ecx, eax
			jmp		[_hhGetVar(Retn)]
		}
	}

	bool __stdcall DoAchievementCloneHallOfFameForms(TESForm* Form)
	{
		if (Form->formID >= HallOfFame::kBaseFormID && Form->formID < 0x800)
			return true;
		else
			return false;
	}

	#define _hhName	AchievementCloneHallOfFameForms
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00413E54);
		_hhSetVar(Call, 0x004793F0);
		__asm
		{
			pushad
			push	edi
			call	DoAchievementCloneHallOfFameForms
			test	al, al
			jz		EXIT

			push	8		// Achievements::kAchievement_MadScientist
			call	Achievements::UnlockAchievement
		EXIT:
			popad

			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoAchievementPluginDescriptionHook(TESFile* File)
	{
		std::stringstream DescriptionStream(File->description.c_str(), std::ios::in);
		char Buffer[0x200] = {0};

		while (DescriptionStream.eof() == false)
		{
			ZeroMemory(Buffer, sizeof(Buffer));
			DescriptionStream.getline(Buffer, sizeof(Buffer));

			std::string Line(Buffer);
			MakeLower(Line);
			if (Line.find("version:") != -1 ||
				Line.find("version :") != -1 ||
				Line.find("version-") != -1 ||
				Line.find("version -") != -1 ||
				Line.find("version=") != -1 ||
				Line.find("version =") != -1)
			{
				Achievements::UnlockAchievement(Achievements::kAchievement_Pedantic);
				break;
			}
		}
	}

	#define _hhName	AchievementPluginDescription
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0040CD25);
		_hhSetVar(Call, 0x004894B0);
		__asm
		{
			pushad
			push	ecx
			call	DoAchievementPluginDescriptionHook
			popad

			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	AchievementBuildRoads
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00563D04);
		__asm
		{
			pushad
			push	18		// Achievements::kAchievement_RoadBuilder
			call	Achievements::UnlockAchievement
			popad

			call	WriteStatusBarText
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName	AchievementDialogResponseCreation
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004F2CC8);
		_hhSetVar(Call, 0x004EA510);
		__asm
		{
			pushad
			push	19		// Achievements::kAchievement_Loquacious
			call	Achievements::UnlockAchievement
			popad

			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(Retn)]
		}
	}

	void _stdcall DoTESDialogBuildSubwindowDiagnosticsHook(void)
	{
		MessageBox(*g_HWND_CSParent, "TESDialog::BuildSubwindow() failed!\n\nError deatils logged to the console.", "CSE", MB_TOPMOST|MB_OK|MB_ICONERROR);
		DebugPrint("TESDialog::BuildSubwindow() returned 0");
		LogWinAPIErrorMessage(GetLastError());
	}

	#define _hhName	TESDialogBuildSubwindowDiagnostics
	_hhBegin()
	{
		__asm
		{
			call	DoTESDialogBuildSubwindowDiagnosticsHook
			xor		al, al
			pop		esi
			retn
		}
	}

	#define _hhName	ExtraTeleportInitItem
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0046270E);
		_hhSetVar(Jump, 0x004626F2);
		__asm
		{
			mov     ecx, [esi]
			mov     edx, [ecx]
			mov     eax, [edx + 0x19C]
			call    eax

			test	eax, eax
			jz		SKIP

			jmp		[_hhGetVar(Retn)]
		SKIP:
			jmp		[_hhGetVar(Jump)]
		}
	}
}