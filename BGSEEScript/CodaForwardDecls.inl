#pragma once

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		class ICodaScriptSyntaxTreeNode;
		class ICodaScriptSyntaxTreeVisitor;
		class ICodaScriptSyntaxTreeEvaluator;
		class ICodaScriptCommand;
		class ICodaScriptExpressionParser;
		class ICodaScriptExecutableCode;
		class ICodaScriptCodeBlock;
		class ICodaScriptConditionalCodeBlock;
		class ICodaScriptLoopBlock;
		class ICodaScriptArrayDataType;
		class ICodaScriptDataStore;
		class ICodaScriptDataStoreOwner;
		class ICodaScriptParseTree;
		class ICodaScriptCommandHandlerHelper;

		class CodaScriptVM;
		class CodaScriptExecutive;
		class CodaScriptExecutionContext;
		class CodaScriptSyntaxTreeCompileVisitor;
		class CodaScriptSyntaxTreeExecuteVisitor;
		class CodaScriptVariable;
		class CodaScriptBackingStore;
		class CodaScriptBackgrounder;
		class CodaScriptGlobalDataStore;
		class CodaScriptFunctionRegistrar;

		class CodaScriptExpression;
		class CodaScriptBEGINBlock;
		class CodaScriptIFBlock;
		class CodaScriptELSEIFBlock;
		class CodaScriptELSEBlock;
		class CodaScriptWHILEBlock;
		class CodaScriptFOREACHBlock;

		class CodaScriptCommandReturn;
		class CodaScriptCommandCall;
		class CodaScriptCommandBreak;
		class CodaScriptCommandContinue;

		typedef double													CodaScriptNumericDataTypeT;
		typedef char*													CodaScriptStringDataTypeT;
		typedef const char*												CodaScriptStringParameterTypeT;
		typedef UInt32													CodaScriptReferenceDataTypeT;
		typedef std::string												CodaScriptSourceCodeT;
		typedef UInt32													CodaScriptKeywordT;
		typedef std::vector<CodaScriptBackingStore>						CodaScriptMutableDataArrayT;
		typedef std::list<ICodaScriptSyntaxTreeNode*>					CodaScriptSyntaxTreeNodeListT;
		typedef std::list<CodaScriptVariable*>							CodaScriptVariableListT;
		typedef std::stack<CodaScriptKeywordT>							CodaScriptKeywordStackT;
		typedef std::stack<ICodaScriptLoopBlock*>						CodaScriptLoopBlockStackT;
		typedef std::stack<CodaScriptExecutionContext*>					CodaScriptExecutionContextStackT;
		typedef std::stack<ICodaScriptExecutableCode*>					CodaScriptExecutableCodeStackT;
		typedef boost::shared_ptr<ICodaScriptArrayDataType>				CodaScriptSharedHandleArrayT;
		typedef boost::shared_ptr<ICodaScriptDataStoreOwner>			CodaScriptScopedHandleDataStoreOwnerT;
		typedef SME::MiscGunk::ElapsedTimeCounter						CodaScriptElapsedTimeCounterT;
		typedef std::list<CodaScriptFunctionRegistrar*>					CodaScriptRegistrarListT;
	}
}