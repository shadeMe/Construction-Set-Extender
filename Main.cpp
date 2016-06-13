#include "Main.h"
#include "PluginAPIManager.h"
#include "VersionInfo.h"

#include "[Common]\CLIWrapper.h"
#include "Hooks\Hooks-Dialog.h"
#include "Hooks\Hooks-LOD.h"
#include "Hooks\Hooks-Plugins.h"
#include "Hooks\Hooks-AssetSelector.h"
#include "Hooks\Hooks-ScriptEditor.h"
#include "Hooks\Hooks-Renderer.h"
#include "Hooks\Hooks-Misc.h"

#include "Achievements.h"
#include "Console.h"
#include "RenderWindowManager.h"
#include "HallOfFame.h"
#include "UIManager.h"
#include "WorkspaceManager.h"
#include "ChangeLogManager.h"
#include "AuxiliaryViewport.h"
#include "OldCSInteropManager.h"
#include "Coda\Coda.h"
#include "GlobalClipboard.h"
#include "FormUndoStack.h"

#include <bgsee\ToolBox.h>
#include <bgsee\Script\CodaVM.h>

namespace cse
{
	OBSEMessagingInterface*						XSEMsgIntfc = NULL;
	PluginHandle								XSEPluginHandle = kPluginHandle_Invalid;

	OBSECommandTableInterface*					XSECommandTableIntfc = NULL;
	componentDLLInterface::CommandTableData		XSECommandTableData;

	ReleaseNameTable							ReleaseNameTable::Instance;
	bool										shadeMeMode = false;
	bool										IsWarholAGenius = false;

	ReleaseNameTable::ReleaseNameTable() :
		bgsee::ReleaseNameTable()
	{
		RegisterRelease(6, 0, "Konniving Kelpie");
		RegisterRelease(6, 1, "Cretinous Codpiece");
		RegisterRelease(6, 2, "Talkative Badger");
		RegisterRelease(6, 3, "Drunken Glaswegian");
		RegisterRelease(6, 4, "Subterranean Homesick Alien");
		RegisterRelease(7, 0, "Patagonian Petticoat");
		RegisterRelease(7, 1, "Bull-buggering Bollocks");
		RegisterRelease(8, 0, "Dead Dove");
	}

	ReleaseNameTable::~ReleaseNameTable()
	{
		;//
	}

	InitCallbackQuery::InitCallbackQuery(const OBSEInterface* OBSE) :
		bgsee::DaemonCallback(),
		OBSE(OBSE)
	{
		;//
	}

	InitCallbackQuery::~InitCallbackQuery()
	{
		;//
	}

	bool InitCallbackQuery::Handle(void* Parameter)
	{
		BGSEECONSOLE_MESSAGE("Initializing OBSE Interfaces");
		BGSEECONSOLE->Indent();
		XSEMsgIntfc = (OBSEMessagingInterface*)OBSE->QueryInterface(kInterface_Messaging);
		XSECommandTableIntfc = (OBSECommandTableInterface*)OBSE->QueryInterface(kInterface_CommandTable);

		if (XSEMsgIntfc == NULL || XSECommandTableIntfc == NULL)
		{
			BGSEECONSOLE_MESSAGE("Messaging/CommandTable interface not found");
			return false;
		}

		XSECommandTableData.GetCommandReturnType = XSECommandTableIntfc->GetReturnType;
		XSECommandTableData.GetParentPlugin = XSECommandTableIntfc->GetParentPlugin;
		XSECommandTableData.GetRequiredOBSEVersion = XSECommandTableIntfc->GetRequiredOBSEVersion;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Component DLLs");
		BGSEECONSOLE->Indent();
		if (cliWrapper::ImportInterfaces(OBSE) == false)
			return false;
		BGSEECONSOLE->Exdent();

		return true;
	}

	InitCallbackLoad::InitCallbackLoad(const OBSEInterface* OBSE) :
		bgsee::DaemonCallback(),
		OBSE(OBSE)
	{
		;//
	}

	InitCallbackLoad::~InitCallbackLoad()
	{
		;//
	}

	bool InitCallbackLoad::Handle(void* Parameter)
	{
		BGSEECONSOLE_MESSAGE("Initializing Hooks");
		BGSEECONSOLE->Indent();
		hooks::PatchEntryPointHooks();
		hooks::PatchDialogHooks();
		hooks::PatchLODHooks();
		hooks::PatchTESFileHooks();
		hooks::PatchAssetSelectorHooks();
		hooks::PatchScriptEditorHooks();
		hooks::PatchRendererHooks();
		hooks::PatchMiscHooks();
		hooks::PatchMessageHanders();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing UI Manager");
		BGSEECONSOLE->Indent();
		bool ComponentInitialized = BGSEEUI->Initialize("TES Construction Set", LoadMenu(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDR_MAINMENU)));
		BGSEECONSOLE->Exdent();

		if (ComponentInitialized == false)
			return false;

		BGSEECONSOLE_MESSAGE("Registering OBSE Plugin Message Handlers");
		XSEMsgIntfc->RegisterListener(XSEPluginHandle, "OBSE", OBSEMessageHandler);

		BGSEECONSOLE_MESSAGE("Initializing Plugin Interface Manager");
		PluginAPIManager::Instance.Initialize();

		return true;
	}

	InitCallbackPostMainWindowInit::~InitCallbackPostMainWindowInit()
	{
		;//
	}

	bool InitCallbackPostMainWindowInit::Handle(void* Parameter)
	{
		BGSEEUI->GetSubclasser()->RegisterMainWindowSubclass(uiManager::MainWindowMenuInitSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterMainWindowSubclass(uiManager::MainWindowMenuSelectSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterMainWindowSubclass(uiManager::MainWindowMiscSubclassProc);

		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ObjectWindow, uiManager::ObjectWindowSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ObjectWindow, uiManager::CommonDialogExtraFittingsSubClassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CellView, uiManager::CellViewWindowSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CellView, uiManager::CommonDialogExtraFittingsSubClassProc);

		BGSEECONSOLE_MESSAGE("Initializing Render Window Manager");
		BGSEECONSOLE->Indent();
		bool ComponentInitialized = renderWindow::RenderWindowManager::Instance.Initialize();
		SME_ASSERT(ComponentInitialized);
		BGSEECONSOLE->Exdent();

		if (settings::dialogs::kShowMainWindowsInTaskbar.GetData().i)
		{
			bgsee::WindowStyler::StyleData RegularAppWindow = { 0 };
			RegularAppWindow.Extended = WS_EX_APPWINDOW;
			RegularAppWindow.ExtendedOp = bgsee::WindowStyler::StyleData::kOperation_OR;

			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_ObjectWindow, RegularAppWindow);
			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_CellView, RegularAppWindow);
			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_RenderWindow, RegularAppWindow);
			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_FindText, RegularAppWindow);
			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_SearchReplace, RegularAppWindow);
		}


		return true;
	}

	InitCallbackEpilog::~InitCallbackEpilog()
	{
		;//
	}

	bool InitCallbackEpilog::Handle(void* Parameter)
	{
		BGSEECONSOLE_MESSAGE("Initializing Component DLL Interfaces");
		BGSEECONSOLE->Indent();
		cliWrapper::QueryInterfaces();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Console");
		BGSEECONSOLE->Indent();
		console::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Auxiliary Viewport");
		BGSEECONSOLE->Indent();
		AUXVIEWPORT->Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing CSInterop Manager");
		BGSEECONSOLE->Indent();
		if (CSIOM->Initialize() == false)
			BGSEECONSOLE_MESSAGE("Failed to initialize successfully! Lip service will be unavailable for this session");
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Achievements");
		BGSEECONSOLE->Indent();
		achievements::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Hall of Fame");
		BGSEECONSOLE->Indent();
		hallOfFame::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing ScriptEditor");
		BGSEECONSOLE->Indent();

		componentDLLInterface::IntelliSenseUpdateData GMSTCollectionData;
		GMSTCollectionData.GMSTCount = GameSettingCollection::Instance->GetGMSTCount();
		GMSTCollectionData.GMSTListHead = new componentDLLInterface::GMSTData[GMSTCollectionData.GMSTCount];
		GameSettingCollection::Instance->SerializeGMSTDataForHandShake(GMSTCollectionData.GMSTListHead);
		cliWrapper::interfaces::SE->InitializeComponents(&XSECommandTableData, &GMSTCollectionData);

		BGSEECONSOLE->Indent();
		BGSEECONSOLE_MESSAGE("Bound %d developer URLs", PluginAPIManager::Instance.ConsumeIntelliSenseInterface());
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Coda \"Virtual Machine\"");
		BGSEECONSOLE->Indent();
		script::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Toolbox");
		BGSEECONSOLE->Indent();
		BGSEETOOLBOX->Initialize(BGSEEMAIN->INIGetter(), BGSEEMAIN->INISetter());
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Workspace Manager");
		BGSEECONSOLE->Indent();
		workspaceManager::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing UI Manager, again");
		BGSEECONSOLE->Indent();
		uiManager::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Render Window");
		BGSEECONSOLE->Indent();
		renderWindow::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing GMST Default Copy");
		BGSEECONSOLE->Indent();
		GameSettingCollection::Instance->CreateDefaultCopy();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing IdleAnim Tree");
		BGSEECONSOLE->Indent();
		TESIdleForm::InitializeIdleFormTreeRootNodes();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Archive Manager");
		BGSEECONSOLE->Indent();
		ArchiveManager::LoadSkippedArchives((std::string(std::string(BGSEEMAIN->GetAPPPath()) + "Data\\")).c_str());
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Change Log Manager");
		BGSEECONSOLE->Indent();
		changeLogManager::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Panic Save Handler");
		BGSEECONSOLE->Indent();
		_DATAHANDLER->PanicSave(true);
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Global Clipboard");
		BGSEECONSOLE->Indent();
		globalClipboard::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Form Undo Stack");
		BGSEECONSOLE->Indent();
		formUndoStack::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Startup Manager");
		BGSEECONSOLE->Indent();
		StartupManager::LoadStartupWorkspace();
		StartupManager::LoadStartupPlugin();
		StartupManager::LoadStartupScript();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE->ExdentAll();
		BGSEECONSOLE_MESSAGE("%s Initialized!", BGSEEMAIN->ExtenderGetDisplayName());
		BGSEECONSOLE->Pad(2);

		BGSEEACHIEVEMENTS->Unlock(achievements::kTheWiseOne);

		if (BGSEECONSOLE->GetLogsWarnings() == false)
			BGSEEACHIEVEMENTS->Unlock(achievements::kFlyingBlind);

		for (const CommandInfo* Itr = XSECommandTableData.CommandTableStart; Itr < XSECommandTableData.CommandTableEnd; ++Itr)
		{
			if (!_stricmp(Itr->longName, ""))
				continue;

			BGSEEACHIEVEMENTS->Unlock(achievements::kCommandant);
		}

		BGSEEACHIEVEMENTS->Unlock(achievements::kHappyBDayMoi, false, false, true);

		char UsernameBuffer[0x200] = { 0 };
		DWORD UsernameSize = sizeof(UsernameBuffer);
		GetUserName(UsernameBuffer, &UsernameSize);

		if (!_stricmp(UsernameBuffer, "shadeMe"))
			shadeMeMode = true;

#ifndef NDEBUG
		if (shadeMeMode == false)
		{
			BGSEECONSOLE->LogMsg("shadeMe", "This is a DEBUG build");
			BGSEECONSOLE->LogMsg("shadeMe", "Please proceed to your local police precinct and turn yourself in for not being me");
			BGSEECONSOLE->LogMsg("shadeMe", "Or get hold of a transmogrifier that goes BOINK!");
			BGSEECONSOLE->Pad(1);
			BGSEECONSOLE->Indent();
			BGSEECONSOLE->Indent();
			BGSEECONSOLE->Indent();
			BGSEECONSOLE->Indent();
			BGSEECONSOLE->LogMsg("shadeMe", "Thank you kindly");
			BGSEECONSOLE->LogMsg("shadeMe", "The guy who wrote this message");
			BGSEECONSOLE->ExdentAll();
			BGSEECONSOLE->Pad(2);
		}
#endif

		return true;
	}

	DeinitCallback::~DeinitCallback()
	{
		;//
	}

	bool DeinitCallback::Handle(void* Parameter)
	{
		TESDialog::WriteBoundsToINI(*TESCSMain::WindowHandle, NULL);
		TESDialog::WriteBoundsToINI(*TESCellViewWindow::WindowHandle, "Cell View");
		TESDialog::WriteBoundsToINI(*TESObjectWindow::WindowHandle, "Object Window");
		TESDialog::WriteBoundsToINI(*TESRenderWindow::WindowHandle, "Render Window");

		BGSEECONSOLE_MESSAGE("Flushed CS INI Settings");

		settings::dialogs::kRenderWindowState.SetInt((GetMenuState(*TESCSMain::MainMenuHandle,
			TESCSMain::kMainMenu_View_RenderWindow, MF_BYCOMMAND) & MF_CHECKED) != 0);
		settings::dialogs::kCellViewWindowState.SetInt((GetMenuState(*TESCSMain::MainMenuHandle,
			TESCSMain::kMainMenu_View_CellViewWindow, MF_BYCOMMAND) & MF_CHECKED) != 0);
		settings::dialogs::kObjectWindowState.SetInt((GetMenuState(*TESCSMain::MainMenuHandle,
			TESCSMain::kMainMenu_View_ObjectWindow, MF_BYCOMMAND) & MF_CHECKED) != 0);

		TESCSMain::DeinitializeCSWindows();

		BGSEECONSOLE_MESSAGE("Deinitializing Plugin Interface Manager");
		PluginAPIManager::Instance.Deinitailize();

		BGSEECONSOLE_MESSAGE("Deinitializing Render Window");
		BGSEECONSOLE->Indent();
		renderWindow::Deinitialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Achievements Manager");
		BGSEECONSOLE->Indent();
		delete BGSEEACHIEVEMENTS;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Hall Of Fame");
		BGSEECONSOLE->Indent();
		delete BGSEEHALLOFFAME;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Tool Manager");
		BGSEECONSOLE->Indent();
		delete BGSEETOOLBOX;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Workspace Manager");
		BGSEECONSOLE->Indent();
		delete BGSEEWORKSPACE;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Coda \"Virtual Machine\"");
		BGSEECONSOLE->Indent();
		delete CODAVM;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Global Clipboard");
		BGSEECONSOLE->Indent();
		delete BGSEECLIPBOARD;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Form Undo Stack");
		BGSEECONSOLE->Indent();
		delete BGSEEUNDOSTACK;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Auxiliary Viewport");
		BGSEECONSOLE->Indent();
		delete AUXVIEWPORT;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Script Editor");
		BGSEECONSOLE->Indent();
		cliWrapper::interfaces::SE->Deinitalize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Change Log Manager");
		BGSEECONSOLE->Indent();
		delete BGSEECHANGELOG;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing CSInterop Manager");
		BGSEECONSOLE->Indent();
		delete CSIOM;
		BGSEECONSOLE->Exdent();

#ifndef NDEBUG
		BGSEECONSOLE_MESSAGE("Performing Diagnostics...");
		BGSEECONSOLE->Indent();
		componentDLLInterface::DumpInstanceCounters();
		BGSEECONSOLE->Exdent();
#endif

		return true;
	}

	CrashCallback::~CrashCallback()
	{
		;//
	}

	bool CrashCallback::Handle(void* Parameter)
	{
		BGSEECONSOLE->Pad(2);
		BGSEECONSOLE_MESSAGE("The editor crashed, dammit!");
		BGSEECONSOLE->Indent();

		BGSEECONSOLE_MESSAGE("Attempting to salvage the active file...");
		BGSEECONSOLE->Indent();

		bool PanicSaved = false;
		if ((PanicSaved = _DATAHANDLER->PanicSave()))
			BGSEECONSOLE_MESSAGE("Yup, we're good! Look for the panic save file in the Backup directory");
		else
			BGSEECONSOLE_MESSAGE("Bollocks-bollocks-bollocks! No can do...");

		if (BGSEEMAIN->GetDaemon()->GetFullInitComplete())
			BGSEEACHIEVEMENTS->Unlock(achievements::kSaboteur, false, true);

		BGSEECONSOLE->Exdent();
		BGSEECONSOLE->Exdent();

		// it's highly inadvisable to do anything inside the handler apart from the bare minimum of diagnostics
		// memory allocations are a big no-no as the CRT state can potentially be corrupted...
		// ... but sod that! Achievements are more important, obviously.
		CR_CRASH_CALLBACK_INFO* CrashInfo = (CR_CRASH_CALLBACK_INFO*)Parameter;
		bool ResumeExecution = false;

		int CrashHandlerMode = settings::general::kCrashHandlerMode.GetData().i;

		if (CrashHandlerMode == kCrashHandlerMode_Terminate)
			ResumeExecution = false;
		else if (CrashHandlerMode == kCrashHandlerMode_Resume)
			ResumeExecution = true;
		else if (CrashHandlerMode == kCrashHandlerMode_Ask)
		{
			bool FunnyGuyUnlocked = BGSEEMAIN->GetDaemon()->GetFullInitComplete() &&
				(achievements::kFunnyGuy->GetUnlocked() || achievements::kFunnyGuy->GetTriggered());
			int MBFlags = MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND | MB_ICONERROR;

			if (FunnyGuyUnlocked == false)
				MBFlags |= MB_YESNOCANCEL;
			else
				MBFlags |= MB_YESNO;

			std::string Jingle = "The editor has encountered a critical error! ";
			if (PanicSaved)
				Jingle += "Unsaved changes were saved to the panic file. ";

			Jingle += "An error report will be generated shortly.\n\n";

			if (FunnyGuyUnlocked == false)
				Jingle += "Do you wish to resume execution once you've:\n   1. Prayed to your various deities\n   2. Walked the dog\n   3. Sent the author of this editor extender plugin a pile of cash\n   4. Pleaded to the editor in a soft but sultry voice, and\n   5. Crossed your appendages...\n...in hopes of preventing it from crashing outright upon selecting 'Yes' in this dialog?";
			else
				Jingle += "Do you wish to resume execution?\n\nPS: It is almost always futile to select 'Yes'.";

			switch (MessageBox(NULL, Jingle.c_str(), BGSEEMAIN->ExtenderGetShortName(), MBFlags))
			{
			case IDYES:
				ResumeExecution = true;
				break;
			case IDNO:
				ResumeExecution = false;
				break;
			case IDCANCEL:
				if (BGSEEMAIN->GetDaemon()->GetFullInitComplete())
					BGSEEACHIEVEMENTS->Unlock(achievements::kFunnyGuy, false, true);

				MessageBox(NULL, "Hah! Nice try, Bob.", BGSEEMAIN->ExtenderGetDisplayName(), MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
				break;
			}
		}

		return ResumeExecution;
	}

	void StartupManager::LoadStartupPlugin()
	{
		bool Load = settings::startup::kLoadPlugin.GetData().i;
		const char* PluginName = settings::startup::kPluginName.GetData().s;

		if (Load)
		{
			hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteJump();

			TESFile* File = _DATAHANDLER->LookupPluginByName(PluginName);
			if (File)
			{
				BGSEECONSOLE_MESSAGE("Loading plugin '%s'", PluginName);
				BGSEECONSOLE->Indent();

				File->SetLoaded(true);
				if (_stricmp(PluginName, "Oblivion.esm"))
					File->SetActive(true);

				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kToolbar_DataFiles, 0);

				BGSEECONSOLE->Exdent();
			}
			else if (strlen(PluginName) >= 1)
				BGSEECONSOLE_MESSAGE("Non-existent startup plugin '%s'", PluginName);

			hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteBuffer();
		}
	}

	void StartupManager::LoadStartupScript()
	{
		bool Load = settings::startup::kOpenScriptWindow.GetData().i;
		const char* ScriptID = settings::startup::kScriptEditorID.GetData().s;

		if (Load)
		{
			if (strcmp(ScriptID, ""))
				TESDialog::ShowScriptEditorDialog(TESForm::LookupByEditorID(ScriptID));
			else
				TESDialog::ShowScriptEditorDialog(NULL);
		}
	}

	void StartupManager::LoadStartupWorkspace()
	{
		bool Load = settings::startup::kSetWorkspace.GetData().i;
		const char* WorkspacePath = settings::startup::kWorkspacePath.GetData().s;

		if (Load)
			BGSEEWORKSPACE->SelectCurrentWorkspace(WorkspacePath);
	}

	void CSEInteropHandler(OBSEMessagingInterface::Message* Msg)
	{
		switch (Msg->type)
		{
		case 'CSEI':
			{
				BGSEECONSOLE_MESSAGE("Dispatching Plugin Interop Interface to '%s'", Msg->sender);
				BGSEECONSOLE->Indent();
				XSEMsgIntfc->Dispatch(XSEPluginHandle, 'CSEI', (void*)PluginAPIManager::Instance.GetInterface(), 4, Msg->sender);
				BGSEECONSOLE->Exdent();
			}

			break;
		}
	}

	void OBSEMessageHandler(OBSEMessagingInterface::Message* Msg)
	{
		switch (Msg->type)
		{
		case OBSEMessagingInterface::kMessage_PostLoad:
			XSECommandTableData.CommandTableStart = XSECommandTableIntfc->Start();
			XSECommandTableData.CommandTableEnd = XSECommandTableIntfc->End();
			XSEMsgIntfc->RegisterListener(XSEPluginHandle, NULL, CSEInteropHandler);
			break;
		case OBSEMessagingInterface::kMessage_PostPostLoad:
			break;
		}
	}
}

using namespace cse;

extern "C"
{
	__declspec(dllexport) bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = BGSEEMAIN_EXTENDERSHORTNAME;
		info->version = PACKED_SME_VERSION;

		XSEPluginHandle = obse->GetPluginHandle();

		if (obse->isEditor == false)
			return false;

		SME::MersenneTwister::init_genrand(GetTickCount());
		if (SME::MersenneTwister::genrand_real1() < 0.05)
			IsWarholAGenius = true;

		bgsee::INISettingDepotT CSEINISettings;

		settings::Register(CSEINISettings);
		AuxiliaryViewport::RegisterINISettings(CSEINISettings);

		bool ComponentInitialized = BGSEEMAIN->Initialize(BGSEEMAIN_EXTENDERLONGNAME,
														  (IsWarholAGenius ? "ConstruKction Set Extender" : NULL),
														  BGSEEMAIN_EXTENDERSHORTNAME,
														  ReleaseNameTable::Instance.LookupRelease(VERSION_MAJOR, VERSION_MINOR),
														  PACKED_SME_VERSION,
														  bgsee::Main::kExtenderParentEditor_TES4CS,
														  CS_VERSION_1_2,
														  obse->editorVersion,
														  obse->GetOblivionDirectory(),
														  XSEPluginHandle,
														  21,
														  obse->obseVersion,
														  CSEINISettings,
														  "v4.0.30319",
#ifdef NDEBUG
														  false,
#else
														  false,		// CLR memory profiling
#endif

#ifdef WAIT_FOR_DEBUGGER
														  true,
#else
														  false,		// wait for debugger
#endif

#ifdef NDEBUG
														  true);		// CrashRpt support
		TODO("Save debug symbols, dammit!")
#else
														  false);
#endif

		SME_ASSERT(ComponentInitialized);

		BGSEEMAIN->GetDaemon()->RegisterInitCallback(bgsee::Daemon::kInitCallback_Query, new InitCallbackQuery(obse));
		BGSEEMAIN->GetDaemon()->RegisterInitCallback(bgsee::Daemon::kInitCallback_Load, new InitCallbackLoad(obse));
		BGSEEMAIN->GetDaemon()->RegisterInitCallback(bgsee::Daemon::kInitCallback_PostMainWindowInit, new InitCallbackPostMainWindowInit());
		BGSEEMAIN->GetDaemon()->RegisterInitCallback(bgsee::Daemon::kInitCallback_Epilog, new InitCallbackEpilog());
		BGSEEMAIN->GetDaemon()->RegisterDeinitCallback(new DeinitCallback());
		BGSEEMAIN->GetDaemon()->RegisterCrashCallback(new CrashCallback());

		if (BGSEEMAIN->GetDaemon()->ExecuteInitCallbacks(bgsee::Daemon::kInitCallback_Query) == false)
		{
			MessageBox(NULL,
					   "The Construction Set Extender failed to initialize correctly!\n\nIt's highly advised that you close the CS right away. More information can be found in the log file (Construction Set Extender.log in the root game directory).",
					   "The Cyrodiil Bunny Ranch",
					   MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONERROR | MB_OK);

			BGSEECONSOLE->OpenDebugLog();
			return false;
		}

		return true;
	}

	__declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		if (BGSEEMAIN->GetDaemon()->ExecuteInitCallbacks(bgsee::Daemon::kInitCallback_Load) == false)
		{
			MessageBox(NULL,
					   "The Construction Set Extender failed to load correctly!\n\nIt's highly advised that you close the CS right away. More information can be found in the log file (Construction Set Extender.log in the root game directory).",
					   "Rumpy-Pumpy!!",
					   MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONERROR | MB_OK);

			BGSEECONSOLE->OpenDebugLog();
			return false;
		}

		return true;
	}
};