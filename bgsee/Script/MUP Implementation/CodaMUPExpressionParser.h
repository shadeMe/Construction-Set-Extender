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
				CodaScriptMUPExpressionParser*	Parser;
				std::auto_ptr<TokenReader>		Tokenizer;

				mutable int						TokenPos;
				mutable RPN						RPNStack;			///< reverse polish notation
				mutable val_vec_type			StackBuffer;
				mutable ValueCache				Cache;				///< A cache for recycling value items instead of deleting them
			public:
				CodaScriptMUPParserByteCode(CodaScriptMUPExpressionParser* Parent, ICodaScriptExecutableCode* Source);
				virtual ~CodaScriptMUPParserByteCode();
			};

			// a stripped-down and slightly different implementation of mup::ParserXBase
			class CodaScriptMUPExpressionParser : public ICodaScriptExpressionParser
			{
				friend class TokenReader;
				friend class CodaScriptMUPFunction;

				CodaScriptMUPExpressionParser(const CodaScriptMUPExpressionParser &a_Parser);
				CodaScriptMUPExpressionParser& operator=(const CodaScriptMUPExpressionParser &a_Parser);

				class ByteCodeAgentStackOperator
				{
					friend class CodaScriptMUPExpressionParser;

					CodaScriptMUPExpressionParser*			Parent;
					CodaScriptMUPParserByteCode*			ByteCode;
					ICodaScriptSyntaxTreeEvaluator*			EvalAgent;

					void									Push(void);
					void									Pop(void);
				public:
					ByteCodeAgentStackOperator(CodaScriptMUPExpressionParser* Parent, CodaScriptMUPParserByteCode* ByteCode, ICodaScriptSyntaxTreeEvaluator* Agent);
					ByteCodeAgentStackOperator(CodaScriptMUPExpressionParser* Parent, CodaScriptMUPParserByteCode* ByteCode);
					ByteCodeAgentStackOperator(CodaScriptMUPExpressionParser* Parent, ICodaScriptSyntaxTreeEvaluator* Agent);
					~ByteCodeAgentStackOperator();
				};

				friend class ByteCodeAgentStackOperator;
			protected:
				static const char_type*												c_DefaultOprt[];
				typedef std::map<CodaScriptExecutionContext*, var_maptype>			ContextSpecificVariableMapT;

				fun_maptype										m_FunDef;           ///< Function definitions
				oprt_pfx_maptype								m_PostOprtDef;		///< Postfix operator callbacks
				oprt_ifx_maptype								m_InfixOprtDef;		///< Infix operator callbacks.
				oprt_bin_maptype								m_OprtDef;			///< Binary operator callbacks
				val_maptype										m_valDef;			///< Definition of parser constants
				val_vec_type									m_valDynVarShadow;  ///< Value objects referenced by variables created at parser runtime
				var_maptype										m_varDef;           ///< User defined variables. Deprecated, always empty
				ContextSpecificVariableMapT						m_CSVarDef;			///< Maps execution contexts to a MUP var_maptype variable map

				string_type										m_sNameChars;       ///< Charset for names
				string_type										m_sOprtChars;       ///< Charset for postfix/ binary operator tokens
				string_type										m_sInfixOprtChars;  ///< Charset for infix operator tokens

				std::stack<CodaScriptMUPParserByteCode*>		m_ByteCodeStack;	///< Only populated inside an Evaluate/Compile call
				std::stack<ICodaScriptSyntaxTreeEvaluator*>		m_EvalAgentStack;	///< Same as above

				void											Error(EErrorCodes a_iErrc, int a_iPos = -1,	const IToken *a_pTok = 0) const;

				void											ApplyFunc(Stack<ptr_tok_type> &a_stOpt, int a_iArgCount) const;
				void											ApplyIfElse(Stack<ptr_tok_type> &a_stOpt) const;
				void											ApplyRemainingOprt(Stack<ptr_tok_type> &a_stOpt) const;

				void											CheckName(const string_type &a_sName, const string_type &a_CharSet) const;
				void											CreateRPN(CodaScriptMUPParserByteCode* OutByteCode) const;
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

				const var_maptype&								GetVar() const;
				const val_maptype&								GetConst() const;
				const fun_maptype&								GetFunDef() const;
				const string_type&								GetExpr() const;
				const char_type**								GetOprtDef() const;

				const char_type*								ValidNameChars() const;
				const char_type*								ValidOprtChars() const;
				const char_type*								ValidInfixOprtChars() const;

				void											DefineNameChars(const char_type *a_szCharset);
				void											DefineOprtChars(const char_type *a_szCharset);
				void											DefineInfixOprtChars(const char_type *a_szCharset);

				CodaScriptMUPParserByteCode*					GetCurrentByteCode(void) const;
				ICodaScriptSyntaxTreeEvaluator*					GetCurrentEvaluationAgent(void) const;

				virtual void									RegisterCommand(ICodaScriptCommand* Command);
				virtual void									RegisterConstant(const char* Name, CodaScriptBackingStore& Value);

				virtual void									RegisterVariables(CodaScriptExecutionContext* ParentContext,
																				CodaScriptVariableArrayT& VariableList);
				virtual void									UnregisterVariables(CodaScriptExecutionContext* ParentContext);

				virtual void									Compile(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
																		ICodaScriptExecutableCode* SourceCode,
																		ICodaScriptExpressionByteCode** OutByteCode);
				virtual void									Evaluate(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
																		ICodaScriptExpressionByteCode* ByteCode,
																		CodaScriptBackingStore* Result = NULL);
			};
		}
	}
}