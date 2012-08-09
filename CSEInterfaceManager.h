#pragma once

#include "CSEInterfaceAPI.h"
#include <CodaVM.h>

namespace ConstructionSetExtender
{
	class CSEInterfaceManager
	{
		static const UInt8				kInterfaceVersion;

		typedef std::map<std::string, std::string>								CommandURLMapT;
		typedef std::list<CSEConsoleInterface::ConsolePrintCallback>			ConsolePrintCallbackListT;

		ConsolePrintCallbackListT												ConsolePrintCallbacks;
		CommandURLMapT															ObScriptCommandURLs;
		BGSEditorExtender::BGSEEScript::CodaScriptCommandRegistrar				CodaScriptPluginCommandRegistrar;
		bool																	Initialized;

		static void						CSEConsolePrintCallbackPrototype(const char* Prefix, const char* Message);
	public:
		CSEInterfaceManager();
		~CSEInterfaceManager();

		void							Initialize(void);
		void							Deinitailize(void);

		const CSEInterface*				GetInterface();

		UInt32							ConsumeIntelliSenseInterface();
		void							ConsumeConsoleInterface();
		void							ConsumeScriptInterface(BGSEditorExtender::BGSEEScript::CodaScriptRegistrarListT& Registrars);

		static UInt8					GetVersion();
		static const void*				InitializeInterface(UInt8 InterfaceType);

		static void						RegisterCommandURL(const char* CommandName, const char* URL);
		static void						RegisterConsoleCallback(CSEConsoleInterface::ConsolePrintCallback Handler);
		static void						RegisterScriptCommand(BGSEditorExtender::BGSEEScript::ICodaScriptCommand* Command);
		static void						PrintToConsole(const char*	Prefix, const char* FormatString, ...);
		static void						PrintToRenderWindow(const char* Message, float DisplayDuration);

		static CSEInterfaceManager		Instance;
	};
}
