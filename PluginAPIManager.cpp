#include "[Common]\CLIWrapper.h"
#include "PluginAPIManager.h"
#include "Render Window\RenderWindowManager.h"

namespace cse
{
	static const CSEInterface				kCSEInterface =
	{
		PluginAPIManager::InitializeInterface,
		PluginAPIManager::GetVersion,
	};

	static const CSEConsoleInterface		kCSEConsoleInterface =
	{
		PluginAPIManager::PrintToConsole,
		PluginAPIManager::RegisterConsoleCallback
	};

	static const CSEIntelliSenseInterface	kCSEIntelliSenseInterface =
	{
		PluginAPIManager::RegisterCommandURL
	};

	static const CSERendererInterface		kCSERendererInterface =
	{
		PluginAPIManager::PrintToRenderWindow
	};

	static const CSEScriptInterface			kCSEScriptInterface =
	{
		PluginAPIManager::RegisterScriptCommand,
	};

	PluginAPIManager						PluginAPIManager::Instance;
	const UInt8								PluginAPIManager::kInterfaceVersion = 3;

	PluginAPIManager::PluginAPIManager() :
		ConsolePrintCallbacks(),
		ObScriptCommandURLs(),
		CodaScriptPluginCommandRegistrar("Plugin Functions"),
		Initialized(false)
	{
		;//
	}

	PluginAPIManager::~PluginAPIManager()
	{
		Deinitailize();
	}

	const CSEInterface* PluginAPIManager::GetInterface()
	{
		SME_ASSERT(Initialized);

		return &kCSEInterface;
	}

	UInt32 PluginAPIManager::ConsumeIntelliSenseInterface()
	{
		SME_ASSERT(Initialized);

		for (CommandURLMapT::const_iterator Itr = ObScriptCommandURLs.begin(); Itr != ObScriptCommandURLs.end(); Itr++)
		{
			cliWrapper::interfaces::SE->AddScriptCommandDeveloperURL(Itr->first.c_str(), Itr->second.c_str());
		}

		return ObScriptCommandURLs.size();
	}

	void PluginAPIManager::ConsumeConsoleInterface()
	{
		SME_ASSERT(Initialized);

		BGSEECONSOLE->RegisterPrintCallback(ConsolePrintCallbackPrototype);
	}

	void PluginAPIManager::ConsumeScriptInterface( bgsee::script::CodaScriptCommandRegistrar::ListT& OutRegistrars )
	{
		SME_ASSERT(Initialized);

		OutRegistrars.push_back(&CodaScriptPluginCommandRegistrar);
	}

	void PluginAPIManager::Initialize( void )
	{
		Initialized = true;			// just for the sake of completeness
	}

	void PluginAPIManager::Deinitailize( void )
	{
		ConsolePrintCallbacks.clear();
		ObScriptCommandURLs.clear();

		Initialized = false;
	}

	void PluginAPIManager::ConsolePrintCallbackPrototype(const char* Prefix, const char* Message)
	{
		if (Instance.Initialized == false)
			return;

		for (ConsolePrintCallbackListT::const_iterator Itr = Instance.ConsolePrintCallbacks.begin(); Itr != Instance.ConsolePrintCallbacks.end(); Itr++)
			(*Itr)(Message, Prefix);
	}

	UInt8 PluginAPIManager::GetVersion()
	{
		return kInterfaceVersion;
	}

	const void* PluginAPIManager::InitializeInterface(UInt8 InterfaceType)
	{
		SME_ASSERT(Instance.Initialized);

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
			return nullptr;
		}
	}

	void PluginAPIManager::RegisterCommandURL(const char *CommandName, const char *URL)
	{
		SME_ASSERT(Instance.Initialized);

		Instance.ObScriptCommandURLs.insert(std::make_pair(CommandName, URL));
	}

	void PluginAPIManager::RegisterConsoleCallback(CSEConsoleInterface::ConsolePrintCallback Handler)
	{
		SME_ASSERT(Instance.Initialized);

		for (ConsolePrintCallbackListT::const_iterator Itr = Instance.ConsolePrintCallbacks.begin(); Itr != Instance.ConsolePrintCallbacks.end(); Itr++)
		{
			if (*Itr == Handler)
				return;
		}

		Instance.ConsolePrintCallbacks.push_back(Handler);
	}

	void PluginAPIManager::RegisterScriptCommand(bgsee::script::ICodaScriptCommand* Command)
	{
		SME_ASSERT(Instance.Initialized);

		Instance.CodaScriptPluginCommandRegistrar.Add(Command);
	}

	void PluginAPIManager::PrintToConsole(const char* Prefix, const char* FormatString, ...)
	{
		if (Instance.Initialized == false)
			return;

		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, FormatString);
		vsprintf_s(Buffer, sizeof(Buffer), FormatString, Args);
		va_end(Args);

		BGSEECONSOLE->LogMsg(Prefix, Buffer);
	}

	void PluginAPIManager::PrintToRenderWindow(const char* Message, float DisplayDuration)
	{
		;// not supported anymore
	}
}