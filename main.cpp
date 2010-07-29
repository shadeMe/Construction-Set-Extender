#include "ExtenderInternals.h"
#include "SEHooks.h"
#include "MiscHooks.h"
#include "Common/CLIWrapper.h"
#include "Exports.h"
#include "Common/HandShakeStructs.h"


PluginHandle						g_pluginHandle = kPluginHandle_Invalid;
OBSEMessagingInterface*				g_msgIntfc = NULL;
OBSECommandTableInterface*			g_commandTableIntfc = NULL;

CommandTableData					g_CommandTableData;


std::fstream						g_DEBUG_LOG;
std::string							g_INIPath;
std::string							g_AppPath;


// PLUGIN INTEROP

void CSEInteropHandler(OBSEMessagingInterface::Message* Msg)
{
	if (Msg->type == 'CSEL') {
		_D_PRINT(std::string("CSEL message dispatched by " + std::string(Msg->sender)).c_str());
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
		g_msgIntfc->RegisterListener(g_pluginHandle, "NifSE", CSEInteropHandler);	
		g_msgIntfc->RegisterListener(g_pluginHandle, "ConScribe", CSEInteropHandler);
	}
}



//	HOUSEKEEPING & INIT

extern "C" {
bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "CSE";
	info->version = 3;

	if (!obse->isEditor)					// we don't want to screw with the game
		return false;
	else 
		g_DEBUG_LOG.open(std::string(std::string(obse->GetOblivionDirectory()) + "Construction Set Extender.log").c_str(), std::fstream::out);

	if(obse->obseVersion < OBSE_VERSION_INTEGER)
	{
		_D_PRINT("OBSE version too old");
		return false;
	}
	else if (obse->editorVersion < CS_VERSION_1_2)
	{
		_D_PRINT("Running CS 1.0. Unsupported !");
		return false;
	}

	g_msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
	g_commandTableIntfc = (OBSECommandTableInterface*)obse->QueryInterface(kInterface_CommandTable);

	if (g_msgIntfc == NULL || g_commandTableIntfc == NULL) {
		_D_PRINT("OBSE Messaging/CommandTable interface not found !");
		return false;
	}
	return true;
}


bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	g_pluginHandle = obse->GetPluginHandle();

	g_AppPath = obse->GetOblivionDirectory();
	g_INIPath = g_AppPath + "Data\\OBSE\\Plugins\\Construction Set Extender.ini";

	if (!CLIWrapper::Import(obse)) {
		return false;
	} else if (!PatchSEHooks() || !PatchMiscHooks())					// initialize modules
		return false;


	
	g_msgIntfc->RegisterListener(g_pluginHandle, "OBSE", OBSEMessageHandler);
	_D_PRINT("CS patched !\n\n");

	return true;
}

};