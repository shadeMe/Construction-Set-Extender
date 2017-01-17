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
#include "CodaUtilities.h"
#include "Main.h"
#include "Console.h"

namespace bgsee
{
	namespace script
	{
		namespace mup
		{
			CodaScriptMUPParserByteCode::ValueBuffer::ValueBuffer() :
				StackBuffer(), Cache()
			{
				;//
			}

			CodaScriptMUPParserByteCode::ValueBuffer::~ValueBuffer()
			{
				// It is important to release the stack buffer before
				// releasing the value cache. Since it may contain
				// Values referencing the cache.
				StackBuffer.clear();
				Cache.ReleaseAll();
			}

			CodaScriptMUPParserByteCode::ValueBuffer* CodaScriptMUPParserByteCode::CreateBufferContext() const
			{
				ValueBuffer* Out = new ValueBuffer;
				Out->StackBuffer.assign(RPNStack.GetRequiredStackSize(), ptr_val_type());

				for (std::size_t i = 0; i < Out->StackBuffer.size(); ++i)
				{
					CodaScriptMUPValue *pValue = new CodaScriptMUPValue;
					pValue->BindToCache(&Out->Cache);
					Out->StackBuffer[i].Reset(pValue);
				}

				return Out;
			}

			void CodaScriptMUPParserByteCode::PushBufferContext()
			{
				SME_ASSERT(CurrentValueCache == nullptr);

				ValueBuffer::PtrT Context(CreateBufferContext());
				CurrentValueCache = Context.get();
				Buffer.push(std::move(Context));
			}

			void CodaScriptMUPParserByteCode::PopBufferContext()
			{
				if (Buffer.empty())
					throw CodaScriptException("Value buffer stack underflow");
				else
					Buffer.pop();

				if (Buffer.empty())
					CurrentValueCache = nullptr;
				else
					CurrentValueCache = Buffer.top().get();
			}

			CodaScriptMUPParserByteCode::CodaScriptMUPParserByteCode(CodaScriptMUPExpressionParser* Parent, ICodaScriptExecutableCode* Source) :
				ICodaScriptExpressionByteCode(Source),
				Parser(Parent),
				TokenPos(0),
				RPNStack(),
				Buffer(),
				CurrentValueCache(nullptr)
			{
				SME_ASSERT(Parser);
			}

			CodaScriptMUPParserByteCode::~CodaScriptMUPParserByteCode()
			{
				if (Buffer.size())
					BGSEECONSOLE_MESSAGE("Value buffer still in use");

				RPNStack.Reset();
			}

			val_vec_type& CodaScriptMUPParserByteCode::GetStackBuffer() const
			{
				return CurrentValueCache->StackBuffer;
			}

			ValueCache& CodaScriptMUPParserByteCode::GetCache() const
			{
				return CurrentValueCache->Cache;
			}

			CodaScriptMUPVariable* CodaScriptMUPParserMetadata::CreateWrapper(const CodaScriptSourceCodeT& Name, bool Global)
			{
				if (GetWrapper(Name, Global))
					throw CodaScriptException("Variable wrapper '%s' already exists", Name);

				VarWrapperMapT& Source = Global ? Globals : Locals;
				CodaScriptMUPVariable* Out = new CodaScriptMUPVariable(Name, Global);
				CodaScriptMUPVariable::PtrT Addend(Out);
				Source.insert(std::make_pair(Name, std::move(Addend)));
				return Out;
			}

			CodaScriptMUPVariable* CodaScriptMUPParserMetadata::GetWrapper(const CodaScriptSourceCodeT& Name, bool Global) const
			{
				const VarWrapperMapT& Source = Global ? Globals : Locals;
				if (Source.count(Name) == 0)
					return nullptr;
				else
					return Source.at(Name).get();
			}

			void CodaScriptMUPParserMetadata::BindWrappers(ICodaScriptExpressionParser::EvaluateData& Data)
			{
				int LocalCount = Data.Context->GetProgram()->GetVariableCount();
				int GlobalCount = Data.GlobalVariables.size();
				std::vector<CodaScriptMUPVariable*> Transaction;

				try
				{
					if (LocalCount != Locals.size())
						throw CodaScriptException("Local variable count mismatch - Expected %d, received %d", Locals.size(), LocalCount);
					else if (GlobalCount < Globals.size())
						// new globals aren't an issue as they aren't referenced by the bytecode
						throw CodaScriptException("Global variable count mismatch - Expected %d, received %d", Globals.size(), GlobalCount);

					for (auto& Itr : Locals)
					{
						CodaScriptVariable* Var = Data.Context->GetVariable(Itr.first);
						if (Var == nullptr)
							throw CodaScriptException("Couldn't find wrapped local variable '%s'", Itr.first.c_str());

						Itr.second->Bind(Var);
						Transaction.push_back(Itr.second.get());
					}

					for (auto& Itr : Globals)
					{
						CodaScriptVariable* Var = Data.GetGlobal(Itr.first);
						if (Var == nullptr)
							throw CodaScriptException("Couldn't find wrapped global variable '%s'", Itr.first.c_str());

						Itr.second->Bind(Var);
						Transaction.push_back(Itr.second.get());

					}
				}
				catch (...)
				{
					// reset the transaction, flag the program and rethrow
					for (auto Itr : Transaction)
						Itr->Unbind();

					Data.Context->GetProgram()->InvalidateBytecode();
					throw;
				}
			}

			void CodaScriptMUPParserMetadata::UnbindWrappers()
			{
				for (auto& Itr : Locals)
					Itr.second->Unbind();

				for (auto& Itr : Globals)
					Itr.second->Unbind();
			}

			CodaScriptMUPParserMetadata::CodaScriptMUPParserMetadata(CodaScriptMUPExpressionParser* Parser, ICodaScriptProgram* Program) :
				Parser(Parser),
				Program(Program),
				Locals(),
				Globals(),
				CompiledBytecode()
			{
				SME_ASSERT(Parser && Program);
			}

			CodaScriptMUPParserMetadata::~CodaScriptMUPParserMetadata()
			{
				for (auto& Itr : Locals)
				{
					if (Itr.second->IsBound())
						BGSEECONSOLE_MESSAGE("Local variable wrapper '%s' still bound to a value", Itr.first.c_str());
				}

				for (auto& Itr : Globals)
				{
					if (Itr.second->IsBound())
						BGSEECONSOLE_MESSAGE("Global variable wrapper '%s' still bound to a value", Itr.first.c_str());
				}
			}

			ICodaScriptProgram* CodaScriptMUPParserMetadata::GetSourceProgram() const
			{
				return Program;
			}

			ICodaScriptExpressionParser* CodaScriptMUPParserMetadata::GetParentParser() const
			{
				return Parser;
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
					err.Expr = string_type(ByteCode->GetSource()->GetSourceCode());

				err.Ident = (a_pTok) ? a_pTok->GetIdent() : _T("");
				throw ParserError(err);
			}

			void CodaScriptMUPExpressionParser::ApplyFunc( Stack<ptr_tok_type> &a_stOpt, int a_iArgCount ) const
			{
				if (a_stOpt.empty())
					return;

				CodaScriptMUPParserByteCode* ByteCode = GetCurrentByteCode();
				SME_ASSERT(ByteCode);

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
				SME_ASSERT(ByteCode);

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
				SME_ASSERT(OutByteCode);

				if (!m_TokenReader->GetExpr().length())
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
					pTok = m_TokenReader->ReadNextToken();

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
							Error(ecUNEXPECTED_COMMA, m_TokenReader->GetPos());

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
									Error(ecTOO_MANY_PARAMS, m_TokenReader->GetPos());

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
				m_TokenReader(),
				m_FunDef(),
				m_PostOprtDef(),
				m_InfixOprtDef(),
				m_OprtDef(),
				m_valDef(),
				m_sNameChars(),
				m_sOprtChars(),
				m_sInfixOprtChars(),
				m_opContext()
			{
				DefineNameChars(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"));
				DefineOprtChars(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*^/?<>=#!$%&|~'_µ{}"));
				DefineInfixOprtChars(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ()/+-*^?<>=#!$%&|~'_"));

				m_TokenReader.reset(new TokenReader(this));

				AddPackage(PackageCommon::Instance());
				AddPackage(PackageNonCmplx::Instance());
				AddPackage(PackageStr::Instance());
			}

			CodaScriptMUPExpressionParser::~CodaScriptMUPExpressionParser()
			{
				SME_ASSERT(m_opContext.empty());
			}

			void CodaScriptMUPExpressionParser::AddValueReader( IValueReader *a_pReader )
			{
				m_TokenReader->AddValueReader(a_pReader);
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
					throw ParserError(ErrorContext(ecFUNOPRT_DEFINED, 0, a_pFunc.Get()->GetIdent()));
				}
			}

			void CodaScriptMUPExpressionParser::DefineConst( const string_type &a_sName, const CodaScriptMUPValue &a_Val )
			{
				CheckName(a_sName, ValidNameChars());

				if (m_valDef.find(a_sName) == m_valDef.end()) {
					// Constant is not added yet, add it.
					m_valDef[a_sName] = ptr_tok_type( a_Val.Clone() );
				}
				else {
					throw ParserError(ErrorContext(ecCONSTANT_DEFINED, 0, a_sName));
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
					throw ParserError(ErrorContext(ecFUNOPRT_DEFINED, 0, a_Oprt.Get()->GetIdent()));
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
					throw ParserError(ErrorContext(ecFUNOPRT_DEFINED, 0, a_pOprt.Get()->GetIdent()));
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
					throw ParserError(ErrorContext(ecFUNOPRT_DEFINED, 0, a_iOprt.Get()->GetIdent()));
				}
			}

			const var_maptype& CodaScriptMUPExpressionParser::GetVar() const
			{
				SME_ASSERT(m_opContext.empty() == false);

				const OperationContext& Context = m_opContext.top();
				SME_ASSERT(Context.Type == OperationType::Compile);

				return Context.CompileData.Variables;
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

			void CodaScriptMUPExpressionParser::RegisterProgram(ICodaScriptProgram* Program)
			{
				;// nothing to do here
			}

			void CodaScriptMUPExpressionParser::DeregisterProgram(ICodaScriptProgram* Program)
			{
				;// nothing to do here
			}

			void CodaScriptMUPExpressionParser::BeginCompilation(ICodaScriptProgram* Program, CompileData Data)
			{
				std::unique_ptr<CodaScriptMUPParserMetadata> Metadata(new CodaScriptMUPParserMetadata(this, Program));
				OperationContext OpContext(OperationType::Compile, Program);

				// register variables and globals
				CodaScriptVariableNameArrayT Locals;
				for (auto& Itr : Program->GetVariables(Locals))
				{
					CheckVariableName(Itr, OpContext.CompileData.Variables);

					CodaScriptMUPVariable* Wrapper = Metadata->CreateWrapper(Itr, false);
					SME_ASSERT(Wrapper);
					OpContext.CompileData.Variables[Itr] = ptr_tok_type(new Variable(Wrapper));
				}

				for (auto Itr : Data.GlobalVariables)
				{
					CheckVariableName(Itr->GetName(), OpContext.CompileData.Variables);

					CodaScriptMUPVariable* Wrapper = Metadata->CreateWrapper(Itr->GetName(), true);
					SME_ASSERT(Wrapper);
					OpContext.CompileData.Variables[Itr->GetName()] = ptr_tok_type(new Variable(Wrapper));
				}

				OpContext.CompileData.Metadata = Metadata.release();
				m_opContext.push(OpContext);
			}

			void CodaScriptMUPExpressionParser::Compile(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
														ICodaScriptExecutableCode* SourceCode,
														ICodaScriptExpressionByteCode** OutByteCode)
			{
				SME_ASSERT(SourceCode && OutByteCode && EvaluationAgent);
				SME_ASSERT(m_opContext.empty() == false);

				OperationContext& Context = m_opContext.top();
				SME_ASSERT(Context.Type == OperationType::Compile && Context.CompileData.Metadata);
				SME_ASSERT(Context.Bytecode == nullptr);
				SME_ASSERT(Context.Program == EvaluationAgent->GetProgram());

				try
				{
					std::unique_ptr<CodaScriptMUPParserByteCode> GeneratedCode(new CodaScriptMUPParserByteCode(this, SourceCode));
					ScopedFunctor Sentinel([&Context, &GeneratedCode, &EvaluationAgent](ScopedFunctor::Event e) {
						if (e == ScopedFunctor::Event::Construction)
						{
							Context.Agent = EvaluationAgent;
							Context.Bytecode = GeneratedCode.get();
						}
						else
						{
							Context.Agent = nullptr;
							Context.Bytecode = nullptr;
						}
					});

					*OutByteCode = nullptr;

					m_TokenReader->SetExpr(SourceCode->GetSourceCode());
					CreateRPN(GeneratedCode.get());
					Context.CompileData.Metadata->CompiledBytecode.push_back(GeneratedCode.get());

					*OutByteCode = GeneratedCode.release();
				}
				catch (ParserError& E)
				{
					throw CodaScriptException(SourceCode, "Position[%d] %s", E.GetPos(), E.GetMsg().c_str());
				}
			}

			void CodaScriptMUPExpressionParser::EndCompilation(ICodaScriptProgram* Program, ICodaScriptCompilerMetadata** OutMetadata)
			{
				SME_ASSERT(OutMetadata);

				if (m_opContext.empty())
					throw CodaScriptException("Parser operation stack underflow");

				OperationContext& Current = m_opContext.top();
				if (Current.Type != OperationType::Compile || Current.Program != Program)
					throw CodaScriptException("Mismatched end compilation call");

				*OutMetadata = Current.CompileData.Metadata;
				m_opContext.pop();
			}

			void CodaScriptMUPExpressionParser::BeginEvaluation(ICodaScriptProgram* Program, EvaluateData Data)
			{
				OperationContext OpContext(OperationType::Evaluate, Program, Data.Context);
				CodaScriptMUPParserMetadata* Metadata = dynamic_cast<CodaScriptMUPParserMetadata*>(Program->GetCompilerMetadata());
				SME_ASSERT(Metadata);

				// bind the variables to the execution context
				Metadata->BindWrappers(Data);
				m_opContext.push(OpContext);

				// create new value buffers for the current context
				for (auto& Itr : Metadata->CompiledBytecode)
					Itr->PushBufferContext();
			}

			void CodaScriptMUPExpressionParser::Evaluate(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
														 ICodaScriptExpressionByteCode* ByteCode,
														 CodaScriptBackingStore* Result /*= nullptr*/)
			{
				SME_ASSERT(ByteCode && EvaluationAgent);

				CodaScriptMUPParserByteCode* CompiledByteCode = dynamic_cast<CodaScriptMUPParserByteCode*>(ByteCode);
				SME_ASSERT(CompiledByteCode);

				SME_ASSERT(m_opContext.empty() == false);
				OperationContext& Context = m_opContext.top();
				SME_ASSERT(Context.Type == OperationType::Evaluate && Context.EvaluateData.ExecutionContext == EvaluationAgent->GetContext());
				SME_ASSERT(Context.Bytecode == nullptr);
				SME_ASSERT(Context.Program == EvaluationAgent->GetProgram());

				try
				{
					ScopedFunctor Sentinel([&Context, &CompiledByteCode, &EvaluationAgent](ScopedFunctor::Event e) {
						if (e == ScopedFunctor::Event::Construction)
						{
							Context.Agent = EvaluationAgent;
							Context.Bytecode = CompiledByteCode;
						}
						else
						{
							Context.Agent = nullptr;
							Context.Bytecode = nullptr;
						}
					});

					ptr_val_type *pStack = &CompiledByteCode->GetStackBuffer()[0];
					if (CompiledByteCode->RPNStack.GetSize() == 0)
					{
						ErrorContext err;
						err.Expr = CompiledByteCode->GetSource()->GetSourceCode();
						err.Errc = ecUNEXPECTED_EOF;
						err.Pos = 0;
						throw ParserError(err);
					}

					const ptr_tok_type *pRPN = &(CompiledByteCode->RPNStack.GetData()[0]);

					int sidx = -1;
					std::size_t lenRPN = CompiledByteCode->RPNStack.GetSize();
					for (std::size_t i = 0; i < lenRPN; ++i)
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
								assert(sidx < (int)CompiledByteCode->GetStackBuffer().size());
								if (pVal->IsVariable())
								{
									pStack[sidx].Reset(pVal);
								}
								else
								{
									ptr_val_type &val = pStack[sidx];
									if (val->IsVariable())
										val.Reset(CompiledByteCode->GetCache().CreateFromCache());

									*val = *(static_cast<IValue*>(pTok));
								}
							}
							continue;
						case  cmIC:
							{
								IOprtIndex *pIdxOprt = static_cast<IOprtIndex*>(pTok);
								int nArgs = pIdxOprt->GetArgsPresent();
								sidx -= nArgs - 1;
								assert(sidx >= 0);

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
								assert(sidx >= 0);

								ptr_val_type &val = pStack[sidx];
								try
								{
									if (val->IsVariable())
									{
										ptr_val_type buf(CompiledByteCode->GetCache().CreateFromCache());
										pFun->Eval(buf, &val, nArgs);
										val = buf;
									}
									else
										pFun->Eval(val, &val, nArgs);
								}
								catch (ParserError &exc)
								{
									// <ibg 20130131> Not too happy about that:
									// Multiarg functions may throw ecTOO_FEW_PARAMS from eval. I don't
									// want this to be converted to ecEVAL because fixed argument functions
									// already throw ecTOO_FEW_PARAMS in case of missing parameters and two
									// different error codes would be inconsistent.
									if (exc.GetCode() == ecTOO_FEW_PARAMS || exc.GetCode() == ecDOMAIN_ERROR || exc.GetCode() == ecOVERFLOW)
										throw;
									// </ibg>

									ErrorContext err;
									err.Expr = CompiledByteCode->GetSource()->GetSourceCode();
									err.Ident = pFun->GetIdent();
									err.Errc = ecEVAL;
									err.Pos = pFun->GetExprPos();
									err.Hint = exc.GetMsg();
									throw ParserError(err);
								}
								catch (MatrixError& /*exc*/)
								{
									ErrorContext err;
									err.Expr = CompiledByteCode->GetSource()->GetSourceCode();
									err.Ident = pFun->GetIdent();
									err.Errc = ecMATRIX_DIMENSION_MISMATCH;
									err.Pos = pFun->GetExprPos();
									throw ParserError(err);
								}
							}
							continue;
						case cmIF:
							MUP_ASSERT(sidx >= 0);
							if (pStack[sidx--]->GetBool() == false)
								i += static_cast<TokenIfThenElse*>(pTok)->GetOffset();
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


			void CodaScriptMUPExpressionParser::EndEvaluation(ICodaScriptProgram* Program)
			{
				CodaScriptMUPParserMetadata* Metadata = dynamic_cast<CodaScriptMUPParserMetadata*>(Program->GetCompilerMetadata());
				SME_ASSERT(Metadata);

				if (m_opContext.empty())
					throw CodaScriptException("Parser operation stack underflow");

				OperationContext& Current = m_opContext.top();
				if (Current.Type != OperationType::Evaluate || Current.Program != Program)
					throw CodaScriptException("Mismatched end evaluation call");

				// restore the variables to their previous binding
				Metadata->UnbindWrappers();
				m_opContext.pop();

				// release the current value buffer
				for (auto& Itr : Metadata->CompiledBytecode)
					Itr->PopBufferContext();
			}

			string_type CodaScriptMUPExpressionParser::GetVersion() const
			{
				return MUP_PARSER_VERSION;
			}

			void CodaScriptMUPExpressionParser::CheckVariableName(const CodaScriptSourceCodeT& Name, const var_maptype& RegisteredVars) const
			{
				try
				{
					CheckName(string_type(Name), ValidNameChars());

					if (RegisteredVars.find(Name) != RegisteredVars.end())
						throw CodaScriptException("Variable '%s' already exists", Name.c_str());
					else if (m_valDef.find(Name) != m_valDef.end())
						throw CodaScriptException("Variable name collision - Constant '%s' already exists", Name.c_str());
				}
				catch (ParserError& E)
				{
					throw CodaScriptException("P[%d] %s", E.GetPos(), E.GetMsg().c_str());
				}
			}

			ICodaScriptSyntaxTreeEvaluator* CodaScriptMUPExpressionParser::GetCurrentEvaluationAgent() const
			{
				if (m_opContext.size())
					return m_opContext.top().Agent;
				else
					return nullptr;
			}

			CodaScriptMUPParserByteCode* CodaScriptMUPExpressionParser::GetCurrentByteCode(void) const
			{
				if (m_opContext.size())
					return m_opContext.top().Bytecode;
				else
					return nullptr;
			}



		}
	}
}