#pragma once

#include "ScriptTypes.h"

namespace CSAutomationScript
{
	class CSASCommand;
	class CommandTable;

	struct CSASParamInfo
	{
		const char*						ParamName;
		UInt32							ParamType;
	};

	class CSASCommandInfo
	{
#pragma region Internal Use Only
		std::string						Prototype;
		const char*						GetCommandPrototype();
		void							DumpDocumentation(std::string& Out);
#pragma endregion
	public:
		friend class CSASCommand;
		friend class CommandTable;

		typedef bool (* CommandHandler)(CSASDataElement* ArgArray, CSASDataElement* Result, CSASParamInfo* ParamInfo, UInt32 NoOfParams);

		const char*						Name;			// name of the command
		const char*						Alias;			// command alias 
		const char*						Description;	// description and other relevant info
		UInt32							NoOfParams;
		CSASParamInfo*					Parameters;		// pointer to an array of paraminfo objs, NULL when zero params
		UInt32							ReturnType;
		CommandHandler					Handler;		// command execute handler
														// the result isn't validated if the handler returns false
		CSASCommandInfo(const char* Name, const char* Alias, const char* Description, UInt32 NoOfParams, CSASParamInfo* Parameters, UInt32 ReturnType, CommandHandler Handler) :
						Name(Name), Alias(Alias), Description(Description), NoOfParams(NoOfParams), Parameters(Parameters), ReturnType(ReturnType), Handler(Handler), Prototype("") {}
	};
}

#define DEFINE_CSASCOMMAND_PARAM(name, alias, desc, retntype, paraminfo)	CSASCommandInfo kCSASCommandInfo_ ##name (#name, alias, desc, sizeof(paraminfo), paraminfo, retntype, CSASCmd_ ##name## _Execute)
#define DEFINE_CSASCOMMAND_NOPARAM(name, alias, desc, retntype)				CSASCommandInfo kCSASCommandInfo_ ##name (#name, alias, desc, 0, 0, retntype, CSASCmd_ ##name## _Execute)
#define DEFINE_CSASCOMMAND(name, desc, retntype)							CSASCommandInfo kCSASCommandInfo_ ##name (#name, 0, desc, sizeof(kParams_ ##name), kParams_ ##name, retntype, CSASCmd_ ##name## _Execute)
#define DEFINE_CSASCOMMAND_ALIAS(name, alias, desc, retntype)				CSASCommandInfo kCSASCommandInfo_ ##name (#name, alias, desc, sizeof(kParams_ ##name), kParams_ ##name, retntype, CSASCmd_ ##name## _Execute)
#define DECLARE_CSASCOMMAND(commandname)									extern CSASCommandInfo kCSASCommandInfo_ ##commandname
#define BEGIN_CSASCOMMAND_HANDLER(commandname)								static bool CSASCmd_ ##commandname## _Execute(CSASDataElement* ArgArray, CSASDataElement* Result, CSASParamInfo* ParamInfo, UInt32 NoOfParams)
#define BEGIN_CSASCOMMAND_PARAMINFO(commandname, noofparams)				static CSASParamInfo kParams_ ##commandname## [ ##noofparams## ] =