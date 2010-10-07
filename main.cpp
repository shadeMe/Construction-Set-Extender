#include "ExtenderInternals.h"
#include "SEHooks.h"
#include "MiscHooks.h"
#include "Common/CLIWrapper.h"
#include "Common/HandShakeStructs.h"


PluginHandle						g_pluginHandle = kPluginHandle_Invalid;
OBSEMessagingInterface*				g_msgIntfc = NULL;
OBSECommandTableInterface*			g_commandTableIntfc = NULL;

CommandTableData					g_CommandTableData;


std::string							g_INIPath;
std::string							g_AppPath;
bool								g_PluginPostLoad = false;


// PLUGIN INTEROP

void CSEInteropHandler(OBSEMessagingInterface::Message* Msg)
{
	if (Msg->type == 'CSEL') {
		CONSOLE->LogMessage(Console::e_CSE, "CSEL message dispatched by %s", Msg->sender);
		std::map<const  char*, const char*>* URLMap = static_cast<std::map<const  char*, const char*>*>(Msg->data);
		
		for (std::map<const  char*, const char*>::const_iterator Itr = URLMap->begin(); Itr != URLMap->end(); Itr++) {
			CLIWrapper::SE_AddToURLMap(Itr->first, Itr->second);
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
	//	g_msgIntfc->RegisterListener(g_pluginHandle, "ConScribe", CSEInteropHandler);
		g_PluginPostLoad = true;
	}
}



//	HOUSEKEEPING & INIT

extern "C" {

bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "CSE";
	info->version = 3;

	g_AppPath = obse->GetOblivionDirectory();
	g_INIPath = g_AppPath + "Data\\OBSE\\Plugins\\Construction Set Extender.ini";
	CONSOLE->InitializeLog(g_AppPath.c_str());
	CONSOLE->LogMessage(Console::e_CSE, "Construction Set Extender Initializing ...");

	g_DLLInstance = (HINSTANCE)GetModuleHandle(std::string(g_AppPath + "Data\\OBSE\\Plugins\\Construction Set Extender.dll").c_str());
	if (!g_DLLInstance) {
		CONSOLE->LogMessage(Console::e_CSE, "Couldn't fetch the DLL's handle!");
		return false;
	}

	if (!obse->isEditor)					// we don't want to screw with the game
		return false;

	if(obse->obseVersion < OBSE_VERSION_INTEGER)
	{
		CONSOLE->LogMessage(Console::e_CSE, "OBSE version too old");
		return false;
	}
	else if (obse->editorVersion < CS_VERSION_1_2)
	{
		CONSOLE->LogMessage(Console::e_CSE, "Running CS 1.0. Unsupported !");
		return false;
	}

	g_msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
	g_commandTableIntfc = (OBSECommandTableInterface*)obse->QueryInterface(kInterface_CommandTable);

	if (!g_msgIntfc|| !g_commandTableIntfc) {
		CONSOLE->LogMessage(Console::e_CSE, "OBSE Messaging/CommandTable interface not found !");
		return false;
	}
	return true;
}


bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	g_pluginHandle = obse->GetPluginHandle();

	if (!CLIWrapper::Import(obse)) {
		return false;
	} else if (!PatchSEHooks() || !PatchMiscHooks())					// initialize modules
		return false;
	
	g_msgIntfc->RegisterListener(g_pluginHandle, "OBSE", OBSEMessageHandler);
	CONSOLE->LogMessage(Console::e_CSE, "CS patched !\n\n");

	return true;
}

};