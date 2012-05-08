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

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		namespace mup
		{
			CodaScriptMUPParserByteCode::CodaScriptMUPParserByteCode( CodaScriptMUPExpressionParser* Parent, ICodaScriptExecutableCode* Source ) :
				ICodaScriptObject(),
				ICodaScriptExpressionByteCode(Source),
				Parser(Parent),
				Tokenizer(),
				FinalResultIndex(0),
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
			const char_type *g_sCmdCode[] = { _T("BRCK. OPEN     "),
				_T("BRCK. CLOSE    "),
				_T("IDX OPEN       "),
				_T("IDX CLOSE      "),
				_T("ARG_SEP        "),
				_T("IF             "),
				_T("ELSE           "),
				_T("ENDIF          "),
				_T("JMP            "),
				_T("VAR            "),
				_T("VAL            "),
				_T("FUNC           "),
				_T("OPRT_BIN       "),
				_T("OPRT_IFX       "),
				_T("OPRT_PFX       "),
				_T("END            "),
				_T("SCRIPT_GOTO    "),
				_T("SCRIPT_LABEL   "),
				_T("SCRIPT_FOR     "),
				_T("SCRIPT_IF      "),
				_T("SCRIPT_ELSE    "),
				_T("SCRIPT_ELSEIF  "),
				_T("SCRIPT_ENDIF   "),
				_T("SCRIPT_NEWLINE "),
				_T("SCRIPT_FUNCTION"),
				_T("UNKNOWN        ") };

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
				if (m_pByteCode)
					err.Expr = string_type(m_pByteCode->Tokenizer->GetExpr());

				err.Ident = (a_pTok) ? a_pTok->GetIdent() : _T("");
				throw ParserError(err);
			}

			void CodaScriptMUPExpressionParser::ApplyFunc( Stack<ptr_tok_type> &a_stOpt, Stack<ptr_val_type> &a_stVal, int a_iArgCount ) const
			{
				if (a_stOpt.empty())
					return;

				SME_ASSERT(m_pByteCode && m_pByteCode->Tokenizer.get());

				ptr_tok_type tok = a_stOpt.pop();
				ICallback *pFun = tok->AsICallback();

				int iArgCount = (pFun->GetArgc()>=0) ? pFun->GetArgc() : a_iArgCount;
				int iOffset = a_stVal.size() - iArgCount;
				MUP_ASSERT(iOffset>=0);

				// The paramater stack may be empty since functions may not
				// have a parameter. They do always have a return value though.
				// If the param stack is empty create an entry for the function
				// return value.
				if (iArgCount==0)
					a_stVal.push(ptr_val_type(new CodaScriptMUPValue()));

				MUP_ASSERT((std::size_t)iOffset<a_stVal.size());
				ptr_val_type *pArg = a_stVal.get_data() + iOffset;

				try
				{
					// Make sure to pass on a volatile flag to the function result
					bool bResultIsVolatile = false;
					for (int i=0; i<iArgCount && bResultIsVolatile==false; ++i)
					{
						if (pArg[i]->IsFlagSet(IToken::flVOLATILE))
							bResultIsVolatile = true;
					}

					// Instead of evaluating the function merely a dummy value of the same type as the function return value
					// is created
					*pArg = ptr_val_type(new CodaScriptMUPValue());
					pFun->SetNumArgsPresent(iArgCount);

					if (bResultIsVolatile)
						(*pArg)->AddFlags(IToken::flVOLATILE);

					m_pByteCode->RPNStack.Add(tok);
				}
				catch(ParserError &e)
				{
					ErrorContext &err = e.GetContext();
					err.Pos   = m_pByteCode->Tokenizer->GetPos();
					err.Expr  = m_pByteCode->Tokenizer->GetExpr();

					if (err.Ident.empty())
						err.Ident = pFun->GetIdent();

					throw;
				}

				if (iArgCount>0)
					a_stVal.pop(iArgCount-1); // remove the arguments
			}

			void CodaScriptMUPExpressionParser::ApplyIfElse( Stack<ptr_tok_type> &a_stOpt, Stack<ptr_val_type> &a_stVal ) const
			{
				SME_ASSERT(m_pByteCode && m_pByteCode->Tokenizer.get());

				while (a_stOpt.size() && a_stOpt.top()->GetCode()==cmELSE)
				{
					MUP_ASSERT(a_stOpt.size()>0);
					MUP_ASSERT(a_stVal.size()>=3);
					MUP_ASSERT(a_stOpt.top()->GetCode()==cmELSE);

					// it then else is a ternary operator Pop all three values from the value
					// stack and just return the right value
					ptr_val_type vVal2 = a_stVal.pop();
					ptr_val_type vVal1 = a_stVal.pop();
					ptr_val_type bExpr = a_stVal.pop();

					// Push a dummy value of the correct type
					a_stVal.push(ptr_val_type(new CodaScriptMUPValue(/*cType*/)));

					// Pass on volatile flags
					if (vVal1->IsFlagSet(IToken::flVOLATILE) || vVal2->IsFlagSet(IToken::flVOLATILE))
						a_stVal.top()->AddFlags(IToken::flVOLATILE);

					ptr_tok_type opElse = a_stOpt.pop();
					ptr_tok_type opIf = a_stOpt.pop();
					MUP_ASSERT(opElse->GetCode()==cmELSE)
					MUP_ASSERT(opIf->GetCode()==cmIF)

					m_pByteCode->RPNStack.Add(ptr_tok_type(new TokenIfThenElse(cmENDIF)));
				}
			}

			void CodaScriptMUPExpressionParser::ApplyRemainingOprt( Stack<ptr_tok_type> &a_stOpt, Stack<ptr_val_type> &a_stVal ) const
			{
				while (a_stOpt.size() &&
					a_stOpt.top()->GetCode() != cmBO &&
					a_stOpt.top()->GetCode() != cmIO &&
					a_stOpt.top()->GetCode() != cmIF)
				{
					ptr_tok_type &op = a_stOpt.top();

					switch(op->GetCode())
					{
					case  cmOPRT_BIN:
						MUP_ASSERT(a_stOpt.top()->GetCode()==cmOPRT_BIN);
						ApplyFunc(a_stOpt, a_stVal, 2);
						break;

					case  cmELSE:
						ApplyIfElse(a_stOpt, a_stVal);
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
				Stack<ptr_val_type> stVal;
				Stack<ICallback*>   stFunc;
				Stack<int>  stArgCount;
				Stack<int>  stIdxCount;
				ptr_tok_type pTok, pTokPrev;
				CodaScriptMUPValue val;

				// The outermost counter counts the number of seperated items
				// such as in "a=10,b=20,c=c+a"
				stArgCount.push(1);

				for(bool bLoop=true; bLoop;)
				{
					pTokPrev = pTok;
					pTok = OutByteCode->Tokenizer->ReadNextToken();

					ECmdCode eCmd = pTok->GetCode();
					switch (eCmd)
					{
					case  cmVAR:
					case  cmVAL:
						{
							IValue *pVal = pTok->AsIValue();
							if (stFunc.empty() && pVal->GetType()=='n')
							{
								ErrorContext err;
								err.Errc  = ecUNEXPECTED_PARENS;
								err.Ident = _T(")");
								err.Pos   = pTok->GetExprPos();
								throw ParserError(err);
							}

							stVal.push( ptr_val_type(pVal) );

							// Arrays can't be added directly to the reverse polish notation
							// since there may be an index operator following next...
							OutByteCode->RPNStack.Add(pTok);

							// Apply infix operator if existant
							if (stOpt.size() && stOpt.top()->GetCode()==cmOPRT_INFIX)
								ApplyFunc(stOpt, stVal, 1);
						}
						break;

					case  cmIC:
						{
							// The argument count for parameterless functions is zero
							// by default an opening bracket sets parameter count to 1
							// in preparation of arguments to come. If the last token
							// was an opening bracket we know better...
							if (pTokPrev.Get()!=NULL && pTokPrev->GetCode()==cmIO)
								--stArgCount.top();

							ApplyRemainingOprt(stOpt, stVal);

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
								MUP_ASSERT(pOprtIndex!=NULL);

								pOprtIndex->SetNumArgsPresent(iArgc);
								OutByteCode->RPNStack.Add(pTok);

								// Pop the index values from the stack
								MUP_ASSERT(stVal.size()>=iArgc+1);
								for (std::size_t i=0; i<iArgc; ++i)
									stVal.pop();

								// Now i would need to pop the topmost value from the stack, apply the index
								// opertor and push the result back to the stack. But here we are just creating the
								// RPN and are working with dummy values anyway so i just mark the topmost value as
								// volatile and leave it were it is. The real index logic is in the RPN evaluator...
								stVal.top()->AddFlags(IToken::flVOLATILE);
							} // if opening index bracket is on top of operator stack
						}
						break;

					case  cmBC:
						{
							// The argument count for parameterless functions is zero
							// by default an opening bracket sets parameter count to 1
							// in preparation of arguments to come. If the last token
							// was an opening bracket we know better...
							if (pTokPrev.Get()!=NULL && pTokPrev->GetCode()==cmBO)
								--stArgCount.top();

							ApplyRemainingOprt(stOpt, stVal);

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

								// Evaluate the function
								ApplyFunc(stOpt, stVal, iArgc);

								// Apply an infix operator, if present
								if (stOpt.size() && stOpt.top()->GetCode()==cmOPRT_INFIX)
									ApplyFunc(stOpt, stVal, 1);
							}
						}
						break;

					case  cmELSE:
						ApplyRemainingOprt(stOpt, stVal);
						OutByteCode->RPNStack.Add(pTok);
						stOpt.push(pTok);
						break;

					case  cmSCRIPT_NEWLINE:
						{
							ApplyRemainingOprt(stOpt, stVal);

							// Value stack plätten
							// Stack der RPN um die Anzahl im stack enthaltener Werte zurück setzen
							int n = stVal.size();
							OutByteCode->RPNStack.AddNewline(pTok, n);
							stVal.clear();
							stOpt.clear();
						}
						break;

					case  cmARG_SEP:
						if (stArgCount.empty())
							Error(ecUNEXPECTED_COMMA, OutByteCode->Tokenizer->GetPos());

						++stArgCount.top();

						//if (stVal.size()) // increase argument counter
						//  stArgCount.top()++;

						ApplyRemainingOprt(stOpt, stVal);
						break;

					case  cmEOE:
						ApplyRemainingOprt(stOpt, stVal);
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
								ApplyFunc(stOpt, stVal, 2);
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
							MUP_ASSERT(stVal.size());

							ptr_val_type &pVal(stVal.top());
							try
							{
								// place a dummy return value into the value stack, do not
								// evaluate pOprt (this is important for lazy evaluation!)
								// The only place where evaluation takes place is the RPN
								// engine!
								pVal = ptr_val_type(new CodaScriptMUPValue());
								OutByteCode->RPNStack.Add(pTok);
							}
							catch(ParserError &)
							{
								throw;
							}
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
						bLoop = false;
				} // for (all tokens)

				OutByteCode->FinalResultIndex = stArgCount.top()-1;
				MUP_ASSERT(stVal.size());
			}

			CodaScriptMUPExpressionParser::CodaScriptMUPExpressionParser() :
				ICodaScriptObject(),
				ICodaScriptExpressionParser(),
				m_FunDef(),
				m_PostOprtDef(),
				m_InfixOprtDef(),
				m_OprtDef(),
				m_valConst(),
				m_valDynVarShadow(),
				m_VarDef(),
				m_CSVarDef(),
				m_sNameChars(),
				m_sOprtChars(),
				m_sInfixOprtChars(),
				m_pByteCode(NULL),
				m_pEvalAgent(NULL)
			{
				DefineNameChars(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"));
				DefineOprtChars(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*^/?<>=#!$%&|~'_µ{}"));
				DefineInfixOprtChars(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ()/+-*^?<>=#!$%&|~'_"));
			}

			CodaScriptMUPExpressionParser::CodaScriptMUPExpressionParser( const CodaScriptMUPExpressionParser &a_Parser )
			{
				;// Cannot be copied or assigned
			}

			CodaScriptMUPExpressionParser& CodaScriptMUPExpressionParser::operator=( const CodaScriptMUPExpressionParser &a_Parser )
			{
				;// Cannot be copied or assigned
				return *this;
			}

			CodaScriptMUPExpressionParser::~CodaScriptMUPExpressionParser()
			{
				SME_ASSERT(m_pByteCode == NULL && m_pEvalAgent == NULL);
			}

			void CodaScriptMUPExpressionParser::AddValueReader( IValueReader *a_pReader )
			{
				SME_ASSERT(m_pByteCode && m_pByteCode->Tokenizer.get());

				m_pByteCode->Tokenizer->AddValueReader(a_pReader);
			}

			void CodaScriptMUPExpressionParser::AddPackage( IPackage *p )
			{
				p->AddToParser(this);
			}

			void CodaScriptMUPExpressionParser::DefineFun( ICallback *a_pCallback )
			{
				a_pCallback->SetParent(this);
				m_FunDef[ a_pCallback->GetIdent() ] = ptr_tok_type(a_pCallback);
			}

			void CodaScriptMUPExpressionParser::DefineConst( const string_type &a_sName, const CodaScriptMUPValue &a_Val )
			{
				CheckName(a_sName, ValidNameChars());
				m_valConst[a_sName] = ptr_tok_type( a_Val.Clone() );
			}

			void CodaScriptMUPExpressionParser::DefineOprt( IOprtBin *a_pCallback )
			{
				a_pCallback->SetParent(this);
				m_OprtDef.insert(make_pair(a_pCallback->GetIdent(), ptr_tok_type(a_pCallback)));
			}

			void CodaScriptMUPExpressionParser::DefinePostfixOprt( IOprtPostfix *a_pCallback )
			{
				m_PostOprtDef[a_pCallback->GetIdent()] = ptr_tok_type(a_pCallback);
			}

			void CodaScriptMUPExpressionParser::DefineInfixOprt( IOprtInfix *a_pCallback )
			{
				m_InfixOprtDef[a_pCallback->GetIdent()] = ptr_tok_type(a_pCallback);
			}

			const var_maptype& CodaScriptMUPExpressionParser::GetVar() const
			{
				SME_ASSERT(m_pEvalAgent && m_pEvalAgent->GetContext());

				ContextSpecificVariableMapT::const_iterator Match = m_CSVarDef.find(m_pEvalAgent->GetContext());
				if (Match == m_CSVarDef.end())
					return m_VarDef;					// return the empty collection
				else
					return Match->second;
			}

			const val_maptype& CodaScriptMUPExpressionParser::GetConst() const
			{
				return m_valConst;
			}

			const fun_maptype& CodaScriptMUPExpressionParser::GetFunDef() const
			{
				return m_FunDef;
			}

			const string_type& CodaScriptMUPExpressionParser::GetExpr() const
			{
				SME_ASSERT(m_pByteCode && m_pByteCode->Tokenizer.get());

				return m_pByteCode->Tokenizer->GetExpr();
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

			void CodaScriptMUPExpressionParser::RegisterVariables( CodaScriptExecutionContext* ParentContext, CodaScriptVariableListT& VariableList )
			{
				SME_ASSERT(ParentContext);

				ContextSpecificVariableMapT::const_iterator Match = m_CSVarDef.find(ParentContext);
				if (Match == m_CSVarDef.end())
					m_CSVarDef[ParentContext] = m_VarDef;

				for (CodaScriptVariableListT::const_iterator Itr = VariableList.begin(); Itr != VariableList.end(); Itr++)
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
						throw CodaScriptException(NULL, "P[%d] %s", E.GetPos(), E.GetMsg().c_str());
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
					SME::MiscGunk::ScopedSetter<ICodaScriptSyntaxTreeEvaluator*>	GuardEvalData(m_pEvalAgent, EvaluationAgent);
					SME::MiscGunk::ScopedSetter<CodaScriptMUPParserByteCode*>		GuardByteCode(m_pByteCode, new CodaScriptMUPParserByteCode(this, SourceCode));
					*OutByteCode = NULL;

					try									// ugly, but meh
					{
						AddPackage(PackageCommon::Instance());
						AddPackage(PackageNonCmplx::Instance());
						AddPackage(PackageStr::Instance());
					}
					catch (...)
					{
						SAFEDELETE(m_pByteCode);
						throw;
					}

					std::auto_ptr<CodaScriptMUPParserByteCode> CompiledByteCode(m_pByteCode);
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
					throw CodaScriptException(SourceCode, "P[%d] %s", E.GetPos(), E.GetMsg().c_str());
				}
			}

			void CodaScriptMUPExpressionParser::Evaluate( ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
														ICodaScriptExpressionByteCode* ByteCode,
														CodaScriptBackingStore* Result /*= NULL*/ )
			{
				CodaScriptMUPParserByteCode* CompiledByteCode = dynamic_cast<CodaScriptMUPParserByteCode*>(ByteCode);

				// m_pByteCode/m_pEvalAgent can be valid in here if the execution stack depth is > 1
				SME_ASSERT(ByteCode && EvaluationAgent->GetContext() && EvaluationAgent->GetVM());
				SME_ASSERT(CompiledByteCode && CompiledByteCode->Tokenizer.get());

				try
				{
					SME::MiscGunk::ScopedSetter<ICodaScriptSyntaxTreeEvaluator*>		GuardEvalData(m_pEvalAgent, EvaluationAgent);
					SME::MiscGunk::ScopedSetter<CodaScriptMUPParserByteCode*>			GuardByteCode(m_pByteCode, CompiledByteCode);

					ptr_val_type *pStack = &m_pByteCode->StackBuffer[0];
					const ptr_tok_type *pRPN = &(m_pByteCode->RPNStack.GetData()[0]);

					int sidx = -1;
					std::size_t lenRPN = m_pByteCode->RPNStack.GetSize();
					for (std::size_t i=0; i<lenRPN; ++i)
					{
						IToken *pTok = pRPN[i].Get();
						ECmdCode eCode = pTok->GetCode();

						switch (eCode)
						{
						case cmSCRIPT_NEWLINE:
							sidx = -1;
							m_pByteCode->FinalResultIndex = 0;
							continue;
						case cmVAR:
							{
								sidx++;
								assert(sidx<(int)m_pByteCode->StackBuffer.size());
								pStack[sidx].Reset(static_cast<IValue*>(pTok));
							}
							continue;
						case cmVAL:
							{
								sidx++;
								assert(sidx<(int)m_pByteCode->StackBuffer.size());
								ptr_val_type &val = pStack[sidx];
								if (val->GetCode()==cmVAR)
									val.Reset(m_pByteCode->Cache.CreateFromCache());

								*val = *(static_cast<IValue*>(pTok));
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
									if (val->GetCode()==cmVAR)
									{
										ptr_val_type buf(m_pByteCode->Cache.CreateFromCache());
										pFun->Eval(buf, &val, nArgs);
										val = buf;
									}
									else
										pFun->Eval(val, &val, nArgs);
								}
								catch(ParserError &exc)
								{
									ErrorContext err;
									err.Expr = m_pByteCode->Tokenizer->GetExpr();
									err.Ident = pFun->GetIdent();
									err.Errc = ecEVAL;
									err.Pos = pFun->GetExprPos();
									err.Hint = exc.GetMsg();
									throw ParserError(err);
								}
								catch(MatrixError& /*exc*/)
								{
									ErrorContext err;
									err.Expr = m_pByteCode->Tokenizer->GetExpr();
									err.Ident = pFun->GetIdent();
									err.Errc = ecEVAL;
									err.Pos = pFun->GetExprPos();
									err.Hint = _T("Matrix dimension mismatch");
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
						SME_ASSERT(m_pByteCode->FinalResultIndex != -1);
						*Result = *pStack[m_pByteCode->FinalResultIndex]->GetStore();
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

			CodaScriptMUPParserByteCode* CodaScriptMUPExpressionParser::GetByteCode( void )
			{
				return m_pByteCode;
			}

			ICodaScriptSyntaxTreeEvaluator* CodaScriptMUPExpressionParser::GetEvaluationAgent( void )
			{
				return m_pEvalAgent;
			}
		}
	}
}