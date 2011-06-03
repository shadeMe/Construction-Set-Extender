#pragma once

#include <string>
#include <vector>
#include <map>
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

		ExecutableCode()
		{
			GlobalInstanceCount++;
		}

		virtual ~ExecutableCode()
		{
			Release();
		}

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser) = 0;	// return false to stop execution

		bool									IsVaid() const { return Valid; }
		
		static UInt32							GetGlobalInstanceCount() { return GlobalInstanceCount; }
	};

	class CodeBlock : public ExecutableCode
	{
	protected:
		typedef std::vector<ExecutableCode*>	ExecutableBlockCode;
		typedef std::map<UInt32, std::string>	ContentMap;

		UInt32									EndLineNumber;							// line the declares the end of the block, i.e., end, endIf, etc
		ExecutableBlockCode						LinesOfCode;

		bool									GetIsTokenInMask(UInt32 Mask, ScriptParser::TokenType Token);
		void									GetCodeSubString(ContentMap& BlockText, UInt32 StartLine, UInt32 EndLine, std::string& CodeOut, bool ClearBuffer);
		ExecutableCode*							GenerateGenericCodeFromContent(ScriptParser* Tokenizer, ContentMap& BlockText, std::string& Buffer, UInt32 LineNumber, UInt32* LineOut, bool* EmptyLine);
		bool									RunCode(ScriptRunner* Executor, mup::ParserX* Parser);

		virtual void							Release();
		virtual bool							LocateJumpBlock(std::string& Source, UInt32 StartLineNumber, CmdToken PrologToken, UInt32 AuxPrologTokenMask, UInt32 EpilogTokenMask, ContentMap& BlockCodeOut);
		virtual void							InitializeCodeBlock(std::string& Source, UInt32 StartLineNumber, CmdToken PrologToken, UInt32 AuxPrologTokenMask, UInt32 EpilogTokenMask);
	public:
		virtual ~CodeBlock()
		{
			Release();
		}

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser) = 0;
	};

	class ControlBlock : public CodeBlock
	{
	protected:	
		mup::ParserX*							ConditionParser;

		virtual std::string						GetConditionExpression();

		virtual void							Release();
		virtual bool							EvaluateCondition(ScriptRunner* Executor, mup::ParserX* Parser);
	public:	
		ControlBlock() : ConditionParser(ConditionParser = new mup::ParserX(mup::pckALL_NON_COMPLEX)) {}
		virtual ~ControlBlock()
		{
			Release();
		}

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser) = 0;
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

		LoopBlock() : State(kState_Default) {}

		virtual ~LoopBlock()
		{
			;//
		}

		void									Continue() { State = kState_Continue; }
		void									Break() { State = kState_Break; }
	};

	// derivatives
	class LineOfCode : public ExecutableCode
	{
	public:
		LineOfCode(std::string& Source, UInt32 LineNumber);

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser);
	};

	class BeginBlock : public CodeBlock
	{
	public:
		BeginBlock(std::string& Source, UInt32 StartLineNumber);

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser);
	};

	class ElseBlock;
	class ElseIfBlock;

	class IfBlock : public ControlBlock
	{
	protected:
		typedef std::vector<ElseIfBlock*>		ElseIfBlockList;

		ElseBlock*								Else;
		ElseIfBlockList							ElseIfs;

		bool									EvaluateElseIfBlocks(ScriptRunner* Executor, mup::ParserX* Parser, bool* Failed);

		virtual void							Release();
		virtual void							InitializeCodeBlock(std::string& Source, UInt32 StartLineNumber, CmdToken PrologToken, UInt32 AuxPrologTokenMask, UInt32 EpilogTokenMask);
	public:
		IfBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut);	// BlockEndOut = the block epilog declaration line
		virtual ~IfBlock()
		{
			Release();
		}

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser);
	};

	class ElseIfBlock : public ControlBlock
	{
	public:
		ElseIfBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut);

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser);

		bool									PreEvaluateCondition(ScriptRunner* Executor, mup::ParserX* Parser) { return EvaluateCondition(Executor, Parser); }
	};

	class ElseBlock : public CodeBlock
	{
	public:
		ElseBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut);

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser);
	};

	class WhileBlock : public LoopBlock
	{
	public:
		WhileBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut);

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser);
	};	

	class ForEachBlock : public LoopBlock
	{
	protected:
		std::string								BufferVarID;
		std::string								Expression;
	public:
		ForEachBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut);

		virtual bool							Execute(ScriptRunner* Executor, mup::ParserX* Parser);
	};	
}