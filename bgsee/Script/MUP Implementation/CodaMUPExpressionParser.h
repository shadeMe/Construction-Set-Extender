#pragma once

#include "mpDefines.h"
#include "mpTypes.h"
#include "mpError.h"
#include "mpIOprt.h"
#include "mpIPackage.h"
#include "mpIValReader.h"
#include "mpStack.h"
#include "mpRPN.h"
#include "mpVariable.h"
#include "mpValueCache.h"
#include "mpTokenReader.h"
#include "CodaInterpreter.h"
#include "CodaMUPValue.h"
#include "CodaMUPVariable.h"

namespace bgsee
{
	namespace script
	{
		namespace mup
		{
			class CodaScriptMUPExpressionParser;
			class CodaScriptMUPFunction;

			class CodaScriptMUPParserByteCode : public ICodaScriptExpressionByteCode
			{
				friend class CodaScriptMUPExpressionParser;

				CodaScriptMUPParserByteCode(const CodaScriptMUPParserByteCode &ByteCode);
				CodaScriptMUPParserByteCode& operator=(const CodaScriptMUPParserByteCode &ByteCode);
			protected:
				struct ValueBuffer
				{
					val_vec_type					StackBuffer;
					ValueCache						Cache;				///< A cache for recycling value items instead of deleting them

					ValueBuffer();
					~ValueBuffer();

					typedef std::unique_ptr<ValueBuffer>	PtrT;
					typedef std::stack<PtrT>				StackT;
				};

				CodaScriptMUPExpressionParser*	Parser;

				int								TokenPos;
				RPN								RPNStack;			///< reverse polish notation
				ValueBuffer::StackT				Buffer;				// buffers for currently executing contexts

				ValueBuffer*					CreateBufferContext() const;
				ValueBuffer&					GetCurrentBufferContext() const;
			public:
				CodaScriptMUPParserByteCode(CodaScriptMUPExpressionParser* Parent, ICodaScriptExecutableCode* Source);
				virtual ~CodaScriptMUPParserByteCode();

				void							PushBufferContext();
				void							PopBufferContext();

				val_vec_type&					GetStackBuffer() const;
				ValueCache&						GetCache() const;

				typedef std::vector<CodaScriptMUPParserByteCode*>		ArrayT;
			};

			class CodaScriptMUPParserMetadata : public ICodaScriptCompilerMetadata
			{
				friend class CodaScriptMUPExpressionParser;

				CodaScriptMUPParserMetadata(const CodaScriptMUPParserMetadata &ByteCode);
				CodaScriptMUPParserMetadata& operator=(const CodaScriptMUPParserMetadata &ByteCode);
			protected:
				typedef std::unordered_map<CodaScriptSourceCodeT, CodaScriptMUPVariable::PtrT>		VarWrapperMapT;		// key = name

				CodaScriptMUPExpressionParser*			Parser;
				ICodaScriptProgram*						Program;
				VarWrapperMapT							Locals;
				VarWrapperMapT							Globals;
				CodaScriptMUPParserByteCode::ArrayT		CompiledBytecode;

				CodaScriptMUPVariable*			CreateWrapper(const CodaScriptSourceCodeT& Name, bool Global);
				CodaScriptMUPVariable*			GetWrapper(const CodaScriptSourceCodeT& Name, bool Global) const;

				void							BindWrappers(ICodaScriptExpressionParser::EvaluateData& Data);
				void							UnbindWrappers();
			public:
				CodaScriptMUPParserMetadata(CodaScriptMUPExpressionParser* Parser, ICodaScriptProgram* Program);
				virtual ~CodaScriptMUPParserMetadata();

				virtual ICodaScriptExpressionParser*	GetParentParser() const override;
				virtual ICodaScriptProgram*				GetSourceProgram() const override;
			};

			// a stripped-down and slightly different implementation of mup::ParserXBase
			class CodaScriptMUPExpressionParser : public ICodaScriptExpressionParser
			{
				friend class TokenReader;

				CodaScriptMUPExpressionParser(const CodaScriptMUPExpressionParser &a_Parser);
				CodaScriptMUPExpressionParser& operator=(const CodaScriptMUPExpressionParser &a_Parser);
			protected:
				static const char_type*												c_DefaultOprt[];

				enum class OperationType
				{
					Compile, Evaluate
				};

				struct OperationContext
				{
					typedef std::stack<OperationContext>		StackT;

					OperationType						Type;
					ICodaScriptProgram*					Program;
					ICodaScriptSyntaxTreeEvaluator*		Agent;
					CodaScriptMUPParserByteCode*		Bytecode;		// currently being compiled/evaluated

					struct
					{
						ICodaScriptExecutionContext*	ExecutionContext;
					} EvaluateData;

					struct
					{
						CodaScriptMUPParserMetadata*	Metadata;
						var_maptype						Variables;		// locals and globals
					} CompileData;

					OperationContext(OperationType Type, ICodaScriptProgram* Program, ICodaScriptExecutionContext* Context) :
						Type(Type), Program(Program), Agent(nullptr), Bytecode(nullptr), EvaluateData{ Context }, CompileData{ nullptr }
					{
					}

					OperationContext(OperationType Type, ICodaScriptProgram* Program) :
						Type(Type), Program(Program), Agent(nullptr), Bytecode(nullptr), EvaluateData{ nullptr }, CompileData{ nullptr }
					{
					}
				};

				std::unique_ptr<TokenReader>					m_TokenReader;
				fun_maptype										m_FunDef;           ///< Function definitions
				oprt_pfx_maptype								m_PostOprtDef;		///< Postfix operator callbacks
				oprt_ifx_maptype								m_InfixOprtDef;		///< Infix operator callbacks.
				oprt_bin_maptype								m_OprtDef;			///< Binary operator callbacks
				val_maptype										m_valDef;			///< Definition of parser constants
				OperationContext::StackT						m_opContext;		///< Stores the contexts of the executing parser operations

				string_type										m_sNameChars;       ///< Charset for names
				string_type										m_sOprtChars;       ///< Charset for postfix/ binary operator tokens
				string_type										m_sInfixOprtChars;  ///< Charset for infix operator tokens

				void											Error(EErrorCodes a_iErrc, int a_iPos = -1,	const IToken *a_pTok = 0) const;

				void											ApplyFunc(Stack<ptr_tok_type> &a_stOpt, int a_iArgCount) const;
				void											ApplyIfElse(Stack<ptr_tok_type> &a_stOpt) const;
				void											ApplyRemainingOprt(Stack<ptr_tok_type> &a_stOpt) const;

				void											CheckName(const string_type &a_sName, const string_type &a_CharSet) const;
				void											CreateRPN(CodaScriptMUPParserByteCode* OutByteCode) const;

				const var_maptype&								GetVar() const;
				const char_type**								GetOprtDef() const;

				const char_type*								ValidNameChars() const;
				const char_type*								ValidOprtChars() const;
				const char_type*								ValidInfixOprtChars() const;

				void											DefineNameChars(const char_type *a_szCharset);
				void											DefineOprtChars(const char_type *a_szCharset);
				void											DefineInfixOprtChars(const char_type *a_szCharset);

				void											CheckVariableName(const CodaScriptSourceCodeT& Name, const var_maptype& RegisteredVars) const;
			public:
				CodaScriptMUPExpressionParser();
				virtual ~CodaScriptMUPExpressionParser();

				string_type										GetVersion() const;

				void											AddValueReader(IValueReader *a_pReader);
				void											AddPackage(IPackage *p);

				void											DefineFun(const ptr_cal_type &a_pFunc);
				void											DefineConst(const string_type &a_sName, const CodaScriptMUPValue &a_Val);
				void											DefineOprt(const TokenPtr<IOprtBin> &a_Oprt);
				void											DefinePostfixOprt(const TokenPtr<IOprtPostfix> &a_pOprt);
				void											DefineInfixOprt(const TokenPtr<IOprtInfix> &a_iOprt);

				virtual void									RegisterCommand(ICodaScriptCommand* Command) override;
				virtual void									RegisterConstant(const char* Name, CodaScriptBackingStore& Value) override;

				virtual void									RegisterProgram(ICodaScriptProgram* Program) override;
				virtual void									DeregisterProgram(ICodaScriptProgram* Program) override;

				virtual void									BeginCompilation(ICodaScriptProgram* Program, CompileData Data) override;
				virtual void									Compile(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
																		ICodaScriptExecutableCode* SourceCode,
																		ICodaScriptExpressionByteCode** OutByteCode) override;
				virtual void									EndCompilation(ICodaScriptProgram* Program, ICodaScriptCompilerMetadata** OutMetadata) override;

				virtual void									BeginEvaluation(ICodaScriptProgram* Program, EvaluateData Data) override;
				virtual void									Evaluate(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
																		 ICodaScriptExpressionByteCode* ByteCode,
																		 CodaScriptBackingStore* Result = nullptr) override;
				virtual void									EndEvaluation(ICodaScriptProgram* Program) override;

				ICodaScriptSyntaxTreeEvaluator*					GetCurrentEvaluationAgent() const;
				CodaScriptMUPParserByteCode*					GetCurrentByteCode(void) const;
			};
		}
	}
}