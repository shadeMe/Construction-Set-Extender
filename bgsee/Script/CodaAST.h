#pragma once
#include "CodaDataTypes.h"

namespace bgsee
{
	namespace script
	{
		class ICodaScriptSyntaxTreeNode
		{
			friend class CodaScriptCompiler;
		protected:
			typedef std::vector<ICodaScriptSyntaxTreeNode*>	CodaScriptSyntaxTreeNodeArrayT;

			ICodaScriptSyntaxTreeNode*						Parent;
			CodaScriptSyntaxTreeNodeArrayT					Children;

			bool											LookupChild(ICodaScriptSyntaxTreeNode* Child, CodaScriptSyntaxTreeNodeArrayT::iterator& Match);
			void											Purge(void);

			bool											Attach(ICodaScriptSyntaxTreeNode* NewParent);
			void											Detach(bool UpdateParent = true);
		public:
			ICodaScriptSyntaxTreeNode(ICodaScriptSyntaxTreeNode* BiologicalParent = nullptr);
			virtual ~ICodaScriptSyntaxTreeNode() = 0;

			bool											IsRoot(void) const;
			bool											IsLeaf(void) const;

			virtual void									Traverse(ICodaScriptSyntaxTreeVisitor* Visitor);
			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor) = 0;
		};

		class ICodaScriptExecutableCode : public ICodaScriptSyntaxTreeNode
		{
			friend class CodaScriptCompiler;
			friend class CodaScriptSyntaxTreeCompileVisitor;

			static const char*						kTypeIDs[];
			static int								GIC;
		protected:
			UInt8									Type;

			UInt32									Line;
			CodaScriptSourceCodeT					Source;
			ICodaScriptExpressionByteCode*			ByteCode;
		public:
			ICodaScriptExecutableCode();
			virtual ~ICodaScriptExecutableCode() = 0;

			enum
			{
				kCodeType_Default_INVALID = 0,

				kCodeType_Line_EXPRESSION,

				kCodeType_Block_BEGIN,

				kCodeType_Block_IF,
				kCodeType_Block_ELSEIF,
				kCodeType_Block_ELSE,

				kCodeType_Loop_WHILE,
				kCodeType_Loop_FOREACH,

				kCodeType__MAX
			};

			UInt8											GetType() const;
			UInt32											GetLine() const;
			const char*										GetTypeString() const;
			virtual const CodaScriptSourceCodeT&			GetSourceCode() const;
			virtual ICodaScriptExpressionByteCode*			GetByteCode() const;

			static const int&								GetGIC() { return GIC; }

			typedef std::vector<ICodaScriptExecutableCode*>		ArrayT;
			typedef std::stack<ICodaScriptExecutableCode*>		StackT;
		};

		class ICodaScriptCodeBlock : public ICodaScriptExecutableCode
		{
		public:
			virtual ~ICodaScriptCodeBlock() = 0 { }
		};

		typedef ICodaScriptCodeBlock	ICodaScriptUnconditionalCodeBlockT;

		class ICodaScriptConditionalCodeBlock : public ICodaScriptCodeBlock
		{
			friend class CodaScriptCompiler;
			friend class CodaScriptSyntaxTreeCompileVisitor;
		protected:
			CodaScriptSourceCodeT					Condition;
		public:
			ICodaScriptConditionalCodeBlock();
			virtual ~ICodaScriptConditionalCodeBlock() = 0 {}

			virtual const CodaScriptSourceCodeT&	GetSourceCode() const override;		// returns the condition expression
		};

		class ICodaScriptLoopBlock : public ICodaScriptConditionalCodeBlock
		{
		public:
			ICodaScriptLoopBlock();
			virtual ~ICodaScriptLoopBlock() = 0 {}
		};

		class CodaScriptExpression : public ICodaScriptExecutableCode
		{
		public:
			CodaScriptExpression(CodaScriptSourceCodeT& Source, UInt32 Line);
			inline virtual ~CodaScriptExpression() = default;

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor) override;
		};

		class CodaScriptBEGINBlock : public ICodaScriptUnconditionalCodeBlockT
		{
		public:
			CodaScriptBEGINBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			inline virtual ~CodaScriptBEGINBlock() = default;

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor) override;
		};

		class CodaScriptIFBlock : public ICodaScriptConditionalCodeBlock
		{
			friend class CodaScriptCompiler;
			friend class CodaScriptSyntaxTreeCompileVisitor;
		public:
			typedef std::vector<CodaScriptELSEIFBlock*>		ElseIfBlockArrayT;
		protected:
			ElseIfBlockArrayT								BranchELSEIF;
			CodaScriptELSEBlock*							BranchELSE;
		public:
			CodaScriptIFBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			virtual ~CodaScriptIFBlock();

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor) override;

			const ElseIfBlockArrayT&						GetElseIfBlocks() const;
			CodaScriptELSEBlock&							GetElseBlock() const;

			bool											HasElseBlock() const;
		};

		class CodaScriptELSEIFBlock : public ICodaScriptConditionalCodeBlock
		{
		public:
			CodaScriptELSEIFBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			inline virtual ~CodaScriptELSEIFBlock() = default;

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor) override;
		};

		class CodaScriptELSEBlock : public ICodaScriptUnconditionalCodeBlockT
		{
		public:
			CodaScriptELSEBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			inline virtual ~CodaScriptELSEBlock() = default;

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor) override;
		};

		class CodaScriptWHILEBlock : public ICodaScriptLoopBlock
		{
		public:
			CodaScriptWHILEBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			inline virtual ~CodaScriptWHILEBlock() = default;

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor) override;
		};

		class CodaScriptFOREACHBlock : public ICodaScriptLoopBlock
		{
			friend class CodaScriptCompiler;
			friend class CodaScriptSyntaxTreeCompileVisitor;
		protected:
			CodaScriptSourceCodeT							IteratorName;
		public:
			CodaScriptFOREACHBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			inline virtual ~CodaScriptFOREACHBlock() = default;

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor) override;

			const CodaScriptSourceCodeT&					GetIteratorName() const;
		};

		class CodaScriptAbstractSyntaxTree
		{
			ICodaScriptExecutableCode::ArrayT				VariableInitalizers;
			CodaScriptBEGINBlock*							Root;
		public:
			CodaScriptAbstractSyntaxTree(CodaScriptBEGINBlock* Root, ICodaScriptExecutableCode::ArrayT& VarIniters);		// takes ownership of the arguments
			~CodaScriptAbstractSyntaxTree();

			void											Accept(ICodaScriptSyntaxTreeEvaluator* Visitor) noexcept;
		};

		class ICodaScriptSyntaxTreeVisitor
		{
		public:
			virtual ~ICodaScriptSyntaxTreeVisitor() = 0 {}

			virtual void									Visit(CodaScriptExpression* Node) = 0;
			virtual void									Visit(CodaScriptBEGINBlock* Node) = 0;
			virtual void									Visit(CodaScriptIFBlock* Node) = 0;
			virtual void									Visit(CodaScriptELSEIFBlock* Node) = 0;
			virtual void									Visit(CodaScriptELSEBlock* Node) = 0;
			virtual void									Visit(CodaScriptWHILEBlock* Node) = 0;
			virtual void									Visit(CodaScriptFOREACHBlock* Node) = 0;
		};

		class ICodaScriptSyntaxTreeEvaluator : public ICodaScriptSyntaxTreeVisitor
		{
		protected:
			ICodaScriptVirtualMachine*		VM;
			ICodaScriptExpressionParser*	Parser;
			ICodaScriptProgram*				Program;
			ICodaScriptExecutionContext*	Context;
		public:
			ICodaScriptSyntaxTreeEvaluator(ICodaScriptVirtualMachine* VM,
										   ICodaScriptExpressionParser* Parser,
										   ICodaScriptExecutionContext* Context);
			ICodaScriptSyntaxTreeEvaluator(ICodaScriptVirtualMachine* VM,
										   ICodaScriptExpressionParser* Parser,
										   ICodaScriptProgram* Program);
			inline virtual  ~ICodaScriptSyntaxTreeEvaluator() = default;

			ICodaScriptVirtualMachine*		GetVM() const;
			ICodaScriptExpressionParser*	GetParser() const;
			ICodaScriptExecutionContext*	GetContext() const;
			bool							HasContext() const;
			ICodaScriptProgram*				GetProgram() const;
		};
	}
}