#include "CSEInterfaceManager.h"
#include "UtilityBox.h"
#include "[Common]\CLIWrapper.h"
#include "Console.h"

static CSEInterface				s_CSEInterface =
{
	CSEInterfaceManager::InitializeInterface
};
static CSEConsoleInterface		s_CSEConsoleInterface =
{
	CSEInterfaceManager::PrintToConsole,
	CSEInterfaceManager::RegisterConsoleCallback
};
static CSEIntelliSenseInterface	s_CSEIntelliSenseInterface =
{
	CSEInterfaceManager::RegisterCommandURL
};

void* CSEInterfaceManager::InitializeInterface(UInt8 InterfaceType)
{
	switch (InterfaceType)
	{
	case CSEInterface::kCSEInterface_Console:
		return &s_CSEConsoleInterface;
	case CSEInterface::kCSEInterface_IntelliSense:
		return &s_CSEIntelliSenseInterface;
	default:
		return NULL;
	}
}

void CSEInterfaceManager::PrintToConsole(const char* Prefix, const char* FormatString, ...)
{
	va_list Args;
	va_start(Args, FormatString);
	CONSOLE->LogMessage(Prefix, FormatString, Args);
	va_end(Args);
}

static CSEInterfaceManager::ConsolePrintCallbackList	s_ConsolePrintCallbackList;

void CSEInterfaceManager::RegisterConsoleCallback(CSEConsoleInterface::ConsolePrintCallback Handler)
{
	for (ConsolePrintCallbackList::const_iterator Itr = s_ConsolePrintCallbackList.begin(); Itr != s_ConsolePrintCallbackList.end(); Itr++)
	{
		if (*Itr == Handler)
			return;
	}

	s_ConsolePrintCallbackList.push_back(Handler);
}

void CSEInterfaceManager::RegisterCommandURL(const char *CommandName, const char *URL)
{
	CLIWrapper::ScriptEditor::AddToURLMap(CommandName, URL);
}

void CSEInterfaceManager::HandleConsoleCallback(const char *Message, const char *Prefix)
{
	for (ConsolePrintCallbackList::const_iterator Itr = s_ConsolePrintCallbackList.begin(); Itr != s_ConsolePrintCallbackList.end(); Itr++)
	{
		(*Itr)(Message, Prefix);
	}	
}
CSEInterface* CSEInterfaceManager::GetInterface()
{
	return &s_CSEInterface;
}


