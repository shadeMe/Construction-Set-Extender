#include "CSEMain.h"
#include "CSEInterfaceManager.h"

#include "[Common]\CLIWrapper.h"
#include "Hooks\Dialog.h"
#include "Hooks\LOD.h"
#include "Hooks\TESFile.h"
#include "Hooks\AssetSelector.h"
#include "Hooks\ScriptEditor.h"
#include "Hooks\Renderer.h"
#include "Hooks\Misc.h"

#include "CSEAchievements.h"
#include "CSEConsole.h"
#include "CSERenderWindowPainter.h"
#include "CSEHallOfFame.h"
#include "CSEUIManager.h"
#include "CSEWorkspaceManager.h"
#include "ChangeLogManager.h"
#include "AuxiliaryViewport.h"
#include "CSInterop.h"
#include "Coda\CSECoda.h"

#include <BGSEEToolBox.h>
#include <BGSEEScript\CodaVM.h>

namespace ConstructionSetExtender
{
	OBSEMessagingInterface*						XSEMsgIntfc = NULL;
	PluginHandle								XSEPluginHandle = kPluginHandle_Invalid;

	OBSECommandTableInterface*					XSECommandTableIntfc = NULL;
	ComponentDLLInterface::CommandTableData		XSECommandTableData;

	InitCallbackQuery::InitCallbackQuery( const OBSEInterface* OBSE ) :
		BoolRFunctorBase(),
		OBSE(OBSE)
	{
		;//
	}

	InitCallbackQuery::~InitCallbackQuery()
	{
		;//
	}

	bool InitCallbackQuery::operator()()
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
		if (!CLIWrapper::ImportInterfaces(OBSE))
			return false;
		BGSEECONSOLE->Exdent();

		return true;
	}

	InitCallbackLoad::InitCallbackLoad( const OBSEInterface* OBSE ) :
		BoolRFunctorBase(),
		OBSE(OBSE)
	{
		;//
	}

	InitCallbackLoad::~InitCallbackLoad()
	{
		;//
	}

	bool InitCallbackLoad::operator()()
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

		bool ComponentInitialized = BGSEEUI->Initialize("TES Construction Set",
												LoadMenu(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDR_MAINMENU)));

		if (ComponentInitialized == false)
			return false;

		BGSEEUI->GetSubclasser()->RegisterMainWindowSubclass(UIManager::MainWindowMenuInitSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterMainWindowSubclass(UIManager::MainWindowMenuSelectSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterMainWindowSubclass(UIManager::MainWindowMiscSubclassProc);

		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ObjectWindow, UIManager::ObjectWindowSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CellView, UIManager::CellViewWindowSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RenderWindow, UIManager::RenderWindowMenuInitSelectSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RenderWindow, UIManager::RenderWindowMiscSubclassProc);

		BGSEEUI->GetMenuHotSwapper()->RegisterTemplateReplacer(IDR_RENDERWINDOWCONTEXT, BGSEEMAIN->GetExtenderHandle());

		BGSEECONSOLE_MESSAGE("Registering OBSE Plugin Message Handlers");
		XSEMsgIntfc->RegisterListener(XSEPluginHandle, "OBSE", OBSEMessageHandler);

		return true;
	}

	InitCallbackEpilog::~InitCallbackEpilog()
	{
		;//
	}

	bool InitCallbackEpilog::operator()()
	{
		SME::MersenneTwister::init_genrand(GetTickCount());

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
		{
			BGSEECONSOLE_MESSAGE("Failed to initialize successfully! Lip service will be unavailable during this session");
		}
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

		ComponentDLLInterface::IntelliSenseUpdateData* GMSTCollectionData = new ComponentDLLInterface::IntelliSenseUpdateData();
		GMSTCollectionData->GMSTCount = GameSettingCollection::Instance->GetGMSTCount();
		GMSTCollectionData->GMSTListHead = new ComponentDLLInterface::GMSTData[GMSTCollectionData->GMSTCount];
		GameSettingCollection::Instance->SerializeGMSTDataForHandShake(GMSTCollectionData->GMSTListHead);
		CLIWrapper::Interfaces::SE->InitializeComponents(&XSECommandTableData, GMSTCollectionData);
		delete GMSTCollectionData;

		BGSEECONSOLE->Indent();
		TODO("Implement")
/*
		for (std::map<std::string, std::string>::const_iterator Itr = g_URLMapBuffer.begin(); Itr != g_URLMapBuffer.end(); Itr++)
			CLIWrapper::Interfaces::SE->AddScriptCommandDeveloperURL(Itr->first.c_str(), Itr->second.c_str());
		BGSEECONSOLE_MESSAGE(Console::e_SE, "Bound %d developer URLs", g_URLMapBuffer.size());
		g_URLMapBuffer.clear();*/
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Coda Virtual Machine");
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

		BGSEECONSOLE_MESSAGE("Initializing UI Manager");
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

		BGSEECONSOLE_MESSAGE("Initializing Render Window Text Painter");
		BGSEECONSOLE->Indent();
		RenderWindowPainter::Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Change Log Manager");
		BGSEECONSOLE->Indent();
		VersionControl::CHANGELOG->Initialize();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Initializing Startup Manager");
		BGSEECONSOLE->Indent();
		CSEStartupManager::LoadStartupWorkspace();
		CSEStartupManager::LoadStartupPlugin();
		CSEStartupManager::LoadStartupScript();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE->ExdentAll();
		BGSEECONSOLE_MESSAGE("Construction Set Extender Initialized!");
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

		return true;
	}

	DeinitCallback::~DeinitCallback()
	{
		;//
	}

	bool DeinitCallback::operator()()
	{
		TESDialog::WritePositionToINI(*g_HWND_CSParent, NULL);
		TESDialog::WritePositionToINI(*g_HWND_CellView, "Cell View");
		TESDialog::WritePositionToINI(*g_HWND_ObjectWindow, "Object Window");
		TESDialog::WritePositionToINI(*g_HWND_RenderWindow, "Render Window");

		BGSEECONSOLE_MESSAGE("Flushed CS INI Settings");

		INISettings::GetDialogs()->Set(INISettings::kDialogs_RenderWindowState,
									BGSEEMAIN->INISetter(),
									(GetMenuState(*g_HMENU_MainMenu, 40423, MF_BYCOMMAND) & MF_CHECKED)?"1":"0");

		INISettings::GetDialogs()->Set(INISettings::kDialogs_ObjectWindowState,
									BGSEEMAIN->INISetter(),
									(GetMenuState(*g_HMENU_MainMenu, 40199, MF_BYCOMMAND) & MF_CHECKED)?"1":"0");

		INISettings::GetDialogs()->Set(INISettings::kDialogs_CellViewWindowState,
									BGSEEMAIN->INISetter(),
									(GetMenuState(*g_HMENU_MainMenu, 40200, MF_BYCOMMAND) & MF_CHECKED)?"1":"0");

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

		BGSEECONSOLE_MESSAGE("Deinitializing Coda Virtual Machine");
		BGSEECONSOLE->Indent();
		delete CODAVM;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Auxiliary Viewport");
		BGSEECONSOLE->Indent();
		delete AUXVIEWPORT;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Script Editor");
		BGSEECONSOLE->Indent();
		CLIWrapper::Interfaces::SE->CleanupAutoRecoveryCache();
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing Change Log Manager");
		BGSEECONSOLE->Indent();
		delete VersionControl::CHANGELOG;
		BGSEECONSOLE->Exdent();

		BGSEECONSOLE_MESSAGE("Deinitializing CSInterop Manager");
		BGSEECONSOLE->Indent();
		delete CSIOM;
		BGSEECONSOLE->Exdent();

		return true;
	}

	CrashCallback::~CrashCallback()
	{
		;//
	}

	bool CrashCallback::operator()()
	{
		BGSEECONSOLE->Pad(2);
		BGSEECONSOLE_MESSAGE("The editor crashed, dammit!");

		BGSEEACHIEVEMENTS->Unlock(Achievements::kSaboteur, false, true);
//		BGSEECONSOLE->OpenDebugLog();

		return true;
	}

	void CSEStartupManager::LoadStartupPlugin()
	{
		bool Load = atoi(INISettings::GetStartupPlugin()->Get(INISettings::kStartupPlugin_LoadPlugin, BGSEEMAIN->INIGetter()));
		const char* PluginName = INISettings::GetStartupPlugin()->Get(INISettings::kStartupPlugin_PluginName, BGSEEMAIN->INIGetter());

		if (Load)
		{
			Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteJump();

			TESFile* File = _DATAHANDLER->LookupPluginByName(PluginName);
			if (File)
			{
				BGSEECONSOLE_MESSAGE("Loading plugin '%s'", PluginName);
				BGSEECONSOLE->Indent();

				if (_stricmp(PluginName, "Oblivion.esm"))
					SME::MiscGunk::ToggleFlag(&File->fileFlags, TESFile::kFileFlag_Active, true);

				SME::MiscGunk::ToggleFlag(&File->fileFlags, TESFile::kFileFlag_Loaded, true);
				SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CD1, 0);

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
		bool Load = atoi(INISettings::GetStartupScript()->Get(INISettings::kStartupScript_OpenScriptWindow, BGSEEMAIN->INIGetter()));
		const char* ScriptID = INISettings::GetStartupScript()->Get(INISettings::kStartupScript_ScriptEditorID, BGSEEMAIN->INIGetter());

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
		bool Load = atoi(INISettings::GetStartupWorkspace()->Get(INISettings::kStartupWorkspace_SetWorkspace, BGSEEMAIN->INIGetter()));
		const char* WorkspacePath = INISettings::GetStartupWorkspace()->Get(INISettings::kStartupWorkspace_WorkspacePath, BGSEEMAIN->INIGetter());

		if (Load)
		{
			BGSEEWORKSPACE->SelectCurrentWorkspace(WorkspacePath);
		}
	}

	void CSEInteropHandler(OBSEMessagingInterface::Message* Msg)
	{
		TODO("Implement")
/*
		if (Msg->type == 'CSEI')
		{
			BGSEECONSOLE_MESSAGE("Dispatching Plugin Interop Interface to '%s'", Msg->sender);
			CONSOLE->Indent();
			XSEMsgIntfc->Dispatch(XSEPluginHandle, 'CSEI', CSEInterfaceManager::GetInterface(), 4, Msg->sender);
			CONSOLE->Exdent();
		}*/
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

		BGSEditorExtender::SettingFactoryListT CSEINISettings;

		CSEINISettings.push_back(INISettings::GetAuxiliaryViewport());
		CSEINISettings.push_back(INISettings::GetDialogs());
		CSEINISettings.push_back(INISettings::GetGeneral());
		CSEINISettings.push_back(INISettings::GetLOD());
		CSEINISettings.push_back(INISettings::GetPlugins());
		CSEINISettings.push_back(INISettings::GetRenderer());
		CSEINISettings.push_back(INISettings::GetRenderWindowPainter());
		CSEINISettings.push_back(INISettings::GetStartupPlugin());
		CSEINISettings.push_back(INISettings::GetStartupScript());
		CSEINISettings.push_back(INISettings::GetStartupWorkspace());
		CSEINISettings.push_back(INISettings::GetVersionControl());

		bool ComponentInitialized = BGSEEMAIN->Initialize(BGSEEMAIN_EXTENDERLONGNAME, BGSEEMAIN_EXTENDERSHORTNAME, PACKED_SME_VERSION,
														BGSEditorExtender::BGSEEMain::kExtenderParentEditor_TES4CS, CS_VERSION_1_2, obse->editorVersion,
														obse->GetOblivionDirectory(), XSEPluginHandle,
														OBSE_VERSION_INTEGER, obse->obseVersion,
														CSEINISettings,
														"v4.0.30319", false, false,
#ifdef NDEBUG
														true);
		TODO("Save debug symbols, dammit!")
#else
														false);
		TODO("Enable CrashRpt support for Public builds")
#endif

		SME_ASSERT(ComponentInitialized);

		BGSEEMAIN->Daemon()->RegisterInitCallback(BGSEditorExtender::BGSEEDaemon::kInitCallback_Query, new InitCallbackQuery(obse));
		BGSEEMAIN->Daemon()->RegisterInitCallback(BGSEditorExtender::BGSEEDaemon::kInitCallback_Load, new InitCallbackLoad(obse));
		BGSEEMAIN->Daemon()->RegisterInitCallback(BGSEditorExtender::BGSEEDaemon::kInitCallback_Epilog, new InitCallbackEpilog());
		BGSEEMAIN->Daemon()->RegisterDeinitCallback(new DeinitCallback());
		BGSEEMAIN->Daemon()->RegisterCrashCallback(new CrashCallback());

		if (BGSEEMAIN->Daemon()->ExecuteInitCallbacks(BGSEditorExtender::BGSEEDaemon::kInitCallback_Query) == false)
		{
			MessageBox(NULL, "The Construction Set Extender failed to initialize correctly!\n\nCheck the logs for more information.", "zOMG!", MB_OK|MB_ICONERROR);
			return false;
		}

		return true;
	}

	__declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		if (BGSEEMAIN->Daemon()->ExecuteInitCallbacks(BGSEditorExtender::BGSEEDaemon::kInitCallback_Load) == false)
		{
			MessageBox(NULL, "The Construction Set Extender failed to load correctly!\n\nCheck the logs for more information.", "zOMG!", MB_OK|MB_ICONERROR);
			return false;
		}

		return true;
	}
};