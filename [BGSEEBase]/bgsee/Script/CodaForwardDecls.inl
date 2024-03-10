#pragma once

namespace bgsee
{
	namespace script
	{
		class ICodaScriptSyntaxTreeNode;
		class ICodaScriptSyntaxTreeVisitor;
		class ICodaScriptCommand;
		class ICodaScriptExpressionParser;
		class ICodaScriptExecutableCode;
		class ICodaScriptCodeBlock;
		class ICodaScriptConditionalCodeBlock;
		class ICodaScriptLoopBlock;
		class ICodaScriptArrayDataType;
		class ICodaScriptDataStore;
		class ICodaScriptDataStoreOwner;
		class CodaScriptAbstractSyntaxTree;
		class ICodaScriptExpressionByteCode;
		class ICodaScriptCompilerMetadata;
		class ICodaScriptExecutionContext;
		class ICodaScriptProgram;
		class ICodaScriptBackgroundDaemon;
		class ICodaScriptVirtualMachine;
		class ICodaScriptExecutor;
		class ICodaScriptProgramCache;

		class CodaScriptVM;
		class CodaScriptExecutive;
		class CodaScriptExecutionContext;
		class CodaScriptSyntaxTreeCompileVisitor;
		class CodaScriptSyntaxTreeExecuteVisitor;
		class CodaScriptVariable;
		class CodaScriptBackingStore;
		class CodaScriptBackgrounder;
		class CodaScriptGlobalDataStore;
		class CodaScriptCommandRegistrar;
		class CodaScriptProgram;
		class CodaScriptCompiler;

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

		typedef std::string										CodaScriptSourceCodeT;
		typedef UInt32											CodaScriptKeywordT;
		typedef std::vector<CodaScriptSourceCodeT>				CodaScriptVariableNameArrayT;
	}
}