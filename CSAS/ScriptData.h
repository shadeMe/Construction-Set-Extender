#pragma once

#include "mpValue.h"
#include "mpParser.h"
#include "Array.h"

namespace CSAutomationScript
{
	class ScriptRunner;

	// abstract base classes
	class ExecutableCode
	{
		static UInt32							GlobalInstanceCount;
	protected:
		UInt32									LineNumber;				// doubles as StartLineNumber for CodeBlocks
		std::string								Text;
		bool									Valid;
		UInt8									Type;
		mup::ParserX							CodeParser;				// child class c'tor initialize user-defined data

		virtual void							Release();
	public:
		enum
		{
			kType_LineOfCode	= 0,
			kType_BeginBlock,
			kType_IfBlock,
			kType_ElseIfBlock,
			kType_ElseBlock,
			kType_WhileBlock,
			kType_ForEachBlock
		};

		ExecutableCode();
		virtual ~ExecutableCode();

		virtual bool							Execute(ScriptRunner* Executor) = 0;	// return false to stop execution
		bool									IsVaid() const;

		static UInt32							GetGlobalInstanceCount();
	};

	class CodeBlock : public ExecutableCode
	{
	protected:
		typedef std::vector<ExecutableCode*>	ExecutableBlockCode;
		typedef std::map<UInt32, std::string>	ContentMap;

		UInt32									EndLineNumber;							// line the declares the end of the block, i.e., end, endIf, etc
		ExecutableBlockCode						LinesOfCode;

		bool									GetTokenInMask(UInt32 Mask, ScriptParser::TokenType Token);
		void									ExtractCodeText(ContentMap& BlockText, UInt32 StartLine, UInt32 EndLine, std::string& CodeOut, bool ClearBuffer);
		ExecutableCode*							GenerateCodeFromText(ScriptParser* Tokenizer, ContentMap& BlockText, std::string& Buffer, UInt32 LineNumber, UInt32* LineOut, bool* EmptyLine, mup::ParserX* PrimaryParser);
		bool									RunCode(ScriptRunner* Executor);

		virtual void							Release();
		virtual bool							LocateJumpBlock(std::string& Source, UInt32 StartLineNumber, CmdToken PrologToken, UInt32 AuxPrologTokenMask, UInt32 EpilogTokenMask, ContentMap& BlockCodeOut);
		virtual void							InitializeCodeBlock(std::string& Source, UInt32 StartLineNumber, CmdToken PrologToken, UInt32 AuxPrologTokenMask, UInt32 EpilogTokenMask, mup::ParserX* PrimaryParser);
	public:
		virtual ~CodeBlock();

		virtual bool							Execute(ScriptRunner* Executor) = 0;
	};

	class ControlBlock : public CodeBlock
	{
	protected:
		std::string								ConditionExpression;

		virtual std::string&					GetConditionExpression();

		virtual void							Release();
		virtual bool							EvaluateCondition(ScriptRunner* Executor);
	public:
		ControlBlock();
		virtual ~ControlBlock();

		virtual bool							Execute(ScriptRunner* Executor) = 0;
	};

	class LoopBlock : public ControlBlock
	{
	protected:
		UInt8									State;

		void									BeginLoopExecution(ScriptRunner* Executor);
		void									EndLoopExecution(ScriptRunner* Executor);
	public:
		enum
		{
			kState_Default	= 0,
			kState_Break,
			kState_Continue
		};

		LoopBlock();
		virtual ~LoopBlock();

		void									Break();
		void									Continue();
	};

	// derivatives
	class LineOfCode : public ExecutableCode
	{
	public:
		LineOfCode(std::string& Source, UInt32 LineNumber, mup::ParserX* PrimaryParser);
		virtual ~LineOfCode();

		virtual bool							Execute(ScriptRunner* Executor);
	};

	class BeginBlock : public CodeBlock
	{
	public:
		BeginBlock(std::string& Source, UInt32 StartLineNumber, mup::ParserX* PrimaryParser);
		virtual ~BeginBlock();

		virtual bool							Execute(ScriptRunner* Executor);
	};

	class ElseBlock;
	class ElseIfBlock;

	class IfBlock : public ControlBlock
	{
	protected:
		typedef std::vector<ElseIfBlock*>		ElseIfBlockList;

		ElseBlock*								Else;
		ElseIfBlockList							ElseIfs;

		bool									EvaluateElseIfBlocks(ScriptRunner* Executor, bool* Failed);

		virtual void							Release();
		virtual void							InitializeCodeBlock(std::string& Source, UInt32 StartLineNumber, CmdToken PrologToken, UInt32 AuxPrologTokenMask, UInt32 EpilogTokenMask, mup::ParserX* PrimaryParser);
	public:
		IfBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser);	// BlockEndOut = the block epilog declaration line
		virtual ~IfBlock();

		virtual bool							Execute(ScriptRunner* Executor);
	};

	class ElseIfBlock : public ControlBlock
	{
	public:
		ElseIfBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser);
		virtual ~ElseIfBlock();

		virtual bool							Execute(ScriptRunner* Executor);

		bool									PreEvaluateCondition(ScriptRunner* Executor);
	};

	class ElseBlock : public CodeBlock
	{
	public:
		ElseBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser);
		virtual ~ElseBlock();

		virtual bool							Execute(ScriptRunner* Executor);
	};

	class WhileBlock : public LoopBlock
	{
	public:
		WhileBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser);
		virtual ~WhileBlock();

		virtual bool							Execute(ScriptRunner* Executor);
	};

	class ForEachBlock : public LoopBlock
	{
	protected:
		std::string								BufferVarID;

		virtual std::string&					GetConditionExpression();
	public:
		ForEachBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser);
		virtual ~ForEachBlock();

		virtual bool							Execute(ScriptRunner* Executor);
	};
}