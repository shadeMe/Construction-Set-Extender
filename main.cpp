#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Version.lib")

#include "[Common]/CLIWrapper.h"
#include "[Common]/HandShakeStructs.h"
#include "ExtenderInternals.h"
#include "WindowManager.h"
#include "Console.h"
#include "CSInterop.h"
#include "CSEInterfaceManager.h"

#include "Hooks\Dialog.h"
#include "Hooks\LOD.h"
#include "Hooks\TESFile.h"
#include "Hooks\AssetSelector.h"
#include "Hooks\ScriptEditor.h"
#include "Hooks\Renderer.h"
#include "Hooks\Misc.h"
#include "Hooks\VersionControl.h"

PluginHandle						g_pluginHandle = kPluginHandle_Invalid;
OBSEMessagingInterface*				g_msgIntfc = NULL;
OBSECommandTableInterface*			g_commandTableIntfc = NULL;

CommandTableData					g_CommandTableData;

void CSEInteropHandler(OBSEMessagingInterface::Message* Msg)
{
	if (Msg->type == 'CSEI')
	{
		DebugPrint("Dispatching Plugin Interop Interface to '%s'", Msg->sender);
		CONSOLE->Indent();
		g_msgIntfc->Dispatch(g_pluginHandle, 'CSEI', CSEInterfaceManager::GetInterface(), 4, Msg->sender);
		CONSOLE->Exdent();
	}
}

void OBSEMessageHandler(OBSEMessagingInterface::Message* Msg)
{
	switch (Msg->type)
	{
	case OBSEMessagingInterface::kMessage_PostLoad:
		g_CommandTableData.CommandTableStart = g_commandTableIntfc->Start();
		g_CommandTableData.CommandTableEnd = g_commandTableIntfc->End();
		g_msgIntfc->RegisterListener(g_pluginHandle, NULL, CSEInteropHandler);
		break;
	case OBSEMessagingInterface::kMessage_PostPostLoad:
		if (!CSIOM->Initialize("Data\\OBSE\\Plugins\\CSE\\LipSyncPipeClient.dll"))
		{
			CONSOLE->Indent();
			DebugPrint("CSInterop Manager failed to initialize successfully! LIP service will be unavailable during this session");
			CONSOLE->Exdent();
		}
		g_PluginPostLoad = true;
		break;
	}
}

extern "C"
{
	bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		if (!obse->isEditor)					// we don't want to screw with the game
			return false;

		CONSOLE->InitializeLog(g_AppPath.c_str());
		DebugPrint("Construction Set Extender Initializing ...");
		CONSOLE->Indent();

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "CSE";
		info->version = 5;

		g_AppPath = obse->GetOblivionDirectory();
		g_INIPath = g_AppPath + "Data\\OBSE\\Plugins\\Construction Set Extender.ini";
		g_DLLPath = g_AppPath + "Data\\OBSE\\Plugins\\Construction Set Extender.dll";

		DebugPrint("Initializing INI Manager");
		CONSOLE->Indent();
		g_INIManager->SetINIPath(g_INIPath);
		g_INIManager->Initialize();
		CONSOLE->Exdent();

		g_DLLInstance = (HINSTANCE)GetModuleHandle(g_DLLPath.c_str());
		if (!g_DLLInstance)
		{
			DebugPrint("Couldn't fetch the DLL's handle!");
			return false;
		}

		DebugPrint("Checking Versions");
		CONSOLE->Indent();
		if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			DebugPrint("OBSE version too old");
			return false;
		}
		else if (obse->editorVersion < CS_VERSION_1_2)
		{
			DebugPrint("CSE requires CS v1.2 to run");
			return false;
		}

		OSVERSIONINFO OSInfo;
		GetVersionEx(&OSInfo);
		if (OSInfo.dwMajorVersion < 5)
		{
			DebugPrint("CSE requires Windows XP or greater to run");
			return false;
		}
		CONSOLE->Exdent();

		DebugPrint("Initializing OBSE Interfaces");
		CONSOLE->Indent();
		g_msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
		g_commandTableIntfc = (OBSECommandTableInterface*)obse->QueryInterface(kInterface_CommandTable);

		if (!g_msgIntfc || !g_commandTableIntfc)
		{
			DebugPrint("OBSE Messaging/CommandTable interface not found");
			return false;
		}

		g_CommandTableData.GetCommandReturnType = g_commandTableIntfc->GetReturnType;
		g_CommandTableData.GetParentPlugin = g_commandTableIntfc->GetParentPlugin;
		CONSOLE->Exdent();

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		INITCOMMONCONTROLSEX icex;
																 // ensure that the common control DLL is loaded.
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC  = ICC_LISTVIEW_CLASSES;
		InitCommonControlsEx(&icex);

		g_pluginHandle = obse->GetPluginHandle();

		DebugPrint("Loading Component DLLs");
		CONSOLE->Indent();
		if (!CLIWrapper::Import(obse))
			return false;
		CONSOLE->Exdent();

		DebugPrint("Initializing Hooks");
		CONSOLE->Indent();
		Hooks::PatchDialogHooks();
		Hooks::PatchLODHooks();
		Hooks::PatchTESFileHooks();
		Hooks::PatchAssetSelectorHooks();
		Hooks::PatchScriptEditorHooks();
		Hooks::PatchRendererHooks();
		Hooks::PatchMiscHooks();	
		CONSOLE->Exdent();

		DebugPrint("Initializing OBSE Message Handler");
		g_msgIntfc->RegisterListener(g_pluginHandle, "OBSE", OBSEMessageHandler);

		return true;
	}
};

// (mostly)dummy entry point
BOOL WINAPI DllMain(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	//	WaitUntilDebuggerAttached();
		break;
	}
	return TRUE;
}