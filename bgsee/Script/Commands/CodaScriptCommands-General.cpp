#include "CodaScriptCommands-General.h"
#include "Console.h"
#include "CodaUtilities.h"
#include "CodaCompiler.h"

namespace bgsee
{
	namespace script
	{
		namespace commands
		{
			namespace general
			{
				CodaScriptCommandRegistrarDef("General")

				CodaScriptCommandPrototypeDef(Return);
				CodaScriptCommandPrototypeDef(Call);
				CodaScriptCommandPrototypeDef(Break);
				CodaScriptCommandPrototypeDef(Continue);
				CodaScriptCommandPrototypeDef(GetSecondsPassed);
				CodaScriptCommandPrototypeDef(FormatNumber);
				CodaScriptCommandPrototypeDef(PrintToConsole);
				CodaScriptCommandPrototypeDef(RandomNumber);

				CodaScriptCommandParamData(FormatNumber, 3)
				{
					{ "Format String",					ICodaScriptDataStore::kDataType_String	},
					{ "Number",							ICodaScriptDataStore::kDataType_Numeric	},
					{ "Interpret As Unsigned Int",		ICodaScriptDataStore::kDataType_Numeric	}
				};

				CodaScriptCommandParamData(RandomNumber, 2)
				{
					{ "Minimum",						ICodaScriptDataStore::kDataType_Numeric	},
					{ "Maximum",						ICodaScriptDataStore::kDataType_Numeric	}
				};

				CodaScriptCommandHandler(Return)
				{
					if (ArgumentCount > 2)
						throw CodaScriptException(ByteCode->GetSource(), "Too many arguments passed to Return command");

					ExecutionAgent->GetContext()->Return(ArgumentCount ? dynamic_cast<CodaScriptBackingStore*>(Arguments) : nullptr,
														 ArgumentCount == 2);
					return true;
				}

				CodaScriptCommandHandler(Call)
				{
					if (ArgumentCount < 1)
						throw CodaScriptException(ByteCode->GetSource(), "Script name not passed to Call command");
					else if (ArgumentCount > 1 + CodaScriptCompiler::kMaxParameters)
						throw CodaScriptException(ByteCode->GetSource(), "Too many arguments passed to Call command - Maximum allowed = %d",
												  CodaScriptCompiler::kMaxParameters);

					const char* ScriptName = Arguments[0].GetString();
					CodaScriptBackingStore* ArgumentStore = dynamic_cast<CodaScriptBackingStore*>(Arguments);
					SME_ASSERT(ArgumentStore);

					script::ICodaScriptVirtualMachine::ExecuteParams Input;
					script::ICodaScriptVirtualMachine::ExecuteResult Output;

					Input.ScriptName = ScriptName;
					for (int i = 1; i < ArgumentCount; i++)
					{
						const CodaScriptBackingStore* Store = &ArgumentStore[i];
						Input.Parameters.push_back(*Store);
					}

					CODAVM->RunScript(Input, Output);

					if (Output.Success && Output.HasResult())
						*Result = *Output.Result;
					else
						*Result = 0.0;

					return true;
				}

				CodaScriptCommandHandler(Break)
				{
					ExecutionAgent->GetContext()->BreakLoop();
					return true;
				}

				CodaScriptCommandHandler(Continue)
				{
					ExecutionAgent->GetContext()->ContinueLoop();
					return true;
				}

				CodaScriptCommandHandler(GetSecondsPassed)
				{
					CodaScriptNumericDataTypeT TimePassed = ExecutionAgent->GetContext()->GetSecondsPassed();
					*Result = TimePassed;

					return true;
				}

				CodaScriptCommandHandler(FormatNumber)
				{
					CodaScriptStringParameterTypeT FormatString = nullptr;
					CodaScriptNumericDataTypeT Number = 0.0, InterpretAsUInt32 = 0.0;

					CodaScriptCommandExtractArgs(&FormatString, &Number, &InterpretAsUInt32);

					if (strlen(FormatString) == 0)
						throw CodaScriptException(ByteCode->GetSource(), "Format string was empty");

					char OutBuffer[0x50] = {0};

					if (InterpretAsUInt32)
						sprintf_s(OutBuffer, sizeof(OutBuffer), FormatString, (UInt32)Number);
					else
						sprintf_s(OutBuffer, sizeof(OutBuffer), FormatString, Number);

					*Result = OutBuffer;
					return true;
				}

				CodaScriptCommandHandler(PrintToConsole)
				{
					CodaScriptStringParameterTypeT Message = nullptr;

					CodaScriptCommandExtractArgs(&Message);

					ExecutionAgent->GetVM()->GetMessageHandler()->Log("%s", Message);
					return true;
				}

				CodaScriptCommandHandler(RandomNumber)
				{
					CodaScriptNumericDataTypeT Min = 0.0, Max = 0.0, Range = 0.0, Value = 0.0;

					CodaScriptCommandExtractArgs(&Min, &Max);

					if (Max < Min)
					{
						Range = Min - Max;
						Value = SME::MersenneTwister::genrand_real2() * Range;
						Value += Max;
					}
					else
					{
						Range = Max - Min;
						Value = SME::MersenneTwister::genrand_real2() * Range;
						Value += Min;
					}

					*Result = Value;
					return true;
				}
			}
		}
	}
}