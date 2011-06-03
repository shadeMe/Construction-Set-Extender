#pragma once

#include "mpParser.h"
#include "mpICallback.h"
#include "mpIOprt.h"
#include "mpOprtBinCommon.h"
#include "ScriptCommandInfo.h"
#include <vector>
#include <fstream>

namespace CSAutomationScript
{
	class CommandTable
	{
		typedef std::vector<CSASCommandInfo*>			CommandInfoList;
		CommandInfoList									CommandList;
	public:
		CSASCommandInfo*								LookupCommandByIdentifier(const char* ID, bool AsAlias = false);
		bool											RegisterCommand(CSASCommandInfo* Command);
		void											DumpDocumentation(std::fstream& Out);

		void											RegisterWithParser(mup::ParserX* Parser);
		static void										InitializeCommandTable();
	};

	extern CommandTable									g_CSASCommandTable;
	#define REGISTER_CSASCOMMAND(name)					g_CSASCommandTable.RegisterCommand(&kCSASCommandInfo_ ##name## )

	class CSASCommand : public mup::ICallback
	{
	protected:
		CSASCommandInfo*					CommandData;
		bool								AliasInstance;
	public:
		CSASCommand(CSASCommandInfo* Data, bool Alias) : ICallback(mup::cmFUNC, ((Alias) ? Data->Alias : Data->Name), Data->GetCommandPrototype()),
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
		ReturnCommand() : ICallback(mup::cmFUNC, "return", "i:*") {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};

	class CallCommand : public mup::ICallback
	{
	public:
		CallCommand() : ICallback(mup::cmFUNC, "call", "i:*") {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};

	class OprtCastToRef : public mup::IOprtInfix
	{
	public:
		OprtCastToRef() : IOprtInfix("(ref)", "i:.") {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};

	class OprtCastToStr : public mup::IOprtInfix
	{
	public:
		OprtCastToStr(const char* ID) : IOprtInfix(ID, "s:.") {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};

	class BreakCommand : public mup::ICallback
	{
	public:
		BreakCommand() : ICallback(mup::cmFUNC, "break", "i:v") {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};

	class ContinueCommand : public mup::ICallback
	{
	public:
		ContinueCommand() : ICallback(mup::cmFUNC, "continue", "i:v") {}

		virtual void						Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc);
		virtual const mup::char_type*		GetDesc() const;
		virtual mup::IToken*				Clone() const;
	};

	using namespace mup;
	MUP_BINARY_OPERATOR(OprtBAndSymb, _T("&&"), _T("b:bb"),  bool_type, _T("less than"), 1, oaLEFT, a&&b)
	MUP_BINARY_OPERATOR(OprtBOrSymb,  _T("||"),  _T("b:bb"),  bool_type, _T("less than"), 1, oaLEFT, a||b)
}

#define EXTRACT_CSASARGS(commandname, noofparams, ...)						CSASCommand::ExtractCommandArgs(ArgArray, kParams_ ##commandname##, noofparams, ##__VA_ARGS__)