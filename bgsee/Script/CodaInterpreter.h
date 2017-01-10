#pragma  once
#include "CodaDataTypes.h"
#include "CodaCompiler.h"
#include "WorkspaceManager.h"

namespace bgsee
{
	namespace script
	{
		class ICodaScriptVirtualMachine : public IObjectFactory
		{
		public:
			ICodaScriptVirtualMachine(ObjectFactoryType Type);
			inline ~ICodaScriptVirtualMachine() = default;

			struct ExecuteParams
			{
				std::string						Filepath;			// path to the script (must be relative to the repository)
				ICodaScriptProgram*				Program;
				CodaScriptBackingStore::ArrayT	Parameters;
				bool							RunInBackground;
				bool							Recompile;

				ExecuteParams() : Filepath(), Program(nullptr),
					Parameters(), RunInBackground(false), Recompile(false) {}
			};

			struct ExecuteResult
			{
				ICodaScriptDataStore::PtrT		Result;
				bool							Success;			// true if the execution was succesful (no exceptions)

				ExecuteResult() : Result(), Success(false) {}
				bool HasResult() const { return Result != nullptr; }
			};

			virtual const ResourceLocation&				GetScriptRepository() const = 0;
			virtual const std::string&					GetScriptFileExtension() const = 0;

			virtual CodaScriptVariable*					GetGlobal(const char* Name) const = 0;
			virtual const CodaScriptVariable::ArrayT&	GetGlobals() const = 0;

			virtual CodaScriptMessageHandler*			GetMessageHandler() const = 0;
			virtual ICodaScriptExpressionParser*		GetParser() const = 0;
			virtual ICodaScriptExecutor*				GetExecutor() const = 0;
			virtual ICodaScriptBackgroundDaemon*		GetBackgroundDaemon() const = 0;
			virtual ICodaScriptProgramCache*			GetProgramCache() const = 0;

			virtual bool								IsProgramExecuting(ICodaScriptProgram* Program) const = 0;
			virtual void								RunScript(ExecuteParams& Input, ExecuteResult& Output) = 0;
		};

		class ICodaScriptExecutor
		{
		public:
			virtual ~ICodaScriptExecutor() = 0 {}

			virtual void		Execute(ICodaScriptExecutionContext* Context,
										ICodaScriptVirtualMachine::ExecuteResult& Out) = 0;
			virtual bool		IsBusy() const = 0;							// true if any script's currently executing
			virtual bool		IsProgramExecuting(ICodaScriptProgram* Program) const = 0;

			typedef std::unique_ptr<ICodaScriptExecutor>		PtrT;
		};

		class ICodaScriptBackgroundDaemon
		{
		public:
			virtual ~ICodaScriptBackgroundDaemon() = 0 {}

			virtual const ResourceLocation&		GetBackgroundScriptRepository() const = 0;

			virtual void						Suspend() = 0;					// suspends background execution
			virtual void						Resume() = 0;					// resumes background executinon
			virtual bool						IsEnabled() const = 0;			// true if background script execution is allowed
			virtual bool						IsBackgrounding() const = 0;	// true if background scripts are being executed

			virtual void						Rebuild() = 0;					// recompiles the stable background scripts
			virtual void						Queue(ICodaScriptExecutionContext* Context) = 0;	// queues a (regular)script for background execution, takes ownership of pointer


			typedef std::unique_ptr<ICodaScriptBackgroundDaemon>		PtrT;
		};

		class ICodaScriptProgramCache
		{
		public:
			virtual ~ICodaScriptProgramCache() = 0 {}

			virtual ICodaScriptProgram*			Get(const ResourceLocation& Filepath, bool Recompile = false) = 0;		// returns a nullptr if no valid script was found
			virtual void						Invalidate() = 0;

			typedef std::unique_ptr<ICodaScriptProgramCache>		PtrT;
		};

		class ICodaScriptExecutionContext
		{
		public:
			virtual ~ICodaScriptExecutionContext() = 0 {}

			virtual bool								HasError() const = 0;					// returns true if the script encountered an exception during runtime
			virtual bool								HasEnded() const = 0;					// returns true if the (background) script was terminated

			virtual bool								CanExecute() const = 0;					// returns false if execution is disabled, true otherwise
			virtual void								FlagError() = 0;

			virtual ICodaScriptProgram*					GetProgram() const = 0;

			virtual double								GetSecondsPassed() = 0;							// since the last call
			virtual bool								TickPollingInterval(double TimePassed) = 0;		// returns true and resets the polling timer if it elapsed

			virtual CodaScriptVariable*					GetVariable(const CodaScriptSourceCodeT& Name) const = 0;
			virtual CodaScriptVariable*					GetVariable(const char* Name) const = 0;

			virtual const CodaScriptBackingStore&		GetResult() const = 0;
			virtual bool								HasResult() const = 0;

			virtual void								Return(CodaScriptBackingStore* Result = nullptr, bool EOL = false) = 0;		// breaks execution

			virtual void								SetParameters(CodaScriptBackingStore::ArrayT& Parameters) = 0;

			virtual void								BeginLoop(ICodaScriptLoopBlock* Block) = 0;
			virtual void								BreakLoop() = 0;		// flags the currently executing loop to break
			virtual void								ContinueLoop() = 0;		// flags the currently executing loop to skip the current iteration
			virtual bool								EvaluateLoop() = 0;		// returns false if the currently executing loop needs to end immediately
			virtual void								EndLoop(ICodaScriptLoopBlock* Block) = 0;

			virtual void								ResetState(bool ResetVars = false) = 0;		// resets the context's mutable state

			typedef std::unique_ptr<ICodaScriptExecutionContext>	PtrT;
			typedef std::stack<ICodaScriptExecutionContext*>		StackT;
		};

		class CodaScriptExecutionContext : public ICodaScriptExecutionContext
		{
		protected:
			enum
			{
				kExecutionState_Default = 0,				// normal execution
				kExecutionState_Break,						// break execution without error; set by the return, break and continue commands
				kExecutionState_Terminate,					// break execution with error
				kExecutionState_End							// break execution without error; special case, signifies EOL
			};

			struct LoopInfo
			{
				ICodaScriptLoopBlock*			Block;
				bool							BreakExecution;
			};

			typedef std::stack<LoopInfo>		LoopStackT;
			typedef std::unordered_map<CodaScriptSourceCodeT, CodaScriptVariable::PtrT>		VarMapT;		// key = name, value = variable

			ICodaScriptProgram*					Parent;
			VarMapT								Variables;
			UInt8								ExecutionState;
			double								PollingIntervalReminder;
			CodaScriptElapsedTimeCounterT		ElapsedTimeCounter;
			CodaScriptBackingStore				Result;
			bool								HasReturned;
			LoopStackT							ExecutingLoops;
		public:
			CodaScriptExecutionContext(ICodaScriptVirtualMachine* VM, ICodaScriptProgram* Parent);
			virtual ~CodaScriptExecutionContext();

			virtual bool								HasError() const override;
			virtual bool								HasEnded() const override;

			virtual bool								CanExecute() const override;
			virtual void								FlagError() override;

			virtual ICodaScriptProgram*					GetProgram() const override;

			virtual double								GetSecondsPassed() override;
			virtual bool								TickPollingInterval(double TimePassed) override;

			virtual const CodaScriptBackingStore&		GetResult() const override;
			virtual bool								HasResult() const override;

			virtual void								Return(CodaScriptBackingStore* Result = nullptr, bool EOL = false) override;

			virtual CodaScriptVariable*					GetVariable(const CodaScriptSourceCodeT& Name) const override;
			virtual CodaScriptVariable*					GetVariable(const char* Name) const override;

			virtual void								SetParameters(CodaScriptBackingStore::ArrayT& Parameters) override;

			virtual void								BeginLoop(ICodaScriptLoopBlock* Block) override;
			virtual void								BreakLoop() override;
			virtual void								ContinueLoop() override;
			virtual bool								EvaluateLoop() override;
			virtual void								EndLoop(ICodaScriptLoopBlock* Block) override;

			virtual void								ResetState(bool ResetVars = false) override;
		};

		class CodaScriptSyntaxTreeExecuteVisitor : public ICodaScriptSyntaxTreeEvaluator
		{
			static const UInt32					kLoopOverrunLimit = 0xFFFFFF;

			bool								EvaluateCondition(ICodaScriptConditionalCodeBlock* Block);
		public:
			CodaScriptSyntaxTreeExecuteVisitor(ICodaScriptVirtualMachine* VM, ICodaScriptExecutionContext* Context);
			inline virtual ~CodaScriptSyntaxTreeExecuteVisitor() = default;

			virtual void									Visit(CodaScriptExpression* Node) override;
			virtual void									Visit(CodaScriptBEGINBlock* Node) override;
			virtual void									Visit(CodaScriptIFBlock* Node) override;
			virtual void									Visit(CodaScriptELSEIFBlock* Node) override;
			virtual void									Visit(CodaScriptELSEBlock* Node) override;
			virtual void									Visit(CodaScriptWHILEBlock* Node) override;
			virtual void									Visit(CodaScriptFOREACHBlock* Node) override;
		};


		class CodaScriptCommandHandlerUtilities : public ICodaScriptCommandHandlerHelper
		{
		protected:
			typedef std::vector<ICodaScriptDataStore::PtrT>		DataStoreAllocationArrayT;

			ICodaScriptVirtualMachine*							VM;
			DataStoreAllocationArrayT							AllocatedWrappers;

			CodaScriptBackingStore*								CreateWrapper(ICodaScriptArrayDataType::SharedPtrT Array);
			CodaScriptBackingStore*								CreateWrapper(CodaScriptBackingStore* Source);
		public:
			CodaScriptCommandHandlerUtilities(ICodaScriptVirtualMachine* VM);
			virtual ~CodaScriptCommandHandlerUtilities();

			virtual ICodaScriptDataStore*						ArrayAllocate(UInt32 InitialSize = 0) override;
			virtual bool										ArrayPushback(ICodaScriptDataStore* AllocatedArray, CodaScriptNumericDataTypeT Data) override;
			virtual bool										ArrayPushback(ICodaScriptDataStore* AllocatedArray, CodaScriptStringParameterTypeT Data) override;
			virtual	bool										ArrayPushback(ICodaScriptDataStore* AllocatedArray, CodaScriptReferenceDataTypeT Data) override;
			virtual bool										ArrayPushback(ICodaScriptDataStore* AllocatedArray, ICodaScriptDataStore* ArrayData) override;
			virtual bool										ArrayAt(ICodaScriptDataStore* AllocatedArray, UInt32 Index, ICodaScriptDataStore** OutBuffer) override;
			virtual bool										ArrayErase(ICodaScriptDataStore* AllocatedArray, UInt32 Index) override;
			virtual void										ArrayClear(ICodaScriptDataStore* AllocatedArray) override;
			virtual UInt32										ArraySize(ICodaScriptDataStore* AllocatedArray) override;

			virtual bool										ExtractArguments(ICodaScriptDataStore* Arguments,
																				ICodaScriptCommand::ParameterInfo* ParameterData,
																				UInt32 ArgumentCount, ...) override;
		};
	}
}