#include "ScriptRunner.h"

namespace CSAutomationScript
{
	UInt32		ExecutableCode::GlobalInstanceCount = 0;

	void ExecutableCode::Release()
	{
		GlobalInstanceCount--;
	}

	bool CodeBlock::GetIsTokenInMask(UInt32 Mask, ScriptParser::TokenType Token)
	{
		return (Mask & (int)Token);
	}

	void CodeBlock::GetCodeSubString(ContentMap& BlockText, UInt32 StartLine, UInt32 EndLine, std::string& CodeOut, bool ClearBuffer)
	{
		if (EndLine < StartLine)
			return;

		if (ClearBuffer)
			CodeOut.clear();

		for (int i = StartLine; i <= EndLine; i++)
		{
			CodeOut += BlockText.at(i) + "\n";
		}
	}

	ExecutableCode* CodeBlock::GenerateGenericCodeFromContent(ScriptParser* Tokenizer, ContentMap& BlockText, std::string& Buffer, UInt32 LineNumber, UInt32* LineOut, bool* EmptyLine, mup::ParserX* PrimaryParser)
	{
		UInt32 LineOutBuffer = 0;
		std::string& CurrentLine = BlockText.at(LineNumber);

		if (Tokenizer->Tokenize(CurrentLine, false))
		{
			ExecutableCode* Code = NULL;

			switch (Tokenizer->GetFirstTokenType())
			{
			case ScriptParser::kTokenType_Invalid:
			case ScriptParser::kTokenType_Call:
			case ScriptParser::kTokenType_Return:
			case ScriptParser::kTokenType_Continue:
			case ScriptParser::kTokenType_Break:
				Code = new LineOfCode(CurrentLine, LineNumber, PrimaryParser);
				LineOutBuffer = LineNumber;
				break;
			case ScriptParser::kTokenType_Loop:
			case ScriptParser::kTokenType_End:
			case ScriptParser::kTokenType_EndIf:
				*EmptyLine = true;
				break;
			case ScriptParser::kTokenType_While:
			case ScriptParser::kTokenType_ForEach:
			case ScriptParser::kTokenType_If:
			case ScriptParser::kTokenType_ElseIf:
			case ScriptParser::kTokenType_Else:
				GetCodeSubString(BlockText, LineNumber, this->EndLineNumber, Buffer, true);
				switch (Tokenizer->GetFirstTokenType())
				{
				case ScriptParser::kTokenType_While:
					Code = new WhileBlock(Buffer, LineNumber, &LineOutBuffer, PrimaryParser);
					break;
				case ScriptParser::kTokenType_ForEach:
					Code = new ForEachBlock(Buffer, LineNumber, &LineOutBuffer, PrimaryParser);
					break;
				case ScriptParser::kTokenType_If:
					Code = new IfBlock(Buffer, LineNumber, &LineOutBuffer, PrimaryParser);
					break;
				case ScriptParser::kTokenType_Else:
					Code = new ElseBlock(Buffer, LineNumber, &LineOutBuffer, PrimaryParser);
					break;
				case ScriptParser::kTokenType_ElseIf:
					Code = new ElseIfBlock(Buffer, LineNumber, &LineOutBuffer, PrimaryParser);
					break;
				}
				break;
			}

			if (Code)
			{
				*LineOut = LineOutBuffer;

				if (!Code->IsVaid())
					DebugPrint("Couldn't parse control block at %d", LineNumber);
			}

			return Code;
		}
		else
			*EmptyLine = true;

		return NULL;
	}

	bool CodeBlock::RunCode(ScriptRunner* Executor)
	{
		for (ExecutableBlockCode::iterator Itr = LinesOfCode.begin(); Itr != LinesOfCode.end(); Itr++)
		{
			if ((*Itr)->Execute(Executor) == false)
				return false;
		}

		return true;
	}

	void CodeBlock::Release()
	{
		for (ExecutableBlockCode::iterator Itr = LinesOfCode.begin(); Itr != LinesOfCode.end(); Itr++)
		{
			delete (*Itr);
		}
	}

	bool CodeBlock::LocateJumpBlock(std::string& Source, UInt32 StartLineNumber, CmdToken PrologToken, UInt32 AuxPrologTokenMask, UInt32 EpilogTokenMask, CodeBlock::ContentMap& BlockCodeOut)
	{
		bool Result = false;

		ScriptParser Tokenizer;
		std::stringstream InputStream(Source, std::stringstream::in);

		char Buffer[0x512] = {0};
		int BeginLine = -1, EndLine = -1, LineCounter = StartLineNumber;
		bool GotProlog = false, GotEpilog = false, Done = false;
		int ExtraBlockCount = 0;		// to make sure we locate the correct epilog

		while (InputStream.eof() == false)
		{
			ZeroMemory(Buffer, sizeof(Buffer));
			InputStream.getline(Buffer, sizeof(Buffer));

			if (Tokenizer.Tokenize(Buffer, false))
			{
				ScriptParser::TokenType FirstToken = Tokenizer.GetFirstTokenType();

				assert(!(GetIsTokenInMask(AuxPrologTokenMask, FirstToken) && GetIsTokenInMask(EpilogTokenMask, FirstToken)));

				if (!GotProlog && PrologToken == FirstToken)
				{
					GotProlog = true;

					Tokenizer.Sanitize(std::string(Buffer),
						this->Text, ScriptParser::kSanitizeOps_StripTabCharacters|ScriptParser::kSanitizeOps_StripComments|ScriptParser::kSanitizeOps_StripLeadingWhitespace);
					this->LineNumber = BeginLine = LineCounter;

					LineCounter++;
					continue;
				}										// special cased for ElseIfBlock
				else if (GetIsTokenInMask(AuxPrologTokenMask, FirstToken) && !GetIsTokenInMask(EpilogTokenMask, FirstToken))
				{
					ExtraBlockCount++;
				}

				if (GetIsTokenInMask(EpilogTokenMask, FirstToken))
				{
					if (!GotEpilog && !ExtraBlockCount)
					{
						GotEpilog = true;
						this->EndLineNumber = EndLine = LineCounter;

						Done = true;
					}

					ExtraBlockCount--;
				}
			}

			BlockCodeOut.insert(std::make_pair<UInt32, std::string>(LineCounter, Buffer));
			LineCounter++;

			if (Done)
				break;
		}

		if (GotProlog && GotEpilog)
			Result = true;

		return Result;
	}

	void CodeBlock::InitializeCodeBlock(std::string& Source, UInt32 StartLineNumber, CmdToken PrologToken, UInt32 AuxPrologTokenMask, UInt32 EpilogTokenMask, mup::ParserX* PrimaryParser)
	{
		this->Valid = true;

		ContentMap BlockText;
		ScriptParser Tokenizer;

		if (LocateJumpBlock(Source, StartLineNumber, PrologToken, AuxPrologTokenMask, EpilogTokenMask, BlockText))
		{
			std::string Buffer = "";
			UInt32 LineOut = 0;

			for (int i = this->LineNumber + 1; i < this->EndLineNumber; i++)
			{
				bool EmptyLine = false;
				ExecutableCode* GeneratedCode = GenerateGenericCodeFromContent(&Tokenizer, BlockText, Buffer, i, &LineOut, &EmptyLine, PrimaryParser);

				if (!EmptyLine)
				{
					if (GeneratedCode)
						LinesOfCode.push_back(GeneratedCode);

					if (GeneratedCode && GeneratedCode->IsVaid())
					{
						i = LineOut;
						continue;
					}
					else
					{
						DebugPrint("Failed to generate executable code for line %d", i);
						this->Valid = false;
						break;
					}
				}
			}
		}

		BlockText.clear();
	}

	std::string& ControlBlock::GetConditionExpression()
	{
		if (ConditionExpression == "")
		{
			ScriptParser Tokenizer;
			std::string Buffer;

			if (Tokenizer.Tokenize(this->Text, false))
			{
				Buffer = this->Text.substr(Tokenizer.Indices[0] + Tokenizer.Tokens[0].length());		// everything following the first token
				Tokenizer.Sanitize(Buffer, ConditionExpression, ScriptParser::kSanitizeOps_StripTabCharacters|ScriptParser::kSanitizeOps_StripComments);
			}
		}

		return ConditionExpression;
	}

	void ControlBlock::Release()
	{
		;//
	}

	bool ControlBlock::EvaluateCondition(ScriptRunner* Executor)
	{
		return CodeParser.Eval().GetFloat();
	}

	void LoopBlock::BeginLoopExecution(ScriptRunner* Executor)
	{
		Executor->GetExecutingContext()->PushLoop(this);
	}

	void LoopBlock::EndLoopExecution(ScriptRunner* Executor)
	{
		assert(Executor->GetExecutingContext()->PopLoop() == this);
		State = kState_Default;
	}

	LineOfCode::LineOfCode(std::string& Source, UInt32 LineNumber, mup::ParserX* PrimaryParser)
	{
		ScriptParser Tokenizer;
		Tokenizer.Sanitize(Source, this->Text, ScriptParser::kSanitizeOps_StripTabCharacters|ScriptParser::kSanitizeOps_StripComments|ScriptParser::kSanitizeOps_StripLeadingWhitespace);

		this->LineNumber = LineNumber;
		this->Valid = true;
		this->Type = kType_LineOfCode;

		CodeParser = *PrimaryParser;
		CodeParser.SetExpr(this->Text);
	}

	bool LineOfCode::Execute(ScriptRunner* Executor)
	{
		if (!Valid)
		{
			DebugPrint("Couldn't execute code at %d - Line was not correctly initialized", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
			return false;
		}

		try
		{
			CodeParser.Eval();									// no result

			if (Executor->GetExecutingContext()->GetExecutionState() == ScriptContext::kExecutionState_Break)
			{
				return false;
			}
			else
				return true;
		}
		catch (mup::ParserError& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.GetMsg().c_str());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (std::exception& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.what());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (...)
		{
			DebugPrint("Couldn't execute code at %d - Unknown exception", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}

		return false;
	}

	BeginBlock::BeginBlock(std::string& Source, UInt32 StartLineNumber, mup::ParserX* PrimaryParser)
	{
		this->Type = kType_BeginBlock;
		InitializeCodeBlock(Source, StartLineNumber,
							ScriptParser::kTokenType_Begin,
							ScriptParser::kTokenType_Begin,
							ScriptParser::kTokenType_End,
							PrimaryParser);
	}

	bool BeginBlock::Execute(ScriptRunner* Executor)
	{
		if (!Valid)
		{
			DebugPrint("Couldn't execute BEGIN at %d - Block was not correctly initialized", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
			return false;
		}

		return RunCode(Executor);
	}

	bool IfBlock::EvaluateElseIfBlocks(ScriptRunner* Executor, bool* Failed)
	{
		for (ElseIfBlockList::iterator Itr = ElseIfs.begin(); Itr != ElseIfs.end(); Itr++)
		{
			if ((*Itr)->PreEvaluateCondition(Executor))
			{
				*Failed = false;
				return (*Itr)->Execute(Executor);
			}
		}

		*Failed = true;
		return true;
	}

	void IfBlock::Release()
	{
		delete Else;

		for (ElseIfBlockList::iterator Itr = ElseIfs.begin(); Itr != ElseIfs.end(); Itr++)
		{
			delete (*Itr);
		}
	}

	void IfBlock::InitializeCodeBlock(std::string& Source, UInt32 StartLineNumber, CmdToken PrologToken, UInt32 AuxPrologTokenMask, UInt32 EpilogTokenMask, mup::ParserX* PrimaryParser)
	{
		this->Valid = true;
		this->Else = NULL;

		ContentMap BlockText;
		ScriptParser Tokenizer;

		if (LocateJumpBlock(Source, StartLineNumber, PrologToken, AuxPrologTokenMask, EpilogTokenMask, BlockText))
		{
			std::string Buffer = "";
			UInt32 LineOut = 0;

			for (int i = this->LineNumber + 1; i < this->EndLineNumber; i++)
			{
				bool EmptyLine = false;
				ExecutableCode* GeneratedCode = GenerateGenericCodeFromContent(&Tokenizer, BlockText, Buffer, i, &LineOut, &EmptyLine, PrimaryParser);

				if (!EmptyLine)
				{
					if (GeneratedCode)
					{
						bool Exit = false;
						switch (Tokenizer.GetFirstTokenType())
						{
						case ScriptParser::kTokenType_Else:
							if (!this->Else)
								this->Else = dynamic_cast<ElseBlock*>(GeneratedCode);
							else
							{
								delete GeneratedCode;
								GeneratedCode = NULL;

								DebugPrint("Multiple else blocks for IF at %d", i);
							}
							break;
						case ScriptParser::kTokenType_ElseIf:
							this->ElseIfs.push_back(dynamic_cast<ElseIfBlock*>(GeneratedCode));
							break;
						default:
							LinesOfCode.push_back(GeneratedCode);
							break;
						}
					}

					if (GeneratedCode && GeneratedCode->IsVaid())
					{
						i = LineOut;
						continue;
					}
					else
					{
						DebugPrint("Failed to generate executable code for line %d", i);
						this->Valid = false;
						break;
					}
				}
			}
		}

		BlockText.clear();
	}

	IfBlock::IfBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser)
	{
		this->Type = kType_IfBlock;
		InitializeCodeBlock(Source, StartLineNumber,
							ScriptParser::kTokenType_If,
							ScriptParser::kTokenType_If,
							ScriptParser::kTokenType_EndIf,
							PrimaryParser);

		CodeParser = *PrimaryParser;
		CodeParser.SetExpr(GetConditionExpression());

		*BlockEndOut = this->EndLineNumber;
	}

	bool IfBlock::Execute(ScriptRunner* Executor)
	{
		if (!Valid)
		{
			DebugPrint("Couldn't execute IF at %d - Block was not correctly initialized", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
			return false;
		}

		try
		{
			if (EvaluateCondition(Executor))
				return RunCode(Executor);

			bool ElseIfFailed = false, ElseIfResult = EvaluateElseIfBlocks(Executor, &ElseIfFailed);

			if (ElseIfFailed)
			{
				if (Else)
					return Else->Execute(Executor);
			}
			else
				return ElseIfResult;

			return true;
		}
		catch (mup::ParserError& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.GetMsg().c_str());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (std::exception& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.what());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (...)
		{
			DebugPrint("Couldn't execute code at %d - Unknown exception", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}

		return false;
	}

	ElseIfBlock::ElseIfBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser)
	{
		this->Type = kType_ElseIfBlock;
		InitializeCodeBlock(Source, StartLineNumber,
							ScriptParser::kTokenType_ElseIf,
							ScriptParser::kTokenType_If,
							ScriptParser::kTokenType_ElseIf|ScriptParser::kTokenType_Else|ScriptParser::kTokenType_EndIf,
							PrimaryParser);

		CodeParser = *PrimaryParser;
		CodeParser.SetExpr(GetConditionExpression());

		*BlockEndOut = this->EndLineNumber - 1;	// one less than the actual end line, in order to not skip else/elseIf tokens that act as the epilog declarant
	}

	bool ElseIfBlock::Execute(ScriptRunner* Executor)
	{
		if (!Valid)
		{
			DebugPrint("Couldn't execute ELSEIF at %d - Block was not correctly initialized", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
			return false;
		}

		try
		{
			if (EvaluateCondition(Executor))
				return RunCode(Executor);
			else
				return true;
		}
		catch (mup::ParserError& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.GetMsg().c_str());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (std::exception& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.what());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (...)
		{
			DebugPrint("Couldn't execute code at %d - Unknown exception", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}

		return false;
	}

	ElseBlock::ElseBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser)
	{
		this->Type = kType_ElseBlock;
		InitializeCodeBlock(Source, StartLineNumber,
							ScriptParser::kTokenType_Else,
							ScriptParser::kTokenType_If,
							ScriptParser::kTokenType_EndIf,
							PrimaryParser);

		CodeParser = *PrimaryParser;
		*BlockEndOut = this->EndLineNumber;
	}

	bool ElseBlock::Execute(ScriptRunner* Executor)
	{
		if (!Valid)
		{
			DebugPrint("Couldn't execute ELSE at %d - Block was not correctly initialized", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
			return false;
		}

		return RunCode(Executor);
	}

	WhileBlock::WhileBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser)
	{
		this->Type = kType_WhileBlock;
		InitializeCodeBlock(Source, StartLineNumber,
							ScriptParser::kTokenType_While,
							ScriptParser::kTokenType_ForEach,
							ScriptParser::kTokenType_Loop,
							PrimaryParser);

		CodeParser = *PrimaryParser;
		CodeParser.SetExpr(GetConditionExpression());

		*BlockEndOut = this->EndLineNumber;
	}

	bool WhileBlock::Execute(ScriptRunner* Executor)
	{
		if (!Valid)
		{
			DebugPrint("Couldn't execute WHILE at %d - Block was not correctly initialized", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
			return false;
		}

		try
		{
			BeginLoopExecution(Executor);

			while (EvaluateCondition(Executor))
			{
				if (!RunCode(Executor))
				{
					if (State == kState_Continue)
					{
						State = kState_Default;
						Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Default);
						continue;
					}
					else if (State = kState_Break)
					{
						State = kState_Default;
						Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Default);
						break;
					}
					else
					{
						EndLoopExecution(Executor);
						return false;					// execution was stopped by other means
					}
				}
			}

			EndLoopExecution(Executor);
			return true;
		}
		catch (mup::ParserError& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.GetMsg().c_str());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (std::exception& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.what());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (...)
		{
			DebugPrint("Couldn't execute code at %d - Unknown exception", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}

		EndLoopExecution(Executor);
		return false;
	}

	std::string& ForEachBlock::GetConditionExpression()
	{
		if (ConditionExpression == "")
		{
			ScriptParser Tokenizer;
			std::string Buffer;

			if (Tokenizer.Tokenize(this->Text, false))
			{
				BufferVarID = Tokenizer.Tokens[1];		// second token
				Buffer = this->Text.substr(Tokenizer.Indices[2] + Tokenizer.Tokens[2].length() + 1);	// everything past the third token (<-)
				Tokenizer.Sanitize(Buffer, ConditionExpression, ScriptParser::kSanitizeOps_StripTabCharacters|ScriptParser::kSanitizeOps_StripComments);
			}
		}

		return ConditionExpression;
	}

	ForEachBlock::ForEachBlock(std::string& Source, UInt32 StartLineNumber, UInt32* BlockEndOut, mup::ParserX* PrimaryParser)
	{
		this->Type = kType_ForEachBlock;
		InitializeCodeBlock(Source, StartLineNumber,
							ScriptParser::kTokenType_ForEach,
							ScriptParser::kTokenType_While,
							ScriptParser::kTokenType_Loop,
							PrimaryParser);

		CodeParser = *PrimaryParser;
		CodeParser.SetExpr(GetConditionExpression());

		*BlockEndOut = this->EndLineNumber;
	}

	bool ForEachBlock::Execute(ScriptRunner* Executor)
	{
		if (!Valid)
		{
			DebugPrint("Couldn't execute FOREACH at %d - Block was not correctly initialized", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
			return false;
		}

		try
		{
			BeginLoopExecution(Executor);

			if (BufferVarID == "" || ConditionExpression == "")
				throw std::exception("Invalid FOREACH expression");

			ScriptVariable* BufferVar = Executor->GetExecutingContext()->LookupVariableByName(BufferVarID);
			if (!BufferVar)
				throw std::exception("Invalid buffer variable");

			mup::Value& Buffer = const_cast<mup::Value&>(BufferVar->GetValue());

			mup::Value ExpressionArray = CodeParser.Eval();
			if (!ExpressionArray.IsArray())
				throw std::exception("Non-array value returned by FOREACH expression");

			for (mup::array_type::const_iterator Itr = ExpressionArray.GetArray().begin(); Itr != ExpressionArray.GetArray().end(); Itr++)
			{
				Buffer = *Itr;

				if (!RunCode(Executor))
				{
					if (State == kState_Continue)
					{
						State = kState_Default;
						Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Default);
						continue;
					}
					else if (State = kState_Break)
					{
						State = kState_Default;
						Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Default);
						break;
					}
					else
					{
						EndLoopExecution(Executor);
						return false;					// execution was stopped by other means
					}
				}
			}

			EndLoopExecution(Executor);
			return true;
		}
		catch (mup::ParserError& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.GetMsg().c_str());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (std::exception& Exception)
		{
			DebugPrint("Couldn't execute code at %d - Exception: %s", this->LineNumber, Exception.what());
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}
		catch (...)
		{
			DebugPrint("Couldn't execute code at %d - Unknown exception", this->LineNumber);
			Executor->GetExecutingContext()->SetExecutionState(ScriptContext::kExecutionState_Terminate);
		}

		EndLoopExecution(Executor);
		return false;
	}
}