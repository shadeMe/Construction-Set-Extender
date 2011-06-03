#pragma once

#include "ScriptVariables.h"
#include "ScriptParser.h"
#include "ScriptData.h"

#include <stack>

namespace CSAutomationScript
{
	class ScriptRunner;
	class CallCommand;

	typedef std::vector<mup::Value>				ParameterList;
	typedef std::vector<ScriptVariable*>		VariableList;
	typedef std::stack<CmdToken>				ScriptBlockStack;
	typedef std::stack<LoopBlock*>				ExecutingLoopStack;

	class ScriptContext
	{
		mup::ParserX							ContextParser;
		VariableList							Variables;
		mup::Value								ReturnValue;
		ExecutingLoopStack						LoopStack;
		BeginBlock*								MainScriptBlock;
		std::string								ScriptName;

		bool									Valid;
		bool									HasReturnValue;
		bool									ExecutionState;			// set by the return, break and continue commands
																		// true to continue execution, false to stop

		void									Release();
		void									AddVariable(std::string& Name);
		bool									SetParameter(std::string& VariableName, mup::IValue& Value);
		void									RegisterVariables();
		void									RegisterFunctions();
		bool									LookupBlockInStack(ScriptBlockStack& BlockStack, CmdToken Block);
	public:
		ScriptContext(std::fstream& InputStream, ParameterList* PassedParameters);
		~ScriptContext()
		{
			Release();
		}
		
		bool									Execute(ScriptRunner* Executor, mup::Value& ReturnValue, bool* HasReturnValue);

		void									SetReturnValue(mup::IValue& Value);
		void									SetExecutionState(bool State);

		bool									GetExecutionState() const { return ExecutionState; }
		ScriptVariable*							LookupVariableByName(std::string& Name);
		ScriptVariable*							LookupVariableByName(const char* Name);

		void									PushLoop(LoopBlock* Block);
		LoopBlock*								PopLoop();
		void									SetExecutingLoopState(UInt8 State);
	};

	#define MAX_RECURSE_DEPTH					30
	#define MAX_BEGIN_BLOCK_PARAMS				10

	class ScriptRunner
	{
		static ScriptRunner*					Singleton;

		ScriptRunner();

		std::stack<ScriptContext*>				ExecutingContexts;
	public:
		static ScriptRunner*					GetSingleton();

		bool									RunScript(std::string& ScriptName, ParameterList* PassedParameters, mup::Value& ReturnValue, bool* HasReturnValue);
		ScriptContext*							GetExecutingContext();
	};

	#define SCRIPTRUNNER						CSAutomationScript::ScriptRunner::GetSingleton()
}