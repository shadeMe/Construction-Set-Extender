#include "[Common]\CLIWrapper.h"
#include "CSEInterfaceManager.h"
#include "CSERenderWindowPainter.h"

namespace ConstructionSetExtender
{
	static const CSEInterface				kCSEInterface =
	{
		CSEInterfaceManager::InitializeInterface,
		CSEInterfaceManager::GetVersion,
	};

	static const CSEConsoleInterface		kCSEConsoleInterface =
	{
		CSEInterfaceManager::PrintToConsole,
		CSEInterfaceManager::RegisterConsoleCallback
	};

	static const CSEIntelliSenseInterface	kCSEIntelliSenseInterface =
	{
		CSEInterfaceManager::RegisterCommandURL
	};

	static const CSERendererInterface		kCSERendererInterface =
	{
		CSEInterfaceManager::PrintToRenderWindow
	};

	static const CSEScriptInterface			kCSEScriptInterface =
	{
		CSEInterfaceManager::RegisterScriptCommand,
	};

	CSEInterfaceManager						CSEInterfaceManager::Instance;
	const UInt8								CSEInterfaceManager::kInterfaceVersion = 2;

	CSEInterfaceManager::CSEInterfaceManager() :
		ConsolePrintCallbacks(),
		ObScriptCommandURLs(),
		CodaScriptPluginCommandRegistrar("Plugin Functions")
	{
		;//
	}

	CSEInterfaceManager::~CSEInterfaceManager()
	{
		ConsolePrintCallbacks.clear();
		ObScriptCommandURLs.clear();
	}

	const CSEInterface* CSEInterfaceManager::GetInterface()
	{
		return &kCSEInterface;
	}

	UInt32 CSEInterfaceManager::ConsumeIntelliSenseInterface()
	{
		for (CommandURLMapT::const_iterator Itr = ObScriptCommandURLs.begin(); Itr != ObScriptCommandURLs.end(); Itr++)
		{
			CLIWrapper::Interfaces::SE->AddScriptCommandDeveloperURL(Itr->first.c_str(), Itr->second.c_str());
		}

		return ObScriptCommandURLs.size();
	}

	void CSEInterfaceManager::ConsumeConsoleInterface()
	{
		BGSEECONSOLE->RegisterPrintCallback(CSEConsolePrintCallbackPrototype);
	}

	void CSEInterfaceManager::ConsumeScriptInterface( BGSEditorExtender::BGSEEScript::CodaScriptRegistrarListT& Registrars )
	{
		Registrars.push_back(&CodaScriptPluginCommandRegistrar);
	}

	void CSEInterfaceManager::CSEConsolePrintCallbackPrototype(const char* Prefix, const char* Message)
	{
		for (ConsolePrintCallbackListT::const_iterator Itr = Instance.ConsolePrintCallbacks.begin(); Itr != Instance.ConsolePrintCallbacks.end(); Itr++)
			(*Itr)(Message, Prefix);
	}

	const void* CSEInterfaceManager::InitializeInterface(UInt8 InterfaceType)
	{
		switch (InterfaceType)
		{
		case CSEInterface::kCSEInterface_Console:
			return &kCSEConsoleInterface;
		case CSEInterface::kCSEInterface_IntelliSense:
			return &kCSEIntelliSenseInterface;
		case CSEInterface::kCSEInterface_Renderer:
			return &kCSERendererInterface;
		case CSEInterface::kCSEInterface_Script:
			return &kCSEScriptInterface;
		default:
			return NULL;
		}
	}

	UInt8 CSEInterfaceManager::GetVersion()
	{
		return kInterfaceVersion;
	}

	void CSEInterfaceManager::RegisterCommandURL(const char *CommandName, const char *URL)
	{
		Instance.ObScriptCommandURLs.insert(std::make_pair(CommandName, URL));
	}

	void CSEInterfaceManager::PrintToConsole(const char* Prefix, const char* FormatString, ...)
	{
		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, FormatString);
		vsprintf_s(Buffer, sizeof(Buffer), FormatString, Args);
		va_end(Args);

		BGSEECONSOLE->LogMsg(Prefix, Buffer);
	}

	void CSEInterfaceManager::RegisterConsoleCallback(CSEConsoleInterface::ConsolePrintCallback Handler)
	{
		for (ConsolePrintCallbackListT::const_iterator Itr = Instance.ConsolePrintCallbacks.begin(); Itr != Instance.ConsolePrintCallbacks.end(); Itr++)
		{
			if (*Itr == Handler)
				return;
		}

		Instance.ConsolePrintCallbacks.push_back(Handler);
	}

	void CSEInterfaceManager::PrintToRenderWindow(const char* Message, float DisplayDuration)
	{
		RenderWindowPainter::RenderChannelNotifications->Queue(DisplayDuration, "%s", Message);
	}

	void CSEInterfaceManager::RegisterScriptCommand(BGSEditorExtender::BGSEEScript::ICodaScriptCommand* Command)
	{
		Instance.CodaScriptPluginCommandRegistrar.Add(Command);
	}
}