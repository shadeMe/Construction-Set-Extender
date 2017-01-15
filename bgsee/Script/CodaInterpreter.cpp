#include "..\Main.h"
#include "..\Console.h"
#include "CodaDataTypes.h"
#include "CodaVM.h"
#include "CodaInterpreter.h"
#include "CodaUtilities.h"

namespace bgsee
{
	namespace script
	{
		ICodaScriptVirtualMachine::ICodaScriptVirtualMachine(ObjectFactoryType Type) :
			IObjectFactory(Type)
		{
			;//
		}

		CodaScriptVariable* CodaScriptExecutionContext::GetVariable(const CodaScriptSourceCodeT& Name) const
		{
			return GetVariable(Name.c_str());
		}

		CodaScriptVariable* CodaScriptExecutionContext::GetVariable(const char* Name) const
		{
			if (Variables.count(Name) == 0)
				return nullptr;
			else
				return Variables.at(Name).get();
		}

		CodaScriptExecutionContext::CodaScriptExecutionContext(ICodaScriptVirtualMachine* VM, ICodaScriptProgram* Parent) :
			Parent(Parent),
			Variables(),
			ExecutionState(kExecutionState_Default),
			PollingIntervalReminder(0.0),
			ElapsedTimeCounter(),
			Result(),
			HasReturned(false),
			ExecutingLoops()
		{
			SME_ASSERT(VM && Parent && Parent->IsValid());

			PollingIntervalReminder = Parent->GetPollingInteval();

			// initalize the variables from the parent program
			CodaScriptVariableNameArrayT VarNames;
			Parent->GetVariables(VarNames);

			for (auto& Itr : VarNames)
			{
				CodaScriptVariable::PtrT Addend(new CodaScriptVariable(Itr, VM->BuildDataStoreOwner()));
				Variables.insert(std::make_pair(Itr, std::move(Addend)));
			}
		}

		CodaScriptExecutionContext::~CodaScriptExecutionContext()
		{
			Variables.clear();
		}

		double CodaScriptExecutionContext::GetSecondsPassed()
		{
			ElapsedTimeCounter.Update();
			return ElapsedTimeCounter.GetTimePassed() / 1000.0f;
		}

		bool CodaScriptExecutionContext::HasError() const
		{
			return ExecutionState == kExecutionState_Terminate;
		}

		bool CodaScriptExecutionContext::HasEnded() const
		{
			return ExecutionState == kExecutionState_End;
		}

		bool CodaScriptExecutionContext::CanExecute() const
		{
			return ExecutionState == kExecutionState_Default;
		}

		void CodaScriptExecutionContext::FlagError()
{
			ExecutionState = kExecutionState_Terminate;
		}

		ICodaScriptProgram* CodaScriptExecutionContext::GetProgram() const
		{
			return Parent;
		}

		bool CodaScriptExecutionContext::TickPollingInterval(double TimePassed)
		{
			PollingIntervalReminder -= TimePassed;
			if (PollingIntervalReminder < 0)
			{
				PollingIntervalReminder = Parent->GetPollingInteval();
				return true;
			}
			else
				return false;
		}

		const CodaScriptBackingStore& CodaScriptExecutionContext::GetResult() const
		{
			if (HasResult() == false)
				throw CodaScriptException("Context has no result");
			else
				return Result;
		}

		bool CodaScriptExecutionContext::HasResult() const
		{
			return Result.IsValid();
		}

		void CodaScriptExecutionContext::Return(CodaScriptBackingStore* Result /*= nullptr*/, bool EOL /*= false*/)
		{
			if (HasResult())
				throw CodaScriptException("Context already has a result");
			else if (HasReturned)
				throw CodaScriptException("Context has already returned");

			if (Result)
				this->Result = *Result;

			HasReturned = true;

			if (EOL)
				// the calling script is requesting destruction
				ExecutionState = kExecutionState_End;
			else
				// break as normal, just end the current execution cycle
				ExecutionState = kExecutionState_Break;
		}

		void CodaScriptExecutionContext::SetParameters(CodaScriptBackingStore::ArrayT& Parameters)
		{
			CodaScriptVariableNameArrayT ParamNames;
			Parent->GetParameters(ParamNames);

			if (ParamNames.size() != Parameters.size())
				throw CodaScriptException("Incorrect number of parameters passed - Received %d, expected %d", Parameters.size(), ParamNames.size());

			for (int i = 0; i < ParamNames.size(); i++)
			{
				CodaScriptSourceCodeT& Name = ParamNames.at(i);
				CodaScriptVariable* ParamVar = GetVariable(Name);
				if (ParamVar == nullptr)
					throw CodaScriptException("Parameter variable '%s' not found", Name.c_str());

				*ParamVar->GetStoreOwner() = Parameters.at(i);
			}
		}

		void CodaScriptExecutionContext::BeginLoop(ICodaScriptLoopBlock* Block)
		{
			SME_ASSERT(Block);

			LoopInfo NewLoop{ Block, false };
			ExecutingLoops.push(NewLoop);
		}

		void CodaScriptExecutionContext::BreakLoop()
		{
			if (ExecutingLoops.size() == 0)
				throw CodaScriptException("Loop stack empty");

			LoopInfo& Current = ExecutingLoops.top();
			SME_ASSERT(Current.BreakExecution == false);

			Current.BreakExecution = true;
			ExecutionState = kExecutionState_Break;
		}

		void CodaScriptExecutionContext::ContinueLoop()
		{
			if (ExecutingLoops.size() == 0)
				throw CodaScriptException("Loop stack empty");

			LoopInfo& Current = ExecutingLoops.top();
			SME_ASSERT(Current.BreakExecution == false);

			ExecutionState = kExecutionState_Break;
		}

		bool CodaScriptExecutionContext::EvaluateLoop()
		{
			bool ContineLoop = true;
			if (ExecutingLoops.size() == 0)
				throw CodaScriptException("Loop stack empty");

			LoopInfo& Current = ExecutingLoops.top();
			if (HasError())
				ContineLoop = false;
			else if (HasReturned)
				ContineLoop = false;
			else if (Current.BreakExecution)
			{
				// break was called
				SME_ASSERT(ExecutionState == kExecutionState_Break);
				ContineLoop = false;
			}
			else if (ExecutionState == kExecutionState_Break)
			{
				// continue was called
				ExecutionState = kExecutionState_Default;
			}
			else
				SME_ASSERT(ExecutionState == kExecutionState_Default);

			return ContineLoop;
		}

		void CodaScriptExecutionContext::EndLoop(ICodaScriptLoopBlock* Block)
		{
			SME_ASSERT(Block);

			if (ExecutingLoops.size() == 0)
				throw CodaScriptException("Loop stack empty");

			LoopInfo& Current = ExecutingLoops.top();
			if (Current.Block != Block)
				throw CodaScriptException("Loop mismatch");

			ExecutingLoops.pop();
		}

		void CodaScriptExecutionContext::ResetState(bool ResetVars /*= false*/)
		{
			if (ExecutingLoops.size())
				throw CodaScriptException("Reseting context inside an executing loop");

			CodaScriptBackingStore Empty;
			HasReturned = false;
			Result = Empty;
			SME_ASSERT(Result.IsValid() == false);

			ExecutionState = kExecutionState_Default;
			PollingIntervalReminder = Parent->GetPollingInteval();

			if (ResetVars)
			{
				for (auto& Itr : Variables)
					*Itr.second->GetStoreOwner() = Empty;
			}
		}

		bool CodaScriptSyntaxTreeExecuteVisitor::EvaluateCondition(ICodaScriptConditionalCodeBlock* Block)
		{
			CodaScriptBackingStore Result;
			Parser->Evaluate(this, Block->GetByteCode(), &Result);

			if (Result.GetIsNumber() == false)
				throw CodaScriptException(Block, "Condition expression didn't evaluate to a boolean value");

			return Result.GetNumber();
		}

		CodaScriptSyntaxTreeExecuteVisitor::CodaScriptSyntaxTreeExecuteVisitor(ICodaScriptVirtualMachine* VM,
																			   ICodaScriptExecutionContext* Context) :
			ICodaScriptSyntaxTreeEvaluator(VM, VM->GetParser(), Context),
			CurrentCode(nullptr)
		{
			SME_ASSERT(Context->GetProgram()->GetBoundParser() == Parser);
		}


#define CODASCRIPT_EXECUTEHNDLR_PROLOG										\
			ScopedFunctor Sentinel([&Node, this](ScopedFunctor::Event e) {	\
				if (e == ScopedFunctor::Event::Construction)				\
					CurrentCode = Node;										\
				else														\
					CurrentCode = nullptr;									\
				});															\
			try																\
			{																\
				if (Context->CanExecute() == false)							\
					return;													\

#define CODASCRIPT_EXECUTEHNDLR_EPILOG										\
				return;														\
			}

#define CODASCRIPT_EXECUTEERROR_CATCHER										\
			catch (CodaScriptException& E)									\
			{																\
				VM->GetMessageHandler()->Log("Runtime Error [Script: %s] - %s", Context->GetProgram()->GetName().c_str(), E.Get());		\
			}																\
			catch (std::exception& E)										\
			{																\
				VM->GetMessageHandler()->Log("Runtime Error [Script: %s] - %s", Context->GetProgram()->GetName().c_str(), E.what());		\
			}																\
			catch (...)														\
			{																\
				VM->GetMessageHandler()->Log("Unknown Runtime Error [Script: %s]", Context->GetProgram()->GetName().c_str());				\
			}																\
			Context->FlagError();											\
			VM->GetExecutor()->PrintStackTrace();							\
			return;

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( CodaScriptExpression* Node )
		{
			CODASCRIPT_EXECUTEHNDLR_PROLOG

			Parser->Evaluate(this, Node->GetByteCode());

			CODASCRIPT_EXECUTEHNDLR_EPILOG

			CODASCRIPT_EXECUTEERROR_CATCHER
		}

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( CodaScriptBEGINBlock* Node )
		{
			;//
		}

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( CodaScriptIFBlock* Node )
		{
			CODASCRIPT_EXECUTEHNDLR_PROLOG

			if (EvaluateCondition(Node))
			{
				Node->Traverse(this);
				return;
			}
			else
			{
				for (auto Itr : Node->GetElseIfBlocks())
				{
					if (EvaluateCondition(Itr))
					{
						Itr->Accept(this);
						return;
					}
				}
			}

			if (Node->HasElseBlock())
				Node->GetElseBlock().Accept(this);

			CODASCRIPT_EXECUTEHNDLR_EPILOG

			CODASCRIPT_EXECUTEERROR_CATCHER
		}

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( CodaScriptELSEIFBlock* Node )
		{
			Node->Traverse(this);
		}

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( CodaScriptELSEBlock* Node )
		{
			;//
		}

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( CodaScriptWHILEBlock* Node )
		{
			CODASCRIPT_EXECUTEHNDLR_PROLOG

			ScopedFunctor Sentinel([Node, this](ScopedFunctor::Event e) {
				if (e == ScopedFunctor::Event::Construction)
					Context->BeginLoop(Node);
				else
					Context->EndLoop(Node);
			});
			UInt32 IterationCounter = 0;

			while (EvaluateCondition(Node))
			{
				IterationCounter++;
				if (IterationCounter >= kLoopOverrunLimit)
					throw CodaScriptException(Node, "Loop overrun - When will it ennnnnnd?!");

				Node->Traverse(this);
				if (Context->EvaluateLoop() == false)
					break;
			}

			CODASCRIPT_EXECUTEHNDLR_EPILOG

			CODASCRIPT_EXECUTEERROR_CATCHER
		}

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( CodaScriptFOREACHBlock* Node )
		{
			CODASCRIPT_EXECUTEHNDLR_PROLOG

			ScopedFunctor Sentinel([Node, this](ScopedFunctor::Event e) {
				if (e == ScopedFunctor::Event::Construction)
					Context->BeginLoop(Node);
				else
					Context->EndLoop(Node);
			});
			CodaScriptVariable* Iterator = nullptr;
			const CodaScriptSourceCodeT& IteratorName = Node->GetIteratorName();

			if (IteratorName.empty())
				throw CodaScriptException(Node, "Invalid expression - No iterator specified");
			else if ((Iterator = Context->GetVariable(IteratorName)) == nullptr)
				throw CodaScriptException(Node, "Invalid iterator '%s'", IteratorName.c_str());

			CodaScriptBackingStore ArrayResult((CodaScriptNumericDataTypeT)0),
								IteratorBuffer((CodaScriptNumericDataTypeT)0),
								IteratorContents;

			IteratorContents = *Iterator->GetStoreOwner()->GetDataStore();
			Parser->Evaluate(this, Node->GetByteCode(), &ArrayResult);

			if (ArrayResult.GetType() != ICodaScriptDataStore::kDataType_Array)
				throw CodaScriptException(Node, "Invalid expression - Non-array result");

			ICodaScriptArrayDataType::SharedPtrT ArrayInstance(ArrayResult.GetArray());

			for (int i = 0, j = ArrayInstance->Size(); i < j; i++)
			{
				if (ArrayInstance->At(i, IteratorBuffer) == false)
					throw CodaScriptException(Node, "Index operator error - I[%d] S[%d]", i, ArrayInstance->Size());

				*Iterator->GetStoreOwner() = IteratorBuffer;

				Node->Traverse(this);
				if (Context->EvaluateLoop() == false)
					break;
			}

			*Iterator->GetStoreOwner() = IteratorContents;

			CODASCRIPT_EXECUTEHNDLR_EPILOG

			CODASCRIPT_EXECUTEERROR_CATCHER
		}

		ICodaScriptExecutableCode* CodaScriptSyntaxTreeExecuteVisitor::GetCurrentCode() const
		{
			return CurrentCode;
		}

		CodaScriptCommandHandlerUtilities::CodaScriptCommandHandlerUtilities(ICodaScriptVirtualMachine* VM) :
			ICodaScriptCommandHandlerHelper(),
			VM(VM),
			AllocatedWrappers()
		{
			SME_ASSERT(VM);
		}

		CodaScriptCommandHandlerUtilities::~CodaScriptCommandHandlerUtilities()
		{
			AllocatedWrappers.clear();
		}

		CodaScriptBackingStore* CodaScriptCommandHandlerUtilities::CreateWrapper(ICodaScriptArrayDataType::SharedPtrT Array )
		{
			CodaScriptBackingStore* Result = new CodaScriptBackingStore(Array);

			ICodaScriptDataStore::PtrT Wrapper(Result);
			AllocatedWrappers.push_back(std::move(Wrapper));
			return Result;
		}

		CodaScriptBackingStore* CodaScriptCommandHandlerUtilities::CreateWrapper( CodaScriptBackingStore* Source )
		{
			CodaScriptBackingStore* Result = nullptr;
			if (Source)
				Result = new CodaScriptBackingStore(Source);
			else
				Result = new CodaScriptBackingStore(0.0);

			ICodaScriptDataStore::PtrT Wrapper(Result);
			AllocatedWrappers.push_back(std::move(Wrapper));
			return Result;
		}

		ICodaScriptDataStore* CodaScriptCommandHandlerUtilities::ArrayAllocate( UInt32 InitialSize /*= 0*/ )
		{
			ICodaScriptArrayDataType::SharedPtrT NewArray = VM->BuildArray(InitialSize);

			return CreateWrapper(NewArray);
		}

		bool CodaScriptCommandHandlerUtilities::ArrayPushback( ICodaScriptDataStore* AllocatedArray, CodaScriptNumericDataTypeT Data )
		{
			CodaScriptBackingStore* Wrapper = dynamic_cast<CodaScriptBackingStore*>(AllocatedArray);
			SME_ASSERT(Wrapper && Wrapper->GetType() == ICodaScriptDataStore::kDataType_Array);

			return Wrapper->GetArray()->Insert(Data);
		}

		bool CodaScriptCommandHandlerUtilities::ArrayPushback( ICodaScriptDataStore* AllocatedArray, CodaScriptStringParameterTypeT Data )
		{
			CodaScriptBackingStore* Wrapper = dynamic_cast<CodaScriptBackingStore*>(AllocatedArray);
			SME_ASSERT(Wrapper && Wrapper->GetType() == ICodaScriptDataStore::kDataType_Array);

			return Wrapper->GetArray()->Insert(Data);
		}

		bool CodaScriptCommandHandlerUtilities::ArrayPushback( ICodaScriptDataStore* AllocatedArray, CodaScriptReferenceDataTypeT Data )
		{
			CodaScriptBackingStore* Wrapper = dynamic_cast<CodaScriptBackingStore*>(AllocatedArray);
			SME_ASSERT(Wrapper && Wrapper->GetType() == ICodaScriptDataStore::kDataType_Array);

			return Wrapper->GetArray()->Insert(Data);
		}

		bool CodaScriptCommandHandlerUtilities::ArrayPushback( ICodaScriptDataStore* AllocatedArray, ICodaScriptDataStore* ArrayData )
		{
			CodaScriptBackingStore* Wrapper = dynamic_cast<CodaScriptBackingStore*>(AllocatedArray);
			CodaScriptBackingStore* DataWrapper = dynamic_cast<CodaScriptBackingStore*>(ArrayData);

			SME_ASSERT(Wrapper && Wrapper->GetType() == ICodaScriptDataStore::kDataType_Array);
			SME_ASSERT(DataWrapper && DataWrapper->GetType() == ICodaScriptDataStore::kDataType_Array);

			return Wrapper->GetArray()->Insert(DataWrapper->GetArray());
		}

		bool CodaScriptCommandHandlerUtilities::ArrayAt( ICodaScriptDataStore* AllocatedArray, UInt32 Index, ICodaScriptDataStore** OutBuffer )
		{
			CodaScriptBackingStore* Wrapper = dynamic_cast<CodaScriptBackingStore*>(AllocatedArray);
			SME_ASSERT(Wrapper && Wrapper->GetType() == ICodaScriptDataStore::kDataType_Array);

			SME_ASSERT(OutBuffer);
			CodaScriptBackingStore* BufferWrapper = CreateWrapper(nullptr);
			if (Wrapper->GetArray()->At(Index, *BufferWrapper))
			{
				*OutBuffer = BufferWrapper;
				return true;
			}
			else
				return false;
		}

		bool CodaScriptCommandHandlerUtilities::ArrayErase( ICodaScriptDataStore* AllocatedArray, UInt32 Index )
		{
			CodaScriptBackingStore* Wrapper = dynamic_cast<CodaScriptBackingStore*>(AllocatedArray);
			SME_ASSERT(Wrapper && Wrapper->GetType() == ICodaScriptDataStore::kDataType_Array);

			return Wrapper->GetArray()->Erase(Index);
		}

		void CodaScriptCommandHandlerUtilities::ArrayClear( ICodaScriptDataStore* AllocatedArray )
		{
			CodaScriptBackingStore* Wrapper = dynamic_cast<CodaScriptBackingStore*>(AllocatedArray);
			SME_ASSERT(Wrapper && Wrapper->GetType() == ICodaScriptDataStore::kDataType_Array);

			Wrapper->GetArray()->Clear();
		}

		UInt32 CodaScriptCommandHandlerUtilities::ArraySize( ICodaScriptDataStore* AllocatedArray )
		{
			CodaScriptBackingStore* Wrapper = dynamic_cast<CodaScriptBackingStore*>(AllocatedArray);
			SME_ASSERT(Wrapper && Wrapper->GetType() == ICodaScriptDataStore::kDataType_Array);

			return Wrapper->GetArray()->Size();
		}

		bool CodaScriptCommandHandlerUtilities::ExtractArguments(ICodaScriptDataStore* Arguments,
																 ICodaScriptCommand::ParameterInfo* ParameterData,
																 UInt32 ArgumentCount, ...)
		{
			bool Result = true;

			va_list Args;
			va_start(Args, ArgumentCount);

			// need to cast to the correct type as we're basically performing pointer arithmetic
			// ug-gah-ly but how many levels of abstraction am I gonna contend with just to provide some isolation for 3rd parties?!
			// who'll probably ne'er use this, I might add...
			CodaScriptBackingStore* ArgumentStore = dynamic_cast<CodaScriptBackingStore*>(Arguments);
			SME_ASSERT(ArgumentStore && ParameterData);

			for (int i = 0; i < ArgumentCount; i++)
			{
				ICodaScriptCommand::ParameterInfo* CurrentParam = &ParameterData[i];
				CodaScriptBackingStore* CurrentArg = &ArgumentStore[i];

				SME_ASSERT(CurrentArg->GetType() != ICodaScriptDataStore::kDataType_Invalid);
				SME_ASSERT(CurrentArg->GetType() == CurrentParam->Type ||
						   CurrentParam->Type == ICodaScriptCommand::ParameterInfo::kType_Multi ||
						   CurrentArg->GetHasImplicitCast((ICodaScriptDataStore::DataType)CurrentParam->Type));

				switch (CurrentParam->Type)
				{
				case ICodaScriptDataStore::kDataType_Numeric:
					{
						CodaScriptNumericDataTypeT* Out = va_arg(Args, CodaScriptNumericDataTypeT*);
						*Out = CurrentArg->GetNumber();
					}

					break;
				case ICodaScriptDataStore::kDataType_String:
					{
						CodaScriptStringParameterTypeT* Out = va_arg(Args, CodaScriptStringParameterTypeT*);
						*Out = CurrentArg->GetString();
					}

					break;
				case ICodaScriptDataStore::kDataType_Reference:
					{
						CodaScriptReferenceDataTypeT* Out = va_arg(Args, CodaScriptReferenceDataTypeT*);
						*Out = CurrentArg->GetFormID();
					}

					break;
				case ICodaScriptDataStore::kDataType_Array:
					{
						ICodaScriptDataStore** Out = va_arg(Args, ICodaScriptDataStore**);
						*Out = CreateWrapper(CurrentArg->GetArray());
					}

					break;
				case ICodaScriptCommand::ParameterInfo::kType_Multi:
					{
						ICodaScriptDataStore** Out = va_arg(Args, ICodaScriptDataStore**);
						*Out = CreateWrapper(CurrentArg);
					}

					break;
				default:
					Result = false;
					break;
				}
			}

			va_end(Args);

			return Result;
		}
	}
}