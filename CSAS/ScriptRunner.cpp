#include "ScriptRunner.h"
#include "..\ExtenderInternals.h"
#include "ScriptCommands.h"

namespace CSAutomationScript
{
	void ScriptContext::Release()
	{
		for (VariableList::iterator Itr = Variables.begin(); Itr != Variables.end(); Itr++)
		{
			delete (*Itr);
		}

		if (MainScriptBlock)
			delete MainScriptBlock;
	}

	ScriptVariable*	ScriptContext::LookupVariableByName(std::string& Name)
	{
		for (VariableList::iterator Itr = Variables.begin(); Itr != Variables.end(); Itr++)
		{
			if (!_stricmp((*Itr)->GetName(), Name.c_str()))
				return *Itr;
		}

		return NULL;
	}

	ScriptVariable*	ScriptContext::LookupVariableByName(const char* Name)
	{
		for (VariableList::iterator Itr = Variables.begin(); Itr != Variables.end(); Itr++)
		{
			if (!_stricmp((*Itr)->GetName(), Name))
				return *Itr;
		}

		return NULL;
	}

	void ScriptContext::AddVariable(std::string& Name)
	{
		Variables.push_back(new ScriptVariable(Name));
	}

	bool ScriptContext::SetParameter(std::string& VariableName, mup::IValue& Value)
	{
		ScriptVariable* Variable = LookupVariableByName(VariableName);

		if (Variable)
			return Variable->SetValue(Value);
		else
			return false;
	}

	void ScriptContext::RegisterVariables()
	{
		for (VariableList::iterator Itr = Variables.begin(); Itr != Variables.end(); Itr++)
		{
			ContextParser.DefineVar((*Itr)->GetName(), mup::Variable(&((mup::Value&)(*Itr)->GetValue())));
		}
	}

	void ScriptContext::RegisterFunctions()
	{
		g_CSASCommandTable.RegisterWithParser(&ContextParser);
	}

	bool ScriptContext::LookupBlockInStack(ScriptBlockStack& BlockStack, CmdToken Block)
	{
		ScriptBlockStack Buffer = BlockStack;

		while (Buffer.size())
		{
			if (Buffer.top() == Block)
				return true;
			else
				Buffer.pop();
		}

		return false;
	}

	ScriptContext::ScriptContext(std::fstream& InputStream, ParameterList* PassedParameters) : ContextParser(mup::pckALL_NON_COMPLEX)
	{
		this->Valid = true;
		this->MainScriptBlock = NULL;
		this->ScriptName = "<unknown>";

		ScriptParser Tokenizer;

		char Buffer[0x512] = {0};
		int CurrentLine = 1, MainBlockLine = -1;
		ScriptBlockStack BlockStack;
		std::vector<std::string> MainBlockContents, ParameterIDs;
		bool Result = true;

		BlockStack.push(ScriptParser::kTokenType_Invalid);
		MainBlockContents.reserve(20);

		while (InputStream.eof() == false)
		{
			ZeroMemory(Buffer, sizeof(Buffer));
			InputStream.getline(Buffer, sizeof(Buffer));

			if (Tokenizer.Tokenize(Buffer, false))
			{
				std::string FirstToken = Tokenizer.Tokens[0];
				std::string SecondToken = (Tokenizer.GetTokenCount() > 1) ? Tokenizer.Tokens[1] : "";

				if (CurrentLine == 1)
				{
					if (Tokenizer.GetFirstTokenType() == ScriptParser::kTokenType_ScriptName)
					{
						if (SecondToken != "")
							this->ScriptName = SecondToken;
						else
						{
							DebugPrint("Line %d: Invalid script name", CurrentLine);
							Result = false;
						}
					}
					else
					{
						DebugPrint("Line %d: Scripts should start with a script name declaration", CurrentLine);
						Result = false;
					}
				}

				switch (Tokenizer.GetFirstTokenType())
				{
				case ScriptParser::kTokenType_Variable:
					if (BlockStack.top() != ScriptParser::kTokenType_Invalid)
					{
						DebugPrint("Line %d: Variable '%s' declared inside a script block", CurrentLine, SecondToken.c_str());
						Result = false;
					}
					else if (LookupVariableByName(SecondToken))
					{
						DebugPrint("Line %d: Variable '%s' redeclaration", CurrentLine, SecondToken.c_str());
						Result = false;
					}
					else if (SecondToken == "")
					{
						DebugPrint("Line %d: Invalid variable name", CurrentLine);
						Result = false;
					}
					else
						AddVariable(SecondToken);

					break;
				case ScriptParser::kTokenType_Begin:
					if (BlockStack.top() != ScriptParser::kTokenType_Invalid)
					{
						DebugPrint("Line %d: Nested Begin block", CurrentLine);
						Result = false;
					}
					else if (MainBlockLine != -1)
					{
						DebugPrint("Line %d: Multiple Begin blocks", CurrentLine);
						Result = false;
					}
					else
					{
						if (PassedParameters)
						{
							for (ScriptParser::TokenList::iterator Itr = ++(Tokenizer.Tokens.begin()); Itr != Tokenizer.Tokens.end(); Itr++)
								ParameterIDs.push_back(*Itr);

							if (ParameterIDs.size() > MAX_BEGIN_BLOCK_PARAMS)
							{
								DebugPrint("Line %d: Too many parameters for Begin block", CurrentLine);
								Result = false;
							}
							else if (PassedParameters->size() != ParameterIDs.size())
							{
								DebugPrint("Line %d: Incorrect number of parameters passed to Begin block - Received %d, expected %d", CurrentLine, PassedParameters->size(), ParameterIDs.size());
								Result = false;
							}
							else
							{
								for (int i = 0; i < PassedParameters->size(); i++)
								{
									if (SetParameter(ParameterIDs[i], PassedParameters->at(i)) == false)
									{
										DebugPrint("Line %d: Couldn't initialize parameter '%s'. Possible reasons: Mismatching types, non-existent parameter variable", CurrentLine, ParameterIDs[i]);
										Result = false;
									}
								}
							}
						}

						MainBlockLine = CurrentLine;
						BlockStack.push(ScriptParser::kTokenType_Begin);
					}
					break;
				case ScriptParser::kTokenType_End:
					if (BlockStack.top() != ScriptParser::kTokenType_Begin)
					{
						DebugPrint("Line %d: Invalid block structure. Command 'End' has no matching 'Begin'", CurrentLine);
						Result = false;
					}
					else
						BlockStack.pop();

					break;
				case ScriptParser::kTokenType_While:
					if (BlockStack.top() == ScriptParser::kTokenType_Invalid)
					{
						DebugPrint("Line %d: Invalid block structure. Script commands must be inside a 'Begin' block", CurrentLine);
						Result = false;
					}
					else if (Tokenizer.GetTokenCount() < 2)
					{
						DebugPrint("Line %d: Invalid condition expression", CurrentLine);
						Result = false;
					}
					else
						BlockStack.push(ScriptParser::kTokenType_Loop);

					break;
				case ScriptParser::kTokenType_ForEach:
					if (BlockStack.top() == ScriptParser::kTokenType_Invalid)
					{
						DebugPrint("Line %d: Invalid block structure. Script commands must be inside a 'Begin' block", CurrentLine);
						Result = false;
					}
					else if (Tokenizer.GetTokenCount() < 4)
					{
						DebugPrint("Line %d: Invalid ForEach expression", CurrentLine);
						Result = false;
					}
					else
						BlockStack.push(ScriptParser::kTokenType_Loop);

					break;
				case ScriptParser::kTokenType_Loop:
					if (BlockStack.top() != ScriptParser::kTokenType_Loop)
					{
						DebugPrint("Line %d: Invalid block structure. Command 'Loop' has no matching 'While' or 'ForEach'", CurrentLine);
						Result = false;
					}
					else
						BlockStack.pop();

					break;
				case ScriptParser::kTokenType_If:
					if (BlockStack.top() == ScriptParser::kTokenType_Invalid)
					{
						DebugPrint("Line %d: Invalid block structure. Script commands must be inside a 'Begin' block", CurrentLine);
						Result = false;
					}
					else if (Tokenizer.GetTokenCount() < 2)
					{
						DebugPrint("Line %d: Invalid condition expression", CurrentLine);
						Result = false;
					}
					else
						BlockStack.push(ScriptParser::kTokenType_If);

					break;
				case ScriptParser::kTokenType_ElseIf:
					if (BlockStack.top() != ScriptParser::kTokenType_If)
					{
						DebugPrint("Line %d: Invalid block structure. Command 'ElseIf' has no matching 'If'", CurrentLine);
						Result = false;
					}
					else if (Tokenizer.GetTokenCount() < 2)
					{
						DebugPrint("Line %d: Invalid condition expression", CurrentLine);
						Result = false;
					}

					break;
				case ScriptParser::kTokenType_Else:
					if (BlockStack.top() != ScriptParser::kTokenType_If)
					{
						DebugPrint("Line %d: Invalid block structure. Command 'Else' has no matching 'If'", CurrentLine);
						Result = false;
					}

					break;
				case ScriptParser::kTokenType_EndIf:
					if (BlockStack.top() != ScriptParser::kTokenType_If)
					{
						DebugPrint("Line %d: Invalid block structure. Command 'EndIf' has no matching 'If'", CurrentLine);
						Result = false;
					}
					else
						BlockStack.pop();

					break;
				case ScriptParser::kTokenType_Return:
					if (LookupBlockInStack(BlockStack, ScriptParser::kTokenType_Loop))
					{
						DebugPrint("Line %d: Command 'Return' called inside a loop context", CurrentLine);
						Result = false;
					}
					break;
				case ScriptParser::kTokenType_Break:
				case ScriptParser::kTokenType_Continue:
					if (!LookupBlockInStack(BlockStack, ScriptParser::kTokenType_Loop))
					{
						DebugPrint("Line %d: Command 'Break'/'Loop' called outside a loop context", CurrentLine);
						Result = false;
					}
					break;
				}
			}

			if (MainBlockLine != -1)
				MainBlockContents.push_back(Buffer);

			CurrentLine++;
		}

		if (!Result)
		{
			this->Valid = false;
		}
		else
		{
			this->ExecutionState = true;
			this->HasReturnValue = false;

			RegisterVariables();
			RegisterFunctions();

			std::string MainBlockText = "";
			for (std::vector<std::string>::iterator Itr = MainBlockContents.begin(); Itr != MainBlockContents.end(); Itr++)
				MainBlockText += *Itr + "\n";

			MainScriptBlock = new BeginBlock(MainBlockText, MainBlockLine);
			if (!MainScriptBlock->IsVaid())
			{
				this->Valid = false;
			}
		}

		while (BlockStack.size())
			BlockStack.pop();

		MainBlockContents.clear();
		ParameterIDs.clear();
	}

	bool ScriptContext::Execute(ScriptRunner* Executor, mup::Value& ReturnValue, bool* HasReturnValue)
	{
		DebugPrint("Executing script '%s'", this->ScriptName.c_str());
		CONSOLE->Indent();

		*HasReturnValue = false;

		if (!Valid)
		{
			DebugPrint("Failed - Errors were found while parsing");
			CONSOLE->Exdent();
			return false;
		}

		MainScriptBlock->Execute(Executor, &ContextParser);
		ReturnValue = this->ReturnValue;

		if (this->HasReturnValue)		
			*HasReturnValue = true;

		CONSOLE->Exdent();
		return true;
	}

	void ScriptContext::SetReturnValue(mup::IValue& Value)
	{
		this->ReturnValue = Value;
		this->HasReturnValue = true;
	}

	void ScriptContext::SetExecutionState(bool State)
	{
		this->ExecutionState = State;
	}

	void ScriptContext::PushLoop(LoopBlock* Block)
	{
		LoopStack.push(Block);
	}

	LoopBlock* ScriptContext::PopLoop()
	{
		ASSERT(LoopStack.size());

		LoopBlock* Result = LoopStack.top();
		LoopStack.pop();
		return Result;
	}

	void ScriptContext::SetExecutingLoopState(UInt8 State)
	{
		ASSERT(LoopStack.size());

		switch (State)
		{
		case LoopBlock::kState_Continue:
			LoopStack.top()->Continue();
			break;
		case LoopBlock::kState_Break:
			LoopStack.top()->Break();
			break;
		}
	}


	ScriptRunner*			ScriptRunner::Singleton = NULL;

	ScriptRunner* ScriptRunner::GetSingleton()
	{
		if (Singleton == NULL)
			Singleton = new ScriptRunner();

		return Singleton;
	}

	ScriptRunner::ScriptRunner()
	{
		;//
	}

	bool ScriptRunner::RunScript(std::string& ScriptName, ParameterList* PassedParameters, mup::Value& ReturnValue, bool* HasReturnValue)
	{
		if (ExecutingContexts.size() >= MAX_RECURSE_DEPTH)
		{
			DebugPrint("Maximum script recursion depth hit");
			return false;
		}

		std::string Path("Data\\Scripts\\" + ScriptName + ".txt");
		std::fstream InputStream(Path.c_str(), std::iostream::in);

		if (!InputStream.fail())
		{
			mup::Value ScriptReturn;
			ScriptContext* NewContext = new ScriptContext(InputStream, PassedParameters);
			ExecutingContexts.push(NewContext);

			bool Result = NewContext->Execute(this, ScriptReturn, HasReturnValue);
			ReturnValue = ScriptReturn;

			delete NewContext;
			ExecutingContexts.pop();

			if (!ExecutingContexts.size() && Array::GetGlobalInstanceCount())
				DebugPrint("Array leakage! Leaked instances: %d", Array::GetGlobalInstanceCount());
			else if (!ExecutingContexts.size() && ExecutableCode::GetGlobalInstanceCount())
				DebugPrint("ExecutableCode leakage! Leaked instances: %d", ExecutableCode::GetGlobalInstanceCount());
			else if (!ExecutingContexts.size() && CSASDataElement::GetGlobalInstanceCount())
				DebugPrint("CSASDataElement leakage! Leaked instances: %d", CSASDataElement::GetGlobalInstanceCount());

			return Result;
		}
		else
		{
			DebugPrint("Couldn't find script at path '%s'", Path.c_str());
			return false;
		}
	}

	ScriptContext* ScriptRunner::GetExecutingContext()
	{
		if (!ExecutingContexts.size())
			return NULL;
		else
			return ExecutingContexts.top();
	}
}