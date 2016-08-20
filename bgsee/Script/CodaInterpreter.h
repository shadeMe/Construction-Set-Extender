#pragma  once
#include "CodaDataTypes.h"

namespace bgsee
{
	namespace script
	{
		class CodaScriptTokenizer
		{
		public:
			typedef std::vector<CodaScriptSourceCodeT>		ParsedTokenListT;
			typedef std::vector<UInt32>						ParsedTokenOffsetListT;
			typedef std::vector<char>						ParsedDelimiterListT;

			ParsedTokenListT								Tokens;
			ParsedTokenOffsetListT							Indices;		// the position of each token relative to its parent line
			ParsedDelimiterListT							Delimiters;

			enum
			{
				kTokenType_Invalid							= 1 << 0,
				kTokenType_Variable							= 1 << 1,
				kTokenType_Begin							= 1 << 2,
				kTokenType_End								= 1 << 3,
				kTokenType_While							= 1 << 4,
				kTokenType_ForEach							= 1 << 5,
				kTokenType_Loop								= 1 << 6,
				kTokenType_If								= 1 << 7,
				kTokenType_ElseIf							= 1 << 8,
				kTokenType_Else								= 1 << 9,
				kTokenType_EndIf							= 1 << 10,
				kTokenType_Return							= 1 << 11,
				kTokenType_Call								= 1 << 12,
				kTokenType_ScriptName						= 1 << 13,
				kTokenType_Continue							= 1 << 14,
				kTokenType_Break							= 1 << 15,
			};

			enum
			{
				kSanitizeOps_StripLeadingWhitespace			= 1 << 0,
				kSanitizeOps_StripTabCharacters				= 1 << 1,
				kSanitizeOps_StripComments					= 1 << 2
			};

			CodaScriptTokenizer();
			~CodaScriptTokenizer();

			bool											Tokenize(CodaScriptSourceCodeT Source, bool CollectEmptyTokens = false);		// returns true if at least one token was parsed, skips comments
			void											ResetState(void);

			UInt32											GetParsedTokenCount() const;
			CodaScriptKeywordT								GetFirstTokenKeywordType();

			void											Sanitize(const CodaScriptSourceCodeT& In, CodaScriptSourceCodeT& Out, UInt32 OperationMask);

			static CodaScriptKeywordT						GetKeywordType(CodaScriptSourceCodeT& Token);
			static const CodaScriptSourceCodeT&				GetKeywordName(CodaScriptKeywordT Keyword);
		private:
			CodaScriptSourceCodeT							SourceString;

			bool											IsIndexStringLiteral(const CodaScriptSourceCodeT& Source, int Index);		// returns true if the index is inside a string literal

			static const CodaScriptSourceCodeT				kWhitespace;
			static const CodaScriptSourceCodeT				kValidDelimiters;
			static const CodaScriptSourceCodeT				kCommentDelimiter;
			static const UInt32								kCodaKeywordCount;
			static const CodaScriptSourceCodeT				kCodaKeywordArray[];
		};

		class ICodaScriptExpressionByteCode
		{
		protected:
			ICodaScriptExecutableCode*						Source;
		public:
			ICodaScriptExpressionByteCode(ICodaScriptExecutableCode* SourceCode);
			virtual ~ICodaScriptExpressionByteCode() = 0;

			ICodaScriptExecutableCode*						GetSource(void) const;
		};

		// since the language syntax and feature set is predominantly based around the muParserX expression parser's,
		// the interface expects all implementations to support at least as much as the former
		class ICodaScriptExpressionParser
		{
		public:
			virtual ~ICodaScriptExpressionParser() = 0;

			virtual void									RegisterCommand(ICodaScriptCommand* Command) = 0;
			virtual void									RegisterConstant(const char* Name, CodaScriptBackingStore& Value) = 0;

			virtual void									RegisterVariables(CodaScriptExecutionContext* ParentContext, CodaScriptVariableArrayT& VariableList) = 0;
			virtual void									UnregisterVariables(CodaScriptExecutionContext* ParentContext) = 0;

			virtual void									Compile(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
																	ICodaScriptExecutableCode* SourceCode,
																	ICodaScriptExpressionByteCode** OutByteCode) = 0;
			virtual void									Evaluate(ICodaScriptSyntaxTreeEvaluator* EvaluationAgent,
																	ICodaScriptExpressionByteCode* ByteCode,
																	CodaScriptBackingStore* Result = nullptr) = 0;
		};

		class ICodaScriptSyntaxTreeVisitor
		{
		public:
			virtual ~ICodaScriptSyntaxTreeVisitor() = 0;

			virtual void									Visit(ICodaScriptParseTree* Node) = 0;
			virtual void									Visit(CodaScriptExpression* Node) = 0;
			virtual void									Visit(CodaScriptBEGINBlock* Node) = 0;
			virtual void									Visit(CodaScriptIFBlock* Node) = 0;
			virtual void									Visit(CodaScriptELSEIFBlock* Node) = 0;
			virtual void									Visit(CodaScriptELSEBlock* Node) = 0;
			virtual void									Visit(CodaScriptWHILEBlock* Node) = 0;
			virtual void									Visit(CodaScriptFOREACHBlock* Node) = 0;
		};

		class ICodaScriptSyntaxTreeNode
		{
			friend class CodaScriptSyntaxTreeCompileVisitor;
			friend class CodaScriptSyntaxTreeExecuteVisitor;
			friend class CodaScriptExecutionContext;
		protected:
			ICodaScriptSyntaxTreeNode*						Parent;
			CodaScriptSyntaxTreeNodeArrayT					Children;

			bool											LookupChild(ICodaScriptSyntaxTreeNode* Child, CodaScriptSyntaxTreeNodeArrayT::iterator& Match);

			bool											Attach(ICodaScriptSyntaxTreeNode* NewParent);
			void											Detach(bool UpdateParent = true);
			void											Purge(void);

			virtual void									Traverse(ICodaScriptSyntaxTreeVisitor* Visitor);
		public:
			ICodaScriptSyntaxTreeNode(ICodaScriptSyntaxTreeNode* BiologicalParent = nullptr);
			virtual ~ICodaScriptSyntaxTreeNode() = 0;

			bool											GetIsRoot(void) const;
			bool											GetIsLeaf(void) const;

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor) = 0;
		};

		class ICodaScriptSyntaxTreeEvaluator : public ICodaScriptSyntaxTreeVisitor
		{
		protected:
			CodaScriptVM*									VirtualMachine;
			CodaScriptExecutionContext*						ScriptContext;
			ICodaScriptExpressionParser*					ParserAgent;
		public:
			ICodaScriptSyntaxTreeEvaluator(CodaScriptVM* VM,
										CodaScriptExecutionContext* Context,
										ICodaScriptExpressionParser* Parser);

			virtual ~ICodaScriptSyntaxTreeEvaluator() = 0;

			virtual CodaScriptVariable*						LookupVariable(const char* Name);

			CodaScriptVM*									GetVM(void) const;
			CodaScriptExecutionContext*						GetContext(void) const;
			ICodaScriptExpressionParser*					GetParser(void) const;
		};

		class ICodaScriptParseTree : public ICodaScriptSyntaxTreeNode
		{
		protected:
			CodaScriptVM*									VirtualMachine;

			bool											GetKeywordInStack(CodaScriptKeywordStackT& Stack, CodaScriptKeywordT Keyword);
			bool											GetKeywordOnStackTop(CodaScriptKeywordStackT& Stack, CodaScriptKeywordT Keyword);
		public:
			ICodaScriptParseTree(CodaScriptVM* VM);
			virtual ~ICodaScriptParseTree() = 0;

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor);
		};

		class ICodaScriptExecutableCode : public ICodaScriptSyntaxTreeNode
		{
			friend class CodaScriptSyntaxTreeCompileVisitor;
			friend class CodaScriptSyntaxTreeExecuteVisitor;

			static const char*						kTypeIDs[];
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
			virtual const CodaScriptSourceCodeT&			GetCode() const;

			static int										GIC;
		};

		class ICodaScriptCodeBlock : public ICodaScriptExecutableCode
		{
			friend class CodaScriptSyntaxTreeCompileVisitor;
			friend class CodaScriptSyntaxTreeExecuteVisitor;
		public:
			virtual ~ICodaScriptCodeBlock() = 0;
		};

		typedef ICodaScriptCodeBlock	ICodaScriptUnconditionalCodeBlockT;

		class ICodaScriptConditionalCodeBlock : public ICodaScriptCodeBlock
		{
			friend class CodaScriptSyntaxTreeCompileVisitor;
			friend class CodaScriptSyntaxTreeExecuteVisitor;
		protected:
			CodaScriptSourceCodeT					Condition;

			bool									EvaluateCondition(ICodaScriptSyntaxTreeEvaluator* Context);
		public:
			ICodaScriptConditionalCodeBlock();
			virtual ~ICodaScriptConditionalCodeBlock() = 0;

			virtual const CodaScriptSourceCodeT&	GetCode() const;
		};

		class ICodaScriptLoopBlock : public ICodaScriptConditionalCodeBlock
		{
			friend class CodaScriptSyntaxTreeCompileVisitor;
			friend class CodaScriptSyntaxTreeExecuteVisitor;

			class LoopStackOperator
			{
				ICodaScriptLoopBlock*					LoopBlock;
				CodaScriptSyntaxTreeExecuteVisitor*		Context;
			public:
				LoopStackOperator(ICodaScriptLoopBlock* Source, CodaScriptSyntaxTreeExecuteVisitor* Visitor);
				~LoopStackOperator();

				void									Reset(void);
			};
		protected:
			static const UInt32						kOverrunLimit;

			UInt8									State;

			void									BeginLooping(CodaScriptSyntaxTreeExecuteVisitor* Context);
			void									EndLooping(CodaScriptSyntaxTreeExecuteVisitor* Context);
		public:
			ICodaScriptLoopBlock();
			virtual ~ICodaScriptLoopBlock() = 0;

			enum
			{
				kLoopState_Default	= 0,
				kLoopState_Break
			};

			void									Break();
		};

		class CodaScriptExpression : public ICodaScriptExecutableCode
		{
		public:
			CodaScriptExpression(CodaScriptSourceCodeT& Source, UInt32 Line);
			virtual ~CodaScriptExpression();

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor);
		};

		class CodaScriptBEGINBlock : public ICodaScriptUnconditionalCodeBlockT
		{
		public:
			CodaScriptBEGINBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			virtual ~CodaScriptBEGINBlock();

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor);
		};

		class CodaScriptIFBlock : public ICodaScriptConditionalCodeBlock
		{
			friend class CodaScriptExecutionContext;
			friend class CodaScriptSyntaxTreeCompileVisitor;
			friend class CodaScriptSyntaxTreeExecuteVisitor;
		protected:
			typedef std::vector<CodaScriptELSEIFBlock*>		ElseIfBlockArrayT;

			ElseIfBlockArrayT								BranchELSEIF;
			CodaScriptELSEBlock*							BranchELSE;
		public:
			CodaScriptIFBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			virtual ~CodaScriptIFBlock();

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor);
		};

		class CodaScriptELSEIFBlock : public ICodaScriptConditionalCodeBlock
		{
		public:
			CodaScriptELSEIFBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			virtual ~CodaScriptELSEIFBlock();

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor);
		};

		class CodaScriptELSEBlock : public ICodaScriptUnconditionalCodeBlockT
		{
		public:
			CodaScriptELSEBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			virtual ~CodaScriptELSEBlock();

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor);
		};

		class CodaScriptWHILEBlock : public ICodaScriptLoopBlock
		{
		public:
			CodaScriptWHILEBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			virtual ~CodaScriptWHILEBlock();

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor);
		};

		class CodaScriptFOREACHBlock : public ICodaScriptLoopBlock
		{
			friend class CodaScriptSyntaxTreeCompileVisitor;
			friend class CodaScriptSyntaxTreeExecuteVisitor;
		protected:
			CodaScriptSourceCodeT							IteratorName;
		public:
			CodaScriptFOREACHBlock(CodaScriptSourceCodeT& Source, UInt32 Line);
			virtual ~CodaScriptFOREACHBlock();

			virtual void									Accept(ICodaScriptSyntaxTreeVisitor* Visitor);
		};

		class CodaScriptExecutionContext : public ICodaScriptParseTree
		{
			friend class CodaScriptVM;
			friend class CodaScriptBackgrounder;
			friend class CodaScriptExecutive;
			friend class ICodaScriptSyntaxTreeEvaluator;
			friend class CodaScriptSyntaxTreeCompileVisitor;
			friend class CodaScriptSyntaxTreeExecuteVisitor;
		protected:
			enum
			{
				kValidity_Unknown	= 0,
				kValidity_Good,
				kValidity_Bad,								// context encountered an exception during runtime
				kValidity_Egregious,						// source code's got outstanding syntactical errors, needs to be re-parsed/compiled
				kValidity_Ended,							// context is content having lived an eventful life and now awaits destruction
			};

			CodaScriptSourceCodeT							ScriptName;
			CodaScriptVariableArrayT							Variables;
			UInt8											Validity;
			ICodaScriptExpressionParser*					BoundParser;
			double											PollingInterval;			// only used by backgrounded scripts
			double											PollingIntervalReminder;
			CodaScriptElapsedTimeCounterT					ElapsedTimeCounter;

			CodaScriptVariable*								AddVariable(CodaScriptSourceCodeT& Name);
			void											SetVariable(CodaScriptSourceCodeT& Name, CodaScriptBackingStore& Value);
			CodaScriptVariable*								GetVariable(CodaScriptSourceCodeT& Name);
			CodaScriptVariable*								GetVariable(const char* Name);
			void											ReleaseVariables(void);

			bool											Compile(CodaScriptVM* VirtualMachine);
		public:
			CodaScriptExecutionContext(CodaScriptVM* VirtualMachine,
									std::fstream& SourceCode,
									ICodaScriptExpressionParser* ExpressionParser,
									CodaScriptMutableDataArrayT* Parameters = nullptr);

			virtual ~CodaScriptExecutionContext();

			bool											GetIsValid(void) const;
			bool											GetIsEnded(void) const;

			bool											Execute(CodaScriptSyntaxTreeExecuteVisitor* Agent,
																CodaScriptBackingStore* Result,
																bool& ReturnedResult);
															// returns false if an unexpected exception was thrown

			double											GetSecondsPassed(void);

			static const UInt32								kMaxParameters;
		};

		class CodaScriptSyntaxTreeCompileVisitor : public ICodaScriptSyntaxTreeEvaluator
		{
		protected:
			bool											Failed;		// set to true when the compile operation fails

			void											GenericCompile(ICodaScriptExecutableCode* Node);
		public:
			CodaScriptSyntaxTreeCompileVisitor(CodaScriptVM* VM,
											CodaScriptExecutionContext* Context,
											ICodaScriptExpressionParser* Parser);

			virtual ~CodaScriptSyntaxTreeCompileVisitor();

			virtual void									Visit(ICodaScriptParseTree* Node);
			virtual void									Visit(CodaScriptExpression* Node);
			virtual void									Visit(CodaScriptBEGINBlock* Node);
			virtual void									Visit(CodaScriptIFBlock* Node);
			virtual void									Visit(CodaScriptELSEIFBlock* Node);
			virtual void									Visit(CodaScriptELSEBlock* Node);
			virtual void									Visit(CodaScriptWHILEBlock* Node);
			virtual void									Visit(CodaScriptFOREACHBlock* Node);

			bool											GetCompilationFailed(void) const;
		};

		class CodaScriptSyntaxTreeExecuteVisitor : public ICodaScriptSyntaxTreeEvaluator
		{
			friend class ICodaScriptLoopBlock;
		protected:
			CodaScriptBackingStore*							Result;
			CodaScriptLoopBlockStackT						ExecutingLoops;
			UInt8											ExecutionState;
		public:
			CodaScriptSyntaxTreeExecuteVisitor(CodaScriptVM* VM,
											CodaScriptExecutionContext* Context,
											ICodaScriptExpressionParser* Parser);

			virtual ~CodaScriptSyntaxTreeExecuteVisitor();

			enum
			{
				kExecutionState_Default	= 0,				// normal execution
				kExecutionState_Break,						// break execution without error; set by the return, break and continue commands
				kExecutionState_Terminate,					// break execution with error
				kExecutionState_End							// break execution without error; special case, signifies EOL
			};

			UInt8											GetState(void) const;
			void											SetState(UInt8 State);

			ICodaScriptLoopBlock*							GetCurrentLoop(void) const;

			void											SetResult(const CodaScriptBackingStore& Value);
			CodaScriptBackingStore*							GetResult(void) const;

			virtual void									Visit(ICodaScriptParseTree* Node);
			virtual void									Visit(CodaScriptExpression* Node);
			virtual void									Visit(CodaScriptBEGINBlock* Node);
			virtual void									Visit(CodaScriptIFBlock* Node);
			virtual void									Visit(CodaScriptELSEIFBlock* Node);
			virtual void									Visit(CodaScriptELSEBlock* Node);
			virtual void									Visit(CodaScriptWHILEBlock* Node);
			virtual void									Visit(CodaScriptFOREACHBlock* Node);
		};

		class CodaScriptException
		{
		protected:
			char											ErrorString[0x768];
		public:
			CodaScriptException(ICodaScriptExecutableCode* Source, const char* Message, ...);
			virtual ~CodaScriptException();

			virtual const char*								Get() const;
		};

		class CodaScriptCommandHandlerUtilities : public ICodaScriptCommandHandlerHelper
		{
		protected:
			typedef std::vector<CodaScriptScopedHandleDataStoreT>		DataStoreAllocationArrayT;

			DataStoreAllocationArrayT							AllocatedWrappers;

			CodaScriptBackingStore*								CreateWrapper(CodaScriptSharedHandleArrayT Array);
			CodaScriptBackingStore*								CreateWrapper(CodaScriptBackingStore* Source);
		public:
			CodaScriptCommandHandlerUtilities();
			virtual ~CodaScriptCommandHandlerUtilities();

			virtual ICodaScriptDataStore*						ArrayAllocate(UInt32 InitialSize = 0);
			virtual bool										ArrayPushback(ICodaScriptDataStore* AllocatedArray, CodaScriptNumericDataTypeT Data);
			virtual bool										ArrayPushback(ICodaScriptDataStore* AllocatedArray, CodaScriptStringParameterTypeT Data);
			virtual	bool										ArrayPushback(ICodaScriptDataStore* AllocatedArray, CodaScriptReferenceDataTypeT Data);
			virtual bool										ArrayPushback(ICodaScriptDataStore* AllocatedArray, ICodaScriptDataStore* ArrayData);
			virtual bool										ArrayAt(ICodaScriptDataStore* AllocatedArray, UInt32 Index, ICodaScriptDataStore** OutBuffer);
			virtual bool										ArrayErase(ICodaScriptDataStore* AllocatedArray, UInt32 Index);
			virtual void										ArrayClear(ICodaScriptDataStore* AllocatedArray);
			virtual UInt32										ArraySize(ICodaScriptDataStore* AllocatedArray);

			virtual bool										ExtractArguments(ICodaScriptDataStore* Arguments,
																				ICodaScriptCommand::ParameterInfo* ParameterData,
																				UInt32 ArgumentCount, ...);
		};
	}
}