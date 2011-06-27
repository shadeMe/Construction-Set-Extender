#pragma once

#include "ScriptVariables.h"
#include "ScriptParser.h"
#include "ScriptData.h"
#include "..\ElapsedTimeCounter.h"

namespace CSAutomationScript
{
	class ScriptRunner;
	class CallCommand;
	class ScriptContext;
	class GlobalScriptManager;

	typedef std::vector<mup::Value>				ParameterList;
	typedef std::vector<ScriptVariable*>		VariableList;
	typedef std::stack<CmdToken>				ScriptBlockStack;
	typedef std::stack<LoopBlock*>				ExecutingLoopStack;
	typedef std::vector<ScriptContext*>			GlobalScriptCacheList;

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
		UInt8									ExecutionState;

		void									Release();
		void									AddVariable(std::string& Name);
		bool									SetParameter(std::string& VariableName, mup::IValue& Value);
		void									RegisterVariables();
		void									RegisterFunctions();
		void									RegisterGlobals();
		bool									LookupBlockInStack(ScriptBlockStack& BlockStack, CmdToken Block);
	public:
		ScriptContext(std::fstream& InputStream, ParameterList* PassedParameters);
		~ScriptContext()
		{
			Release();
		}

		friend class GlobalScriptManager;
		friend class ScriptRunner;

		enum
		{
			kExecutionState_Default		= 0,	// normal execution
			kExecutionState_Break,				// break execution without error; set by the return, break and continue commands
			kExecutionState_Terminate			// break execution with error
		};
		
		bool									Execute(ScriptRunner* Executor, mup::Value& ReturnValue, bool* HasReturnValue);

		void									SetReturnValue(mup::IValue& Value);
		void									SetExecutionState(UInt8 State);

		UInt8									GetExecutionState() const { return ExecutionState; }
		ScriptVariable*							LookupVariableByName(std::string& Name);
		ScriptVariable*							LookupVariableByName(const char* Name);

		void									PushLoop(LoopBlock* Block);
		LoopBlock*								PopLoop();
		void									SetExecutingLoopState(UInt8 State);
		bool									GetIsLoopExecuting() const { return LoopStack.size() != 0; }
		
		bool									IsValid() const { return Valid; }
	};

	#define MAX_RECURSE_DEPTH					30
	#define MAX_BEGIN_BLOCK_PARAMS				10
	#define SCRIPT_PROFILING					0

	class ScriptRunner
	{
		static ScriptRunner*					Singleton;

		ScriptRunner();

		std::stack<ScriptContext*>				ExecutingContexts;

		bool									ExecuteContext(ScriptContext* Script, mup::Value& ReturnValue, bool* HasReturnValue);
	public:
		friend class GlobalScriptManager;
		static ScriptRunner*					GetSingleton();

		bool									RunScript(std::string& ScriptName, ParameterList* PassedParameters, mup::Value& ReturnValue, bool* HasReturnValue);
		ScriptContext*							GetExecutingContext();

		static void								PerformLeakageCheck();
	};

	#define SCRIPTRUNNER						CSAutomationScript::ScriptRunner::GetSingleton()

	class GlobalScriptManager
	{
		static GlobalScriptManager*				Singleton;

		GlobalScriptManager();

		mup::Value								ReturnBuffer;
		GlobalScriptCacheList					GlobalScriptCache;
		ElapsedTimeCounter						TimeCounter;
		VariableList							GlobalVariableCache;		// stores the variables that are bound to expression parsers
		VariableList							GlobalVariableBuffer;		// stores changes made through the dialog; all ops are performed on this container

		bool									InExecutionLoop;
		bool									Initialized;

		void									DeinitializeScriptCache();
		void									InitializeScriptCache();

		void									InitializeVariableCache(const char* INIPath);		// read from INI
		void									DeinitializeVariableCache(const char* INIPath);		// write to INI

		void									AddGlobalVariable(ScriptVariable* Data);
		void									ClearGlobalVariables(bool ReleaseVariables);
		VariableList::iterator					GetGlobalVariableExists(const char* Name);
		void									FillGlobalVariableCacheFromBuffer();
	public:
		static GlobalScriptManager*				GetSingleton();

		void									InitializeGlobalTimer();
		void									DeinitializeGlobalTimer();

		bool									GetInExecutionLoop() const { return InExecutionLoop; }
		long double								GetSecondsPassed() const;

		void									Initialize();
		void									Deinitialize();

		void									ExecuteScripts();
		void									ReloadScripts();

		void									ShowGlobalVariableDialog();

		void									PopulateListBoxWithGlobalVariables(HWND ListBox);
		ScriptVariable*							AddGlobalVariable(const char* Name, const char* Value);
		ScriptVariable*							AddGlobalVariable(const char* Name, double Value);
		void									RemoveGlobalVariable(const char* Name, bool ReleaseVariable);
		ScriptVariable*							FetchGlobalVariableByName(const char* Name);

		void									RegisterGlobalVariablesWithParser(mup::ParserX* Parser);
	};

	#define GLOBALSCRIPTMANAGER					CSAutomationScript::GlobalScriptManager::GetSingleton()
	#define GLOBALSCRIPT_EXECUTION_TIMER		0x99

	#define INICSASGlobalsSection				"Extender::CSASGlobals"

	void										InitializeCSASEngine();
	void										DeitializeCSASEngine();
}