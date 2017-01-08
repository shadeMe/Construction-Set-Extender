#include "CodaAST.h"
#include "CodaUtilities.h"
#include "CodaInterpreter.h"

namespace bgsee
{
	namespace script
	{
		bool ICodaScriptSyntaxTreeNode::LookupChild(ICodaScriptSyntaxTreeNode* Child, CodaScriptSyntaxTreeNodeArrayT::iterator& Match)
		{
			bool Result = false;

			SME_ASSERT(Child && Child != this && Child != Parent);

			for (CodaScriptSyntaxTreeNodeArrayT::iterator Itr = Children.begin(); Itr != Children.end(); Itr++)
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

		bool ICodaScriptSyntaxTreeNode::Attach(ICodaScriptSyntaxTreeNode* NewParent)
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

		void ICodaScriptSyntaxTreeNode::Detach(bool UpdateParent)
		{
			if (Parent)
			{
				CodaScriptSyntaxTreeNodeArrayT::iterator Match;
				bool Result = Parent->LookupChild(this, Match);
				SME_ASSERT(Result);

				if (UpdateParent)
					Parent->Children.erase(Match);

				Parent = nullptr;
			}
		}

		void ICodaScriptSyntaxTreeNode::Purge(void)
		{
			for (CodaScriptSyntaxTreeNodeArrayT::iterator Itr = Children.begin(); Itr != Children.end(); Itr++)
				delete (*Itr);

			Children.clear();
		}

		ICodaScriptSyntaxTreeNode::ICodaScriptSyntaxTreeNode(ICodaScriptSyntaxTreeNode* BiologicalParent /*= NULL*/) :
			Parent(nullptr), Children()
		{
			if (BiologicalParent)
				Attach(BiologicalParent);
		}

		ICodaScriptSyntaxTreeNode::~ICodaScriptSyntaxTreeNode()
		{
			Purge();
			Detach(false);
		}

		bool ICodaScriptSyntaxTreeNode::IsRoot(void) const
		{
			return (Parent == nullptr);
		}

		bool ICodaScriptSyntaxTreeNode::IsLeaf(void) const
		{
			return (Children.size() == 0);
		}

		void ICodaScriptSyntaxTreeNode::Traverse(ICodaScriptSyntaxTreeVisitor* Visitor)
		{
			for (CodaScriptSyntaxTreeNodeArrayT::iterator Itr = Children.begin(); Itr != Children.end(); Itr++)
				(*Itr)->Accept(Visitor);
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
			Type(kCodeType_Default_INVALID), Line(0), Source(""), ByteCode(nullptr)
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

		const CodaScriptSourceCodeT& ICodaScriptExecutableCode::GetSourceCode() const
		{
			return Source;
		}

		ICodaScriptExpressionByteCode* ICodaScriptExecutableCode::GetByteCode() const
		{
			return ByteCode;
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

		ICodaScriptConditionalCodeBlock::ICodaScriptConditionalCodeBlock() :
			ICodaScriptCodeBlock(), Condition("")
		{
			;//
		}

		const CodaScriptSourceCodeT& ICodaScriptConditionalCodeBlock::GetSourceCode() const
		{
			return Condition;
		}


		ICodaScriptLoopBlock::ICodaScriptLoopBlock() :
			ICodaScriptConditionalCodeBlock()
		{
			;//
		}

		CodaScriptExpression::CodaScriptExpression(CodaScriptSourceCodeT& Source, UInt32 Line) :
			ICodaScriptExecutableCode()
		{
			this->Type = kCodeType_Line_EXPRESSION;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
							   CodaScriptTokenizer::kSanitizeOps_StripComments |
							   CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
							   CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
		}

		void CodaScriptExpression::Accept(ICodaScriptSyntaxTreeVisitor* Visitor)
		{
			Visitor->Visit(this);
		}

		CodaScriptBEGINBlock::CodaScriptBEGINBlock(CodaScriptSourceCodeT& Source, UInt32 Line) :
			ICodaScriptUnconditionalCodeBlockT()
		{
			this->Type = kCodeType_Block_BEGIN;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
							   CodaScriptTokenizer::kSanitizeOps_StripComments |
							   CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
							   CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
		}

		void CodaScriptBEGINBlock::Accept(ICodaScriptSyntaxTreeVisitor* Visitor)
		{
			this->Traverse(Visitor);
		}

		CodaScriptIFBlock::CodaScriptIFBlock(CodaScriptSourceCodeT& Source, UInt32 Line) :
			ICodaScriptConditionalCodeBlock(), BranchELSE(nullptr), BranchELSEIF()
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

			for (ElseIfBlockArrayT::iterator Itr = BranchELSEIF.begin(); Itr != BranchELSEIF.end(); Itr++)
				delete (*Itr);
		}

		void CodaScriptIFBlock::Accept(ICodaScriptSyntaxTreeVisitor* Visitor)
		{
			Visitor->Visit(this);
		}

		const CodaScriptIFBlock::ElseIfBlockArrayT& CodaScriptIFBlock::GetElseIfBlocks() const
		{
			return BranchELSEIF;
		}

		CodaScriptELSEBlock& CodaScriptIFBlock::GetElseBlock() const
		{
			if (HasElseBlock() == false)
				throw CodaScriptException(this, "Invalid ELSE block");

			return *BranchELSE;
		}

		bool CodaScriptIFBlock::HasElseBlock() const
		{
			return BranchELSE != nullptr;
		}

		CodaScriptELSEIFBlock::CodaScriptELSEIFBlock(CodaScriptSourceCodeT& Source, UInt32 Line) :
			ICodaScriptConditionalCodeBlock()
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

		void CodaScriptELSEIFBlock::Accept(ICodaScriptSyntaxTreeVisitor* Visitor)
		{
			Visitor->Visit(this);
		}

		CodaScriptELSEBlock::CodaScriptELSEBlock(CodaScriptSourceCodeT& Source, UInt32 Line) :
			ICodaScriptUnconditionalCodeBlockT()
		{
			this->Type = kCodeType_Block_ELSE;
			this->Line = Line;

			CodaScriptTokenizer Tokenizer;
			Tokenizer.Sanitize(Source, this->Source,
							   CodaScriptTokenizer::kSanitizeOps_StripComments |
							   CodaScriptTokenizer::kSanitizeOps_StripLeadingWhitespace |
							   CodaScriptTokenizer::kSanitizeOps_StripTabCharacters);
		}

		void CodaScriptELSEBlock::Accept(ICodaScriptSyntaxTreeVisitor* Visitor)
		{
			this->Traverse(Visitor);
		}

		CodaScriptWHILEBlock::CodaScriptWHILEBlock(CodaScriptSourceCodeT& Source, UInt32 Line) :
			ICodaScriptLoopBlock()
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

		void CodaScriptWHILEBlock::Accept(ICodaScriptSyntaxTreeVisitor* Visitor)
		{
			Visitor->Visit(this);
		}

		CodaScriptFOREACHBlock::CodaScriptFOREACHBlock(CodaScriptSourceCodeT& Source, UInt32 Line) :
			ICodaScriptLoopBlock(), IteratorName("")
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

		void CodaScriptFOREACHBlock::Accept(ICodaScriptSyntaxTreeVisitor* Visitor)
		{
			Visitor->Visit(this);
		}

		const CodaScriptSourceCodeT& CodaScriptFOREACHBlock::GetIteratorName() const
		{
			return IteratorName;
		}

		CodaScriptAbstractSyntaxTree::CodaScriptAbstractSyntaxTree(CodaScriptBEGINBlock* Root, ICodaScriptExecutableCode::ArrayT& VarIniters) :
			VariableInitalizers(VarIniters),
			Root(Root)
		{
			SME_ASSERT(Root);
		}

		CodaScriptAbstractSyntaxTree::~CodaScriptAbstractSyntaxTree()
		{
			for (auto Itr : VariableInitalizers)
				delete Itr;

			delete Root;
		}

		void CodaScriptAbstractSyntaxTree::Accept(ICodaScriptSyntaxTreeEvaluator* Visitor) noexcept
		{
			for (auto Itr : VariableInitalizers)
				Itr->Accept(Visitor);

			Root->Accept(Visitor);
		}

		ICodaScriptSyntaxTreeEvaluator::ICodaScriptSyntaxTreeEvaluator(ICodaScriptVirtualMachine* VM,
																	   ICodaScriptExpressionParser* Parser,
																	   ICodaScriptExecutionContext* Context) :
			VM(VM),
			Parser(Parser),
			Program(nullptr),
			Context(Context)
		{
			SME_ASSERT(VM && Parser && Context);
			Program = Context->GetProgram();
		}

		ICodaScriptSyntaxTreeEvaluator::ICodaScriptSyntaxTreeEvaluator(ICodaScriptVirtualMachine* VM,
																	   ICodaScriptExpressionParser* Parser,
																	   ICodaScriptProgram* Program) :
			VM(VM),
			Parser(Parser),
			Program(Program),
			Context(nullptr)
		{
			SME_ASSERT(VM && Parser && Program);
		}

		ICodaScriptVirtualMachine* ICodaScriptSyntaxTreeEvaluator::GetVM() const
		{
			return VM;
		}

		ICodaScriptExpressionParser* ICodaScriptSyntaxTreeEvaluator::GetParser() const
		{
			return Parser;
		}

		ICodaScriptExecutionContext* ICodaScriptSyntaxTreeEvaluator::GetContext() const
		{
			return Context;
		}

		bool ICodaScriptSyntaxTreeEvaluator::HasContext() const
		{
			return Context != nullptr;
		}

		ICodaScriptProgram* ICodaScriptSyntaxTreeEvaluator::GetProgram() const
		{
			return Program;
		}

	}
}