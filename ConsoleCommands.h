#pragma once

void ConsoleCommandCallback(const char* Message, const char* Prefix);

struct CSEConsoleCommandInfo
{
	typedef void				(* CSEConsoleCommandHandler)(UInt32 ParamCount, const char* Args);

	const char*					CommandName;
	UInt32						ParamCount;
	CSEConsoleCommandHandler	Handler;

};

#define CSECCMD_ARGS	UInt32 ParamCount, const char* Args	
#define DEFINE_CSECCMD(name, paramcount)	\
	CSEConsoleCommandInfo kCSECCmd_## name = {	\
	# name,										\
	## paramcount,									\
	CSECCmd_ ## name ## _Handler					\
	}

class CSEConsoleCommandTable
{
	typedef std::vector<CSEConsoleCommandInfo*>		CmdTbl;
	CmdTbl											CommandTable;

	void								AddCommandToTable(CSEConsoleCommandInfo* Command);
public:
	void								InitializeCommandTable();
	CSEConsoleCommandInfo*				GetCommandByName(const char* CommandName);
};


extern CSEConsoleCommandTable	g_ConsoleCommandTable;

extern CSEConsoleCommandInfo	kCSECCmd_88MPH;
extern CSEConsoleCommandInfo	kCSECCmd_LoadPlugin;
extern CSEConsoleCommandInfo	kCSECCmd_LoadForm;
extern CSEConsoleCommandInfo	kCSECCmd_SavePlugin;
extern CSEConsoleCommandInfo	kCSECCmd_AutoSave;
extern CSEConsoleCommandInfo	kCSECCmd_Exit;
