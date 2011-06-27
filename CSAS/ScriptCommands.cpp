#include "ScriptCommands.h"
#include "Array.h"
#include "ScriptRunner.h"
#include "..\ExtenderInternals.h"

#include "Commands\Commands_General.h"
#include "Commands\Commands_Form.h"

namespace CSAutomationScript
{
	CommandTable				g_CSASCommandTable;

	CSASCommandInfo* CommandTable::LookupCommandByIdentifier(const char* ID, bool AsAlias)
	{
		if (!ID)
			return NULL;

		for (CommandInfoList::iterator Itr = CommandList.begin(); Itr != CommandList.end(); Itr++)
		{
			if ((!AsAlias && (*Itr)->Name && !_stricmp(ID, (*Itr)->Name)) || 
				(AsAlias && (*Itr)->Alias && !_stricmp(ID, (*Itr)->Alias)))
			{
				return *Itr;
			}
		}

		return NULL;
	}

	bool CommandTable::RegisterCommand(CSASCommandInfo* Command)
	{
		if (!Command->Name)
			return false;

		if (LookupCommandByIdentifier(Command->Name) ||
			LookupCommandByIdentifier(Command->Alias, true))
		{
			return false;
		}

		CommandList.push_back(Command);
		return true;
	}

	void CommandTable::DumpDocumentation(std::fstream& Out)
	{
		// ### TODO
	}

	void CommandTable::RegisterWithParser(mup::ParserX* Parser)
	{
		for (CommandInfoList::iterator Itr = CommandList.begin(); Itr != CommandList.end(); Itr++)
		{
			CSASCommandInfo* Command = *Itr;

			Parser->DefineFun(new CSASCommand(Command, false));		// register with name

			if (Command->Alias)
				Parser->DefineFun(new CSASCommand(Command, true));	// register with alias
		}

		// register special commands/operators
		Parser->DefineFun(new ReturnCommand());
		Parser->DefineFun(new CallCommand());
		Parser->DefineFun(new BreakCommand());
		Parser->DefineFun(new ContinueCommand());
		Parser->DefineFun(new GetSecondsPassedCommand());

		Parser->DefineInfixOprt(new OprtCastToRef());
		Parser->DefineInfixOprt(new OprtCastToStr("(str)"));
		Parser->DefineInfixOprt(new OprtCastToStr("$"));
		Parser->DefineOprt(new OprtBAndSymb());
		Parser->DefineOprt(new OprtBOrSymb());
	}

	void CommandTable::InitializeCommandTable()
	{
		RegisterGeneralCommands();
		RegisterFormCommands();
	}

	void CSASCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		CSASDataElement Result;
		CSASDataElement* Arguments = NULL;

		try
		{	
			if (a_iArgc)
			{
				Arguments = new CSASDataElement[a_iArgc]();
				for (int i = 0; i < a_iArgc; i++)
				{
					CSASParamInfo* CurrentParam = &CommandData->Parameters[i];

					switch (CurrentParam->ParamType)
					{
					case CSASDataElement::kParamType_Numeric:
						if (a_pArg[i]->IsScalar() == false)
							throw std::exception(PrintToBuffer("Non-scalar argument %d passed to command %s", i, CommandData->Name));

						Arguments[i].SetNumber(a_pArg[i]->GetFloat());
						break;
					case CSASDataElement::kParamType_String:
						if (a_pArg[i]->IsString() == false)
							throw std::exception(PrintToBuffer("Non-string argument %d passed to command %s", i, CommandData->Name));

						Arguments[i].SetString(a_pArg[i]->GetString().c_str());
						break;
					case CSASDataElement::kParamType_Reference:
						if (a_pArg[i]->IsScalar() == false)
							throw std::exception(PrintToBuffer("Non-reference type argument %d passed to command %s", i, CommandData->Name));

						Arguments[i].SetForm(TESForm_LookupByFormID((UInt32)((int)a_pArg[i]->GetFloat())));
						break;
					}
				}
			}

			if (CommandData->Handler(Arguments, &Result, CommandData->Parameters, CommandData->NoOfParams))
			{
				switch (CommandData->ReturnType)
				{
				case CSASDataElement::kParamType_Numeric:
					*ret = Result.GetNumber();
					break;
				case CSASDataElement::kParamType_String:
					*ret = std::string(Result.GetString());
					break;
				case CSASDataElement::kParamType_Reference:
					*ret = (int)Result.GetForm()->formID;
					break;
				case CSASDataElement::kParamType_Array:
					Result.GetArray()->ConvertToMUPArray(*ret);
					break;
				}
			}
		}
		catch (...)													// catch any exceptions thrown by the command handler and rethrow
		{
			if (Arguments)
				delete [] Arguments;

			throw;
		}

		if (Arguments)
			delete [] Arguments;
	}

	const mup::char_type* CSASCommand::GetDesc() const
	{
		return CommandData->Description;
	}

	mup::IToken* CSASCommand::Clone() const
	{
		return new CSASCommand(*this);
	}

	bool CSASCommand::ExtractCommandArgs(CSASDataElement* ArgArray, CSASParamInfo* Parameters, UInt32 NoOfParams, ...)
	{
		bool Result = true;

		va_list Args;
		va_start(Args, NoOfParams);

		for (int i = 0; i < NoOfParams; i++)
		{
			CSASParamInfo* CurrentParam = &Parameters[i];
			CSASDataElement* CurrentArg = &ArgArray[i];

			assert(CurrentArg->GetType() == CurrentParam->ParamType);

			switch (CurrentParam->ParamType)
			{
			case CSASDataElement::kParamType_Numeric:
				{
					double* Out = va_arg(Args, double*);
					*Out = CurrentArg->GetNumber();
					break;
				}
			case CSASDataElement::kParamType_String:
				{
					char* Out = va_arg(Args, char*);
					sprintf_s(Out, strlen(CurrentArg->GetString()) + 1, "%s", CurrentArg->GetString());
					break;
				}
			case CSASDataElement::kParamType_Reference:
				{
					TESForm** Out = va_arg(Args, TESForm**);
					*Out = CurrentArg->GetForm();
					break;
				}
			default:
				assert(0);		// Unexpected param type
				break;
			}
		}

		va_end(Args);

		return Result;
	}

	void ReturnCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		ScriptContext* ExecutingScript = SCRIPTRUNNER->GetExecutingContext();
		assert(ExecutingScript && ExecutingScript->GetExecutionState() == ScriptContext::kExecutionState_Default);

		if (ExecutingScript->GetIsLoopExecuting())
			throw std::exception("Return command called inside a loop context");

		if (a_iArgc == 1)
		{
			if (a_pArg[0]->IsArray())
				throw std::exception("Array passed to Return command");

			mup::IValue* ArgVal = &(*a_pArg[0]);

			switch (ArgVal->GetCode())
			{
			case mup::cmVAR:
				ExecutingScript->SetReturnValue(*((dynamic_cast<mup::Variable*>(ArgVal))->GetPtr()));
				break;
			case mup::cmVAL:
				ExecutingScript->SetReturnValue(*dynamic_cast<mup::Value*>(ArgVal));
				break;
			}
		}
		else if (a_iArgc)
			throw std::exception("Too many arguments passed to Return command");

		ExecutingScript->SetExecutionState(ScriptContext::kExecutionState_Break);
	}

	const mup::char_type* ReturnCommand::GetDesc() const
	{
		return "Stops the execution of the calling script and optionally returns a value.";
	}

	mup::IToken* ReturnCommand::Clone() const
	{
		return new ReturnCommand(*this);
	}

	void CallCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		int ArgCount = a_iArgc;
		
		if (ArgCount < 1)
			throw std::exception("Too few arguments passed to Call command");
		else if (ArgCount > 1 + MAX_BEGIN_BLOCK_PARAMS)		// arg1 = script name
			throw std::exception("Too many arguments passed to Call command");

		std::string ScriptName = a_pArg[0]->GetString();

		ParameterList Parameters;
		mup::Value ReturnValueBuffer;
		bool HasReturnedValue = false;

		for (int i = 1; i < ArgCount; i++)
		{
			mup::IValue* ArgVal = &(*a_pArg[i]);
			mup::Value Param;

			switch (ArgVal->GetCode())
			{
			case mup::cmVAR:
				Param = *((dynamic_cast<mup::Variable*>(ArgVal))->GetPtr());
				break;
			case mup::cmVAL:
				Param = *dynamic_cast<mup::Value*>(ArgVal);
				break;
			}

			Parameters.push_back(Param);
		}

		bool Result = SCRIPTRUNNER->RunScript(ScriptName, (Parameters.size() ? &Parameters : NULL), ReturnValueBuffer, &HasReturnedValue);

		if (HasReturnedValue)
			*ret = ReturnValueBuffer;
	}

	const mup::char_type* CallCommand::GetDesc() const
	{
		return "Invokes a function call.";
	}

	mup::IToken* CallCommand::Clone() const
	{
		return new CallCommand(*this);
	}

	void OprtCastToRef::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int /*a_iArgc*/)  
	{ 
		switch(a_pArg[0]->GetType())
		{
		case 'f':  
		case 'i': 
		case 'b':  
			*ret = (mup::int_type)a_pArg[0]->GetFloat();
			break;
		default:
			{
			mup::ErrorContext err;
			err.Errc = mup::ecINVALID_TYPECAST;
			err.Type1 = a_pArg[0]->GetType();
			err.Type2 = 'i';
			throw mup::ParserError(err);
			} 
		}
	}

	const mup::char_type* OprtCastToRef::GetDesc() const 
	{ 
		return _T("Cast a value into a reference pointer"); 
	}

	mup::IToken* OprtCastToRef::Clone() const 
	{ 
		return new OprtCastToRef(*this); 
	}

	void OprtCastToStr::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int /*a_iArgc*/)  
	{ 
		char Buffer[0x32] = {0};

		switch(a_pArg[0]->GetType())
		{
		case 'f':  
		case 'i': 
		case 'b':
			sprintf_s(Buffer, sizeof(Buffer), "%0.0f", a_pArg[0]->GetFloat());
			*ret = std::string(Buffer);
			break;
		case 's':
			*ret = a_pArg[0]->GetString();
			break;
		default:
			{
			mup::ErrorContext err;
			err.Errc = mup::ecINVALID_TYPECAST;
			err.Type1 = a_pArg[0]->GetType();
			err.Type2 = 's';
			throw mup::ParserError(err);
			} 
		}
	}

	const mup::char_type* OprtCastToStr::GetDesc() const 
	{ 
		return _T("Cast a value into a string"); 
	}

	mup::IToken* OprtCastToStr::Clone() const 
	{ 
		return new OprtCastToStr(*this); 
	}

	void BreakCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		ScriptContext* ExecutingScript = SCRIPTRUNNER->GetExecutingContext();
		assert(ExecutingScript);

		ExecutingScript->SetExecutingLoopState(LoopBlock::kState_Break);
		ExecutingScript->SetExecutionState(ScriptContext::kExecutionState_Break);

		*ret = (int)false;
	}

	const mup::char_type* BreakCommand::GetDesc() const
	{
		return "Breaks the execution of the currently executing loop.";
	}

	mup::IToken* BreakCommand::Clone() const
	{
		return new BreakCommand(*this);
	}

	void ContinueCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		ScriptContext* ExecutingScript = SCRIPTRUNNER->GetExecutingContext();
		assert(ExecutingScript);

		ExecutingScript->SetExecutingLoopState(LoopBlock::kState_Continue);
		ExecutingScript->SetExecutionState(ScriptContext::kExecutionState_Break);

		*ret = (int)false;
	}

	const mup::char_type* ContinueCommand::GetDesc() const
	{
		return "Skips to the end of the currently executing loop.";
	}

	mup::IToken* ContinueCommand::Clone() const
	{
		return new ContinueCommand(*this);
	}

	void GetSecondsPassedCommand::Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
	{
		if (!GLOBALSCRIPTMANAGER->GetInExecutionLoop())
			throw std::exception("GetSecondsPassed command called in a non-global script");

		*ret = (float)GLOBALSCRIPTMANAGER->GetSecondsPassed();
	}

	const mup::char_type* GetSecondsPassedCommand::GetDesc() const
	{
		return "Returns the amount of time passed since the last execution of the calling script. Only applicable to global scripts.";
	}

	mup::IToken* GetSecondsPassedCommand::Clone() const
	{
		return new GetSecondsPassedCommand(*this);
	}

	CSASParamInfo kParams_OneForm[1] =
	{
		{ "Form", CSASDataElement::kParamType_Reference }
	};
}
