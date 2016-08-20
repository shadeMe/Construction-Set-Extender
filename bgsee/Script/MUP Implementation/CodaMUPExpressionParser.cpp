#include "CodaMUPExpressionParser.h"
#include "utGeneric.h"
#include "mpDefines.h"
#include "mpIfThenElse.h"
#include "mpScriptTokens.h"
#include "mpPackageUnit.h"
#include "mpPackageStr.h"
#include "mpPackageCmplx.h"
#include "mpPackageNonCmplx.h"
#include "mpPackageCommon.h"
#include "mpPackageMatrix.h"
#include "CodaMUPScriptCommand.h"

namespace bgsee
{
	namespace script
	{
		namespace mup
		{
			CodaScriptMUPParserByteCode::CodaScriptMUPParserByteCode( CodaScriptMUPExpressionParser* Parent, ICodaScriptExecutableCode* Source ) :
				ICodaScriptExpressionByteCode(Source),
				Parser(Parent),
				Tokenizer(),
				TokenPos(0),
				RPNStack(),
				StackBuffer(),
				Cache()
			{
				SME_ASSERT(Parser);

				Tokenizer.reset(new TokenReader(Parser));
				Tokenizer->SetExpr(Source->GetCode());
			}

			CodaScriptMUPParserByteCode::~CodaScriptMUPParserByteCode()
			{
				// It is important to release the stack buffer before
				// releasing the value cache. Since it may contain
				// Values referencing the cache.
				RPNStack.Reset();
				StackBuffer.clear();
				Cache.ReleaseAll();
			}

			//------------------------------------------------------------------------------
			const char_type *g_sCmdCode[] = { _T("BRCK. OPEN  "),
				_T("BRCK. CLOSE "),
				_T("IDX OPEN    "),
				_T("IDX CLOSE   "),
				_T("ARG_SEP     "),
				_T("IF          "),
				_T("ELSE        "),
				_T("ENDIF       "),
				_T("JMP         "),
				_T("VAL         "),
				_T("FUNC        "),
				_T("OPRT_BIN    "),
				_T("OPRT_IFX    "),
				_T("OPRT_PFX    "),
				_T("END         "),
				_T("SCRIPT_ENDL "),
				_T("SCRIPT_CMT  "),
				_T("SCRIPT_GOTO "),
				_T("SCRIPT_LABEL"),
				_T("SCRIPT_FOR  "),
				_T("SCRIPT_IF   "),
				_T("SCRIPT_ELSE "),
				_T("SCRIPT_ELIF "),
				_T("SCRIPT_ENDIF"),
				_T("SCRIPT_FUNC "),
				_T("UNKNOWN     ") };

			CodaScriptMUPExpressionParser::ByteCodeAgentStackOperator::ByteCodeAgentStackOperator( CodaScriptMUPExpressionParser* Parent,
																								CodaScriptMUPParserByteCode* ByteCode,
																								ICodaScriptSyntaxTreeEvaluator* Agent ) :
				Parent(Parent),
				ByteCode(ByteCode),
				EvalAgent(Agent)
			{
				SME_ASSERT(Parent && ByteCode && Agent);

				Push();
			}

			CodaScriptMUPExpressionParser::ByteCodeAgentStackOperator::ByteCodeAgentStackOperator( CodaScriptMUPExpressionParser* Parent,
																								CodaScriptMUPParserByteCode* ByteCode ) :
				Parent(Parent),
				ByteCode(ByteCode),
				EvalAgent(nullptr)
			{
				SME_ASSERT(Parent && ByteCode);

				Push();
			}

			CodaScriptMUPExpressionParser::ByteCodeAgentStackOperator::ByteCodeAgentStackOperator( CodaScriptMUPExpressionParser* Parent,
																								ICodaScriptSyntaxTreeEvaluator* Agent ) :
				Parent(Parent),
				ByteCode(nullptr),
				EvalAgent(Agent)
			{
				SME_ASSERT(Parent && Agent);

				Push();
			}

			CodaScriptMUPExpressionParser::ByteCodeAgentStackOperator::~ByteCodeAgentStackOperator()
			{
				Pop();
			}

			void CodaScriptMUPExpressionParser::ByteCodeAgentStackOperator::Push( void )
			{
				if (ByteCode)
					Parent->m_ByteCodeStack.push(ByteCode);

				if (EvalAgent)
					Parent->m_EvalAgentStack.push(EvalAgent);
			}

			void CodaScriptMUPExpressionParser::ByteCodeAgentStackOperator::Pop( void )
			{
				if (ByteCode)
				{
					SME_ASSERT(Parent->m_ByteCodeStack.top() == ByteCode);
					Parent->m_ByteCodeStack.pop();
				}

				if (EvalAgent)
				{
					SME_ASSERT(Parent->m_EvalAgentStack.top() == EvalAgent);
					Parent->m_EvalAgentStack.pop();
				}
			}

			const char_type* CodaScriptMUPExpressionParser::c_DefaultOprt[] = { _T("("),
				_T(")"),
				_T("["),
				_T("]"),
				_T(","),
				_T("?"),
				_T(":"),
				0 };

			void CodaScriptMUPExpressionParser::Error( EErrorCodes a_iErrc, int a_iPos /*= -1*/, const IToken *a_pTok /*= 0*/ ) const
			{
				ErrorContext err;
				err.Errc = a_iErrc;
				err.Pos = a_iPos;

				CodaScriptMUPParserByteCode* ByteCode = GetCurrentByteCode();
				if (ByteCode)
					err.Expr = string_type(ByteCode->Tokenizer->GetExpr());

				err.Ident = (a_pTok) ? a_pTok->GetIdent() : _T("");
				throw ParserError(err);
			}

			void CodaScriptMUPExpressionParser::ApplyFunc( Stack<ptr_tok_type> &a_stOpt, int a_iArgCount ) const
			{
				if (a_stOpt.empty())
					return;

				CodaScriptMUPParserByteCode* ByteCode = GetCurrentByteCode();
				SME_ASSERT(ByteCode && ByteCode->Tokenizer.get());

				ptr_tok_type tok = a_stOpt.pop();
				ICallback *pFun = tok->AsICallback();

				int iArgCount = (pFun->GetArgc()>=0) ? pFun->GetArgc() : a_iArgCount;
				pFun->SetNumArgsPresent(iArgCount);

				ByteCode->TokenPos -= (iArgCount - 1);
				ByteCode->RPNStack.Add(tok);
			}

			void CodaScriptMUPExpressionParser::ApplyIfElse( Stack<ptr_tok_type> &a_stOpt ) const
			{
				CodaScriptMUPParserByteCode* ByteCode = GetCurrentByteCode();
				SME_ASSERT(ByteCode && ByteCode->Tokenizer.get());

				while (a_stOpt.size() && a_stOpt.top()->GetCode()==cmELSE)
				{
					MUP_ASSERT(a_stOpt.size()>0);
					MUP_ASSERT(ByteCode->TokenPos>=3);
					MUP_ASSERT(a_stOpt.top()->GetCode()==cmELSE);
					
					ptr_tok_type opElse = a_stOpt.pop();
					ptr_tok_type opIf = a_stOpt.pop();
					MUP_ASSERT(opElse->GetCode()==cmELSE)
					MUP_ASSERT(opIf->GetCode()==cmIF)

					// If then else hat 3 argumente und erzeugt einen rückgabewert (3-1=2)
					ByteCode->TokenPos -= 2;
					ByteCode->RPNStack.Add(ptr_tok_type(new TokenIfThenElse(cmENDIF)));
				}
			}

			void CodaScriptMUPExpressionParser::ApplyRemainingOprt( Stack<ptr_tok_type> &a_stOpt ) const
			{
				while (a_stOpt.size() &&
					a_stOpt.top()->GetCode() != cmBO &&
					a_stOpt.top()->GetCode() != cmIO &&
					a_stOpt.top()->GetCode() != cmIF)
				{
					ptr_tok_type &op = a_stOpt.top();

					switch(op->GetCode())
					{
					case  cmOPRT_INFIX:
					case  cmOPRT_BIN:
						ApplyFunc(a_stOpt, 2);
						break;

					case  cmELSE:
						ApplyIfElse(a_stOpt);
						break;

					default:
						Error(ecINTERNAL_ERROR);
					} // switch operator token type
				} // While operator stack not empty
			}

			void CodaScriptMUPExpressionParser::CheckName( const string_type &a_sName, const string_type &a_CharSet ) const
			{
				if ( !a_sName.length() ||
					(a_sName.find_first_not_of(a_CharSet)!=string_type::npos) ||
					(a_sName[0]>=(char_type)'0' && a_CharSet[0]<=(char_type)'9'))
				{
					Error(ecINVALID_NAME);
				}
			}

			void CodaScriptMUPExpressionParser::CreateRPN( CodaScriptMUPParserByteCode* OutByteCode ) const
			{
				SME_ASSERT(OutByteCode && OutByteCode->Tokenizer.get());

				if (!OutByteCode->Tokenizer->GetExpr().length())
					Error(ecUNEXPECTED_EOF, 0);

				// The Stacks take the ownership over the tokens
				Stack<ptr_tok_type> stOpt;
				Stack<ICallback*>   stFunc;
				Stack<int>  stArgCount;
				Stack<int>  stIdxCount;
				ptr_tok_type pTok, pTokPrev;
				CodaScriptMUPValue val;
				
				for(;;)
				{
					pTokPrev = pTok;
					pTok = OutByteCode->Tokenizer->ReadNextToken();

					ECmdCode eCmd = pTok->GetCode();
					switch (eCmd)
					{
					case  cmVAL:
						{
							OutByteCode->TokenPos++;
							OutByteCode->RPNStack.Add(pTok);
						}
						break;

					case  cmIC:
						{
							// The argument count for parameterless functions is zero
							// by default an opening bracket sets parameter count to 1
							// in preparation of arguments to come. If the last token
							// was an opening bracket we know better...
							if (pTokPrev.Get()!=nullptr && pTokPrev->GetCode()==cmIO)
								--stArgCount.top();

							ApplyRemainingOprt(stOpt);

							// if opt is "]" and opta is "[" the bracket content has been evaluated.
							// Now its time to check if there is either a function or a sign pending.
							// - Neither the opening nor the closing bracket will be pushed back to
							//   the operator stack
							// - Check if a function is standing in front of the opening bracket,
							//   if so evaluate it afterwards to apply an infix operator.
							if ( stOpt.size() && stOpt.top()->GetCode()==cmIO )
							{
								//
								// Find out how many dimensions were used in the index operator.
								//
								std::size_t iArgc = stArgCount.pop();
								stOpt.pop(); // Take opening bracket from stack

								IOprtIndex *pOprtIndex = pTok->AsIOprtIndex();
								MUP_ASSERT(pOprtIndex!=nullptr);

								pOprtIndex->SetNumArgsPresent(iArgc);
								OutByteCode->RPNStack.Add(pTok);

								// Pop the index values from the stack
								MUP_ASSERT(OutByteCode->TokenPos>=iArgc+1);
								OutByteCode->TokenPos -= iArgc;
							} // if opening index bracket is on top of operator stack
						}
						break;

					case  cmBC:
						{
							// The argument count for parameterless functions is zero
							// by default an opening bracket sets parameter count to 1
							// in preparation of arguments to come. If the last token
							// was an opening bracket we know better...
							if (pTokPrev.Get()!=nullptr && pTokPrev->GetCode()==cmBO)
								--stArgCount.top();

							ApplyRemainingOprt(stOpt);

							// if opt is ")" and opta is "(" the bracket content has been evaluated.
							// Now its time to check if there is either a function or a sign pending.
							// - Neither the opening nor the closing bracket will be pushed back to
							//   the operator stack
							// - Check if a function is standing in front of the opening bracket,
							//   if so evaluate it afterwards to apply an infix operator.
							if ( stOpt.size() && stOpt.top()->GetCode()==cmBO )
							{
								//
								// Here is the stuff to evaluate a function token
								//
								int iArgc = stArgCount.pop();

								stOpt.pop(); // Take opening bracket from stack
								if ( stOpt.empty() )
									break;

								if ( (stOpt.top()->GetCode()!=cmFUNC) && (stOpt.top()->GetCode()!=cmOPRT_INFIX) )
									break;

								ICallback *pFun = stOpt.top()->AsICallback();
								stFunc.pop();

								if (pFun->GetArgc()!=-1 && iArgc > pFun->GetArgc())
									Error(ecTOO_MANY_PARAMS, pTok->GetExprPos(), pFun);

								if (iArgc < pFun->GetArgc())
									Error(ecTOO_FEW_PARAMS, pTok->GetExprPos(), pFun);

								// Apply function, if present
								if (stOpt.size() && 
									stOpt.top()->GetCode()!=cmOPRT_INFIX && 
									stOpt.top()->GetCode()!=cmOPRT_BIN)
								{
									ApplyFunc(stOpt, iArgc);
								}
							}
						}
						break;

					case  cmELSE:
						ApplyRemainingOprt(stOpt);
						OutByteCode->RPNStack.Add(pTok);
						stOpt.push(pTok);
						break;

					case  cmSCRIPT_NEWLINE:
						{
							ApplyRemainingOprt(stOpt);

							// Value stack plätten
							// Stack der RPN um die Anzahl im stack enthaltener Werte zurück setzen
							OutByteCode->RPNStack.AddNewline(pTok, OutByteCode->TokenPos);
							stOpt.clear();
							OutByteCode->TokenPos = 0;
						}
						break;

					case  cmARG_SEP:
						if (stArgCount.empty())
							Error(ecUNEXPECTED_COMMA, OutByteCode->Tokenizer->GetPos());

						++stArgCount.top();

						ApplyRemainingOprt(stOpt);
						break;

					case  cmEOE:
						ApplyRemainingOprt(stOpt);
						OutByteCode->RPNStack.Finalize();
						break;

					case  cmIF:
					case  cmOPRT_BIN:
						{
							while ( stOpt.size() &&
								stOpt.top()->GetCode() != cmBO &&
								stOpt.top()->GetCode() != cmIO &&
								stOpt.top()->GetCode() != cmELSE &&
								stOpt.top()->GetCode() != cmIF)
							{
								IToken *pOprt1 = stOpt.top().Get();
								IToken *pOprt2 = pTok.Get();
								MUP_ASSERT(pOprt1 && pOprt2);
								MUP_ASSERT(pOprt1->AsIPrecedence() && pOprt2->AsIPrecedence());

								int nPrec1 = pOprt1->AsIPrecedence()->GetPri(),
									nPrec2 = pOprt2->AsIPrecedence()->GetPri();

								if (pOprt1->GetCode()==pOprt2->GetCode())
								{
									// Deal with operator associativity
									EOprtAsct eOprtAsct = pOprt1->AsIPrecedence()->GetAssociativity();
									if ( (eOprtAsct==oaRIGHT && (nPrec1 <= nPrec2)) ||
										(eOprtAsct==oaLEFT  && (nPrec1 <  nPrec2)) )
									{
										break;
									}
								}
								else if (nPrec1 < nPrec2)
								{
									break;
								}

								// apply the operator now
								// (binary operators are identic to functions with two arguments)
								ApplyFunc(stOpt, 2);
							} // while ( ... )

							if (pTok->GetCode()==cmIF)
								OutByteCode->RPNStack.Add(pTok);

							stOpt.push(pTok);
						}
						break;

						//
						//  Postfix Operators
						//
					case  cmOPRT_POSTFIX:
						{
							MUP_ASSERT(OutByteCode->TokenPos);
							OutByteCode->RPNStack.Add(pTok);
						}
						break;

					case  cmIO:
					case  cmBO:
						stOpt.push(pTok);
						stArgCount.push(1);
						break;

						//
						// Functions
						//
					case  cmOPRT_INFIX:
					case  cmFUNC:
						{
							ICallback *pFunc = pTok->AsICallback();
							MUP_ASSERT(pFunc);

							// Check if this function is a argument to another function
							// if so check if the the return type fits.
							if (!stFunc.empty() && stFunc.top()->GetCode()==cmFUNC)
							{
								MUP_ASSERT(stArgCount.size());
								int iArgc = (int)stArgCount.top() /*+ 1*/;

								ICallback *pOuterFunc = stFunc.top();
								if (pOuterFunc->GetArgc()!=-1 && iArgc>pOuterFunc->GetArgc())
									Error(ecTOO_MANY_PARAMS, OutByteCode->Tokenizer->GetPos());

								MUP_ASSERT(pOuterFunc->GetArgc()==-1 || iArgc<=pOuterFunc->GetArgc());
							}

							stOpt.push(pTok);
							stFunc.push(pFunc); // to collect runtime type information
						}
						break;

					default:
						Error(ecINTERNAL_ERROR);
					} // switch Code

					if ( pTok->GetCode() == cmEOE )
						break;
				} // for (all tokens)

				if (OutByteCode->TokenPos > 1)
				{
					Error(ecUNEXPECTED_COMMA, -1);
				}
			}

			CodaScriptMUPExpressionParser::CodaScriptMUPExpressionParser() :
				ICodaScriptExpressionParser(),
				m_FunDef(),
				m_PostOprtDef(),
				m_InfixOprtDef(),
				m_OprtDef(),
				m_valDef(),
				m_valDynVarShadow(),
				m_varDef(),
				m_CSVarDef(),
				m_sNameChars(),
				m_sOprtChars(),
				m_sInfixOprtChars(),
				m_ByteCodeStack(),
				m_EvalAgentStack()
			{
				DefineNameChars(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"));
				DefineOprtChars(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*^/?<>=#!$%&|~'_µ{}"));
				DefineInfixOprtChars(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ()/+-*^?<>=#!$%&|~'_"));
			}

			CodaScriptMUPExpressionParser::~CodaScriptMUPExpressionParser()
			{
				SME_ASSERT(m_ByteCodeStack.size() == 0 && m_EvalAgentStack.size() == 0);
			}

			void CodaScriptMUPExpressionParser::AddValueReader( IValueReader *a_pReader )
			{
				CodaScriptMUPParserByteCode* ByteCode = GetCurrentByteCode();
				SME_ASSERT(ByteCode && ByteCode->Tokenizer.get());

				ByteCode->Tokenizer->AddValueReader(a_pReader);
			}

			void CodaScriptMUPExpressionParser::AddPackage( IPackage *p )
			{
				p->AddToParser(this);
			}

			void CodaScriptMUPExpressionParser::DefineFun(const ptr_cal_type &a_pFunc)
			{
				if (m_FunDef.find(a_pFunc.Get()->GetIdent()) == m_FunDef.end()) {
					// Function is not added yet, add it.
					a_pFunc.Get()->SetParent(this);
					m_FunDef[a_pFunc.Get()->GetIdent()] = ptr_tok_type(a_pFunc.Get());
				}
				else {
		;//			throw ParserError(ErrorContext(ecFUNOPRT_DEFINED, 0, a_pFunc.Get()->GetIdent()));
				}
			}

			void CodaScriptMUPExpressionParser::DefineConst( const string_type &a_sName, const CodaScriptMUPValue &a_Val )
			{
				CheckName(a_sName, ValidNameChars());

				if (m_varDef.find(a_sName) == m_varDef.end()) {
					// Constant is not added yet, add it.
					m_valDef[a_sName] = ptr_tok_type( a_Val.Clone() );
				}
				else {
			;//		throw ParserError(ErrorContext(ecCONSTANT_DEFINED, 0, a_sName));
				}
			}

			void CodaScriptMUPExpressionParser::DefineOprt(const TokenPtr<IOprtBin> &a_Oprt)
			{
				if (m_OprtDef.find(a_Oprt.Get()->GetIdent()) == m_OprtDef.end()) {
					// Operator is not added yet, add it.
					a_Oprt.Get()->SetParent(this);
					m_OprtDef.insert(make_pair(a_Oprt.Get()->GetIdent(),
						ptr_tok_type(a_Oprt.Get())));
				}
				else {
			;//		throw ParserError(ErrorContext(ecFUNOPRT_DEFINED, 0, a_Oprt.Get()->GetIdent()));
				}
			}

			void CodaScriptMUPExpressionParser::DefinePostfixOprt(const TokenPtr<IOprtPostfix> &a_pOprt)
			{
				if (m_PostOprtDef.find(a_pOprt.Get()->GetIdent()) == m_PostOprtDef.end()) {
					// Operator is not added yet, add it.
					a_pOprt.Get()->SetParent(this);
					m_PostOprtDef[a_pOprt.Get()->GetIdent()] = ptr_tok_type(a_pOprt.Get());
				}
				else {
			;//		throw ParserError(ErrorContext(ecFUNOPRT_DEFINED, 0, a_pOprt.Get()->GetIdent()));
				}
			}

			void CodaScriptMUPExpressionParser::DefineInfixOprt(const TokenPtr<IOprtInfix> &a_iOprt)
			{
				if (m_InfixOprtDef.find(a_iOprt.Get()->GetIdent()) == m_InfixOprtDef.end()) {
					// Operator is not added yet, add it.
					a_iOprt.Get()->SetParent(this);
					m_InfixOprtDef[a_iOprt.Get()->GetIdent()] = ptr_tok_type(a_iOprt.Get());
				}
				else {
			;//		throw ParserError(ErrorContext(ecFUNOPRT_DEFINED, 0, a_iOprt.Get()->GetIdent()));
				}
			}

			const var_maptype& CodaScriptMUPExpressionParser::GetVar() const
			{
				ICodaScriptSyntaxTreeEvaluator* EvalAgent = GetCurrentEvaluationAgent();
				SME_ASSERT(EvalAgent && EvalAgent->GetContext());

				ContextSpecificVariableMapT::const_iterator Match = m_CSVarDef.find(EvalAgent->GetContext());
				if (Match == m_CSVarDef.end())
					return m_varDef;					// return the empty collection
				else
					return Match->second;
			}

			const val_maptype& CodaScriptMUPExpressionParser::GetConst() const
			{
				return m_valDef;
			}

			const fun_maptype& CodaScriptMUPExpressionParser::GetFunDef() const
			{
				return m_FunDef;
			}

			const string_type& CodaScriptMUPExpressionParser::GetExpr() const
			{
				CodaScriptMUPParserByteCode* ByteCode = GetCurrentByteCode();
				SME_ASSERT(ByteCode && ByteCode->Tokenizer.get());

				return ByteCode->Tokenizer->GetExpr();
			}

			const char_type** CodaScriptMUPExpressionParser::GetOprtDef() const
			{
				return (const char_type **)(&c_DefaultOprt[0]);
			}

			const char_type* CodaScriptMUPExpressionParser::ValidNameChars() const
			{
				assert(m_sNameChars.size());
				return m_sNameChars.c_str();
			}

			const char_type* CodaScriptMUPExpressionParser::ValidOprtChars() const
			{
				assert(m_sOprtChars.size());
				return m_sOprtChars.c_str();
			}

			const char_type* CodaScriptMUPExpressionParser::ValidInfixOprtChars() const
			{
				assert(m_sInfixOprtChars.size());
				return m_sInfixOprtChars.c_str();
			}

			void CodaScriptMUPExpressionParser::DefineNameChars( const char_type *a_szCharset )
			{
				m_sNameChars = a_szCharset;
			}

			void CodaScriptMUPExpressionParser::DefineOprtChars( const char_type *a_szCharset )
			{
				m_sOprtChars = a_szCharset;
			}

			void CodaScriptMUPExpressionParser::DefineInfixOprtChars( const char_type *a_szCharset )
			{
				m_sInfixOprtChars = a_szCharset;
			}

			void CodaScriptMUPExpressionParser::RegisterCommand( ICodaScriptCommand* Command )
			{
				DefineFun(new CodaScriptMUPScriptCommand(Command));

				if (Command->GetAlias())
					DefineFun(new CodaScriptMUPScriptCommand(Command, true));
			}

			void CodaScriptMUPExpressionParser::RegisterConstant( const char* Name, CodaScriptBackingStore& Value )
			{
				DefineConst(Name, CodaScriptMUPValue(Value));
			}

			void CodaScriptMUPExpressionParser::RegisterVariables( CodaScriptExecutionContext* ParentContext, CodaScriptVariableArrayT& VariableList )
			{
				SME_ASSERT(ParentContext);

				ContextSpecificVariableMapT::const_iterator Match = m_CSVarDef.find(ParentContext);
				if (Match == m_CSVarDef.end())
					m_CSVarDef[ParentContext] = m_varDef;

				for (CodaScriptVariableArrayT::const_iterator Itr = VariableList.begin(); Itr != VariableList.end(); Itr++)
				{
					CodaScriptVariable* Var = *Itr;
					CodaScriptMUPValue* Bound = dynamic_cast<CodaScriptMUPValue*>(Var->GetStoreOwner());
					SME_ASSERT(Bound);

					try
					{
						CheckName(string_type(Var->GetName()), ValidNameChars());
					}
					catch (ParserError& E)
					{
						throw CodaScriptException(nullptr, "P[%d] %s", E.GetPos(), E.GetMsg().c_str());
					}

					m_CSVarDef[ParentContext][Var->GetName()] = ptr_tok_type(new Variable(Bound));
				}
			}

			void CodaScriptMUPExpressionParser::UnregisterVariables( CodaScriptExecutionContext* ParentContext )
			{
				SME_ASSERT(ParentContext);

				ContextSpecificVariableMapT::const_iterator Match = m_CSVarDef.find(ParentContext);
				if (Match != m_CSVarDef.end())
					m_CSVarDef.erase(Match);
			}

			void CodaScriptMUPExpressionParser::Compile( ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
														ICodaScriptExecutableCode* SourceCode,
														ICodaScriptExpressionByteCode** OutByteCode )
			{
				SME_ASSERT(SourceCode && OutByteCode && EvaluationAgent->GetContext() && EvaluationAgent->GetVM());

				try
				{
					ByteCodeAgentStackOperator PrologAgent(this, EvaluationAgent);
					ByteCodeAgentStackOperator PrologByteCode(this, new CodaScriptMUPParserByteCode(this, SourceCode));
					CodaScriptMUPParserByteCode* NewByteCode = PrologByteCode.ByteCode;

					*OutByteCode = nullptr;

					try									// ugly, but meh
					{
						AddPackage(PackageCommon::Instance());
						AddPackage(PackageNonCmplx::Instance());
						AddPackage(PackageStr::Instance());
					}
					catch (...)
					{
						SAFEDELETE(NewByteCode);
						throw;
					}

					std::auto_ptr<CodaScriptMUPParserByteCode> CompiledByteCode(NewByteCode);
					CreateRPN(CompiledByteCode.get());

					CompiledByteCode->StackBuffer.assign(CompiledByteCode->RPNStack.GetRequiredStackSize(), ptr_val_type());
					for (std::size_t i = 0; i < CompiledByteCode->StackBuffer.size(); ++i)
					{
						CodaScriptMUPValue *pValue = new CodaScriptMUPValue;
						pValue->BindToCache(&CompiledByteCode->Cache);
						CompiledByteCode->StackBuffer[i].Reset(pValue);
					}

					*OutByteCode = CompiledByteCode.release();
				}
				catch (ParserError& E)
				{
					throw CodaScriptException(SourceCode, "Position[%d] %s", E.GetPos(), E.GetMsg().c_str());
				}
			}

			void CodaScriptMUPExpressionParser::Evaluate( ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
														ICodaScriptExpressionByteCode* ByteCode,
														CodaScriptBackingStore* Result /*= NULL*/ )
			{
				CodaScriptMUPParserByteCode* CompiledByteCode = dynamic_cast<CodaScriptMUPParserByteCode*>(ByteCode);

				// ByteCode and EvalAgent stacks can be populated in here if the execution stack depth is > 1
				SME_ASSERT(ByteCode && EvaluationAgent->GetContext() && EvaluationAgent->GetVM());
				SME_ASSERT(CompiledByteCode && CompiledByteCode->Tokenizer.get());

				try
				{
					ByteCodeAgentStackOperator Prolog(this, CompiledByteCode, EvaluationAgent);

					ptr_val_type *pStack = &CompiledByteCode->StackBuffer[0];
					if (CompiledByteCode->RPNStack.GetSize()==0)
					{
						ErrorContext err;
						err.Expr = CompiledByteCode->Tokenizer->GetExpr();
						err.Errc = ecUNEXPECTED_EOF;
						err.Pos  = 0;
						throw ParserError(err);
					}

					const ptr_tok_type *pRPN = &(CompiledByteCode->RPNStack.GetData()[0]);

					int sidx = -1;
					std::size_t lenRPN = CompiledByteCode->RPNStack.GetSize();
					for (std::size_t i=0; i<lenRPN; ++i)
					{
						IToken *pTok = pRPN[i].Get();
						ECmdCode eCode = pTok->GetCode();

						switch (eCode)
						{
						case cmSCRIPT_NEWLINE:
							sidx = -1;
							continue;
						case cmVAL:
							{
								IValue *pVal = static_cast<IValue*>(pTok);

								sidx++;
								assert(sidx<(int)CompiledByteCode->StackBuffer.size());
								if (pVal->IsVariable())
								{
									pStack[sidx].Reset(pVal);
								}
								else
								{
									ptr_val_type &val = pStack[sidx];
									if (val->IsVariable())
										val.Reset(CompiledByteCode->Cache.CreateFromCache());

									*val = *(static_cast<IValue*>(pTok));
								}
							}
							continue;
						case  cmIC:
							{
								IOprtIndex *pIdxOprt = static_cast<IOprtIndex*>(pTok);
								int nArgs = pIdxOprt->GetArgsPresent();
								sidx -= nArgs - 1;
								assert(sidx>=0);

								ptr_val_type &idx = pStack[sidx];     // Pointer to the first index
								ptr_val_type &val = pStack[--sidx];   // Pointer to the variable or value being indexed
								pIdxOprt->At(val, &idx, nArgs);
							}
							continue;
						case cmOPRT_POSTFIX:
						case cmFUNC:
						case cmOPRT_BIN:
						case cmOPRT_INFIX:
							{
								ICallback *pFun = static_cast<ICallback*>(pTok);
								int nArgs = pFun->GetArgsPresent();
								sidx -= nArgs - 1;
								assert(sidx>=0);

								ptr_val_type &val = pStack[sidx];
								try
								{
									if (val->IsVariable())
									{
										ptr_val_type buf(CompiledByteCode->Cache.CreateFromCache());
										pFun->Eval(buf, &val, nArgs);
										val = buf;
									}
									else
										pFun->Eval(val, &val, nArgs);
								}
								catch(ParserError &exc)
								{
									// <ibg 20130131> Not too happy about that:
									// Multiarg functions may throw ecTOO_FEW_PARAMS from eval. I don't 
									// want this to be converted to ecEVAL because fixed argument functions
									// already throw ecTOO_FEW_PARAMS in case of missing parameters and two 
									// different error codes would be inconsistent. 
									if (exc.GetCode()==ecTOO_FEW_PARAMS || exc.GetCode()==ecDOMAIN_ERROR || exc.GetCode()==ecOVERFLOW)
										throw;
									// </ibg>

									ErrorContext err;
									err.Expr = CompiledByteCode->Tokenizer->GetExpr();
									err.Ident = pFun->GetIdent();
									err.Errc = ecEVAL;
									err.Pos = pFun->GetExprPos();
									err.Hint = exc.GetMsg();
									throw ParserError(err);
								}
								catch(MatrixError& /*exc*/)
								{
									ErrorContext err;
									err.Expr = CompiledByteCode->Tokenizer->GetExpr();
									err.Ident = pFun->GetIdent();
									err.Errc = ecMATRIX_DIMENSION_MISMATCH;
									err.Pos = pFun->GetExprPos();
									throw ParserError(err);
								}
							}
							continue;
						case cmIF:
							MUP_ASSERT(sidx>=0);
							if (pStack[sidx--]->GetBool()==false)
								i+=static_cast<TokenIfThenElse*>(pTok)->GetOffset();
							continue;
						case cmELSE:
						case cmJMP:
							i += static_cast<TokenIfThenElse*>(pTok)->GetOffset();
							continue;
						case cmENDIF:
							continue;
						default:
							Error(ecINTERNAL_ERROR);
						} // switch token
					} // for all RPN tokens

					if (Result)
					{
						*Result = *pStack[0]->GetStore();
					}
				}
				catch (ParserError& E)
				{
					throw CodaScriptException(CompiledByteCode->GetSource(), "P[%d] %s", E.GetPos(), E.GetMsg().c_str());
				}
			}

			string_type CodaScriptMUPExpressionParser::GetVersion() const
			{
				return MUP_PARSER_VERSION;
			}

			CodaScriptMUPParserByteCode* CodaScriptMUPExpressionParser::GetCurrentByteCode( void ) const
			{
				if (m_ByteCodeStack.size())
					return m_ByteCodeStack.top();
				else
					return nullptr;
			}

			ICodaScriptSyntaxTreeEvaluator* CodaScriptMUPExpressionParser::GetCurrentEvaluationAgent( void ) const
			{
				if (m_EvalAgentStack.size())
					return m_EvalAgentStack.top();
				else
					return nullptr;
			}
		}
	}
}