#pragma once

#include "mpParser.h"
#include "mpICallback.h"
#include "mpIOprt.h"
#include "mpOprtBinCommon.h"
#include "ScriptCommandInfo.h"

namespace CSAutomationScript
{
	class CommandTable
	{
		typedef std::multimap<std::string, CSASCommandInfo*>	CommandInfoList;
		CommandInfoList											CommandList;

		std::string										SanitizeLinkableString(const char* String);
		bool											GetParamInfoHasDetails(CSASParamInfo* ParamInfo);
		void											AppendToStream(std::fstream& Out, const char* Fmt, ...);
		const char*										GetSimpleStringForParamType(UInt8 ParamType);
		void											DumpCommandParamInfo(std::fstream& Out, CSASCommandInfo* Command);
		void											DumpCommandDefinition(std::fstream& Out, CSASCommandInfo* Command);
		void											DumpParamInfoDetails(std::fstream& Out, const char* ParamName, const char* Details);
		void											DumpFunctionSyntaxInfo(std::fstream& Out);
		void											DumpTOC(std::fstream& Out);
		void											DumpProlog(std::fstream& Out);
	public:
		CSASCommandInfo*								LookupCommandByIdentifier(const char* ID, bool AsAlias = false);
		bool											RegisterCommand(CSASCommandInfo* Command, const char* CategoryName);
		void											DumpDocumentation();

		void											RegisterWithParser(mup::ParserX* Parser);
		void											Initialize();
	};

	extern CommandTable									g_CSASCommandTable;
	#define REGISTER_CSASCOMMAND(name, category)		g_CSASCommandTable.RegisterCommand(&kCSASCommandInfo_ ##name##, category)

	class CSASCommand : public mup::ICallback
	{
	protected:
		CSASCommandInfo*					CommandData;
		bool								AliasInstance;
	public:
		CSASCommand(CSASCommandInfo* Data, bool Alias) : ICallback(mup::cmFUNC, ((Alias) ? Data->Alias : Data->Name), Data->NoOfParams),
														 CommandData(Data), AliasInstance(Alias) {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;

		static bool							ExtractCommandArgs(CSASDataElement* ArgArray, CSASParamInfo* Parameters, UInt32 NoOfParams, ...);
	};

	// special commands/operators
	class ReturnCommand : public mup::ICallback
	{
	public:
		ReturnCommand() : ICallback(mup::cmFUNC, "return", -1) {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};
	DECLARE_CSASCOMMAND(Return);

	class CallCommand : public mup::ICallback
	{
	public:
		CallCommand() : ICallback(mup::cmFUNC, "call", -1) {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};
	DECLARE_CSASCOMMAND(Call);

	class BreakCommand : public mup::ICallback
	{
	public:
		BreakCommand() : ICallback(mup::cmFUNC, "break", 0) {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};
	DECLARE_CSASCOMMAND(Break);

	class ContinueCommand : public mup::ICallback
	{
	public:
		ContinueCommand() : ICallback(mup::cmFUNC, "continue", 0) {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};
	DECLARE_CSASCOMMAND(Continue);

	class GetSecondsPassedCommand : public mup::ICallback
	{
	public:
		GetSecondsPassedCommand() : ICallback(mup::cmFUNC, "getsecondspassed", 0) {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};
	DECLARE_CSASCOMMAND(GetSecondsPassed);

	class OprtCastToRef : public mup::IOprtInfix
	{
	public:
		OprtCastToRef() : IOprtInfix("(ref)") {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};

	class OprtCastToStr : public mup::IOprtInfix
	{
	public:
		OprtCastToStr(const char* ID) : IOprtInfix(ID) {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};

	using namespace mup;
	MUP_BINARY_OPERATOR(OprtBAndSymb, _T("&&"),  bool_type, _T("less than"), 1, oaLEFT, a&&b)
	MUP_BINARY_OPERATOR(OprtBOrSymb,  _T("||"),  bool_type, _T("less than"), 1, oaLEFT, a||b)

	extern CSASParamInfo kParams_OneForm[];
	extern CSASParamInfo kParams_OneNumber[];
	extern CSASParamInfo kParams_OneString[];
	extern CSASParamInfo kParams_FormNumber[];
	extern CSASParamInfo kParams_FormString[];
	extern CSASParamInfo kParams_TwoForms[];
}

#define EXTRACT_CSASARGS(...)						CSASCommand::ExtractCommandArgs(ArgArray, ParamInfo, NoOfParams, ##__VA_ARGS__)