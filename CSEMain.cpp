#include "CSEMain.h"
#include "CSEInterfaceManager.h"
#include "VersionInfo.h"

#include "[Common]\CLIWrapper.h"
#include "Hooks\Hooks-Dialog.h"
#include "Hooks\Hooks-LOD.h"
#include "Hooks\Hooks-Plugins.h"
#include "Hooks\Hooks-AssetSelector.h"
#include "Hooks\Hooks-ScriptEditor.h"
#include "Hooks\Hooks-Renderer.h"
#include "Hooks\Hooks-Misc.h"

#include "CSEAchievements.h"
#include "CSEConsole.h"
#include "CSERenderWindowPainter.h"
#include "CSERenderWindowFlyCamera.h"
#include "CSEHallOfFame.h"
#include "CSEUIManager.h"
#include "CSEWorkspaceManager.h"
#include "CSEChangeLogManager.h"
#include "CSEAuxiliaryViewport.h"
#include "CSInterop.h"
#include "Coda\CSECoda.h"
#include "CSEGlobalClipboard.h"
#include "CSEFormUndoStack.h"

#include <BGSEEToolBox.h>
#include <BGSEEScript\CodaVM.h>

namespace ConstructionSetExtender
{
	OBSEMessagingInterface*						XSEMsgIntfc = NULL;
	PluginHandle								XSEPluginHandle = kPluginHandle_Invalid;

	OBSECommandTableInterface*					XSECommandTableIntfc = NULL;
	ComponentDLLInterface::CommandTableData		XSECommandTableData;

	CSEReleaseNameTable							CSEReleaseNameTable::Instance;
	bool										shadeMeMode = false;
	bool										IsWarholAGenius = false;

	CSEReleaseNameTable::CSEReleaseNameTable() :
		BGSEditorExtender::BGSEEReleaseNameTable()
	{
		RegisterRelease(6, 0, "Konniving Kelpie");
		RegisterRelease(6, 1, "Cretinous Codpiece");
		RegisterRelease(6, 2, "Talkative Badger");
		RegisterRelease(6, 3, "Drunken Glaswegian");
		RegisterRelease(6, 4, "Subterranean Homesick Alien");
		RegisterRelease(6, 5, "Bloody Bull-buggering Bollock");
	}

	CSEReleaseNameTable::~CSEReleaseNameTable()
	{
		;//
	}

	CSEInitCallbackQuery::CSEInitCallbackQuery(const OBSEInterface* OBSE) :
		BGSEditorExtender::BGSEEDaemonCallback(),
		OBSE(OBSE)
	{
		;//
	}

	CSEInitCallbackQuery::~CSEInitCallbackQuery()
	{
		;//
	}

	bool CSEInitCallbackQuery::Handle(void* Parameter)
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
		if (CLIWrapper::ImportInterfaces(OBSE) == false)
			return false;
		BGSEECONSOLE->Exdent();

		return true;
	}

	CSEInitCallbackLoad::CSEInitCallbackLoad(const OBSEInterface* OBSE) :
		BGSEditorExtender::BGSEEDaemonCallback(),
		OBSE(OBSE)
	{
		;//
	}

	CSEInitCallbackLoad::~CSEInitCallbackLoad()
	{
		;//
	}

	bool CSEInitCallbackLoad::Handle(void* Parameter)
	{
		BGSEECONSOLE_MESSAGE("Initializing Hooks");
		BGSEECONSOLE->Indent();
		Hooks::PatchEntryPointHooks();
		Hooks::PatchDialogHooks();
		Hooks::PatchLODHooks();
		Hooks::PatchTESFileHooks();
		Hooks::PatchAssetSelectorHooks();
		Hooks::PatchScriptEditorHooks();
		Hooks::PatchRendererHooks();
		Hooks::PatchMiscHooks();
		Hooks::PatchMessageHanders();
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
		CSEInterfaceManager::Instance.Initialize();

		return true;
	}

	CSEInitCallbackPostMainWindowInit::~CSEInitCallbackPostMainWindowInit()
	{
		;//
	}

	bool CSEInitCallbackPostMainWindowInit::Handle(void* Parameter)
	{
		BGSEEUI->GetSubclasser()->RegisterMainWindowSubclass(UIManager::MainWindowMenuInitSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterMainWindowSubclass(UIManager::MainWindowMenuSelectSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterMainWindowSubclass(UIManager::MainWindowMiscSubclassProc);

		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ObjectWindow, UIManager::ObjectWindowSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ObjectWindow, UIManager::CommonDialogExtraFittingsSubClassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CellView, UIManager::CellViewWindowSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CellView, UIManager::CommonDialogExtraFittingsSubClassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RenderWindow, UIManager::RenderWindowMenuInitSelectSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RenderWindow, UIManager::RenderWindowMiscSubclassProc);

		if (Settings::Dialogs::kShowMainWindowsInTaskbar.GetData().i)
		{
			BGSEditorExtender::BGSEEWindowStyler::StyleData RegularAppWindow = { 0 };
			RegularAppWindow.Extended = WS_EX_APPWINDOW;
			RegularAppWindow.ExtendedOp = BGSEditorExtender::BGSEEWindowStyler::StyleData::kOperation_OR;

			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_ObjectWindow, RegularAppWindow);
			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_CellView, RegularAppWindow);
			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_RenderWindow, RegularAppWindow);
			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_FindText, RegularAppWindow);
			BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_SearchReplace, RegularAppWindow);
		}

		BGSEEUI->GetMenuHotSwapper()->RegisterTemplateReplacer(IDR_RENDERWINDOWCONTEXT, BGSEEMAIN->GetExtenderHandle());

		return true;
	}

	CSEInitCallbackEpilog::~CSEInitCallbackEpilog()
	{
		;//
	}

	bool CSEInitCallbackEpilog::Handle(void* Parameter)
	{
		BGSEECONSOLE_MESSAGE("Initializing Component DLL Interfaces");
		BGSEECONSOLE->Indent();
		CLIWrapper::QueryInterfaces();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Console");
		BGSEECONSOLE->Indent();
		Console::Initialize();
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
		Achievements::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Hall of Fame");
		BGSEECONSOLE->Indent();
		HallOfFame::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing ScriptEditor");
		BGSEECONSOLE->Indent();

		ComponentDLLInterface::IntelliSenseUpdateData GMSTCollectionData;
		GMSTCollectionData.GMSTCount = GameSettingCollection::Instance->GetGMSTCount();
		GMSTCollectionData.GMSTListHead = new ComponentDLLInterface::GMSTData[GMSTCollectionData.GMSTCount];
		GameSettingCollection::Instance->SerializeGMSTDataForHandShake(GMSTCollectionData.GMSTListHead);
		CLIWrapper::Interfaces::SE->InitializeComponents(&XSECommandTableData, &GMSTCollectionData);

		BGSEECONSOLE->Indent();
		BGSEECONSOLE_MESSAGE("Bound %d developer URLs", CSEInterfaceManager::Instance.ConsumeIntelliSenseInterface());
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Coda \"Virtual Machine\"");
		BGSEECONSOLE->Indent();
		BGSEEScript::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Tools");
		BGSEECONSOLE->Indent();
		BGSEETOOLBOX->Initialize(BGSEEMAIN->INIGetter(), BGSEEMAIN->INISetter());
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Workspace Manager");
		BGSEECONSOLE->Indent();
		WorkspaceManager::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing UI Manager, again");
		BGSEECONSOLE->Indent();
		UIManager::Initialize();
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

		BGSEECONSOLE_MESSAGE("Initializing Render Window Painter");
		BGSEECONSOLE->Indent();
		RenderWindowPainter::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Change Log Manager");
		BGSEECONSOLE->Indent();
		ChangeLogManager::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Panic Save Handler");
		BGSEECONSOLE->Indent();
		_DATAHANDLER->PanicSave(true);
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Global Clipboard");
		BGSEECONSOLE->Indent();
		GlobalClipboard::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Form Undo Stack");
		BGSEECONSOLE->Indent();
		FormUndoStack::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Startup Manager");
		BGSEECONSOLE->Indent();
		CSEStartupManager::LoadStartupWorkspace();
		CSEStartupManager::LoadStartupPlugin();
		CSEStartupManager::LoadStartupScript();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE->ExdentAll();
		BGSEECONSOLE_MESSAGE("%s Initialized!", BGSEEMAIN->ExtenderGetDisplayName());
		BGSEECONSOLE->Pad(2);

		BGSEEACHIEVEMENTS->Unlock(Achievements::kTheWiseOne);

		if (BGSEECONSOLE->GetLogsWarnings() == false)
			BGSEEACHIEVEMENTS->Unlock(Achievements::kFlyingBlind);

		for (const CommandInfo* Itr = XSECommandTableData.CommandTableStart; Itr < XSECommandTableData.CommandTableEnd; ++Itr)
		{
			if (!_stricmp(Itr->longName, ""))
				continue;

			BGSEEACHIEVEMENTS->Unlock(Achievements::kCommandant);
		}

		BGSEEACHIEVEMENTS->Unlock(Achievements::kHappyBDayMoi, false, false, true);

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

	CSEDeinitCallback::~CSEDeinitCallback()
	{
		;//
	}

	bool CSEDeinitCallback::Handle(void* Parameter)
	{
		TESDialog::WritePositionToINI(*TESCSMain::WindowHandle, NULL);
		TESDialog::WritePositionToINI(*TESCellViewWindow::WindowHandle, "Cell View");
		TESDialog::WritePositionToINI(*TESObjectWindow::WindowHandle, "Object Window");
		TESDialog::WritePositionToINI(*TESRenderWindow::WindowHandle, "Render Window");

		BGSEECONSOLE_MESSAGE("Flushed CS INI Settings");

		Settings::Dialogs::kRenderWindowState.SetInt((GetMenuState(*TESCSMain::MainMenuHandle, 40423, MF_BYCOMMAND) & MF_CHECKED) != 0);
		Settings::Dialogs::kCellViewWindowState.SetInt((GetMenuState(*TESCSMain::MainMenuHandle, 40200, MF_BYCOMMAND) & MF_CHECKED) != 0);
		Settings::Dialogs::kObjectWindowState.SetInt((GetMenuState(*TESCSMain::MainMenuHandle, 40199, MF_BYCOMMAND) & MF_CHECKED) != 0);

		TESCSMain::DeinitializeCSWindows();

		BGSEECONSOLE_MESSAGE("Deinitializing Plugin Interface Manager");
		CSEInterfaceManager::Instance.Deinitailize();

		BGSEECONSOLE_MESSAGE("Deinitializing Render Window Painter");
		BGSEECONSOLE->Indent();
		delete BGSEERWPAINTER;
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
		CLIWrapper::Interfaces::SE->Deinitalize();
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
		ComponentDLLInterface::DumpInstanceCounters();
		BGSEECONSOLE->Exdent();
#endif

		return true;
	}

	CSECrashCallback::~CSECrashCallback()
	{
		;//
	}

	bool CSECrashCallback::Handle(void* Parameter)
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
			BGSEECONSOLE_MESSAGE("BollocksBollocksBollocks! No can do...");

		if (BGSEEMAIN->Daemon()->GetFullInitComplete())
			BGSEEACHIEVEMENTS->Unlock(Achievements::kSaboteur, false, true);

		BGSEECONSOLE->Exdent();
		BGSEECONSOLE->Exdent();

		CR_CRASH_CALLBACK_INFO* CrashInfo = (CR_CRASH_CALLBACK_INFO*)Parameter;
		bool ResumeExecution = false;

		int CrashHandlerMode = Settings::General::kCrashHandlerMode.GetData().i;

		if (CrashHandlerMode == kCrashHandlerMode_Terminate)
			ResumeExecution = false;
		else if (CrashHandlerMode == kCrashHandlerMode_Resume)
			ResumeExecution = true;
		else if (CrashHandlerMode == kCrashHandlerMode_Ask)
		{
			bool FunnyGuyUnlocked = BGSEEMAIN->Daemon()->GetFullInitComplete() && (Achievements::kFunnyGuy->GetUnlocked() || Achievements::kFunnyGuy->GetTriggered());
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
				if (BGSEEMAIN->Daemon()->GetFullInitComplete())
					BGSEEACHIEVEMENTS->Unlock(Achievements::kFunnyGuy, false, true);

				MessageBox(NULL, "Hah! Nice try, Bob.", BGSEEMAIN->ExtenderGetDisplayName(), MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);

				break;
			}
		}

		return ResumeExecution;
	}

	void CSEStartupManager::LoadStartupPlugin()
	{
		bool Load = Settings::Startup::kLoadPlugin.GetData().i;
		const char* PluginName = Settings::Startup::kPluginName.GetData().s;

		if (Load)
		{
			Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteJump();

			TESFile* File = _DATAHANDLER->LookupPluginByName(PluginName);
			if (File)
			{
				BGSEECONSOLE_MESSAGE("Loading plugin '%s'", PluginName);
				BGSEECONSOLE->Indent();

				File->SetLoaded(true);
				if (_stricmp(PluginName, "Oblivion.esm"))
					File->SetActive(true);

				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, 0x9CD1, 0);

				BGSEECONSOLE->Exdent();
			}
			else if (strlen(PluginName) >= 1)
			{
				BGSEECONSOLE_MESSAGE("Non-existent startup plugin '%s'", PluginName);
			}

			Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteBuffer();
		}
	}

	void CSEStartupManager::LoadStartupScript()
	{
		bool Load = Settings::Startup::kOpenScriptWindow.GetData().i;
		const char* ScriptID = Settings::Startup::kScriptEditorID.GetData().s;

		if (Load)
		{
			if (strcmp(ScriptID, ""))
				TESDialog::ShowScriptEditorDialog(TESForm::LookupByEditorID(ScriptID));
			else
				TESDialog::ShowScriptEditorDialog(NULL);
		}
	}

	void CSEStartupManager::LoadStartupWorkspace()
	{
		bool Load = Settings::Startup::kSetWorkspace.GetData().i;
		const char* WorkspacePath = Settings::Startup::kWorkspacePath.GetData().s;

		if (Load)
		{
			BGSEEWORKSPACE->SelectCurrentWorkspace(WorkspacePath);
		}
	}

	void CSEInteropHandler(OBSEMessagingInterface::Message* Msg)
	{
		switch (Msg->type)
		{
		case 'CSEI':
			{
				BGSEECONSOLE_MESSAGE("Dispatching Plugin Interop Interface to '%s'", Msg->sender);
				BGSEECONSOLE->Indent();
				XSEMsgIntfc->Dispatch(XSEPluginHandle, 'CSEI', (void*)CSEInterfaceManager::Instance.GetInterface(), 4, Msg->sender);
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

using namespace ConstructionSetExtender;

extern "C"
{
	__declspec(dllexport) bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = BGSEEMAIN_EXTENDERSHORTNAME;
		info->version = PACKED_SME_VERSION;

		XSEPluginHandle = obse->GetPluginHandle();

		if (obse->isEditor == false)
		{
			return false;
		}

		SME::MersenneTwister::init_genrand(GetTickCount());
		if (SME::MersenneTwister::genrand_real1() < 0.1)
			IsWarholAGenius = true;

		BGSEditorExtender::INISettingDepotT CSEINISettings;

		Settings::Register(CSEINISettings);
		CSEAuxiliaryViewport::RegisterINISettings(CSEINISettings);

		bool ComponentInitialized = BGSEEMAIN->Initialize(BGSEEMAIN_EXTENDERLONGNAME,
														  (IsWarholAGenius ? "ConstruKction Set Extender" : NULL),
														  BGSEEMAIN_EXTENDERSHORTNAME,
														  CSEReleaseNameTable::Instance.LookupRelease(VERSION_MAJOR, VERSION_MINOR),
														  PACKED_SME_VERSION,
														  BGSEditorExtender::BGSEEMain::kExtenderParentEditor_TES4CS,
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
														  true,
#endif
														  false,
#ifdef NDEBUG
														  true);
		TODO("Save debug symbols, dammit!")
#else
														  false);
#endif

		SME_ASSERT(ComponentInitialized);

		BGSEEMAIN->Daemon()->RegisterInitCallback(BGSEditorExtender::BGSEEDaemon::kInitCallback_Query, new CSEInitCallbackQuery(obse));
		BGSEEMAIN->Daemon()->RegisterInitCallback(BGSEditorExtender::BGSEEDaemon::kInitCallback_Load, new CSEInitCallbackLoad(obse));
		BGSEEMAIN->Daemon()->RegisterInitCallback(BGSEditorExtender::BGSEEDaemon::kInitCallback_PostMainWindowInit, new CSEInitCallbackPostMainWindowInit());
		BGSEEMAIN->Daemon()->RegisterInitCallback(BGSEditorExtender::BGSEEDaemon::kInitCallback_Epilog, new CSEInitCallbackEpilog());
		BGSEEMAIN->Daemon()->RegisterDeinitCallback(new CSEDeinitCallback());
		BGSEEMAIN->Daemon()->RegisterCrashCallback(new CSECrashCallback());

		if (BGSEEMAIN->Daemon()->ExecuteInitCallbacks(BGSEditorExtender::BGSEEDaemon::kInitCallback_Query) == false)
		{
			MessageBox(NULL,
					   "The Construction Set Extender failed to initialize correctly!\n\nIt's highly advised that you close the CS right away. The plugin's log will now open.",
					   "The Cyrodiil Bunny Ranch",
					   MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONERROR | MB_OK);

			BGSEECONSOLE->OpenDebugLog();
			return false;
		}

		return true;
	}

	__declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		if (BGSEEMAIN->Daemon()->ExecuteInitCallbacks(BGSEditorExtender::BGSEEDaemon::kInitCallback_Load) == false)
		{
			MessageBox(NULL,
					   "The Construction Set Extender failed to load correctly!\n\nIt's highly advised that you close the CS right away. The plugin's log will now open.",
					   "Rumpy-Pumpy!!",
					   MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONERROR | MB_OK);

			BGSEECONSOLE->OpenDebugLog();
			return false;
		}

		return true;
	}
};