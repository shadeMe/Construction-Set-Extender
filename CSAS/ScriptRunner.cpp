#include "ScriptRunner.h"
#include "ScriptCommands.h"
#include "resource.h"
#include "..\WindowManager.h"

namespace CSAutomationScript
{
	ElapsedTimeCounter			g_ScriptProfileCounter;

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

	void ScriptContext::RegisterGlobals()
	{
		GLOBALSCRIPTMANAGER->RegisterGlobalVariablesWithParser(&ContextParser);
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
					else if (SecondToken == "" || SecondToken.find_first_of("<>_=") != std::string::npos)
					{
						DebugPrint("Line %d: Invalid variable name", CurrentLine);
						Result = false;
					}
					else if (GLOBALSCRIPTMANAGER->FetchGlobalVariableByName(SecondToken.c_str()))
					{
						DebugPrint("Line %d: Scope conflict - Global variable '%s' exists", CurrentLine, SecondToken.c_str());
						Result = false;
					}
					else
					{
						AddVariable(SecondToken);
						if (Tokenizer.GetTokenCount() > 3 && !strcmp(Tokenizer.Tokens[2].c_str(), "="))
						{
							std::string LineBuffer(Buffer),
										InitializationValue = LineBuffer.substr(Tokenizer.Indices[3]);

							ScriptVariable* NewVar = LookupVariableByName(SecondToken);
							if (InitializationValue.find("\"") == 0 && InitializationValue.length() > 1)
							{
								InitializationValue.erase(InitializationValue.begin()), InitializationValue.erase(InitializationValue.begin() + InitializationValue.length() - 1);
								NewVar->SetValue(mup::Value(InitializationValue));
							}
							else
								NewVar->SetValue(mup::Value(atof(InitializationValue.c_str())));
						}
					}

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
						for (ScriptParser::TokenList::iterator Itr = ++(Tokenizer.Tokens.begin()); Itr != Tokenizer.Tokens.end(); Itr++)
							ParameterIDs.push_back(*Itr);

						if (PassedParameters)
						{
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
						else if (ParameterIDs.size())
						{
							DebugPrint("Line %d: Incorrect number of parameters passed to Begin block - Received %d, expected %d", CurrentLine, 0, ParameterIDs.size());
							Result = false;
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
			this->ExecutionState = kExecutionState_Default;
			this->HasReturnValue = false;

			ContextParser.EnableOptimizer(false);			// needs to be disabled as it short-circuits function calls that involve static arguments

			RegisterVariables();
			RegisterFunctions();
			RegisterGlobals();

			std::string MainBlockText = "";
			for (std::vector<std::string>::iterator Itr = MainBlockContents.begin(); Itr != MainBlockContents.end(); Itr++)
				MainBlockText += *Itr + "\n";

			MainScriptBlock = new BeginBlock(MainBlockText, MainBlockLine, &ContextParser);
			if (!MainScriptBlock->IsVaid())
				this->Valid = false;
		}

		while (BlockStack.size())
			BlockStack.pop();

		MainBlockContents.clear();
		ParameterIDs.clear();
	}

	bool ScriptContext::Execute(ScriptRunner* Executor, mup::Value& ReturnValue, bool* HasReturnValue)
	{
		CONSOLE->Indent();

		*HasReturnValue = false;

		if (!Valid)
		{
			DebugPrint("Failed - Errors were found while parsing");
			CONSOLE->Exdent();
			return false;
		}

#if SCRIPT_PROFILING
		g_ScriptProfileCounter.Update();
#endif

		MainScriptBlock->Execute(Executor);

#if SCRIPT_PROFILING
		g_ScriptProfileCounter.Update();
		DebugPrint("Execution Time = %0.05f ms", g_ScriptProfileCounter.GetTimePassedSinceLastUpdate());
#endif

		ReturnValue = this->ReturnValue;

		if (this->HasReturnValue)
			*HasReturnValue = true;

		assert(LoopStack.size() == 0);

		CONSOLE->Exdent();
		return true;
	}

	void ScriptContext::SetReturnValue(mup::IValue& Value)
	{
		this->ReturnValue = Value;
		this->HasReturnValue = true;
	}

	void ScriptContext::SetExecutionState(UInt8 State)
	{
		this->ExecutionState = State;
	}

	void ScriptContext::PushLoop(LoopBlock* Block)
	{
		LoopStack.push(Block);
	}

	LoopBlock* ScriptContext::PopLoop()
	{
		assert(LoopStack.size());

		LoopBlock* Result = LoopStack.top();
		LoopStack.pop();
		return Result;
	}

	void ScriptContext::SetExecutingLoopState(UInt8 State)
	{
		assert(LoopStack.size());

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

	bool ScriptRunner::ExecuteContext(ScriptContext* Script, mup::Value& ReturnValue, bool* HasReturnValue)
	{
		if (ExecutingContexts.size() >= MAX_RECURSE_DEPTH)
		{
			DebugPrint("Maximum script recursion depth hit");
			return false;
		}

		ExecutingContexts.push(Script);
		bool Result = Script->Execute(this, ReturnValue, HasReturnValue);
		ExecutingContexts.pop();

		return Result;
	}

	bool ScriptRunner::RunScript(std::string& ScriptName, ParameterList* PassedParameters, mup::Value& ReturnValue, bool* HasReturnValue)
	{
		std::string Path("Data\\Scripts\\CSAS\\" + ScriptName + ".txt");
		std::fstream InputStream(Path.c_str(), std::iostream::in);

		if (!InputStream.fail())
		{
			ScriptContext* NewContext = new ScriptContext(InputStream, PassedParameters);
			bool Result = ExecuteContext(NewContext, ReturnValue, HasReturnValue);

			delete NewContext;
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

	void ScriptRunner::PerformLeakageCheck()
	{
		if (Array::GetGlobalInstanceCount())
			DebugPrint("Array leakage! Leaked instances: %d", Array::GetGlobalInstanceCount());
		else if (ExecutableCode::GetGlobalInstanceCount())
			DebugPrint("ExecutableCode leakage! Leaked instances: %d", ExecutableCode::GetGlobalInstanceCount());
		else if (CSASDataElement::GetGlobalInstanceCount())
			DebugPrint("CSASDataElement leakage! Leaked instances: %d", CSASDataElement::GetGlobalInstanceCount());
	}

	GlobalScriptManager*		GlobalScriptManager::Singleton = NULL;

	GlobalScriptManager::GlobalScriptManager() : TimeCounter()
	{
		Initialized = false;
		InExecutionLoop = false;
	}

	void GlobalScriptManager::DeinitializeScriptCache()
	{
		for (GlobalScriptCacheList::iterator Itr = GlobalScriptCache.begin(); Itr != GlobalScriptCache.end(); Itr++)
			delete *Itr;

		GlobalScriptCache.clear();
	}

	void GlobalScriptManager::InitializeScriptCache()
	{
		std::string Directory("Data\\Scripts\\CSAS\\Global Scripts\\");

		for (IDirectoryIterator Itr(Directory.c_str(), "*.txt"); !Itr.Done(); Itr.Next())
		{
			std::string FullPath = Directory + std::string(Itr.Get()->cFileName);
			std::fstream InputStream(FullPath.c_str(), std::ios::in);

			if (!InputStream.fail())
			{
				ScriptContext* NewGlobalScript = new ScriptContext(InputStream, NULL);
				if (NewGlobalScript->IsValid())
				{
					DebugPrint("Initialized Global Script '%s'", NewGlobalScript->ScriptName.c_str());
					GlobalScriptCache.push_back(NewGlobalScript);
				}
				else
				{
					DebugPrint("Couldn't initialize Global Script at %s", Itr.Get()->cFileName);
					delete NewGlobalScript;
				}
			}
		}
	}

	void GlobalScriptManager::DeinitializeGlobalTimer()
	{
		KillTimer(*g_HWND_CSParent, GLOBALSCRIPT_EXECUTION_TIMER);
	}

	void GlobalScriptManager::InitializeGlobalTimer()
	{
		SetTimer(*g_HWND_CSParent, GLOBALSCRIPT_EXECUTION_TIMER, g_INIManager->GetINIInt("GlobalScriptExecutionPeriod", "Extender::CSAS"), NULL);
	}

	GlobalScriptManager* GlobalScriptManager::GetSingleton()
	{
		if (!Singleton)
			Singleton = new GlobalScriptManager();

		return Singleton;
	}

	long double GlobalScriptManager::GetSecondsPassed() const
	{
		return TimeCounter.GetTimePassedSinceLastUpdate() / 1000.0;
	}

	void GlobalScriptManager::Initialize()
	{
		if (Initialized)
			return;

		InitializeGlobalVariableCache(g_INIPath.c_str());
		InitializeScriptCache();
		InitializeGlobalTimer();
		TimeCounter.Update();

		Initialized = true;
	}

	void GlobalScriptManager::Deinitialize()
	{
		DeinitializeGlobalTimer();
		DeinitializeScriptCache();
		DeinitializeGlobalVariableCache(g_INIPath.c_str());

		Initialized = false;
	}

	void GlobalScriptManager::ExecuteScripts()
	{
		assert(!SCRIPTRUNNER->GetExecutingContext());

		InExecutionLoop = true;

		bool HasReturnedValue = false;

		for (GlobalScriptCacheList::iterator Itr = GlobalScriptCache.begin(); Itr != GlobalScriptCache.end();)
		{
			ScriptContext* GlobalScript = *Itr;
			GlobalScript->SetExecutionState(ScriptContext::kExecutionState_Default);

			SCRIPTRUNNER->ExecuteContext(GlobalScript, ReturnBuffer, &HasReturnedValue);

			if (GlobalScript->GetExecutionState() == ScriptContext::kExecutionState_Terminate)
			{
				DebugPrint("Fatal error encounted while executing global script '%s' - Removing it from the queue", GlobalScript->ScriptName.c_str());

				delete *Itr;
				Itr = GlobalScriptCache.erase(Itr);

				continue;
			}

			Itr++;
		}

		TimeCounter.Update();

		InExecutionLoop = false;
	}

	void GlobalScriptManager::ReloadScripts()
	{
		DeinitializeGlobalTimer();
		DeinitializeScriptCache();

		FillGlobalVariableCacheFromBuffer();
		InitializeScriptCache();
		InitializeGlobalTimer();
		TimeCounter.Update();

		MessageBox(*g_HWND_CSParent, "Global Scripts Reinitialized", "CSE", MB_OK|MB_ICONINFORMATION);
	}

	void GlobalScriptManager::InitializeGlobalVariableCache(const char* INIPath)
	{
		char SectionBuffer[0x8000] = {0};

		ClearGlobalVariables(true);

		GetPrivateProfileSection(INICSASGlobalsSection, SectionBuffer, sizeof(SectionBuffer), INIPath);

		for (const char* Itr = SectionBuffer; *Itr != '\0'; Itr += strlen(Itr) + 1)
		{
			std::string StrBuffer(Itr);
			size_t IndexA = StrBuffer.find_first_of("="), IndexB = StrBuffer.find_first_of("|");
			if (IndexA != std::string::npos && IndexB != std::string::npos)
			{
				std::string Name(StrBuffer.substr(0, IndexA)),
							Type(StrBuffer.substr(IndexA + 1, IndexB - IndexA - 1)),
							Value(StrBuffer.substr(IndexB + 1));

				if (Type == "s" || Type == "S")
					AddGlobalVariable(Name.c_str(), Value.c_str());
				else
					AddGlobalVariable(Name.c_str(), atof(Value.c_str()));

				DebugPrint("Global Variable: %s = %s", Name.c_str(), Value.c_str());
			}
		}

		FillGlobalVariableCacheFromBuffer();
	}

	void GlobalScriptManager::DeinitializeGlobalVariableCache(const char* INIPath)
	{
		WritePrivateProfileSection(INICSASGlobalsSection, NULL, INIPath);
		for (VariableList::iterator Itr = GlobalVariableBuffer.begin(); Itr != GlobalVariableBuffer.end(); Itr++)
		{
			switch ((*Itr)->GetDataType())
			{
			case CSASDataElement::kParamType_Reference:
			case CSASDataElement::kParamType_Numeric:
				WritePrivateProfileString(INICSASGlobalsSection,
										(*Itr)->GetName(),
										(std::string("n|" + std::string(PrintToBuffer("%0.6f", (*Itr)->GetValue().GetFloat() * 1.0)))).c_str(),
										INIPath);
				break;
			case CSASDataElement::kParamType_String:
				WritePrivateProfileString(INICSASGlobalsSection,
										(*Itr)->GetName(),
										(std::string("s|" + (*Itr)->GetValue().GetString())).c_str(),
										INIPath);
				break;
			default:
				assert(0);
				break;
			}
		}

		ClearGlobalVariables(true);
		DebugPrint("Saved Global Variable List to INI");
	}

	void GlobalScriptManager::PopulateListBoxWithGlobalVariables(HWND ListBox)
	{
		char Buffer[0x100] = {0};

		for (VariableList::const_iterator Itr = GlobalVariableBuffer.begin(); Itr != GlobalVariableBuffer.end(); Itr++)
		{
			sprintf_s(Buffer, sizeof(Buffer), "%s [%s]", (*Itr)->GetName(),
					(((*Itr)->GetDataType() <  CSAutomationScript::CSASDataElement::kParamType_String)?"i":"s"));
			int Index = SendMessage(ListBox, LB_INSERTSTRING, -1, (LPARAM)Buffer);
			SendMessage(ListBox, LB_SETITEMDATA, Index, (LPARAM)*Itr);
		}
	}

	void GlobalScriptManager::AddGlobalVariable(ScriptVariable* Data)
	{
		GlobalVariableBuffer.push_back(Data);
	}

	ScriptVariable* GlobalScriptManager::AddGlobalVariable(const char* Name, const char* Value)
	{
		if (GetGlobalVariableExists(Name) != GlobalVariableBuffer.end())
			return NULL;

		ScriptVariable* NewGlobal = new ScriptVariable(std::string(Name), std::string(Value));
		GlobalVariableBuffer.push_back(NewGlobal);
		return NewGlobal;
	}

	ScriptVariable* GlobalScriptManager::AddGlobalVariable(const char* Name, double Value)
	{
		if (GetGlobalVariableExists(Name) != GlobalVariableBuffer.end())
			return NULL;

		ScriptVariable* NewGlobal = new ScriptVariable(std::string(Name), Value);
		GlobalVariableBuffer.push_back(NewGlobal);
		return NewGlobal;
	}

	void GlobalScriptManager::RemoveGlobalVariable(const char* Name, bool ReleaseVariable)
	{
		VariableList::iterator Match = GetGlobalVariableExists(Name);
		if (Match == GlobalVariableBuffer.end())
			return;
		else
		{
			if (ReleaseVariable)
				delete *Match;

			GlobalVariableBuffer.erase(Match);
		}
	}

	void GlobalScriptManager::ClearGlobalVariables(bool ReleaseVariables)
	{
		if (ReleaseVariables)
		{
			for (VariableList::iterator Itr = GlobalVariableBuffer.begin(); Itr != GlobalVariableBuffer.end(); Itr++)
					delete *Itr;

			for (VariableList::iterator Itr = GlobalVariableCache.begin(); Itr != GlobalVariableCache.end(); Itr++)
					delete *Itr;
		}

		GlobalVariableBuffer.clear();
		GlobalVariableCache.clear();
	}

	VariableList::iterator GlobalScriptManager::GetGlobalVariableExists(const char* Name)
	{
		for (VariableList::iterator Itr = GlobalVariableBuffer.begin(); Itr != GlobalVariableBuffer.end(); Itr++)
		{
			if (!_stricmp((*Itr)->GetName(), Name))
				return Itr;
		}

		return GlobalVariableBuffer.end();
	}

	ScriptVariable* GlobalScriptManager::FetchGlobalVariableByName(const char* Name)
	{
		for (VariableList::iterator Itr = GlobalVariableBuffer.begin(); Itr != GlobalVariableBuffer.end(); Itr++)
		{
			if (!_stricmp((*Itr)->GetName(), Name))
				return *Itr;
		}

		return NULL;
	}

	void GlobalScriptManager::FillGlobalVariableCacheFromBuffer()
	{
		for (VariableList::iterator Itr = GlobalVariableCache.begin(); Itr != GlobalVariableCache.end(); Itr++)
			delete *Itr;
		GlobalVariableCache.clear();

		for (VariableList::iterator Itr = GlobalVariableBuffer.begin(); Itr != GlobalVariableBuffer.end(); Itr++)
		{
			GlobalVariableCache.push_back(new ScriptVariable(*(*Itr)));
		}
	}

	void GlobalScriptManager::ShowGlobalVariableDialog()
	{
		DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_CSASGLOBALS), *g_HWND_CSParent, CSASGlobalsDlgProc);
	}

	void GlobalScriptManager::RegisterGlobalVariablesWithParser(mup::ParserX* Parser)
	{
		for (VariableList::iterator Itr = GlobalVariableCache.begin(); Itr != GlobalVariableCache.end(); Itr++)
		{
			Parser->DefineVar((*Itr)->GetName(), mup::Variable(&((mup::Value&)(*Itr)->GetValue())));
		}
	}

	void InitializeCSASEngine()
	{
		DebugPrint("Initializing Command Table");
		g_CSASCommandTable.InitializeCommandTable();

		HMENU CSASMenu = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU7); CSASMenu = GetSubMenu(CSASMenu, 0);
		HMENU MainMenu = GetMenu(*g_HWND_CSParent);

		InsertMenu(MainMenu, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)CSASMenu, "CSAS");
		DrawMenuBar(*g_HWND_CSParent);

		DebugPrint("Initializing Global Manager");
		CONSOLE->Indent();
		GLOBALSCRIPTMANAGER->Initialize();
		CONSOLE->Exdent();
	}

	void DeitializeCSASEngine()
	{
		GLOBALSCRIPTMANAGER->Deinitialize();
	}
}