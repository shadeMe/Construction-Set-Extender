#include "CSEConsole.h"
#include "Hooks\Hooks-Plugins.h"
#include "Hooks\Hooks-Dialog.h"
#include "CSEAchievements.h"
#include "CSEInterfaceManager.h"

#include <BGSEditorExtenderBase_Resource.h>

namespace ConstructionSetExtender
{
	namespace Console
	{
		void BGSEEConsoleCmd_88MPH_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			BGSEECONSOLE_MESSAGE("Great Scott! We left Copernicus in the freezer once again!");
		}

		void BGSEEConsoleCmd_Wubble_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			BGSEEACHIEVEMENTS->Unlock(Achievements::kEruditeModder);
		}

		void BGSEEConsoleCmd_LoadPlugin_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			SME::StringHelpers::Tokenizer ArgParser(Args, " ,");
			std::string CurrentArg;

			std::string PluginName;
			bool SetActive = false;
			for (int i = 1; i <= ParamCount; i++)
			{
				ArgParser.NextToken(CurrentArg);
				switch (i)
				{
				case 1:
					PluginName = CurrentArg;
					break;
				case 2:
					SetActive = (_stricmp(CurrentArg.c_str(), "1") == 0);
					break;
				}
			}

			// prolog
			Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteJump();

			TESFile* File = _DATAHANDLER->LookupPluginByName(PluginName.c_str());

			if (File)
			{
				File->SetActive(SetActive);
				File->SetLoaded(true);
				SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, 0x9CD1, 0);
			}
			else
				BGSEECONSOLE_MESSAGE("Plugin '%s' doesn't exist!", PluginName.c_str());

			// epilog
			Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteBuffer();
		}

		void BGSEEConsoleCmd_LoadForm_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			SME::StringHelpers::Tokenizer ArgParser(Args, " ,");
			std::string CurrentArg;

			std::string EditorID;

			for (int i = 1; i <= ParamCount; i++)
			{
				ArgParser.NextToken(CurrentArg);
				switch (i)
				{
				case 1:
					EditorID = CurrentArg;
					break;
				}
			}

			TESForm* Form = TESForm::LookupByEditorID(EditorID.c_str());
			if (Form)
			{
				TESDialog::ShowFormEditDialog(Form);
			}
			else
				BGSEECONSOLE_MESSAGE("Couldn't load form '%s' for editing. Recheck the editorID argument.", EditorID.c_str());
		}

		void BGSEEConsoleCmd_SavePlugin_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, 0x9CD2, 0);
		}

		void BGSEEConsoleCmd_AutoSave_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			TESCSMain::AutoSave();
		}

		void BGSEEConsoleCmd_Exit_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			SendMessage(BGSEEUI->GetMainWindow(), WM_CLOSE, 0, 0);
		}

		void BGSEEConsoleCmd_Crash_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			SME::StringHelpers::Tokenizer ArgParser(Args, " ,");
			std::string CurrentArg;

			bool Assertion = false;

			for (int i = 1; i <= ParamCount; i++)
			{
				ArgParser.NextToken(CurrentArg);
				switch (i)
				{
				case 1:
					Assertion = CurrentArg != "0";
					break;
				}
			}

			if (Assertion)
			{
				SME_ASSERT(1 == 0);
			}
			else
			{
				*((int*)0) = 0;
			}
		}

		DEFINE_BGSEECONSOLECMD(88MPH, 0);
		DEFINE_BGSEECONSOLECMD(Wubble, 0);
		DEFINE_BGSEECONSOLECMD(LoadPlugin, 2);
		DEFINE_BGSEECONSOLECMD(LoadForm, 1);
		DEFINE_BGSEECONSOLECMD(SavePlugin, 0);
		DEFINE_BGSEECONSOLECMD(AutoSave, 0);
		DEFINE_BGSEECONSOLECMD(Exit, 0);
		DEFINE_BGSEECONSOLECMD(Crash, 1);

		void Initialize()
		{
			if (atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_ShowMainWindowsInTaskbar, BGSEEMAIN->INIGetter())))
			{
				BGSEditorExtender::BGSEEWindowStyler::StyleData RegularAppWindow = {0};
				RegularAppWindow.Extended = WS_EX_APPWINDOW;
				RegularAppWindow.ExtendedOp = BGSEditorExtender::BGSEEWindowStyler::StyleData::kOperation_OR;

				BGSEEUI->GetWindowStyler()->RegisterStyle(IDD_BGSEE_CONSOLE, RegularAppWindow);
			}

			BGSEECONSOLE->InitializeUI(BGSEEUI->GetMainWindow(), BGSEEMAIN->GetExtenderHandle());

			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_88MPH);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_Wubble);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_AutoSave);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_Exit);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_LoadForm);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_LoadPlugin);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_SavePlugin);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_Crash);

			CSEInterfaceManager::Instance.ConsumeConsoleInterface();
		}
	}
}