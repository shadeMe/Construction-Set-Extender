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
		CodaScriptPluginCommandRegistrar("Plugin Functions"),
		Initialized(false)
	{
		;//
	}

	CSEInterfaceManager::~CSEInterfaceManager()
	{
		Deinitailize();
	}

	const CSEInterface* CSEInterfaceManager::GetInterface()
	{
		SME_ASSERT(Initialized);

		return &kCSEInterface;
	}

	UInt32 CSEInterfaceManager::ConsumeIntelliSenseInterface()
	{
		SME_ASSERT(Initialized);

		for (CommandURLMapT::const_iterator Itr = ObScriptCommandURLs.begin(); Itr != ObScriptCommandURLs.end(); Itr++)
		{
			CLIWrapper::Interfaces::SE->AddScriptCommandDeveloperURL(Itr->first.c_str(), Itr->second.c_str());
		}

		return ObScriptCommandURLs.size();
	}

	void CSEInterfaceManager::ConsumeConsoleInterface()
	{
		SME_ASSERT(Initialized);

		BGSEECONSOLE->RegisterPrintCallback(CSEConsolePrintCallbackPrototype);
	}

	void CSEInterfaceManager::ConsumeScriptInterface( BGSEditorExtender::BGSEEScript::CodaScriptRegistrarListT& Registrars )
	{
		SME_ASSERT(Initialized);

		Registrars.push_back(&CodaScriptPluginCommandRegistrar);
	}

	void CSEInterfaceManager::Initialize( void )
	{
		Initialized = true;			// just for the sake of completeness
	}

	void CSEInterfaceManager::Deinitailize( void )
	{
		ConsolePrintCallbacks.clear();
		ObScriptCommandURLs.clear();

		Initialized = false;
	}

	void CSEInterfaceManager::CSEConsolePrintCallbackPrototype(const char* Prefix, const char* Message)
	{
		if (Instance.Initialized == false)
			return;

		for (ConsolePrintCallbackListT::const_iterator Itr = Instance.ConsolePrintCallbacks.begin(); Itr != Instance.ConsolePrintCallbacks.end(); Itr++)
			(*Itr)(Message, Prefix);
	}

	UInt8 CSEInterfaceManager::GetVersion()
	{
		return kInterfaceVersion;
	}

	const void* CSEInterfaceManager::InitializeInterface(UInt8 InterfaceType)
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
			return NULL;
		}
	}

	void CSEInterfaceManager::RegisterCommandURL(const char *CommandName, const char *URL)
	{
		SME_ASSERT(Instance.Initialized);

		Instance.ObScriptCommandURLs.insert(std::make_pair(CommandName, URL));
	}

	void CSEInterfaceManager::RegisterConsoleCallback(CSEConsoleInterface::ConsolePrintCallback Handler)
	{
		SME_ASSERT(Instance.Initialized);

		for (ConsolePrintCallbackListT::const_iterator Itr = Instance.ConsolePrintCallbacks.begin(); Itr != Instance.ConsolePrintCallbacks.end(); Itr++)
		{
			if (*Itr == Handler)
				return;
		}

		Instance.ConsolePrintCallbacks.push_back(Handler);
	}

	void CSEInterfaceManager::RegisterScriptCommand(BGSEditorExtender::BGSEEScript::ICodaScriptCommand* Command)
	{
		SME_ASSERT(Instance.Initialized);

		Instance.CodaScriptPluginCommandRegistrar.Add(Command);
	}

	void CSEInterfaceManager::PrintToConsole(const char* Prefix, const char* FormatString, ...)
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

	void CSEInterfaceManager::PrintToRenderWindow(const char* Message, float DisplayDuration)
	{
		if (Instance.Initialized == false)
			return;

		RenderWindowPainter::RenderChannelNotifications->Queue(DisplayDuration, "%s", Message);
	}
}