#include "CSECoda.h"
#include "CSEAchievements.h"
#include "CSEInterfaceManager.h"

#include "Commands\CodaScriptCommands-Form.h"
#include "Commands\CodaScriptCommands-Reference.h"
#include "Commands\CodaScriptCommands-BFC.h"

namespace ConstructionSetExtender
{
	namespace BGSEEScript
	{
		void ExtractFormArguments( UInt32 Count, ... )
		{
			va_list Args;
			va_start(Args, Count);

			for (int i = 0; i < Count; i++)
			{
				TESForm** CurrentArg = va_arg(Args, TESForm**);
				UInt32 FormID = (UInt32)*CurrentArg;

				if (FormID)
					*CurrentArg = TESForm::LookupByFormID(FormID);
				else
					*CurrentArg = NULL;
			}

			va_end(Args);
		}

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
			ScriptCommands.push_back(Commands::Form::GetRegistrar());
			ScriptCommands.push_back(Commands::Reference::GetRegistrar());
			ScriptCommands.push_back(Commands::BaseFormComponent::GetRegistrar());

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