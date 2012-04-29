#include "CodaMUPScriptCommand.h"
#include "CodaMUPExpressionParser.h"
#include "CodaInterpreter.h"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		namespace mup
		{
			CodaScriptMUPScriptCommand::CodaScriptMUPScriptCommand( ICodaScriptCommand* Source, bool UseAlias ) :
				ICodaScriptObject(),
				ICallback(cmFUNC,
						(UseAlias && Source->GetAlias() ? Source->GetAlias() : Source->GetName()),
						Source->GetParameterData(NULL, NULL, NULL)),
				Parent(Source)
			{
				;//
			}

			inline CodaScriptMUPScriptCommand::~CodaScriptMUPScriptCommand()
			{
				Parent = NULL;
			}

			void CodaScriptMUPScriptCommand::Eval( ptr_val_type& ret, const ptr_val_type *arg, int argc )
			{
				ICodaScriptSyntaxTreeEvaluator* ExecutionAgent = GetParent()->GetEvaluationAgent();
				ICodaScriptExpressionByteCode* ByteCode = GetParent()->GetByteCode();

				SME_ASSERT(ExecutionAgent && ByteCode);

				CodaScriptScopedHandleDataStoreArrayT WrappedArgs;
				ICodaScriptCommand::ParameterInfo* ParamArray = NULL;
				UInt8 ReturnType = ICodaScriptDataStore::kDataType_Invalid;
				int ParamCount = Parent->GetParameterData(&ParamCount, &ParamArray, &ReturnType);

				if (argc)
				{
					WrappedArgs = CodaScriptScopedHandleDataStoreArrayT(new CodaScriptBackingStore[argc]);

					if (ParamCount != -1 && ParamArray == NULL)
						throw CodaScriptException(ByteCode->GetSource(),
												"Non-variadic command '%s' has no parameter data",
												Parent->GetName());

					for (int i = 0; i < argc; i++)
					{
						CodaScriptBackingStore* CurrentArg = arg[i].Get()->GetStore();
						SME_ASSERT(CurrentArg);

						if (ParamCount != -1)
						{
							ICodaScriptCommand::ParameterInfo* CurrentParam = &ParamArray[i];

							if (CurrentArg->GetType() != CurrentParam->Type &&
								CurrentParam->Type != ICodaScriptCommand::ParameterInfo::kType_Multi)
							{
									throw CodaScriptException(ByteCode->GetSource(),
															"Parameter %d type mismatch for command '%s' - Expected '%c', received '%c'",
															i,
															Parent->GetName(),
															CurrentParam->Type,
															CurrentArg->GetType());
							}

							SME_ASSERT(CurrentParam->Type != ICodaScriptDataStore::kDataType_Invalid &&
									CurrentArg->GetType() != ICodaScriptDataStore::kDataType_Invalid);
						}

						WrappedArgs[i] = *CurrentArg;
					}
				}

				CodaScriptBackingStore ResultStore;
				CodaScriptCommandHandlerUtilities HandlerHelper;

				*ret = 0.0;
				bool ExecuteResult = Parent->Execute(WrappedArgs.get(),
													&ResultStore,
													ParamArray,
													argc,
													&HandlerHelper,
													ExecutionAgent,
													ByteCode);

				if (ExecuteResult)
				{
					if (ResultStore.GetType() != ReturnType &&
						ReturnType != ICodaScriptCommand::ParameterInfo::kType_Multi)
					{
						throw CodaScriptException(ByteCode->GetSource(),
							"Return type mismatch for command '%s' - Expected '%c', received '%c'",
							Parent->GetName(),
							ReturnType,
							ResultStore.GetType());
					}

					switch (ResultStore.GetType())
					{
					case ICodaScriptDataStore::kDataType_Numeric:
						*ret = ResultStore.GetNumber();
						break;
					case ICodaScriptDataStore::kDataType_String:
						*ret = ResultStore.GetString();
						break;
					case ICodaScriptDataStore::kDataType_Reference:
						*ret = (int_type)ResultStore.GetFormID();
						break;
					case ICodaScriptDataStore::kDataType_Array:
						*ret = CodaScriptMUPValue(ResultStore.GetArray());
						break;
					case ICodaScriptCommand::ParameterInfo::kType_Multi:
						*ret = CodaScriptMUPValue(ResultStore);
						break;
					default:
						break;
					}
				}
			}

			const char_type* CodaScriptMUPScriptCommand::GetDesc() const
			{
				return Parent->GetDescription();
			}

			IToken* CodaScriptMUPScriptCommand::Clone() const
			{
				return new CodaScriptMUPScriptCommand(*this);
			}
		}
	}
}