#include "Misc.h"
#include "Dialog.h"
#include "Renderer.h"
#include "..\ToolManager.h"
#include "..\WorkspaceManager.h"
#include "..\RenderWindowTextPainter.h"
#include "..\RenderSelectionGroupManager.h"
#include "..\ElapsedTimeCounter.h"
#include "..\CSEInterfaceManager.h"
#include "..\ChangeLogManager.h"
#include "..\Achievements.h"
#include "CSAS\ScriptRunner.h"
#include "AuxiliaryViewport.h"

#pragma warning(push)
#pragma optimize("", off)
#pragma warning(disable: 4005 4748)

extern StaticRenderChannel*			g_TXTChannelSelectionInfo;
extern StaticRenderChannel*			g_TXTChannelRAMUsage;
extern DynamicRenderChannel*		g_TXTChannelNotifications;

namespace Hooks
{
	_DefineHookHdlrWithBuffer(CSInit, 0x00419260, 5, 0xE8, 0xEB, 0xC5, 0x2C, 0x0);
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
	_DefineHookHdlr(ExtraTeleportInitItem, 0x00462702);
	_DefineHookHdlr(NewSplashImage, 0x00441D73);
	_DefinePatchHdlr(AllowMultipleEditors, 0x0041C7E1);

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
		_MemHdlr(ExtraTeleportInitItem).WriteJump();
	}

	void PatchEntryPointHooks(void)
	{
		_MemHdlr(CSRegistryEntries).WriteJump();
		_MemHdlr(NewSplashImage).WriteJump();
		_MemHdlr(AllowMultipleEditors).WriteUInt8(0xEB);
	}

	void __stdcall MessageHandlerOverride(const char* Message)
	{
		DebugPrint(Console::e_CS, "%s", Message);
	}

	void PatchMessageHanders(void)
	{
		// patch spammed output function calls to improve performance
		_DefinePatchHdlr(FileFinderLogMessage, 0x00468597);
		_DefineNopHdlr(DataHandlerAutoSave, 0x0043083B, 5);
		_DefineNopHdlr(AnimGroupNote, 0x004CA21D, 5);
		_DefineNopHdlr(TangentSpaceCreation, 0x0076989C, 5);
		_DefineNopHdlr(HeightMapGenA, 0x005E0D9D, 5);
		_DefineNopHdlr(HeightMapGenB, 0x005E0DB6, 5);
		_DefineNopHdlr(ModelLoadError, 0x0046C215, 5);
		_DefineNopHdlr(LoadTerrainLODQuad, 0x005583F1, 5);
		_DefineNopHdlr(FaceGenControlFreeformA, 0x0044B2EA, 5);
		_DefineNopHdlr(FaceGenControlFreeFormB, 0x0044B348, 5);
		_DefineNopHdlr(FaceGenControlStoringUndoA, 0x004DD652, 5);
		_DefineNopHdlr(FaceGenControlStoringUndoB, 0x004E8EC8, 5);
		_DefineNopHdlr(DataHandlerConstructObjectA, 0x004838D2, 5);
		_DefineNopHdlr(DataHandlerConstructObjectB, 0x00483C89, 5);
		_DefineNopHdlr(DataHandlerConstructObjectC, 0x00483D53, 5);
		_DefineNopHdlr(DataHandlerConstructObjectD, 0x0048403C, 5);
		_DefineNopHdlr(DataHandlerConstructObjectE, 0x00484137, 5);
		_DefineNopHdlr(DataHandlerLoadPluginsA, 0x0048557F, 5);
		_DefineNopHdlr(DataHandlerLoadPluginsB, 0x00484BDE, 5);
		_DefineNopHdlr(SpeedTreeMultiBound, 0x00596617, 5);
		_DefineNopHdlr(BackgroundLoaderRefs, 0x0046D3B6, 5);
		_DefineNopHdlr(TESLoadCellCrapA, 0x0052B75A, 5);
		_DefineNopHdlr(TESLoadCellCrapB, 0x00477647, 5);
		_DefineNopHdlr(TESPathGridGenerateNiNode, 0x0054ED59, 5);
		_DefineNopHdlr(TESWorldspaceBuildRoadsA, 0x00563C09, 5);
		_DefineNopHdlr(TESWorldspaceBuildRoadsB, 0x00563C71, 5);

		_MemHdlr(FileFinderLogMessage).WriteUInt8(0xEB);
		_MemHdlr(DataHandlerAutoSave).WriteNop();
		_MemHdlr(AnimGroupNote).WriteNop();
		_MemHdlr(TangentSpaceCreation).WriteNop();
		_MemHdlr(HeightMapGenA).WriteNop();
		_MemHdlr(HeightMapGenB).WriteNop();
		_MemHdlr(ModelLoadError).WriteNop();
		_MemHdlr(LoadTerrainLODQuad).WriteNop();
		_MemHdlr(FaceGenControlFreeformA).WriteNop();
		_MemHdlr(FaceGenControlFreeFormB).WriteNop();
		_MemHdlr(FaceGenControlStoringUndoA).WriteNop();
		_MemHdlr(FaceGenControlStoringUndoB).WriteNop();
		_MemHdlr(DataHandlerConstructObjectA).WriteNop();
		_MemHdlr(DataHandlerConstructObjectB).WriteNop();
		_MemHdlr(DataHandlerConstructObjectC).WriteNop();
		_MemHdlr(DataHandlerConstructObjectD).WriteNop();
		_MemHdlr(DataHandlerConstructObjectE).WriteNop();
		_MemHdlr(DataHandlerLoadPluginsA).WriteNop();
		_MemHdlr(DataHandlerLoadPluginsB).WriteNop();
		_MemHdlr(SpeedTreeMultiBound).WriteNop();
		_MemHdlr(BackgroundLoaderRefs).WriteNop();
		_MemHdlr(TESLoadCellCrapA).WriteNop();
		_MemHdlr(TESLoadCellCrapB).WriteNop();
		_MemHdlr(TESPathGridGenerateNiNode).WriteNop();
		_MemHdlr(TESWorldspaceBuildRoadsA).WriteNop();
		_MemHdlr(TESWorldspaceBuildRoadsB).WriteNop();


		if (!g_INIManager->FetchSetting("LogCSWarnings", "Extender::Console")->GetValueAsInteger())
			return;

		const UInt32 kMessageHandlerVTBL = 0x00940760;

		SafeWrite32(kMessageHandlerVTBL + 0, (UInt32)&MessageHandlerOverride);
		SafeWrite32(kMessageHandlerVTBL + 0x4, (UInt32)&MessageHandlerOverride);
		SafeWrite32(kMessageHandlerVTBL + 0x8, (UInt32)&MessageHandlerOverride);
		SafeWrite32(kMessageHandlerVTBL + 0x10, (UInt32)&MessageHandlerOverride);
		SafeWrite32(kMessageHandlerVTBL + 0x14, (UInt32)&MessageHandlerOverride);
		SafeWrite32(kMessageHandlerVTBL + 0x18, (UInt32)&MessageHandlerOverride);
		SafeWrite32(kMessageHandlerVTBL + 0x1C, (UInt32)&MessageHandlerOverride);
		SafeWrite32(kMessageHandlerVTBL + 0x20, (UInt32)&MessageHandlerOverride);
		SafeWrite32(kMessageHandlerVTBL + 0x24, (UInt32)&MessageHandlerOverride);
	}

	static LPTOP_LEVEL_EXCEPTION_FILTER s_TopLevelExceptionFilter = NULL;

	bool CreateCSEMiniDump( _EXCEPTION_POINTERS *ExceptionInfo)
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%sCSECrashDump.dmp", g_APPPath.c_str());

		HANDLE DumpFile = CreateFile(Buffer,
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
			DebugPrint("Minidump saved to %s", Buffer);

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

		CONSOLE->OpenDebugLog();

		if (s_TopLevelExceptionFilter)
			return s_TopLevelExceptionFilter(ExceptionInfo);
		else
			return EXCEPTION_CONTINUE_EXECUTION;
	}

	void __stdcall DoCSInitHook()
	{
		if (!g_PluginPostLoad)					// ### this check not necessary with the new injection method
			return;
		else if (!*g_HWND_CSParent || !*g_HWND_ObjectWindow || !*g_HWND_CellView || !*g_HWND_RenderWindow)
			return;								// prevents the hook from being called before the full init

		_MemHdlr(CSInit).WriteBuffer();			// removed right away to keep it from hindering the subclassing that follows

		MersenneTwister::init_genrand(GetTickCount());

		DebugPrint("Initializing Component DLL Interfaces");
		CONSOLE->Indent();
		CLIWrapper::QueryInterfaces();
		CONSOLE->Exdent();

		DebugPrint("Initializing Console");
		CONSOLE->Indent();
		CONSOLE->InitializeConsoleWindow();
		CONSOLE->Exdent();

		DebugPrint("Initializing Auxiliary Viewport");
		CONSOLE->Indent();
		AUXVIEWPORT->Initialize();
		CONSOLE->Exdent();

		DebugPrint("Initializing CSInterop Manager");
		CONSOLE->Indent();
		if (!CSIOM->Initialize())
			DebugPrint("CSInterop Manager failed to initialize successfully! Lip service will be unavailable during this session");
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

		ComponentDLLInterface::IntelliSenseUpdateData* GMSTCollectionData = new ComponentDLLInterface::IntelliSenseUpdateData();
		GMSTCollectionData->GMSTCount = g_GMSTCollection->GetGMSTCount();
		GMSTCollectionData->GMSTListHead = new ComponentDLLInterface::GMSTData[GMSTCollectionData->GMSTCount];
		g_GMSTCollection->SerializeGMSTDataForHandShake(GMSTCollectionData->GMSTListHead);
		CLIWrapper::Interfaces::SE->InitializeComponents(&g_CommandTableData, GMSTCollectionData);
		delete GMSTCollectionData;

		CONSOLE->Indent();
		for (std::map<std::string, std::string>::const_iterator Itr = g_URLMapBuffer.begin(); Itr != g_URLMapBuffer.end(); Itr++)
			CLIWrapper::Interfaces::SE->AddScriptCommandDeveloperURL(Itr->first.c_str(), Itr->second.c_str());
		DebugPrint(Console::e_SE, "Bound %d developer URLs", g_URLMapBuffer.size());
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

		DebugPrint("Initializing CSAS Engine");
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

		DebugPrint("Initializing GMST Default Copy");
		CONSOLE->Indent();
		g_GMSTCollection->CreateDefaultCopy();
		CONSOLE->Exdent();

		DebugPrint("Initializing IdleAnim Tree");
		CONSOLE->Indent();
		TESIdleForm::InitializeIdleFormTreeRootNodes();
		CONSOLE->Exdent();

		DebugPrint("Initializing Archive Manager");
		CONSOLE->Indent();
		ArchiveManager::LoadSkippedArchives((std::string(g_APPPath + "Data\\")).c_str());
		CONSOLE->Exdent();

		DebugPrint("Initializing Render Window Text Painter");
		CONSOLE->Indent();
		RECT DrawRect;

		DrawRect.left = 3;
		DrawRect.top = 3;
		DrawRect.right = 800;
		DrawRect.bottom = 600;
		g_TXTChannelSelectionInfo = new StaticRenderChannel(20, 0, FW_MEDIUM, "Consolas",
															D3DCOLOR_ARGB(220, 189, 237, 99),
															&DrawRect,
															DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP,
															0,
															&TXTChannelStaticHandler_RenderSelectionInfo);
		DrawRect.left = -173;
		DrawRect.top = 3;
		DrawRect.right = 173;
		DrawRect.bottom = 100;
		g_TXTChannelRAMUsage = new StaticRenderChannel(20, 0, FW_MEDIUM, "Consolas",
														D3DCOLOR_ARGB(230, 230, 230, 0),
														&DrawRect,
														DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP,
														RenderChannelBase::kDrawAreaFlags_RightAligned,
														&TXTChannelStaticHandler_RAMUsage);
		DrawRect.left = 3;
		DrawRect.top = -150;
		DrawRect.right = 800;
		DrawRect.bottom = 200;
		g_TXTChannelNotifications = new DynamicRenderChannel(20, 0, FW_MEDIUM, "Consolas",
														D3DCOLOR_ARGB(230, 230, 230, 0),
														&DrawRect,
														DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP,
														RenderChannelBase::kDrawAreaFlags_BottomAligned);

		RENDERTEXT->RegisterRenderChannel(g_TXTChannelSelectionInfo);
		RENDERTEXT->RegisterRenderChannel(g_TXTChannelRAMUsage);
		RENDERTEXT->RegisterRenderChannel(g_TXTChannelNotifications);
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
		if (!g_INIManager->FetchSetting("LogCSWarnings", "Extender::Console")->GetValueAsInteger())
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

	void __stdcall DoCSExitHook(HWND MainWindow)
	{
		CONSOLE->Pad(2);
		TESDialog::WritePositionToINI(MainWindow, NULL);
		TESDialog::WritePositionToINI(*g_HWND_CellView, "Cell View");
		TESDialog::WritePositionToINI(*g_HWND_ObjectWindow, "Object Window");
		TESDialog::WritePositionToINI(*g_HWND_RenderWindow, "Render Window");
		g_INIManager->GetINI("RenderWindowState", "Extender::Dialogs")->SetValue((GetMenuState(*g_HMENU_MainMenu, 40423, MF_BYCOMMAND) & MF_CHECKED)?"1":"0");
		g_INIManager->GetINI("ObjectWindowState", "Extender::Dialogs")->SetValue((GetMenuState(*g_HMENU_MainMenu, 40199, MF_BYCOMMAND) & MF_CHECKED)?"1":"0");
		g_INIManager->GetINI("CellWindowState", "Extender::Dialogs")->SetValue((GetMenuState(*g_HMENU_MainMenu, 40200, MF_BYCOMMAND) & MF_CHECKED)?"1":"0");
		DebugPrint("Flushed CS INI Settings");

		DebugPrint("Deinitializing Script Editor");
		CONSOLE->Indent();
		CLIWrapper::Interfaces::SE->CleanupAutoRecoveryCache();
		CONSOLE->Exdent();

		DebugPrint("Deinitializing Change Log Manager");
		CONSOLE->Indent();
		VersionControl::CHANGELOG->Deinitialize();
		CONSOLE->Exdent();

		DebugPrint("Deinitializing Render Window Text Painter");
		CONSOLE->Indent();
		RENDERTEXT->Deinitialize();
		CONSOLE->Exdent();

		DebugPrint("Deinitializing Auxiliary Viewport");
		CONSOLE->Indent();
		AUXVIEWPORT->Deinitialize();
		CONSOLE->Exdent();

		DebugPrint("Deinitializing CSInterop Manager");
		CONSOLE->Indent();
		CSIOM->Deinitialize();
		CONSOLE->Exdent();

		DebugPrint("Deinitializing Tool Manager");
		CONSOLE->Indent();
		g_ToolManager.WriteToINI(g_INIPath.c_str());
		CONSOLE->Exdent();

		DebugPrint("Deinitializing CSAS Engine");
		CONSOLE->Indent();
		CSAutomationScript::DeitializeCSASEngine();
		CONSOLE->Exdent();

		DebugPrint("Deinitializing Console, Flushing CSE INI settings and Closing the CS!");
		CONSOLE->Deinitialize();
		g_INIManager->SaveSettingsToINI();
		g_INIManager->Deinitialize();

		CoUninitialize();
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

	void __stdcall DoAssertOverrideHook(UInt32 EIP)
	{
		Achievements::UnlockAchievement(Achievements::kAchievement_WTF);

		if (!g_INIManager->FetchSetting("LogAssertions", "Extender::Console")->GetValueAsInteger())
			return;

		CONSOLE->Indent();
		DebugPrint("ASSERTION FAILED: 0x%08X", EIP);
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
			call	IATCacheSetWindowTextAddress
			call	[g_TempIATProcBuffer]				// SetWindowTextA
			pushad
		}
		CLIWrapper::Interfaces::SE->UpdateIntelliSenseDatabase();
		__asm
		{
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
		}
		CLIWrapper::Interfaces::SE->UpdateIntelliSenseDatabase();
		FormEnumerationWrapper::ResetFormVisibility();
		__asm
		{
			popad
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoDataHandlerClearDataHook(void)
	{
		TESForm* shadeMeRef = TESForm::LookupByEditorID("TheShadeMeRef");
		if (shadeMeRef)
			shadeMeRef->DeleteInstance();

		g_RenderSelectionGroupManager.Clear();
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
			call	DoDataHandlerClearDataHook
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
		if (g_INIManager->FetchSetting("ShowNumericEditorIDWarning", "Extender::General")->GetValueAsInteger() &&
			g_PluginPostLoad &&
			strlen(EditorID) > 0 &&
			isdigit((int)*EditorID) &&
			(Form->formFlags & TESForm::kFormFlags_Temporary) == 0)
		{
			char Buffer[0x200] = {0};
			FORMAT_STR(Buffer, "The editorID '%s' begins with an integer.\n\nWhile this is generally accepted by the engine, scripts referring this form might fail to run or compile as the script compiler can attempt to parse it as an integer.\n\nConsider starting the editorID with an alphabet.", EditorID);
			MessageBox(*g_HWND_CSParent, Buffer, "CSE", MB_OK|MB_ICONWARNING);
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
			call	[_hhGetVar(Call1)]		// ExtraDataList::CopyList
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
		FormCrossReferenceData* Data = FormCrossReferenceData::LookupFormInCrossReferenceList(ReferenceList, Form);
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
		FormCrossReferenceData* Data = FormCrossReferenceData::LookupFormInCrossReferenceList(ReferenceList, Form);
		if (Data)
		{
			if (Data->DecrementRefCount() == 0)
			{
		//		ReferenceList->Remove(Data);		//	### possible bug in tList::Remove, corrupts the state somehow. investigate
				thisCall<UInt32>(0x00452AE0, ReferenceList, Data);
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
		if (Form->formID >= HallOfFame::kBaseFormID && Form->formID <= HallOfFame::TableSize)
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
		if (File->description.c_str() == NULL)
			return;

		std::stringstream DescriptionStream(File->description.c_str(), std::ios::in);
		char Buffer[0x200] = {0};

		while (DescriptionStream.eof() == false)
		{
			ZeroMemory(Buffer, sizeof(Buffer));
			DescriptionStream.getline(Buffer, sizeof(Buffer));

			std::string Line(Buffer); MakeLower(Line);
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

			call	TESDialog::WriteToStatusBar
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

	void __stdcall DoNewSplashImageHook(HWND Dialog)
	{
		if (g_CSESplashImage == NULL)
			g_CSESplashImage = LoadImage(g_DLLInstance, MAKEINTRESOURCE(BITMAP_SPLASH), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

		HWND PictureControl = GetDlgItem(Dialog, 1962);
		SendMessage(PictureControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_CSESplashImage);
	}

	#define _hhName	NewSplashImage
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00441D79);
		__asm
		{
			pushad
			call	IATCacheShowWindowAddress
			push	esi
			call	DoNewSplashImageHook
			popad

			call	g_TempIATProcBuffer
			jmp		[_hhGetVar(Retn)]
		}
	}
}

#pragma warning(pop)
#pragma optimize("", on)