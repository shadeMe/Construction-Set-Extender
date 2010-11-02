#include "ExtenderInternals.h"
#include "SEHooks.h"
#include "MiscHooks.h"
#include "Common/CLIWrapper.h"
#include "Common/HandShakeStructs.h"
#include "[ Libraries ]\CSE Handshake\CSEL.h"


PluginHandle						g_pluginHandle = kPluginHandle_Invalid;
OBSEMessagingInterface*				g_msgIntfc = NULL;
OBSECommandTableInterface*			g_commandTableIntfc = NULL;

CommandTableData					g_CommandTableData;


std::string							g_INIPath;
std::string							g_AppPath;
bool								g_PluginPostLoad = false;
CSEINIManager*						g_INIManager = new CSEINIManager();

// PLUGIN INTEROP

void CSEInteropHandler(OBSEMessagingInterface::Message* Msg)
{
	if (Msg->type == 'CSEL') {
		DebugPrint("CSEL message dispatched by %s", Msg->sender);
		
		CSELData* Data = (CSELData*)Msg->data;
		for (int i = 0; i < Data->Size; i++) {
			std::string String(Data->Data[i]), Command, URL;
			std::string::size_type Delimiter = String.find(" ");
			if (Delimiter == std::string::npos) {
				DebugPrint("Couldn't find delimiter in %s", Data->Data[i]);
				continue;
			}
			Command = String.substr(0, Delimiter);
			URL = String.substr(Delimiter + 1, String.length());
			CLIWrapper::SE_AddToURLMap( Command.c_str(), URL.c_str());
		}
	}
}


void OBSEMessageHandler(OBSEMessagingInterface::Message* Msg)
{
	if (Msg->type == OBSEMessagingInterface::kMessage_PostLoad) {
														// initialize script editor components
		g_CommandTableData.CommandTableStart = g_commandTableIntfc->Start();
		g_CommandTableData.CommandTableEnd = g_commandTableIntfc->End();
		g_CommandTableData.GetCommandReturnType = g_commandTableIntfc->GetReturnType;
		g_CommandTableData.GetParentPlugin = g_commandTableIntfc->GetParentPlugin;
		CLIWrapper::SE_InitializeComponents(&g_CommandTableData);

														// register known plugins with the messaging API
	//	g_msgIntfc->RegisterListener(g_pluginHandle, "NifSE", CSEInteropHandler);	
		g_msgIntfc->RegisterListener(g_pluginHandle, "ConScribe", CSEInteropHandler);
		g_PluginPostLoad = true;
	}
}



//	HOUSEKEEPING & INIT

extern "C" {

bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	CONSOLE->InitializeLog(g_AppPath.c_str());

	DebugPrint("Construction Set Extender Initializing ...");

	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "CSE";
	info->version = 4;

	g_AppPath = obse->GetOblivionDirectory();
	g_INIPath = g_AppPath + "Data\\OBSE\\Plugins\\Construction Set Extender.ini";	

	g_DLLInstance = (HINSTANCE)GetModuleHandle(std::string(g_AppPath + "Data\\OBSE\\Plugins\\Construction Set Extender.dll").c_str());
	if (!g_DLLInstance) {
		DebugPrint("Couldn't fetch the DLL's handle!");
		return false;
	}

	if (!obse->isEditor)					// we don't want to screw with the game
		return false;

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

	if (!g_msgIntfc|| !g_commandTableIntfc) {
		DebugPrint("OBSE Messaging/CommandTable interface not found !");
		return false;
	}
	return true;
}


bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	g_pluginHandle = obse->GetPluginHandle();
	g_INIManager->SetINIPath(g_INIPath);
	g_INIManager->Initialize();

	if (!CLIWrapper::Import(obse)) {
		return false;
	} else if (!PatchSEHooks() || !PatchMiscHooks())
		return false;
	
	g_msgIntfc->RegisterListener(g_pluginHandle, "OBSE", OBSEMessageHandler);

	DebugPrint("CS patched !\n\n");

	return true;
}

};