#pragma once

#include "CSEInterfaceAPI.h"
#include <vector>

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

	typedef std::vector<CSEConsoleInterface::ConsolePrintCallback> ConsolePrintCallbackList;
};