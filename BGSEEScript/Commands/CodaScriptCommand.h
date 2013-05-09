#pragma once
#include "CodaVM.h"
#include "CodaInterpreter.h"

#define CodaScriptCommandRegistrarDecl						CodaScriptCommandRegistrar* GetRegistrar(void)

#define CodaScriptCommandRegistrarDef(Category)																		\
		CodaScriptCommandRegistrar* GetRegistrar(void)																\
		{																											\
			static CodaScriptCommandRegistrar Instance(Category);													\
			return &Instance;																						\
		}

#define CodaScriptCommandPrototype(Name, Alias, Description, Documentation, ParameterCount, ResultType)				\
		class CodaScriptCommand##Name : public ICodaScriptCommand													\
		{																											\
		protected:																									\
			static ParameterInfo							kParameterInfoArray[##ParameterCount];					\
		public:																										\
			CodaScriptCommand##Name() :																				\
				ICodaScriptCommand()																				\
			{																										\
				GetRegistrar()->Add(this);																			\
			}																										\
			virtual ~CodaScriptCommand##Name()																		\
			{																										\
				;																									\
			}																										\
			virtual const char*					GetName(void)														\
			{																										\
				return #Name;																						\
			}																										\
			virtual const char*					GetAlias(void)														\
			{																										\
				return Alias;																						\
			}																										\
			virtual const char*					GetDescription(void)												\
			{																										\
				return Description;																					\
			}																										\
			virtual const char*					GetDocumentation(void)												\
			{																										\
				return Documentation;																				\
			}																										\
			virtual int							GetParameterData(int* OutParameterCount = NULL,						\
																ParameterInfo** OutParameterInfoArray = NULL,		\
																UInt8* OutResultType = NULL)						\
			{																										\
				if (OutParameterCount)																				\
					*OutParameterCount = ParameterCount;															\
				if (OutParameterInfoArray)																			\
					*OutParameterInfoArray = kParameterInfoArray;													\
				if (OutResultType)																					\
					*OutResultType = ResultType;																	\
				return ParameterCount;																				\
			}																										\
			virtual bool						Execute(ICodaScriptDataStore* Arguments,							\
														ICodaScriptDataStore* Result,								\
														ParameterInfo* ParameterData,								\
														int ArgumentCount,											\
														ICodaScriptCommandHandlerHelper* Utilities,					\
														ICodaScriptSyntaxTreeEvaluator* ExecutionAgent,				\
														ICodaScriptExpressionByteCode* ByteCode);					\
		};																											\
		extern CodaScriptCommand##Name					kCommand##Name##Prototype

#define CodaScriptCommandParamData(Name, ParameterCount)															\
		ICodaScriptCommand::ParameterInfo	CodaScriptCommand##Name##::kParameterInfoArray[##ParameterCount] =

#define CodaScriptParametricCommandPrototype(Name, Alias, Description, Documentation, ParameterCount, ParameterArray, ResultType)				\
		class CodaScriptCommand##Name : public ICodaScriptCommand													\
		{																											\
		public:																										\
			CodaScriptCommand##Name() :																				\
				ICodaScriptCommand()																				\
			{																										\
				GetRegistrar()->Add(this);																			\
			}																										\
			virtual ~CodaScriptCommand##Name()																		\
			{																										\
				;																									\
			}																										\
			virtual const char*					GetName(void)														\
			{																										\
				return #Name;																						\
			}																										\
			virtual const char*					GetAlias(void)														\
			{																										\
				return Alias;																						\
			}																										\
			virtual const char*					GetDescription(void)												\
			{																										\
				return Description;																					\
			}																										\
			virtual const char*					GetDocumentation(void)												\
			{																										\
				return Documentation;																				\
			}																										\
			virtual int							GetParameterData(int* OutParameterCount = NULL,						\
																ParameterInfo** OutParameterInfoArray = NULL,		\
																UInt8* OutResultType = NULL)						\
			{																										\
				if (OutParameterCount)																				\
					*OutParameterCount = ParameterCount;															\
				if (OutParameterInfoArray)																			\
					*OutParameterInfoArray = kCodaScriptCommandCommonParameterInfoArray##ParameterArray;			\
				if (OutResultType)																					\
					*OutResultType = ResultType;																	\
				return ParameterCount;																				\
			}																										\
			virtual bool						Execute(ICodaScriptDataStore* Arguments,							\
														ICodaScriptDataStore* Result,								\
														ParameterInfo* ParameterData,								\
														int ArgumentCount,											\
														ICodaScriptCommandHandlerHelper* Utilities,					\
														ICodaScriptSyntaxTreeEvaluator* ExecutionAgent,				\
														ICodaScriptExpressionByteCode* ByteCode);					\
		};																											\
		extern CodaScriptCommand##Name					kCommand##Name##Prototype

#define CodaScriptCommandCommonParamDef(Name, ParameterCount)														\
		ICodaScriptCommand::ParameterInfo	kCodaScriptCommandCommonParameterInfoArray##Name##[##ParameterCount] =

#define CodaScriptCommandCommonParamDecl(Name)																		\
		extern ICodaScriptCommand::ParameterInfo	kCodaScriptCommandCommonParameterInfoArray##Name##[]

#define CodaScriptVariadicCommandPrototype(Name, Alias, Description, Documentation, ResultType)						\
		class CodaScriptCommand##Name : public ICodaScriptCommand													\
		{																											\
		public:																										\
			CodaScriptCommand##Name() :																				\
				ICodaScriptCommand()																				\
			{																										\
				GetRegistrar()->Add(this);																			\
			}																										\
			virtual ~CodaScriptCommand##Name()																		\
			{																										\
				;																									\
			}																										\
			virtual const char*					GetName(void)														\
			{																										\
				return #Name;																						\
			}																										\
			virtual const char*					GetAlias(void)														\
			{																										\
				return Alias;																						\
			}																										\
			virtual const char*					GetDescription(void)												\
			{																										\
				return Description;																					\
			}																										\
			virtual const char*					GetDocumentation(void)												\
			{																										\
				return Documentation;																				\
			}																										\
			virtual int							GetParameterData(int* OutParameterCount = NULL,						\
																ParameterInfo** OutParameterInfoArray = NULL,		\
																UInt8* OutResultType = NULL)						\
			{																										\
				if (OutParameterCount)																				\
					*OutParameterCount = -1;																		\
				if (OutParameterInfoArray)																			\
					*OutParameterInfoArray = 0;																		\
				if (OutResultType)																					\
					*OutResultType = ResultType;																	\
				return -1;																							\
			}																										\
			virtual bool						Execute(ICodaScriptDataStore* Arguments,							\
														ICodaScriptDataStore* Result,								\
														ParameterInfo* ParameterData,								\
														int ArgumentCount,											\
														ICodaScriptCommandHandlerHelper* Utilities,					\
														ICodaScriptSyntaxTreeEvaluator* ExecutionAgent,				\
														ICodaScriptExpressionByteCode* ByteCode);					\
		};																											\
		extern CodaScriptCommand##Name					kCommand##Name##Prototype

#define CodaScriptSimpleCommandPrototype(Name, Alias, Description, Documentation, ResultType)						\
		class CodaScriptCommand##Name : public ICodaScriptCommand													\
		{																											\
		public:																										\
			CodaScriptCommand##Name() :																				\
				ICodaScriptCommand()																				\
			{																										\
				GetRegistrar()->Add(this);																			\
			}																										\
			virtual ~CodaScriptCommand##Name()																		\
			{																										\
				;																									\
			}																										\
			virtual const char*					GetName(void)														\
			{																										\
				return #Name;																						\
			}																										\
			virtual const char*					GetAlias(void)														\
			{																										\
				return Alias;																						\
			}																										\
			virtual const char*					GetDescription(void)												\
			{																										\
				return Description;																					\
			}																										\
			virtual const char*					GetDocumentation(void)												\
			{																										\
				return Documentation;																				\
			}																										\
			virtual int							GetParameterData(int* OutParameterCount = NULL,						\
																ParameterInfo** OutParameterInfoArray = NULL,		\
																UInt8* OutResultType = NULL)						\
			{																										\
				if (OutParameterCount)																				\
					*OutParameterCount = 0;																			\
				if (OutParameterInfoArray)																			\
					*OutParameterInfoArray = 0;																		\
				if (OutResultType)																					\
					*OutResultType = ResultType;																	\
				return 0;																							\
			}																										\
			virtual bool						Execute(ICodaScriptDataStore* Arguments,							\
														ICodaScriptDataStore* Result,								\
														ParameterInfo* ParameterData,								\
														int ArgumentCount,											\
														ICodaScriptCommandHandlerHelper* Utilities,					\
														ICodaScriptSyntaxTreeEvaluator* ExecutionAgent,				\
														ICodaScriptExpressionByteCode* ByteCode);					\
		};																											\
		extern CodaScriptCommand##Name					kCommand##Name##Prototype

#define CodaScriptCommandPrototypeDef(Name)			CodaScriptCommand##Name		kCommand##Name##Prototype

#define CodaScriptCommandHandler(Name)																				\
		bool CodaScriptCommand##Name##::Execute(ICodaScriptDataStore* Arguments,									\
												ICodaScriptDataStore* Result,										\
												ParameterInfo* ParameterData,										\
												int ArgumentCount,													\
												ICodaScriptCommandHandlerHelper* Utilities,							\
												ICodaScriptSyntaxTreeEvaluator* ExecutionAgent,						\
												ICodaScriptExpressionByteCode* ByteCode)

#define CodaScriptCommandExtractArgs(...)																			\
	if (Utilities->ExtractArguments(Arguments, ParameterData, ArgumentCount, ##__VA_ARGS__) == false)				\
		throw CodaScriptException(ByteCode->GetSource(), "Command '%s' - Couldn't extract arguments", GetName())

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		namespace Commands
		{
			CodaScriptCommandCommonParamDecl(OneForm);
			CodaScriptCommandCommonParamDecl(OneNumber);
			CodaScriptCommandCommonParamDecl(OneString);
			CodaScriptCommandCommonParamDecl(OneArray);
			CodaScriptCommandCommonParamDecl(FormNumber);
			CodaScriptCommandCommonParamDecl(FormString);
			CodaScriptCommandCommonParamDecl(TwoForms);
		}
	}
}