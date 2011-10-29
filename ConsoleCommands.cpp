#include "Console.h"
#include "ConsoleCommands.h"
#include "Hooks\TESFile.h"
#include "CSAS\ScriptRunner.h"
#include "CSAS\ScriptCommands.h"
#include "Achievements.h"
#include "WindowManager.h"
#include "Resource.h"

using namespace Hooks;

CSEConsoleCommandTable		g_ConsoleCommandTable;

void ConsoleCommandCallback(const char* Message, const char* Prefix)
{
	CONSOLE->Indent();

	if (!_stricmp(Prefix, "CMD"))
	{
		if (!_stricmp(Message, "wubble"))
			Achievements::UnlockAchievement(Achievements::kAchievement_EruditeModder);
		else
		{
			Tokenizer MessageToken(Message, " ,");
			std::string OperatingToken, Args = "";

			if (MessageToken.NextToken(OperatingToken) != -1)
			{
				CSEConsoleCommandInfo* Command = g_ConsoleCommandTable.GetCommandByName(OperatingToken.c_str());
				if (Command)
				{
					UInt32 ArgCount = 0;
					UInt32 Offset = MessageToken.NextToken(OperatingToken);
					MessageToken.PrevToken(OperatingToken);

					while (MessageToken.NextToken(OperatingToken) != -1)
						ArgCount++;

					if (ArgCount < Command->ParamCount)
						DebugPrint("Too few arguments passed to console command '%s'. Expected %d.", Command->CommandName, Command->ParamCount);
					else {
						if (Command->ParamCount)
							Args = (std::string(Message)).substr(Offset, strlen(Message));
						Command->Handler(Command->ParamCount, Args.c_str());
					}
				}
			}
		}
	}

	CONSOLE->Exdent();
}

void CSEConsoleCommandTable::AddCommandToTable(CSEConsoleCommandInfo* Command)
{
	CommandTable.push_back(Command);
}

CSEConsoleCommandInfo* CSEConsoleCommandTable::GetCommandByName(const char* CommandName)
{
	for (CmdTbl::const_iterator Itr = CommandTable.begin(); Itr != CommandTable.end(); Itr++)
	{
		if (!_stricmp((*Itr)->CommandName, CommandName))
			return *Itr;
	}
	return NULL;
}

void CSEConsoleCommandTable::InitializeCommandTable()
{
	AddCommandToTable(&kCSECCmd_88MPH);
	AddCommandToTable(&kCSECCmd_LoadPlugin);
	AddCommandToTable(&kCSECCmd_LoadForm);
	AddCommandToTable(&kCSECCmd_SavePlugin);
	AddCommandToTable(&kCSECCmd_AutoSave);
	AddCommandToTable(&kCSECCmd_Exit);
	AddCommandToTable(&kCSECCmd_RunScript);
	AddCommandToTable(&kCSECCmd_DumpDocs);
	AddCommandToTable(&kCSECCmd_DBMP);
}

// COMMANDS

void CSECCmd_88MPH_Handler(CSECCMD_ARGS)
{
	DebugPrint("Great Scott! We left Copernicus in the freezer once again!");
}

void CSECCmd_LoadPlugin_Handler(CSECCMD_ARGS)
{
	Tokenizer ArgParser(Args, " ,");
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
	kAutoLoadActivePluginOnStartup.WriteJump();

	TESFile* File = _DATAHANDLER->LookupPluginByName(PluginName.c_str());

	if (File)
	{
		ToggleFlag(&File->fileFlags, TESFile::kFileFlag_Active, SetActive);
		ToggleFlag(&File->fileFlags, TESFile::kFileFlag_Loaded, true);
		SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CD1, 0);
	}
	else
		DebugPrint("Couldn't load plugin '%s' - It doesn't exist!", PluginName.c_str());

	// epilog
	kAutoLoadActivePluginOnStartup.WriteBuffer();
}

void CSECCmd_LoadForm_Handler(CSECCMD_ARGS)
{
	Tokenizer ArgParser(Args, " ,");
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
		DebugPrint("Couldn't load form '%s' for editing. Recheck the editorID argument.", EditorID.c_str());
}

void CSECCmd_SavePlugin_Handler(CSECCMD_ARGS)
{
	SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CD2, 0);
}

void CSECCmd_AutoSave_Handler(CSECCMD_ARGS)
{
	TESDialog::AutoSave();
}

void CSECCmd_Exit_Handler(CSECCMD_ARGS)
{
	SendMessage(*g_HWND_CSParent, WM_CLOSE, 0, 0);
}

void CSECCmd_RunScript_Handler(CSECCMD_ARGS)
{
	Tokenizer ArgParser(Args, " ,");
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

	bool Throwaway = false;
	DebugPrint("Executing script '%s'", ScriptName.c_str());
	SCRIPTRUNNER->RunScript(ScriptName, NULL, mup::Value(0), &Throwaway);

	Achievements::UnlockAchievement(Achievements::kAchievement_Automaton);
}

void CSECCmd_DBMP_Handler(CSECCMD_ARGS)
{
	Tokenizer ArgParser(Args, " ,");
	std::string CurrentArg;

	std::string Handle;

	for (int i = 1; i <= ParamCount; i++)
	{
		ArgParser.NextToken(CurrentArg);
		switch (i)
		{
		case 1:
			Handle = CurrentArg;
			break;
		}
	}

	UInt32 BMPHandle = 0;
	sscanf_s(Handle.c_str(), "%08X", &BMPHandle);
	DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_DEBUGBMP), NULL, DebugViewBMPDlgProc, (LPARAM)BMPHandle);
}

void CSECCmd_DumpDocs_Handler(CSECCMD_ARGS)
{
	CSAutomationScript::g_CSASCommandTable.DumpDocumentation();
}

DEFINE_CSECCMD(88MPH, 0);
DEFINE_CSECCMD(LoadPlugin, 2);
DEFINE_CSECCMD(LoadForm, 1);
DEFINE_CSECCMD(SavePlugin, 0);
DEFINE_CSECCMD(AutoSave, 0);
DEFINE_CSECCMD(Exit, 0);
DEFINE_CSECCMD(RunScript, 1);
DEFINE_CSECCMD(DumpDocs, 0);
DEFINE_CSECCMD(DBMP, 1);