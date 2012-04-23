#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "CodaDataTypes.h"
#include "CodaVM.h"
#include "CodaInterpreter.h"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		const CodaScriptSourceCodeT				CodaScriptTokenizer::kWhitespace = " \t";
		const CodaScriptSourceCodeT				CodaScriptTokenizer::kCommentDelimiter = ";";
		const CodaScriptSourceCodeT				CodaScriptTokenizer::kValidDelimiters = kCommentDelimiter + kWhitespace + ".,(){}[]\n";
		const UInt32							CodaScriptTokenizer::kCodaKeywordCount = 15 + 1;
		CodaScriptSourceCodeT					CodaScriptTokenizer::kCodaKeywordArray[kCodaKeywordCount] =
		{
			"<UNKNOWN>",
			"VAR",
			"BEGIN",
			"END",
			"WHILE",
			"FOREACH",
			"LOOP",
			"IF",
			"ELSEIF"
			"ELSE",
			"ENDIF",
			"RETURN",
			"CALL",
			"CODA",				// syntax: CODA(<ScriptName>, <PollingInterval:optional>)
			"CONTINUE",
			"BREAK",
		};

		CodaScriptTokenizer::CodaScriptTokenizer() :
			ICodaScriptObject()
		{
			;//
		}

		CodaScriptTokenizer::~CodaScriptTokenizer()
		{
			ResetState();
		}

		bool CodaScriptTokenizer::Tokenize(CodaScriptSourceCodeT Source, bool CollectEmptyTokens)
		{
			Source += "\n";

			bool Result = false;
			int StartPos = -1, LastPos = -1;

			ResetState();

			for (int i = 0; i < Source.length(); i++)
			{
				char Itr = Source[i];
				if (kWhitespace.find(Itr) == std::string::npos)
				{
					StartPos =i;
					break;
				}
			}

			if (StartPos != -1)
			{
				LastPos = StartPos;

				std::string TokenString = Source.substr(StartPos);
				std::string Token, Delimiter;

				for (int Index = TokenString.find_first_of(kValidDelimiters); Index != std::string::npos; Index = TokenString.find_first_of(kValidDelimiters))
				{
					if (TokenString[0] == '\"')
					{
						if (TokenString.find('\"', 1) != std::string::npos)
							Index = TokenString.find('\"', 1) + 1;
						else
							Index = TokenString.length() - 1;
					}

					Token = TokenString.substr(0, Index);
					Delimiter = TokenString.substr(Index, 1);

					if (Delimiter != kCommentDelimiter)
					{
						TokenString = TokenString.erase(0, Index + 1);

						if (Token == "" && !CollectEmptyTokens)
						{
							LastPos++;
							continue;
						}

						Tokens.push_back(Token);
						Indices.push_back(LastPos);
						Delimiters.push_back(Delimiter[0]);
						LastPos += Token.length() + 1;
					}
					else
					{
						if (Token != "")
						{
							Tokens.push_back(Token);
							Indices.push_back(LastPos);
							Delimiters.push_back(Delimiter[0]);
						}
						break;
					}
				}

				if (Tokens.size() > 0)
					Result = true;
			}

			return Result;
		}

		void CodaScriptTokenizer::ResetState()
		{
			Tokens.clear();
			Delimiters.clear();
			Indices.clear();
		}

		CodaScriptKeywordT CodaScriptTokenizer::GetKeywordType(CodaScriptSourceCodeT& Token)
		{
			CodaScriptKeywordT Result = kTokenType_Invalid;

			for (int i = 1; i < kCodaKeywordCount; i++)
			{
				if (!_stricmp(Token.c_str(), kCodaKeywordArray[i].c_str()))
				{
					Result = 1 << i;
					break;
				}
			}

			return Result;
		}

		void CodaScriptTokenizer::Sanitize(CodaScriptSourceCodeT& In, CodaScriptSourceCodeT& Out, UInt32 OperationMask)
		{
			Out.clear();

			if (Tokenize(In, false))
			{
				if ((OperationMask & kSanitizeOps_StripLeadingWhitespace))
					Out = In.substr(Indices[0]);
				else
					Out = In;

				if ((OperationMask & kSanitizeOps_StripTabCharacters))
					std::replace(Out.begin(), Out.end(), '\t', ' ');

				int CommentDelimiter = Out.find(kCommentDelimiter);
				if ((OperationMask & kSanitizeOps_StripComments) && CommentDelimiter != std::string::npos)
				{
					Out.erase(CommentDelimiter, Out.length() - CommentDelimiter);
				}
			}
		}

		UInt32 CodaScriptTokenizer::GetParsedTokenCount() const
		{
			return Tokens.size();
		}

		CodaScriptKeywordT CodaScriptTokenizer::GetFirstTokenKeywordType()
		{
			if (GetParsedTokenCount())
				return GetKeywordType(Tokens[0]);
			else
				return kTokenType_Invalid;
		}

		ICodaScriptExpressionByteCode::ICodaScriptExpressionByteCode( ICodaScriptExecutableCode* SourceCode ) :
			Source(SourceCode)
		{
			SME_ASSERT(SourceCode);
		}

		ICodaScriptExecutableCode* ICodaScriptExpressionByteCode::GetSource( void ) const
		{
			return Source;
		}

		ICodaScriptExpressionParser::~ICodaScriptExpressionParser()
		{
			;//
		}

		ICodaScriptSyntaxTreeVisitor::~ICodaScriptSyntaxTreeVisitor()
		{
			;//
		}

		bool ICodaScriptSyntaxTreeNode::LookupChild( ICodaScriptSyntaxTreeNode* Child, CodaScriptSyntaxTreeNodeListT::iterator& Match )
		{
			bool Result = false;

			SME_ASSERT(Child && Child != this && Child != Parent);

			for (CodaScriptSyntaxTreeNodeListT::iterator Itr = Children.begin(); Itr != Children.end(); Itr++)
			{
				if ((*Itr) == Child)
				{
					Result = true;
					Match = Itr;
					break;
				}
			}

			return Result;
		}

		bool ICodaScriptSyntaxTreeNode::Attach( ICodaScriptSyntaxTreeNode* NewParent )
		{
			bool Result = false;

			SME_ASSERT(NewParent && NewParent != this);

			if (Parent != NewParent)
			{
				Detach();

				Parent = NewParent;
				Parent->Children.push_back(this);

				Result = true;
			}

			return Result;
		}

		void ICodaScriptSyntaxTreeNode::Detach( bool UpdateParent )
		{
			if (Parent)
			{
				CodaScriptSyntaxTreeNodeListT::iterator Match;
				bool Result = Parent->LookupChild(this, Match);
				SME_ASSERT(Result);

				if (UpdateParent)
					Parent->Children.erase(Match);

				Parent = NULL;
			}
		}

		void ICodaScriptSyntaxTreeNode::Purge( void )
		{
			for (CodaScriptSyntaxTreeNodeListT::iterator Itr = Children.begin(); Itr != Children.end(); Itr++)
				delete (*Itr);

			Children.clear();
		}

		ICodaScriptSyntaxTreeNode::ICodaScriptSyntaxTreeNode( ICodaScriptSyntaxTreeNode* BiologicalParent /*= NULL*/ ) :
			Parent(NULL), Children()
		{
			if (BiologicalParent)
				Attach(BiologicalParent);
		}

		ICodaScriptSyntaxTreeNode::~ICodaScriptSyntaxTreeNode()
		{
			Purge();
			Detach(false);
		}

		inline bool ICodaScriptSyntaxTreeNode::GetIsRoot( void ) const
		{
			return (Parent == NULL);
		}

		inline bool ICodaScriptSyntaxTreeNode::GetIsLeaf( void ) const
		{
			return (Children.size() == 0);
		}

		inline void ICodaScriptSyntaxTreeNode::Traverse( ICodaScriptSyntaxTreeVisitor* Visitor )
		{
			for (CodaScriptSyntaxTreeNodeListT::iterator Itr = Children.begin(); Itr != Children.end(); Itr++)
				(*Itr)->Accept(Visitor);
		}

		ICodaScriptSyntaxTreeEvaluator::ICodaScriptSyntaxTreeEvaluator(CodaScriptVM* VM,
																	CodaScriptExecutionContext* Context,
																	ICodaScriptExpressionParser* Parser) :
			VirtualMachine(VM),
			ScriptContext(Context),
			ParserAgent(Parser)
		{
			SME_ASSERT(VM && Context && Parser);
		}

		ICodaScriptSyntaxTreeEvaluator::~ICodaScriptSyntaxTreeEvaluator()
		{
			VirtualMachine = NULL;
			ScriptContext = NULL;
			ParserAgent = NULL;
		}

		CodaScriptVariable* ICodaScriptSyntaxTreeEvaluator::LookupVariable( const char* Name )
		{
			return ScriptContext->GetVariable(Name);
		}

		CodaScriptVM* ICodaScriptSyntaxTreeEvaluator::GetVM( void ) const
		{
			return VirtualMachine;
		}

		CodaScriptExecutionContext* ICodaScriptSyntaxTreeEvaluator::GetContext( void ) const
		{
			return ScriptContext;
		}

		ICodaScriptExpressionParser* ICodaScriptSyntaxTreeEvaluator::GetParser( void ) const
		{
			return ParserAgent;
		}

		bool ICodaScriptParseTree::GetKeywordInStack( CodaScriptKeywordStackT& Stack, CodaScriptKeywordT Keyword )
		{
			CodaScriptKeywordStackT StackBuffer(Stack);

			while (StackBuffer.size())
			{
				if (StackBuffer.top() == Keyword)
					return true;
				else
					StackBuffer.pop();
			}

			return false;
		}

		bool ICodaScriptParseTree::GetKeywordOnStackTop( CodaScriptKeywordStackT& Stack, CodaScriptKeywordT Keyword )
		{
			if (Stack.size() && Stack.top() == Keyword)
				return true;
			else
				return false;
		}

		ICodaScriptParseTree::ICodaScriptParseTree(CodaScriptVM* VM) :
			ICodaScriptSyntaxTreeNode(NULL),
			VirtualMachine(VM)
		{
			SME_ASSERT(VirtualMachine);
		}

		ICodaScriptParseTree::~ICodaScriptParseTree()
		{
			;//
		}

		inline void ICodaScriptParseTree::Accept( ICodaScriptSyntaxTreeVisitor* Visitor )
		{
			SME_ASSERT(Visitor);
			this->Traverse(Visitor);
		}

		int						ICodaScriptExecutableCode::GIC = 0;

		const char*				ICodaScriptExecutableCode::kTypeIDs[] =
		{
			"INVALID",
			"EXPR",
			"BEGIN",
			"IF",
			"ELSEIF",
			"ELSE",
			"WHILE",
			"FOREACH"
		};

		ICodaScriptExecutableCode::ICodaScriptExecutableCode() :
			Type(kCodeType_Default_INVALID), Line(0), Source(""), ByteCode(NULL)
		{
			GIC++;
		}

		ICodaScriptExecutableCode::~ICodaScriptExecutableCode()
		{
			GIC--;
			SME_ASSERT(GIC >= 0);

			SAFEDELETE(ByteCode);
		}

		UInt8 ICodaScriptExecutableCode::GetType() const
		{
			return Type;
		}

		const CodaScriptSourceCodeT& ICodaScriptExecutableCode::GetCode() const
		{
			return Source;
		}

		const char* ICodaScriptExecutableCode::GetTypeString() const
		{
			SME_ASSERT(Type < kCodeType__MAX);
			return kTypeIDs[Type];
		}

		UInt32 ICodaScriptExecutableCode::GetLine() const
		{
			return Line;
		}

		ICodaScriptCodeBlock::~ICodaScriptCodeBlock()
		{
			;//
		}

		bool ICodaScriptConditionalCodeBlock::EvaluateCondition( ICodaScriptSyntaxTreeEvaluator* Context )
		{
			SME_ASSERT(ByteCode && Context);

			CodaScriptBackingStore Result;
			Context->GetParser()->Evaluate(Context, ByteCode, &Result);

			return Result.GetNumber();
		}

		ICodaScriptConditionalCodeBlock::ICodaScriptConditionalCodeBlock() :
			ICodaScriptCodeBlock(), Condition("")
		{
			;//
		}

		ICodaScriptConditionalCodeBlock::~ICodaScriptConditionalCodeBlock()
		{
			;//
		}

		const CodaScriptSourceCodeT& ICodaScriptConditionalCodeBlock::GetCode() const
		{
			return Condition;
		}

		void ICodaScriptLoopBlock::BeginLooping( CodaScriptSyntaxTreeExecuteVisitor* Context )
		{
			SME_ASSERT(Context);
			Context->ExecutingLoops.push(this);
			State = kLoopState_Default;
		}

		void ICodaScriptLoopBlock::EndLooping( CodaScriptSyntaxTreeExecuteVisitor* Context )
		{
			SME_ASSERT(Context && Context->GetCurrentLoop() == this);
			Context->ExecutingLoops.pop();
			State = kLoopState_Default;
		}

		ICodaScriptLoopBlock::ICodaScriptLoopBlock() :
			ICodaScriptConditionalCodeBlock(), State(kLoopState_Default)
		{
			;//
		}

		ICodaScriptLoopBlock::~ICodaScriptLoopBlock()
		{
			State = kLoopState_Default;
		}

		inline void ICodaScriptLoopBlock::Break()
		{
			State = kLoopState_Break;
		}

		ICodaScriptLoopBlock::LoopStackOperator::LoopStackOperator( ICodaScriptLoopBlock* Source, CodaScriptSyntaxTreeExecuteVisitor* Visitor ) :
			LoopBlock(Source),
			Context(Visitor)
		{
			SME_ASSERT(Source && Visitor);
			LoopBlock->BeginLooping(Context);
			LoopBlock->State = ICodaScriptLoopBlock::kLoopState_Default;
		}

		ICodaScriptLoopBlock::LoopStackOperator::~LoopStackOperator()
		{
			Reset();
		}

		void ICodaScriptLoopBlock::LoopStackOperator::Reset( void )
		{
			LoopBlock->EndLooping(Context);
		}

		CodaScriptExpression::CodaScriptExpression( CodaScriptSourceCodeT& Source, UInt32 Line ) :
			ICodaScriptObject(), ICodaScriptExecutableCode()
		{
			this->Type = kCodeType_Line_EXPRESSION;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
				CodaScriptTokenizer::kSanitizeOps_StripComments |
				CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
				CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
		}

		CodaScriptExpression::~CodaScriptExpression()
		{
			SME_ASSERT(GetIsLeaf() == true);
		}

		void CodaScriptExpression::Accept( ICodaScriptSyntaxTreeVisitor* Visitor )
		{
			Visitor->Visit(this);
		}

		CodaScriptBEGINBlock::CodaScriptBEGINBlock( CodaScriptSourceCodeT& Source, UInt32 Line ) :
			ICodaScriptObject(), ICodaScriptUnconditionalCodeBlockT()
		{
			this->Type = kCodeType_Block_BEGIN;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
				CodaScriptTokenizer::kSanitizeOps_StripComments |
				CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
				CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
		}

		CodaScriptBEGINBlock::~CodaScriptBEGINBlock()
		{
			;//
		}

		void CodaScriptBEGINBlock::Accept( ICodaScriptSyntaxTreeVisitor* Visitor )
		{
			this->Traverse(Visitor);
		}

		CodaScriptIFBlock::CodaScriptIFBlock( CodaScriptSourceCodeT& Source, UInt32 Line ) :
			ICodaScriptObject(), ICodaScriptConditionalCodeBlock(), BranchELSE(NULL), BranchELSEIF()
		{
			this->Type = kCodeType_Block_IF;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
				CodaScriptTokenizer::kSanitizeOps_StripComments |
				CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
				CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);

			CodaScriptSourceCodeT ConditionExpression;
			if (Tokenizer.Tokenize(this->Source, false))
			{
				ConditionExpression = this->Source.substr(Tokenizer.Indices[0] + Tokenizer.Tokens[0].length());		// everything following the first token
				Tokenizer.Sanitize(ConditionExpression, this->Condition,
					CodaScriptTokenizer::kSanitizeOps_StripComments |
					CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
			}
		}

		CodaScriptIFBlock::~CodaScriptIFBlock()
		{
			SAFEDELETE(BranchELSE);

			for (ElseIfBlockListT::iterator Itr = BranchELSEIF.begin(); Itr != BranchELSEIF.end(); Itr++)
				delete (*Itr);
		}

		void CodaScriptIFBlock::Accept( ICodaScriptSyntaxTreeVisitor* Visitor )
		{
			Visitor->Visit(this);
		}

		CodaScriptELSEIFBlock::CodaScriptELSEIFBlock( CodaScriptSourceCodeT& Source, UInt32 Line ) :
			ICodaScriptObject(), ICodaScriptConditionalCodeBlock()
		{
			this->Type = kCodeType_Block_ELSEIF;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
				CodaScriptTokenizer::kSanitizeOps_StripComments |
				CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
				CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);

			CodaScriptSourceCodeT ConditionExpression;
			if (Tokenizer.Tokenize(this->Source, false))
			{
				ConditionExpression = this->Source.substr(Tokenizer.Indices[0] + Tokenizer.Tokens[0].length());		// everything following the first token
				Tokenizer.Sanitize(ConditionExpression, this->Condition,
					CodaScriptTokenizer::kSanitizeOps_StripComments |
					CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
			}
		}

		CodaScriptELSEIFBlock::~CodaScriptELSEIFBlock()
		{
			;//
		}

		void CodaScriptELSEIFBlock::Accept( ICodaScriptSyntaxTreeVisitor* Visitor )
		{
			Visitor->Visit(this);
		}

		CodaScriptELSEBlock::CodaScriptELSEBlock( CodaScriptSourceCodeT& Source, UInt32 Line ) :
			ICodaScriptObject(), ICodaScriptUnconditionalCodeBlockT()
		{
			this->Type = kCodeType_Block_ELSE;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
				CodaScriptTokenizer::kSanitizeOps_StripComments |
				CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
				CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
		}

		CodaScriptELSEBlock::~CodaScriptELSEBlock()
		{
			;//
		}

		void CodaScriptELSEBlock::Accept( ICodaScriptSyntaxTreeVisitor* Visitor )
		{
			this->Traverse(Visitor);
		}

		CodaScriptWHILEBlock::CodaScriptWHILEBlock( CodaScriptSourceCodeT& Source, UInt32 Line ) :
			ICodaScriptObject(), ICodaScriptLoopBlock()
		{
			this->Type = kCodeType_Loop_WHILE;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
				CodaScriptTokenizer::kSanitizeOps_StripComments |
				CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
				CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);

			CodaScriptSourceCodeT ConditionExpression;
			if (Tokenizer.Tokenize(this->Source, false))
			{
				ConditionExpression = this->Source.substr(Tokenizer.Indices[0] + Tokenizer.Tokens[0].length());		// everything following the first token
				Tokenizer.Sanitize(ConditionExpression, this->Condition,
					CodaScriptTokenizer::kSanitizeOps_StripComments |
					CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
			}
		}

		CodaScriptWHILEBlock::~CodaScriptWHILEBlock()
		{
			;//
		}

		void CodaScriptWHILEBlock::Accept( ICodaScriptSyntaxTreeVisitor* Visitor )
		{
			Visitor->Visit(this);
		}

		CodaScriptFOREACHBlock::CodaScriptFOREACHBlock( CodaScriptSourceCodeT& Source, UInt32 Line ) :
			ICodaScriptObject(), ICodaScriptLoopBlock(), IteratorName("")
		{
			this->Type = kCodeType_Loop_FOREACH;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
				CodaScriptTokenizer::kSanitizeOps_StripComments |
				CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
				CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);

			CodaScriptSourceCodeT ConditionExpression;
			if (Tokenizer.Tokenize(this->Source, false))
			{
				this->IteratorName = Tokenizer.Tokens[1];		// second token

				ConditionExpression = this->Source.substr(Tokenizer.Indices[2] + Tokenizer.Tokens[2].length());		// everything past the third token (<-)
				Tokenizer.Sanitize(ConditionExpression, this->Condition,
					CodaScriptTokenizer::kSanitizeOps_StripComments |
					CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
			}
		}

		CodaScriptFOREACHBlock::~CodaScriptFOREACHBlock()
		{
			;//
		}

		void CodaScriptFOREACHBlock::Accept( ICodaScriptSyntaxTreeVisitor* Visitor )
		{
			Visitor->Visit(this);
		}

		const UInt32		CodaScriptExecutionContext::kMaxParameters = 10;

		CodaScriptVariable* CodaScriptExecutionContext::AddVariable( CodaScriptSourceCodeT& Name )
		{
			if (GetVariable(Name) == NULL)
			{
				CodaScriptVariable* Addend = new CodaScriptVariable(Name,
																	CodaScriptObjectFactory::BuildDataStoreOwner(
																	CodaScriptObjectFactory::kFactoryType_MUP));
				Variables.push_back(Addend);
				return Addend;
			}

			return NULL;
		}

		void CodaScriptExecutionContext::SetVariable( CodaScriptSourceCodeT& Name, CodaScriptBackingStore& Value )
		{
			CodaScriptVariable* Var = GetVariable(Name);
			if (Var)
			{
				ICodaScriptDataStore* DataStore = Var->GetStoreOwner()->GetDataStore();
				SME_ASSERT(DataStore);

				*DataStore = Value;
			}
		}

		CodaScriptVariable* CodaScriptExecutionContext::GetVariable( CodaScriptSourceCodeT& Name )
		{
			return GetVariable(Name.c_str());
		}

		CodaScriptVariable* CodaScriptExecutionContext::GetVariable( const char* Name )
		{
			for (CodaScriptVariableListT::iterator Itr = Variables.begin(); Itr != Variables.end(); Itr++)
			{
				if (!_stricmp(Name, (*Itr)->GetName()))
					return *Itr;
			}

			return NULL;
		}

		void CodaScriptExecutionContext::ReleaseVariables( void )
		{
			for (CodaScriptVariableListT::iterator Itr = Variables.begin(); Itr != Variables.end(); Itr++)
				delete *Itr;
		}

		CodaScriptExecutionContext::CodaScriptExecutionContext( CodaScriptVM* VirtualMachine,
																std::fstream& SourceCode,
																ICodaScriptExpressionParser* ExpressionParser,
																CodaScriptMutableDataArrayT* Parameters ) :
			ICodaScriptObject(),
			ICodaScriptParseTree(VirtualMachine),
			ScriptName("<unknown>"),
			Variables(),
			Validity(kValidity_Unknown),
			BoundParser(ExpressionParser),
			PollingInterval(0.0),
			PollingIntervalReminder(0.0),
			ElapsedTimeCounter()
		{
			SME_ASSERT(VirtualMachine && ExpressionParser);

			char Buffer[0x512] = {0};
			UInt32 CurrentLine = 0;
			bool Result = true;
			CodaScriptSimpleInstanceCounter<ICodaScriptExecutableCode> CodeInstanceCounter;

			CodaScriptTokenizer Tokenizer;
			CodaScriptKeywordStackT	BlockStack;
			CodaScriptExecutableCodeStackT CodeStack;

			BlockStack.push(CodaScriptTokenizer::kTokenType_Invalid);
			CodeStack.push(NULL);

			while (SourceCode.eof() == false)
			{
				ZeroMemory(Buffer, sizeof(Buffer));
				SourceCode.getline(Buffer, sizeof(Buffer));
				CodaScriptSourceCodeT SourceLine(Buffer);

				if (Tokenizer.Tokenize(Buffer, false))
				{
					CodaScriptSourceCodeT FirstToken(Tokenizer.Tokens[0]);
					CodaScriptSourceCodeT SecondToken((Tokenizer.GetParsedTokenCount() > 1) ? Tokenizer.Tokens[1] : "");

					if (CurrentLine == 1)
					{
						if (Tokenizer.GetFirstTokenKeywordType() == CodaScriptTokenizer::kTokenType_ScriptName)
						{
							if (SecondToken != "")
							{
								this->ScriptName = SecondToken;

								if (Tokenizer.GetParsedTokenCount() >= 3)
								{
									CodaScriptSourceCodeT ThirdToken(Tokenizer.Tokens[2]);
									double DeclaredInteraval = atof(ThirdToken.c_str());
									if (DeclaredInteraval > 0)
										this->PollingInterval = DeclaredInteraval;
								}
							}
							else
							{
								VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid script name", CurrentLine);
								Result = false;
							}
						}
						else
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Scripts should start with a script name declaration", CurrentLine);
							Result = false;
						}
					}

					CodaScriptKeywordT FirstKeyword = Tokenizer.GetFirstTokenKeywordType();
					switch (FirstKeyword)
					{
					case CodaScriptTokenizer::kTokenType_Variable:
						if (BlockStack.top() != CodaScriptTokenizer::kTokenType_Invalid)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Variable '%s' declared inside a script block", CurrentLine, SecondToken.c_str());
							Result = false;
						}
						else if (GetVariable(SecondToken))
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Variable '%s' redeclaration", CurrentLine, SecondToken.c_str());
							Result = false;
						}
						else if (SecondToken == "" || SME::StringHelpers::GetHasNonAlnumCharacter(SecondToken))
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid variable name - Only alphabets and digits allowed", CurrentLine);
							Result = false;
						}
						else if (VirtualMachine->GetGlobal(SecondToken.c_str()))
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Scope conflict - Global variable '%s' exists", CurrentLine, SecondToken.c_str());
							Result = false;
						}
						else
						{
							CodaScriptVariable* NewVar = AddVariable(SecondToken);

							if (Tokenizer.GetParsedTokenCount() > 3 && !strcmp(Tokenizer.Tokens[2].c_str(), "="))
							{
								CodaScriptSourceCodeT LineBuffer(Buffer);
								CodaScriptSourceCodeT InitializationValue = LineBuffer.substr(Tokenizer.Indices[3]);

								if (InitializationValue.find("\"") == 0 && InitializationValue.length() > 1)
								{
									InitializationValue.erase(InitializationValue.begin());
									InitializationValue.erase(InitializationValue.begin() + InitializationValue.length() - 1);

									NewVar->GetStoreOwner()->GetDataStore()->SetString(InitializationValue.c_str());
								}
								else
									NewVar->GetStoreOwner()->GetDataStore()->SetNumber(atof(InitializationValue.c_str()));
							}
						}

						break;
					case CodaScriptTokenizer::kTokenType_Begin:
						if (BlockStack.top() != CodaScriptTokenizer::kTokenType_Invalid)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Nested Begin block", CurrentLine);
							Result = false;
						}
						else if (GetIsLeaf() == false)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Multiple Begin blocks", CurrentLine);
							Result = false;
						}
						else
						{
							std::vector<CodaScriptSourceCodeT> ParameterIDs;

							for (CodaScriptTokenizer::ParsedTokenListT::iterator Itr = ++(Tokenizer.Tokens.begin()); Itr != Tokenizer.Tokens.end(); Itr++)
								ParameterIDs.push_back(*Itr);

							if (Parameters)
							{
								if (ParameterIDs.size() > kMaxParameters)
								{
									VirtualMachine->MsgHdlr()->LogMsg("Line %d: Too many parameters passed", CurrentLine);
									Result = false;
								}
								else if (Parameters->size() != ParameterIDs.size())
								{
									VirtualMachine->MsgHdlr()->LogMsg("Line %d: Incorrect number of parameters passed - Received %d, expected %d", CurrentLine, Parameters->size(), ParameterIDs.size());
									Result = false;
								}
								else
								{
									for (int i = 0; i < Parameters->size(); i++)
									{
										CodaScriptVariable* ParamVar = GetVariable(ParameterIDs[i]);
										if (ParamVar)
										{
											*ParamVar->GetStoreOwner()->GetDataStore() = Parameters->at(i);
										}
										else
										{
											VirtualMachine->MsgHdlr()->LogMsg("Line %d: Couldn't initialize parameter '%s' - Non-existent variable", CurrentLine, ParameterIDs[i]);
											Result = false;
										}
									}
								}
							}
							else if (ParameterIDs.size())
							{
								VirtualMachine->MsgHdlr()->LogMsg("Line %d: Incorrect number of parameters passed - Received %d, expected %d", CurrentLine, 0, ParameterIDs.size());
								Result = false;
							}

							BlockStack.push(CodaScriptTokenizer::kTokenType_Begin);
							if (Result == false)
								break;

							ICodaScriptExecutableCode* NewNode = new CodaScriptBEGINBlock(SourceLine, CurrentLine);
							NewNode->Attach(this);
							CodeStack.push(NewNode);
						}

						break;
					case CodaScriptTokenizer::kTokenType_End:
						if (BlockStack.top() != CodaScriptTokenizer::kTokenType_Begin)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid block structure. Command 'End' has no matching 'Begin'", CurrentLine);
							Result = false;
						}
						else
						{
							if (CodeStack.size())
								CodeStack.pop();

							if (BlockStack.size())
								BlockStack.pop();
						}

						break;
					case CodaScriptTokenizer::kTokenType_While:
						if (BlockStack.top() == CodaScriptTokenizer::kTokenType_Invalid)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid block structure. Script commands must be inside a 'Begin' block", CurrentLine);
							Result = false;
						}
						else if (Tokenizer.GetParsedTokenCount() < 2)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid condition expression", CurrentLine);
							Result = false;
						}
						else
						{
							BlockStack.push(CodaScriptTokenizer::kTokenType_Loop);
							if (Result == false)
								break;

							ICodaScriptExecutableCode* NewNode = new CodaScriptWHILEBlock(SourceLine, CurrentLine);
							ICodaScriptExecutableCode* TopNode = CodeStack.top();
							SME_ASSERT(TopNode && TopNode->GetType() != ICodaScriptExecutableCode::kCodeType_Line_EXPRESSION);
							NewNode->Attach(TopNode);
							CodeStack.push(NewNode);
						}

						break;
					case CodaScriptTokenizer::kTokenType_ForEach:
						if (BlockStack.top() == CodaScriptTokenizer::kTokenType_Invalid)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid block structure. Script commands must be inside a 'Begin' block", CurrentLine);
							Result = false;
						}
						else if (Tokenizer.GetParsedTokenCount() < 4)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid ForEach expression", CurrentLine);
							Result = false;
						}
						else
						{
							BlockStack.push(CodaScriptTokenizer::kTokenType_Loop);
							if (Result == false)
								break;

							ICodaScriptExecutableCode* NewNode = new CodaScriptFOREACHBlock(SourceLine, CurrentLine);
							ICodaScriptExecutableCode* TopNode = CodeStack.top();
							SME_ASSERT(TopNode && TopNode->GetType() != ICodaScriptExecutableCode::kCodeType_Line_EXPRESSION);
							NewNode->Attach(TopNode);
							CodeStack.push(NewNode);
						}

						break;
					case CodaScriptTokenizer::kTokenType_Loop:
						if (BlockStack.top() != CodaScriptTokenizer::kTokenType_Loop)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid block structure. Command 'Loop' has no matching 'While' or 'ForEach'", CurrentLine);
							Result = false;
						}
						else
						{
							if (CodeStack.size())
								CodeStack.pop();

							if (BlockStack.size())
								BlockStack.pop();
						}

						break;
					case CodaScriptTokenizer::kTokenType_If:
						if (BlockStack.top() == CodaScriptTokenizer::kTokenType_Invalid)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid block structure. Script commands must be inside a 'Begin' block", CurrentLine);
							Result = false;
						}
						else if (Tokenizer.GetParsedTokenCount() < 2)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid condition expression", CurrentLine);
							Result = false;
						}
						else
						{
							BlockStack.push(CodaScriptTokenizer::kTokenType_If);
							if (Result == false)
								break;

							ICodaScriptExecutableCode* NewNode = new CodaScriptIFBlock(SourceLine, CurrentLine);
							ICodaScriptExecutableCode* TopNode = CodeStack.top();
							SME_ASSERT(TopNode && TopNode->GetType() != ICodaScriptExecutableCode::kCodeType_Line_EXPRESSION);
							NewNode->Attach(TopNode);
							CodeStack.push(NewNode);
						}

						break;
					case CodaScriptTokenizer::kTokenType_ElseIf:
					case CodaScriptTokenizer::kTokenType_Else:
						if (BlockStack.top() != CodaScriptTokenizer::kTokenType_If &&
							BlockStack.top() != (CodaScriptTokenizer::kTokenType_If|CodaScriptTokenizer::kTokenType_ElseIf))
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid block structure. Command 'Else(If)' has no/incorrectly matching/matched 'If'/'Else(If)'", CurrentLine);
							Result = false;
						}
						else if (FirstKeyword == CodaScriptTokenizer::kTokenType_ElseIf && Tokenizer.GetParsedTokenCount() < 2)
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid condition expression", CurrentLine);
							Result = false;
						}
						else
						{
							if (BlockStack.top() == CodaScriptTokenizer::kTokenType_If)
							{
								BlockStack.pop();		// pop IF
								BlockStack.push(CodaScriptTokenizer::kTokenType_If|CodaScriptTokenizer::kTokenType_ElseIf);

								if (Result == false)
									break;
							}
							else
							{
								if (Result == false)
									break;

								CodeStack.pop();		// pop previous ELSE(IF)
							}

							ICodaScriptExecutableCode* NewNode = NULL;
							ICodaScriptExecutableCode* TopNode = CodeStack.top();
							SME_ASSERT(TopNode && TopNode->GetType() == ICodaScriptExecutableCode::kCodeType_Block_IF);
							CodaScriptIFBlock* IFBlock = dynamic_cast<CodaScriptIFBlock*>(TopNode);

							if (FirstKeyword == CodaScriptTokenizer::kTokenType_ElseIf)
							{
								CodaScriptELSEIFBlock* ElseIfBlock = new CodaScriptELSEIFBlock(SourceLine, CurrentLine);
								NewNode = ElseIfBlock;
								IFBlock->BranchELSEIF.push_back(ElseIfBlock);
							}
							else if (IFBlock->BranchELSE == NULL)
							{
								NewNode = IFBlock->BranchELSE = new CodaScriptELSEBlock(SourceLine, CurrentLine);
							}
							else
							{
								VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid block structure. Multiple 'Else' blocks", CurrentLine);
								Result = false;
							}

							if (Result)
								CodeStack.push(NewNode);
						}

						break;
					case CodaScriptTokenizer::kTokenType_EndIf:
						if (BlockStack.top() != CodaScriptTokenizer::kTokenType_If &&
							BlockStack.top() != (CodaScriptTokenizer::kTokenType_If|CodaScriptTokenizer::kTokenType_ElseIf))
						{
							VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid block structure. Command 'EndIf' has no/incorrectly matching/matched 'If'/'Else(If)'", CurrentLine);
							Result = false;
						}
						else
						{
							if (CodeStack.size())
								CodeStack.pop();

							if (BlockStack.size())
								BlockStack.pop();
						}

						break;
					default:
						{
							switch (FirstKeyword)
							{
							case CodaScriptTokenizer::kTokenType_Return:
								TODO("Remember to unwind the loop stack in the command handler")
								break;
							case CodaScriptTokenizer::kTokenType_Break:
							case CodaScriptTokenizer::kTokenType_Continue:
								if (!GetKeywordInStack(BlockStack, CodaScriptTokenizer::kTokenType_Loop))
								{
									VirtualMachine->MsgHdlr()->LogMsg("Line %d: Command 'Break'/'Loop' called outside a loop context", CurrentLine);
									Result = false;
								}

								break;
							}

							if (BlockStack.top() == CodaScriptTokenizer::kTokenType_Invalid)
							{
								VirtualMachine->MsgHdlr()->LogMsg("Line %d: Invalid block structure. Script commands must be inside a 'Begin' block", CurrentLine);
								Result = false;
							}

							if (Result == false)
								break;

							ICodaScriptExecutableCode* NewNode = new CodaScriptExpression(SourceLine, CurrentLine);
							ICodaScriptExecutableCode* TopNode = CodeStack.top();
							SME_ASSERT(TopNode && TopNode->GetType() != ICodaScriptExecutableCode::kCodeType_Line_EXPRESSION);

							NewNode->Attach(TopNode);
						}

						break;
					}
				}

				CurrentLine++;
			}

			if (Result == false)
			{
				this->Purge();
				this->ReleaseVariables();
				this->Validity = kValidity_Egregious;

				SME_ASSERT(CodeInstanceCounter.GetCount() == 0);						// we're leaking otherwise
			}
			else
			{
				SME_ASSERT(this->Children.size() == 1);
				SME_ASSERT(BlockStack.size() == 1 && BlockStack.top() == CodaScriptTokenizer::kTokenType_Invalid);
				SME_ASSERT(CodeStack.size() == 1 && CodeStack.top() == NULL);

				BoundParser->RegisterVariables(this, VirtualMachine->GetGlobals());		// registers global variables first
				BoundParser->RegisterVariables(this, this->Variables);					// then the local vars

				// compile source to bytecode
				BGSEECONSOLE->Indent();
				CodaScriptSyntaxTreeCompileVisitor Visitor(VirtualMachine, this, BoundParser);
				this->Accept(&Visitor);
				BGSEECONSOLE->Exdent();
			}
		}

		CodaScriptExecutionContext::~CodaScriptExecutionContext()
		{
			if (BoundParser)
				BoundParser->UnregisterVariables(this);

			ReleaseVariables();
			Validity = kValidity_Unknown;
		}

		inline bool CodaScriptExecutionContext::GetIsValid( void ) const
		{
			return (Validity == kValidity_Good);
		}

		bool CodaScriptExecutionContext::Execute( CodaScriptSyntaxTreeExecuteVisitor* Agent,
												CodaScriptBackingStore* Result,
												bool& ReturnedResult )
		{
			SME_ASSERT(Agent && GetIsValid() == true);

			bool ExecuteResult = true;
			ReturnedResult = false;

			this->Accept(Agent);

			if (Agent->GetState() == CodaScriptSyntaxTreeExecuteVisitor::kExecutionState_Terminate)
				ExecuteResult = false;

			if (Agent->GetResult())
			{
				ReturnedResult = true;
				*Result = *Agent->GetResult();
			}

			return ExecuteResult;
		}

		CodaScriptSyntaxTreeCompileVisitor::CodaScriptSyntaxTreeCompileVisitor( CodaScriptVM* VM,
																				CodaScriptExecutionContext* Context,
																				ICodaScriptExpressionParser* Parser ) :
			ICodaScriptObject(),
			ICodaScriptSyntaxTreeEvaluator(VM, Context, Parser)
		{
			;//
		}

		CodaScriptSyntaxTreeCompileVisitor::~CodaScriptSyntaxTreeCompileVisitor()
		{
			;//
		}

#define CODASCRIPT_COMPILERHNDLR_PROLOG										\
			try																\
			{
#define CODASCRIPT_COMPILERHNDLR_EPILOG										\
				return;														\
			}
#define CODASCRIPT_COMPILERERROR_CATCHER									\
			catch (CodaScriptException& E)									\
			{																\
				VirtualMachine->MsgHdlr()->LogMsg("Compiler Error - %s", E.Get());		\
			}																\
			catch (std::exception& E)										\
			{																\
				VirtualMachine->MsgHdlr()->LogMsg("Compiler Error - %s", E.what());		\
			}																\
			catch (...)														\
			{																\
				VirtualMachine->MsgHdlr()->LogMsg("Unknown Compiler Error");				\
			}																\
			ScriptContext->Validity =										\
			CodaScriptExecutionContext::kValidity_Egregious;				\
			return;

		void CodaScriptSyntaxTreeCompileVisitor::Visit( CodaScriptExpression* Node )
		{
			GenericCompile(Node);
		}

		void CodaScriptSyntaxTreeCompileVisitor::Visit( CodaScriptBEGINBlock* Node )
		{
			;//
		}

		void CodaScriptSyntaxTreeCompileVisitor::Visit( CodaScriptIFBlock* Node )
		{
			CODASCRIPT_COMPILERHNDLR_PROLOG

			ParserAgent->Compile(this, Node, &Node->ByteCode);

			if (Node->BranchELSE)
				Node->Accept(this);

			for (CodaScriptIFBlock::ElseIfBlockListT::iterator Itr = Node->BranchELSEIF.begin(); Itr != Node->BranchELSEIF.end(); Itr++)
				(*Itr)->Accept(this);

			Node->Traverse(this);

			CODASCRIPT_COMPILERHNDLR_EPILOG

			CODASCRIPT_COMPILERERROR_CATCHER
		}

		void CodaScriptSyntaxTreeCompileVisitor::Visit( CodaScriptELSEIFBlock* Node )
		{
			GenericCompile(Node);
		}

		void CodaScriptSyntaxTreeCompileVisitor::Visit( CodaScriptELSEBlock* Node )
		{
			;//
		}

		void CodaScriptSyntaxTreeCompileVisitor::Visit( CodaScriptWHILEBlock* Node )
		{
			GenericCompile(Node);
		}

		void CodaScriptSyntaxTreeCompileVisitor::Visit( CodaScriptFOREACHBlock* Node )
		{
			GenericCompile(Node);
		}

		void CodaScriptSyntaxTreeCompileVisitor::Visit( ICodaScriptParseTree* Node )
		{
			;//
		}

		void CodaScriptSyntaxTreeCompileVisitor::GenericCompile( ICodaScriptExecutableCode* Node )
		{
			CODASCRIPT_COMPILERHNDLR_PROLOG

			ParserAgent->Compile(this, Node, &Node->ByteCode);
			Node->Traverse(this);

			CODASCRIPT_COMPILERHNDLR_EPILOG

			CODASCRIPT_COMPILERERROR_CATCHER
		}

		CodaScriptException::CodaScriptException( ICodaScriptExecutableCode* Source, const char* Message, ... ) :
			ICodaScriptObject()
		{
			ZeroMemory(ErrorString, sizeof(ErrorString));
			char Buffer[0x512] = {0};

			va_list Args;
			va_start(Args, Message);
			vsprintf_s(Buffer, sizeof(Buffer), Message, Args);
			va_end(Args);

			sprintf_s(ErrorString, sizeof(ErrorString), "L[%d] T[%s] - %s", Source->GetLine(), Source->GetTypeString(), Buffer);
		}

		CodaScriptException::~CodaScriptException()
		{
			;//
		}

		const char* CodaScriptException::Get() const
		{
			return ErrorString;
		}

		void CodaScriptSyntaxTreeExecuteVisitor::SetResult( CodaScriptBackingStore Value )
		{
			SME_ASSERT(Result == NULL);

			Result = new CodaScriptBackingStore(Value);
		}

		CodaScriptSyntaxTreeExecuteVisitor::CodaScriptSyntaxTreeExecuteVisitor( CodaScriptVM* VM,
																				CodaScriptExecutionContext* Context,
																				ICodaScriptExpressionParser* Parser ) :
			ICodaScriptObject(),
			ICodaScriptSyntaxTreeEvaluator(VM, Context, Parser),
			Result(NULL),
			ExecutingLoops(),
			ExecutionState(kExecutionState_Default)
		{
			;//
		}

		CodaScriptSyntaxTreeExecuteVisitor::~CodaScriptSyntaxTreeExecuteVisitor()
		{
			SAFEDELETE(Result);

			if (ExecutionState != kExecutionState_Terminate)
				SME_ASSERT(ExecutingLoops.size() == 0);
			else
			{
				while (ExecutingLoops.size())
					ExecutingLoops.pop();
			}
		}

		inline UInt8 CodaScriptSyntaxTreeExecuteVisitor::GetState( void ) const
		{
			return ExecutionState;
		}

		inline void CodaScriptSyntaxTreeExecuteVisitor::SetState( UInt8 State )
		{
			ExecutionState = State;
		}

		ICodaScriptLoopBlock* CodaScriptSyntaxTreeExecuteVisitor::GetCurrentLoop( void ) const
		{
			if (ExecutingLoops.size())
				return ExecutingLoops.top();
			else
				return NULL;
		}

		CodaScriptBackingStore* CodaScriptSyntaxTreeExecuteVisitor::GetResult( void ) const
		{
			return Result;
		}

#define CODASCRIPT_EXECUTEHNDLR_PROLOG										\
			try																\
			{																\
				if (ExecutionState == kExecutionState_Terminate ||			\
					ExecutionState == kExecutionState_Break)				\
				{															\
					return;													\
				}
#define CODASCRIPT_EXECUTEHNDLR_EPILOG										\
				return;														\
			}
#define CODASCRIPT_EXECUTEERROR_CATCHER									\
			catch (CodaScriptException& E)									\
			{																\
				VirtualMachine->MsgHdlr()->LogMsg("Evaluate Error - %s", E.Get());		\
			}																\
			catch (std::exception& E)										\
			{																\
				VirtualMachine->MsgHdlr()->LogMsg("Evaluate Error - %s", E.what());		\
			}																\
			catch (...)														\
			{																\
				VirtualMachine->MsgHdlr()->LogMsg("Unknown Evaluate Error");				\
			}																\
			SetState(kExecutionState_Terminate);							\
			ScriptContext->Validity =										\
			CodaScriptExecutionContext::kValidity_Bad;						\
			return;

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( CodaScriptExpression* Node )
		{
			CODASCRIPT_EXECUTEHNDLR_PROLOG

			ParserAgent->Evaluate(this, Node->ByteCode);

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

			if (Node->EvaluateCondition(this))
				Node->Traverse(this);

			for (CodaScriptIFBlock::ElseIfBlockListT::iterator Itr = Node->BranchELSEIF.begin(); Itr != Node->BranchELSEIF.end(); Itr++)
			{
				CodaScriptELSEIFBlock* Block = *Itr;

				if (Block->EvaluateCondition(this))
				{
					Block->Accept(this);
					return;
				}
			}

			if (Node->BranchELSE)
				Node->BranchELSE->Accept(this);

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

			ICodaScriptLoopBlock::LoopStackOperator Operator(Node, this);

			while (Node->EvaluateCondition(this))
			{
				Node->Traverse(this);

				if (ExecutionState == kExecutionState_Terminate)
					break;
				else if (ExecutionState == kExecutionState_Break)
					ExecutionState = kExecutionState_Default;

				if (Node->State == ICodaScriptLoopBlock::kLoopState_Break)
					break;
			}

			CODASCRIPT_EXECUTEHNDLR_EPILOG

			CODASCRIPT_EXECUTEERROR_CATCHER
		}

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( CodaScriptFOREACHBlock* Node )
		{
			CODASCRIPT_EXECUTEHNDLR_PROLOG

			ICodaScriptLoopBlock::LoopStackOperator Operator(Node, this);
			CodaScriptVariable* Iterator = NULL;

			if (Node->IteratorName == "")
				throw CodaScriptException(Node, "Invalid expression - No iterator specified");
			else if ((Iterator = this->LookupVariable(Node->IteratorName.c_str())) == NULL)
				throw CodaScriptException(Node, "Invalid iterator '%s'", Node->IteratorName.c_str());

			CodaScriptBackingStore ArrayResult((CodaScriptNumericDataTypeT)0), IteratorBuffer((CodaScriptNumericDataTypeT)0);
			ParserAgent->Evaluate(this, Node->ByteCode, &ArrayResult);

			if (ArrayResult.GetType() != ICodaScriptDataStore::kDataType_Array)
				throw CodaScriptException(Node, "Invalid expression - Non-array result");

			CodaScriptSharedHandleArrayT ArrayInstance(ArrayResult.GetArray());

			for (int i = 0, j = ArrayInstance->Size(); i < j; i++)
			{
				if (ArrayInstance->At(i, IteratorBuffer) == false)
					throw CodaScriptException(Node, "Index operator error - I[%d] S[%d]", i, ArrayInstance->Size());

				*Iterator->GetStoreOwner() = IteratorBuffer;
				Node->Traverse(this);

				if (ExecutionState == kExecutionState_Terminate)
					break;
				else if (ExecutionState == kExecutionState_Break)
					ExecutionState = kExecutionState_Default;

				if (Node->State == ICodaScriptLoopBlock::kLoopState_Break)
					break;
			}

			CODASCRIPT_EXECUTEHNDLR_EPILOG

			CODASCRIPT_EXECUTEERROR_CATCHER
		}

		void CodaScriptSyntaxTreeExecuteVisitor::Visit( ICodaScriptParseTree* Node )
		{
			;//
		}
	}
}