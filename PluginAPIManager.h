#pragma once

#define CSEAPI_NO_CODA		1
#undef CSEAPI_NO_CODA

#include "CSEInterfaceAPI.h"
#include <bgsee\Script\CodaVM.h>

namespace cse
{
	class PluginAPIManager
	{
		static const UInt8				kInterfaceVersion;

		typedef std::map<std::string, std::string>								CommandURLMapT;
		typedef std::list<CSEConsoleInterface::ConsolePrintCallback>			ConsolePrintCallbackListT;

		ConsolePrintCallbackListT												ConsolePrintCallbacks;
		CommandURLMapT															ObScriptCommandURLs;
		bgsee::script::CodaScriptCommandRegistrar								CodaScriptPluginCommandRegistrar;
		bool																	Initialized;

		static void						ConsolePrintCallbackPrototype(const char* Prefix, const char* Message);
	public:
		PluginAPIManager();
		~PluginAPIManager();

		void							Initialize(void);
		void							Deinitailize(void);

		const CSEInterface*				GetInterface();

		UInt32							ConsumeIntelliSenseInterface();
		void							ConsumeConsoleInterface();
		void							ConsumeScriptInterface(bgsee::script::CodaScriptRegistrarListT& Registrars);

		static UInt8					GetVersion();
		static const void*				InitializeInterface(UInt8 InterfaceType);

		static void						RegisterCommandURL(const char* CommandName, const char* URL);
		static void						RegisterConsoleCallback(CSEConsoleInterface::ConsolePrintCallback Handler);
		static void						RegisterScriptCommand(bgsee::script::ICodaScriptCommand* Command);
		static void						PrintToConsole(const char*	Prefix, const char* FormatString, ...);
		static void						PrintToRenderWindow(const char* Message, float DisplayDuration);

		static PluginAPIManager			Instance;
	};
}
