#include "CSECoda.h"
#include "CSEAchievements.h"
#include "CSEInterfaceManager.h"

namespace ConstructionSetExtender
{
	namespace BGSEEScript
	{
		void BGSEEConsoleCmd_RunCodaScript_ExecuteHandler(UInt32 ParamCount, const char* Args)
		{
			SME::StringHelpers::Tokenizer ArgParser(Args, " ,");
			std::string CurrentArg;

			std::string ScriptName;

			for (int i = 1; i <= ParamCount; i++)
			{
				ArgParser.NextToken(CurrentArg);
				switch (i)
				{
				case 1:
					ScriptName = CurrentArg;
					break;
				}
			}

			BGSEECONSOLE_MESSAGE("Executing Coda Script '%s'", ScriptName.c_str());

			bool ThrowAway = false;
			CODAVM->RunScript(ScriptName, NULL, NULL, ThrowAway);

			BGSEEACHIEVEMENTS->Unlock(Achievements::kAutomaton);
		}

		DEFINE_BGSEECONSOLECMD(RunCodaScript, 1);

		void Initialize()
		{
			BGSEditorExtender::BGSEEScript::CodaScriptRegistrarListT ScriptCommands;
			CSEInterfaceManager::Instance.ConsumeScriptInterface(ScriptCommands);

			bool ComponentInitialized = CODAVM->Initialize(BGSEditorExtender::BGSEEResourceLocation("Coda"),
														"http://cs.elderscrolls.com/index.php/",
														BGSEEMAIN->INIGetter(),
														BGSEEMAIN->INISetter(),
														ScriptCommands);

			SME_ASSERT(ComponentInitialized);

			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_RunCodaScript);
		}
	}
}