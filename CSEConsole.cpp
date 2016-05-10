#include "CSEConsole.h"
#include "Hooks\Hooks-Plugins.h"
#include "Hooks\Hooks-Dialog.h"
#include "CSEAchievements.h"
#include "CSEInterfaceManager.h"
#include "CSEFormUndoStack.h"

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
				SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, TESCSMain::kToolbar_DataFiles, 0);
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
			SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, TESCSMain::kToolbar_Save, 0);
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

		void BGSEEConsoleCmd_Undo_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			BGSEEUNDOSTACK->PerformUndo();
		}

		void BGSEEConsoleCmd_Redo_ExecuteHandler(BGSEECONSOLECMD_ARGS)
		{
			BGSEEUNDOSTACK->PerformRedo();
		}

		DEFINE_BGSEECONSOLECMD(88MPH, 0);
		DEFINE_BGSEECONSOLECMD(Wubble, 0);
		DEFINE_BGSEECONSOLECMD(LoadPlugin, 2);
		DEFINE_BGSEECONSOLECMD(LoadForm, 1);
		DEFINE_BGSEECONSOLECMD(SavePlugin, 0);
		DEFINE_BGSEECONSOLECMD(AutoSave, 0);
		DEFINE_BGSEECONSOLECMD(Exit, 0);
		DEFINE_BGSEECONSOLECMD(Crash, 1);
		DEFINE_BGSEECONSOLECMD(Undo, 0);
		DEFINE_BGSEECONSOLECMD(Redo, 0);

		CSEConsoleWarningRegistrar::~CSEConsoleWarningRegistrar()
		{
			;//
		}

		void CSEConsoleWarningRegistrar::operator()( bgsee::BGSEEConsoleWarningManager* Manager )
		{
			Manager->RegisterWarning(new bgsee::BGSEEConsoleWarning("28976C49-8975-49BD-83C5-871B224504A7",
									"Magic Item has no effects defined",
									2, 0x0056DF1D, 0x0056E32D));

			Manager->RegisterWarning(new bgsee::BGSEEConsoleWarning("0240B41A-C1DF-4641-AF1C-858A32013333",
									"Armor/Clothing needs to have biped slots selected in the editor",
									2, 0x005162F7, 0x00517CB7));

			Manager->RegisterWarning(new bgsee::BGSEEConsoleWarning("C39EB363-D4D0-4CF6-8B9F-90D57E2B5E93",
									"NiControllerSequence::StoreTargets failed to find target",
									4, 0x007379D3, 0x00737B75, 0x00737CF4, 0x00737E1B));

			Manager->RegisterWarning(new bgsee::BGSEEConsoleWarning("2A7BBE4C-492B-4085-8DF7-62A89F393B8A",
									"Duplicate base anim group in files",
									2, 0x004A7E0D, 0x004A7E31));

			Manager->RegisterWarning(new bgsee::BGSEEConsoleWarning("02A85697-A602-41E1-BBDC-20871BE0EC3D",
									"File should/NOT be a looping animation",
									1, 0x004C9E5F));

			Manager->RegisterWarning(new bgsee::BGSEEConsoleWarning("7F709F5D-999D-40FF-B86A-FF17203C5676",
									"Controller priority less than 0 found in sequence on bone",
									1, 0x0046C7D0));

			Manager->RegisterWarning(new bgsee::BGSEEConsoleWarning("9B986781-814C-478A-9D12-1BAD354F7193",
									"Pathgrid for cell contains inter-grid connections",
									1, 0x0054F6BE));

			Manager->RegisterWarning(new bgsee::BGSEEConsoleWarning("2B3B0430-3C6B-40D9-9A8E-FED0B377CC5B",
									"Bad note string in animation frame",
									2, 0x004C9EBD, 0x004CA03A));
		}

		bool Initialized = false;

		void Initialize()
		{
			if (Settings::Dialogs::kShowMainWindowsInTaskbar.GetData().i)
			{
				bgsee::BGSEEWindowStyler::StyleData RegularAppWindow = {0};
				RegularAppWindow.Extended = WS_EX_APPWINDOW;
				RegularAppWindow.ExtendedOp = bgsee::BGSEEWindowStyler::StyleData::kOperation_OR;

				BGSEEUI->GetWindowStyler()->RegisterStyle(IDD_BGSEE_CONSOLE, RegularAppWindow);
			}

			BGSEECONSOLE->InitializeUI(BGSEEUI->GetMainWindow(), BGSEEMAIN->GetExtenderHandle());
			BGSEECONSOLE->InitializeWarningManager(BGSEEMAIN->INIGetter(), BGSEEMAIN->INISetter(), CSEConsoleWarningRegistrar());

			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_88MPH);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_Wubble);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_AutoSave);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_Exit);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_LoadForm);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_LoadPlugin);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_SavePlugin);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_Crash);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_Undo);
			BGSEECONSOLE->RegisterConsoleCommand(&kBGSEEConsoleCmd_Redo);

			CSEInterfaceManager::Instance.ConsumeConsoleInterface();
			Initialized = true;
		}
	}
}