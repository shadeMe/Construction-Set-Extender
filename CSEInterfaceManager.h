#pragma once

#include "CSEInterfaceAPI.h"
#include "CSAS\Array.h"
#include "CSAS\ScriptCommands.h"

extern std::map<std::string, std::string> g_URLMapBuffer;

class CSEInterfaceManager
{
public:
	static CSEInterface*			GetInterface();
	static void						HandleConsoleCallback(const char* Message, const char* Prefix);

	static void*					InitializeInterface(UInt8 InterfaceType);
	static void						RegisterCommandURL(const char* CommandName, const char* URL);
	static void						PrintToConsole(const char*	Prefix, const char* FormatString, ...);
	static void						RegisterConsoleCallback(CSEConsoleInterface::ConsolePrintCallback Handler);
	static void						PrintToRenderWindow(const char* Message, long double DisplayDuration);
	static bool						RegisterScriptCommand(CSAutomationScript::CSASCommandInfo* CommandInfo);

	typedef std::vector<CSEConsoleInterface::ConsolePrintCallback> ConsolePrintCallbackList;
};