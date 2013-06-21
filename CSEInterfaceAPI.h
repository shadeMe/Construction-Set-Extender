#pragma once

#ifndef CSEAPI_NO_CODA
#include <CodaPublicAPI.h>
#endif

/********** CSE Interface API ******************************************************************
*	Interface object passed through the OBSE messaging system. A pointer to the
*	object will be dispatched to plugins that pass an arbitrary message of type
*	'CSEI' to "CSE" inside the OBSE Post-Post-Load message handler. The reply will be of the same type.
*
*	All sub-interfaces need to be initialized with InitializeInterface().
*	All RegisterXXX type calls must be made inside the OBSE Post-Post-Load message handler.
***********************************************************************************************/

struct CSEInterface
{
	enum
	{
		kCSEInterface_Console = 0,
		kCSEInterface_IntelliSense,
		kCSEInterface_Renderer,
		kCSEInterface_Script
	};

	// Used to initialize CSE's interface objects. Similar to OBSE's QueryInterface.
	const void*		(* InitializeInterface)(UInt8 InterfaceType);
	// Returns the implementation version of the extender interface
	UInt8			(* GetInterfaceVersion)(void);
};

struct CSEIntelliSenseInterface
{
	// Registers an arbitrary URL to a script command. Registered URLs will be displayed in the
	// script editor's context menu when the corresponding command is selected.
	void			(* RegisterCommandURL)(const char* CommandName, const char* URL);
};

struct CSEConsoleInterface
{
	typedef void	(* ConsolePrintCallback)(const char* Message, const char* Prefix);

	// Prints a message to the console in the following format: [<Prefix>]\t\t<Message>
	// Mustn't be called inside a print callback.

	// Reserved Prefixes: CMD, CSE, SE, CS, BSAV, USE, BE, TAG
	void			(* PrintToConsole)(const char*	Prefix, const char* FormatString, ...);
	// Registers a handler that gets called whenever a message is printed to the console.
	void			(* RegisterCallback)(ConsolePrintCallback Handler);
};

struct CSERendererInterface
{
	// Queues a text message for display in the render window
	void			(* PrintToRenderWindow)(const char* Message, float DisplayDuration);
};

#ifndef CSEAPI_NO_CODA
struct CSEScriptInterface
{
	// Registers a Coda command with the VM
	// The caller retains ownership of the script command pointer
	void			(* RegisterCommand)(BGSEditorExtender::BGSEEScript::ICodaScriptCommand* Command);
};
#endif

/********** Example ****************************************************************************

OBSEMessagingInterface*					g_msgIntfc		= NULL;
PluginHandle							g_pluginHandle	= kPluginHandle_Invalid;

CSEIntelliSenseInterface*				g_CSEISIntfc	= NULL;
CSEConsoleInterface*					g_CSEConsoleIntfc	= NULL;

void CSEMessageHandler(OBSEMessagingInterface::Message* Msg)
{
	if (Msg->type == 'CSEI')
	{
		CSEInterface* Interface = (CSEInterface*)Msg->data;

		g_CSEConsoleIntfc = (CSEConsoleInterface*)Interface->InitializeInterface(CSEInterface::kCSEInterface_Console);
		g_CSEISIntfc = (CSEIntelliSenseInterface*)Interface->InitializeInterface(CSEInterface::kCSEInterface_IntelliSense);

		_MESSAGE("Received interface from CSE");

		g_CSEConsoleIntfc->PrintToConsole("Sample", "Registering command URLs ...");
		g_CSEISIntfc->RegisterCommandURL("ScriptCommand1", "http://cs.elderscrolls.com/constwiki/index.php/ScriptCommand1");
		g_CSEISIntfc->RegisterCommandURL("ScriptCommand2", "http://cs.elderscrolls.com/constwiki/index.php/ScriptCommand2");
	}
}

void OBSEMessageHandler(OBSEMessagingInterface::Message* Msg)
{
	switch (Msg->type)
	{
	case OBSEMessagingInterface::kMessage_PostLoad:
		g_msgIntfc->RegisterListener(g_pluginHandle, "CSE", CSEMessageHandler);
		_MESSAGE("Registered to receive messages from CSE");

		break;
	case OBSEMessagingInterface::kMessage_PostPostLoad:
		_MESSAGE("Requesting an interface from CSE");
		g_msgIntfc->Dispatch(g_pluginHandle, 'CSEI', NULL, 0, "CSE");

		break;
	}
}

extern "C"
{
	bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "SamplePlugin";
		info->version = 1;

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		g_pluginHandle = obse->GetPluginHandle();

		if(obse->isEditor)
		{
			g_msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
			g_msgIntfc->RegisterListener(g_pluginHandle, "OBSE", OBSEMessageHandler);
		}

		return true;
	}
};
***********************************************************************************************/
