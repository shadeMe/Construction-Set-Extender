#include "ExtenderInternals.h"
#include "SEHooks.h"
#include "MiscHooks.h"
#include "[Common]/CLIWrapper.h"
#include "[Common]/HandShakeStructs.h"
#include "WindowManager.h"
#include "CSEInterfaceManager.h"
#include "Console.h"
#include "CSInterop.h"


PluginHandle						g_pluginHandle = kPluginHandle_Invalid;
OBSEMessagingInterface*				g_msgIntfc = NULL;
OBSECommandTableInterface*			g_commandTableIntfc = NULL;

CommandTableData					g_CommandTableData;


// PLUGIN INTEROP

void CSEInteropHandler(OBSEMessagingInterface::Message* Msg)
{
	if (Msg->type == 'CSEI')
	{
		DebugPrint("Dispatching interface to '%s'", Msg->sender);
		g_msgIntfc->Dispatch(g_pluginHandle, 'CSEI', CSEInterfaceManager::GetInterface(), 4, Msg->sender);
	}
}

void OBSEMessageHandler(OBSEMessagingInterface::Message* Msg)
{
	switch (Msg->type)
	{
	case OBSEMessagingInterface::kMessage_PostLoad:
		g_CommandTableData.CommandTableStart = g_commandTableIntfc->Start();
		g_CommandTableData.CommandTableEnd = g_commandTableIntfc->End();
		CLIWrapper::ScriptEditor::InitializeComponents(&g_CommandTableData);
					
		g_msgIntfc->RegisterListener(g_pluginHandle, NULL, CSEInteropHandler);
		g_PluginPostLoad = true;
		break;
	case OBSEMessagingInterface::kMessage_PostPostLoad:
		if (!CSIOM->Initialize("Data\\OBSE\\Plugins\\ComponentDLLs\\CSE\\LipSyncPipeClient.dll"))
		{
			DebugPrint("CSInterop Manager failed to initialize successfully! LIP service will be unavailable during this session");
		}
		break;
	}
}



//	HOUSEKEEPING & INIT

extern "C" {

bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	if (!obse->isEditor)					// we don't want to screw with the game
		return false;

	CONSOLE->InitializeLog(g_AppPath.c_str());

	DebugPrint("Construction Set Extender Initializing ...");

	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "CSE";
	info->version = 5;

	g_AppPath = obse->GetOblivionDirectory();
	g_INIPath = g_AppPath + "Data\\OBSE\\Plugins\\Construction Set Extender.ini";	

	g_DLLInstance = (HINSTANCE)GetModuleHandle(std::string(g_AppPath + "Data\\OBSE\\Plugins\\Construction Set Extender.dll").c_str());
	if (!g_DLLInstance) {
		DebugPrint("Couldn't fetch the DLL's handle!");
		return false;
	}

	if(obse->obseVersion < OBSE_VERSION_INTEGER)
	{
		DebugPrint("OBSE version too old");
		return false;
	}
	else if (obse->editorVersion < CS_VERSION_1_2)
	{
		DebugPrint("Running CS 1.0. Unsupported !");
		return false;
	}

	g_msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
	g_commandTableIntfc = (OBSECommandTableInterface*)obse->QueryInterface(kInterface_CommandTable);

	if (!g_msgIntfc || !g_commandTableIntfc) {
		DebugPrint("OBSE Messaging/CommandTable interface not found !");
		return false;
	}
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
	g_INIManager->SetINIPath(g_INIPath);
	dynamic_cast<CSEINIManager*>(g_INIManager)->Initialize();

//	WaitUntilDebuggerAttached();

	if (!CLIWrapper::Import(obse))
		return false;
	else if (!PatchSEHooks() || !PatchMiscHooks())
		return false;
	
	g_msgIntfc->RegisterListener(g_pluginHandle, "OBSE", OBSEMessageHandler);
	g_CommandTableData.GetCommandReturnType = g_commandTableIntfc->GetReturnType;
	g_CommandTableData.GetParentPlugin = g_commandTableIntfc->GetParentPlugin;

	DebugPrint("CS patched !\n\n");
	return true;
}

};