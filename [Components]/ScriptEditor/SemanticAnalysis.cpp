#include "SemanticAnalysis.h"

namespace ConstructionSetExtender
{
	UInt32 ByteCodeParser::Read16(Array^% Data, UInt32% CurrentOffset)
	{
		UInt32 LoByte = (Byte)Data->GetValue((int)CurrentOffset++),
			HiByte = (Byte)Data->GetValue((int)CurrentOffset++);
		return LoByte + (HiByte << 8);
	}

	bool ByteCodeParser::LineHasData(String^% Line)
	{
		String^ Extract = Line->Split(';')[0];
		if (Extract == "")		return false;

		int Idx = 0;
		for each (char Itr in Extract)
		{
			if (Itr != '\t' && Itr != ' ')	break;
			Idx++;
		}

		Extract = Extract->Substring(Idx)->Split('\t', '\r', '\n', ' ')[0];
		if (Extract == "")
			return false;

		if (ObScriptSemanticAnalysis::Variable::GetVariableDataType(Extract) != ObScriptSemanticAnalysis::Variable::DataType::None)
			return false;

		else
			return true;
	}

	UInt32 ByteCodeParser::GetOffsetForLine(String^% Line, Array^% Data, UInt32% CurrentOffset)
	{
		if (!ByteCodeParser::LineHasData(Line))
			return 0xFFFFFFFF;

		UInt32 OpCode = Read16(Data, CurrentOffset);

		switch (OpCode)
		{
		case 0x1D:				 // scn
			ByteCodeParser::Read16(Data, CurrentOffset);
			return 0;
		case 0x1C:				 // dot operator
			ByteCodeParser::Read16(Data, CurrentOffset);
			return GetOffsetForLine(Line, Data, CurrentOffset) - 4;
		case 0x11:				 // end
			ByteCodeParser::Read16(Data, CurrentOffset);
			return CurrentOffset - 4;
		case 0x1E:				// return
			ByteCodeParser::Read16(Data, CurrentOffset);
			return CurrentOffset - 4;
		case 0x17:				 // else
			ByteCodeParser::Read16(Data, CurrentOffset);
			ByteCodeParser::Read16(Data, CurrentOffset);
			return CurrentOffset - 6;
		case 0x19:				 // endif
			ByteCodeParser::Read16(Data, CurrentOffset);
			return CurrentOffset - 4;
		default:
			UInt32 Len = ByteCodeParser::Read16(Data, CurrentOffset);
			UInt32 Cur = CurrentOffset - 4;
			CurrentOffset += Len;
			return Cur;
		}
	}

	ObScriptSemanticAnalysis::Tokenizer::Tokenizer()
	{
		Tokens = gcnew List<String^>();
		Indices = gcnew List<UInt32>();
		Delimiters = gcnew List<Char>();
		Good = false;

		this->ReferenceDelimiters = gcnew String(DefaultDelimiters);
		this->ReferenceControlChars = gcnew String(DefaultControlChars);
	}

	ObScriptSemanticAnalysis::Tokenizer::Tokenizer(String^ InputDelimiters, String^ InputControlChars)
	{
		Tokens = gcnew List<String^>();
		Indices = gcnew List<UInt32>();
		Delimiters = gcnew List<Char>();
		Good = false;

		this->ReferenceDelimiters = gcnew String(InputDelimiters);
		this->ReferenceControlChars = gcnew String(InputControlChars);
	}

	ObScriptSemanticAnalysis::Tokenizer::Tokenizer(String^ InputDelimiters)
	{
		Tokens = gcnew List<String^>();
		Indices = gcnew List<UInt32>();
		Delimiters = gcnew List<Char>();
		Good = false;

		this->ReferenceDelimiters = gcnew String(InputDelimiters);
		this->ReferenceControlChars = gcnew String(DefaultControlChars);
	}

	bool ObScriptSemanticAnalysis::Tokenizer::Tokenize(String^ Source, bool CollectEmptyTokens)
	{
		if (Source->Length && Source[Source->Length - 1] != '\n')
			Source += "\n";

		ResetState();
		int StartPos = -1, LastPos = -1;

		for each (Char Itr in Source)
		{
			if (ReferenceControlChars->IndexOf(Itr) == -1)
			{
				StartPos = Source->IndexOf(Itr);
				break;
			}
		}

		if (StartPos)
		{
			LastPos = StartPos;

			String^ TokenString = Source->Substring(StartPos), ^Token, ^Delimiter;
			while (TokenString->IndexOfAny(ReferenceDelimiters->ToCharArray()) != -1)
			{
				int Idx = TokenString->IndexOfAny(ReferenceDelimiters->ToCharArray());
				if (TokenString[0] == '\"')
				{
					if (TokenString->IndexOf('\"', 1) != -1)
						Idx = TokenString->IndexOf('\"', 1) + 1;
					else
						Idx = TokenString->Length - 1;
				}

				Token = TokenString->Substring(0, Idx), Delimiter = TokenString->Substring(Idx, 1);

				TokenString = TokenString->Remove(0, Idx + 1);
				if (Token == "" && !CollectEmptyTokens)
				{
					LastPos++;
					continue;
				}

				Tokens->Add(Token);
				Indices->Add(LastPos);
				Delimiters->Add(Delimiter[0]);
				LastPos += Token->Length + 1;
			}

			if (Tokens->Count > 0)
				Good = true;
		}

		return Good;
	}

	void ObScriptSemanticAnalysis::Tokenizer::ResetState()
	{
		Tokens->Clear();
		Delimiters->Clear();
		Indices->Clear();
		Good = false;
	}

	ObScriptSemanticAnalysis::ScriptTokenType ObScriptSemanticAnalysis::Tokenizer::GetFirstTokenType(void)
	{
		if (Good)
			return GetScriptTokenType(Tokens[0]);
		else
			return ScriptTokenType::None;
	}

	int ObScriptSemanticAnalysis::Tokenizer::GetCommentTokenIndex(int SearchEndIndex)
	{
		int Pos = 0;
		int Result = -1;

		for each (String^ Itr in Tokens)
		{
			if (Pos == SearchEndIndex)
				break;
			if (Itr->IndexOf(";") != -1)
			{
				Result = Pos;
				break;
			}

			++Pos;
		}
		return Result;
	}

	int ObScriptSemanticAnalysis::Tokenizer::GetTokenIndex(String^ Source)
	{
		int Result = -1, Count = 0;

		for each (String^ Itr in Tokens)
		{
			if (!String::Compare(Itr, Source, true))
			{
				Result = Count;
				break;
			}

			++Count;
		}
		return Result;
	}

	ObScriptSemanticAnalysis::ScriptTokenType ObScriptSemanticAnalysis::Tokenizer::GetScriptTokenType(String^ ScriptToken)
	{
		ScriptTokenType Result = ScriptTokenType::None;
		if (ScriptToken->Length)
		{
			if (!String::Compare(ScriptToken, "scn", true) || !String::Compare(ScriptToken, "scriptName", true))
				Result = ScriptTokenType::ScriptName;
			else if (Variable::GetVariableDataType(ScriptToken) != Variable::DataType::None)
				Result = ScriptTokenType::Variable;
			else if (ScriptToken[0] == ';')
				Result = ScriptTokenType::Comment;
			else if (!String::Compare(ScriptToken, "begin", true))
				Result = ScriptTokenType::Begin;
			else if (!String::Compare(ScriptToken, "end", true))
				Result = ScriptTokenType::End;
			else if (!String::Compare(ScriptToken, "while", true))
				Result = ScriptTokenType::While;
			else if (!String::Compare(ScriptToken, "foreach", true))
				Result = ScriptTokenType::ForEach;
			else if (!String::Compare(ScriptToken, "loop", true))
				Result = ScriptTokenType::Loop;
			else if (!String::Compare(ScriptToken, "if", true))
				Result = ScriptTokenType::If;
			else if (!String::Compare(ScriptToken, "elseif", true))
				Result = ScriptTokenType::ElseIf;
			else if (!String::Compare(ScriptToken, "else", true))
				Result = ScriptTokenType::Else;
			else if (!String::Compare(ScriptToken, "endif", true))
				Result = ScriptTokenType::EndIf;
			else if (!String::Compare(ScriptToken, "return", true))
				Result = ScriptTokenType::Return;
			else if (!String::Compare(ScriptToken, "setfunctionvalue", true))
				Result = ScriptTokenType::SetFunctionValue;
			else if (!String::Compare(ScriptToken, "set", true))
				Result = ScriptTokenType::Set;
			else if (!String::Compare(ScriptToken, "let", true))
				Result = ScriptTokenType::Let;
			else if (!String::Compare(ScriptToken, "call", true))
				Result = ScriptTokenType::Call;
			else if (!String::Compare(ScriptToken, "player", true) || !String::Compare(ScriptToken, "playerref", true))
				Result = ScriptTokenType::Player;
		}

		return Result;
	}

	bool ObScriptSemanticAnalysis::Tokenizer::GetIndexInsideString(String^ Source, int Index)
	{
		if (Index >= Source->Length)
			return false;

		int QuoteStack = 0;
		int Idx = 0;
		for each (char Itr in Source)
		{
			if (Itr == '"')
			{
				if (QuoteStack == 0)
					QuoteStack++;
				else if (QuoteStack == 1)
					QuoteStack--;
				else
					break;				// wtf
			}

			if (Index == Idx && QuoteStack)
				return true;

			Idx++;
		}

		return false;
	}

	bool ObScriptSemanticAnalysis::Tokenizer::HasIllegalChar(String^ Source, String^ Includes, String^ Excludes)
	{
		bool Result = false;

		if (Source->IndexOfAny(Includes->ToCharArray()) != -1)
			Result = true;
		else
		{
			for each (Char Itr in Source)
			{
				if (Char::IsSymbol(Itr) && Excludes->IndexOf(Itr) == -1)
				{
					Result = true;
					break;
				}
			}
		}

		return Result;
	}

	ObScriptSemanticAnalysis::Variable::Variable(String^ Name, DataType Type, String^ Comment, UInt32 Line) :
		Name(Name), Comment(Comment), Type(Type), UDFParameter(false), ParameterIndex(0), Line(Line), RefCount(0)
	{
	}

	ObScriptSemanticAnalysis::Variable::DataType ObScriptSemanticAnalysis::Variable::GetVariableDataType(String^ TypeToken)
	{
		DataType VarType = DataType::None;

		if (!String::Compare(TypeToken, "ref", true) || !String::Compare(TypeToken, "reference", true))
			VarType = Variable::DataType::Ref;
		else if (!String::Compare(TypeToken, "short", true) || !String::Compare(TypeToken, "long", true) || !String::Compare(TypeToken, "int", true))
			VarType = Variable::DataType::Integer;
		else if (!String::Compare(TypeToken, "float", true))
			VarType = Variable::DataType::Float;
		else if (!String::Compare(TypeToken, "string_var", true))
			VarType = Variable::DataType::StringVar;
		else if (!String::Compare(TypeToken, "array_var", true))
			VarType = Variable::DataType::ArrayVar;

		return VarType;
	}

	String^ ObScriptSemanticAnalysis::Variable::GetVariableDataTypeToken(Variable::DataType Type)
	{
		switch (Type)
		{
		case DataType::Integer:
			return "int";
		case DataType::Float:
			return "float";
		case DataType::Ref:
			return "ref";
		case DataType::StringVar:
			return "string_var";
		case DataType::ArrayVar:
			return "array_var";
		default:
			return "unk";
		}
	}

	String^ ObScriptSemanticAnalysis::Variable::GetVariableDataTypeDescription(Variable::DataType Type)
	{
		switch (Type)
		{
		case DataType::Integer:
			return "Integer";
		case DataType::Float:
			return "Float";
		case DataType::Ref:
			return "Reference";
		case DataType::StringVar:
			return "String";
		case DataType::ArrayVar:
			return "Array";
		default:
			return "Unknown";
		}
	}

	ObScriptSemanticAnalysis::ControlBlock::ControlBlock(ControlBlockType Type, UInt32 Start, UInt32 Indents, ControlBlock^ Parent) :
		Type(Type), StartLine(Start), EndLine(0), IndentLevel(Indents), Parent(Parent)
	{
	}

	bool ObScriptSemanticAnalysis::ControlBlock::IsMalformed()
	{
		return EndLine == 0;
	}

	ObScriptSemanticAnalysis::ScriptBlock::ScriptBlock(UInt32 Start, UInt32 Indents, ScriptBlockType Type, bool Override) :
		ControlBlock(ControlBlockType::ScriptBlock, Start, Indents, nullptr), SBAttribute(ScriptBlockAttribute::None), SBType(Type), CompilerOverride(Override)
	{
		switch (SBType)
		{
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnActivate:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnActorEquip:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnActorUnequip:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnAdd:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnAlarm:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnAlarmVictim:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnDeath:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnDrop:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnEquip:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnHit:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnHitWith:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnKnockout:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnLoad:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnMagicEffectHit:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnMurder:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnPackageChange:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnPackageDone:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnPackageStart:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnReset:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnSell:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnTrigger:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnTriggerActor:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnTriggerMob:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::OnUnequip:
			SBAttribute = ScriptBlockAttribute::ReferenceSpecific;
			break;
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::ScriptEffectFinish:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::ScriptEffectStart:
		case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType::ScriptEffectUpdate:
			SBAttribute = ScriptBlockAttribute::MagicEffect;
			break;
		default:
			SBAttribute = ScriptBlockAttribute::Universal;
			break;
		}
	}

	bool ObScriptSemanticAnalysis::ScriptBlock::IsBlockValid(ScriptType Type)
	{
		bool Result = true;

		switch (Type)
		{
		case ScriptType::MagicEffect:
			if (SBAttribute == ScriptBlockAttribute::ReferenceSpecific)
				Result = false;
			break;
		case ScriptType::Object:
			if (SBAttribute == ScriptBlockAttribute::MagicEffect)
				Result = false;
			break;
		case ScriptType::Quest:
			if (SBAttribute != ScriptBlockAttribute::Universal)
				Result = false;
			break;
		}

		return Result;
	}

	ObScriptSemanticAnalysis::ScriptBlock::ScriptBlockType ObScriptSemanticAnalysis::ScriptBlock::GetScriptBlockType(String^ TypeToken)
	{
		if (TypeToken->Length && TypeToken[0] == '_')
			TypeToken = TypeToken->Substring(1);

		if (!String::Compare(TypeToken, "GameMode", true))
			return ScriptBlockType::GameMode;
		else if (!String::Compare(TypeToken, "MenuMode", true))
			return ScriptBlockType::MenuMode;
		else if (!String::Compare(TypeToken, "OnActivate", true))
			return ScriptBlockType::OnActivate;
		else if (!String::Compare(TypeToken, "OnActorEquip", true))
			return ScriptBlockType::OnActorEquip;
		else if (!String::Compare(TypeToken, "OnActorUnequip", true))
			return ScriptBlockType::OnActorUnequip;
		else if (!String::Compare(TypeToken, "OnAdd", true))
			return ScriptBlockType::OnAdd;
		else if (!String::Compare(TypeToken, "OnAlarm", true))
			return ScriptBlockType::OnAlarm;
		else if (!String::Compare(TypeToken, "OnAlarmVictim", true))
			return ScriptBlockType::OnAlarmVictim;
		else if (!String::Compare(TypeToken, "OnDeath", true))
			return ScriptBlockType::OnDeath;
		else if (!String::Compare(TypeToken, "OnDrop", true))
			return ScriptBlockType::OnDrop;
		else if (!String::Compare(TypeToken, "OnEquip", true))
			return ScriptBlockType::OnEquip;
		else if (!String::Compare(TypeToken, "OnHit", true))
			return ScriptBlockType::OnHit;
		else if (!String::Compare(TypeToken, "OnHitWith", true))
			return ScriptBlockType::OnHitWith;
		else if (!String::Compare(TypeToken, "OnKnockout", true))
			return ScriptBlockType::OnKnockout;
		else if (!String::Compare(TypeToken, "OnLoad", true))
			return ScriptBlockType::OnLoad;
		else if (!String::Compare(TypeToken, "OnMagicEffectHit", true))
			return ScriptBlockType::OnMagicEffectHit;
		else if (!String::Compare(TypeToken, "OnMurder", true))
			return ScriptBlockType::OnMurder;
		else if (!String::Compare(TypeToken, "OnPackageChange", true))
			return ScriptBlockType::OnPackageChange;
		else if (!String::Compare(TypeToken, "OnPackageDone", true))
			return ScriptBlockType::OnPackageDone;
		else if (!String::Compare(TypeToken, "OnPackageStart", true))
			return ScriptBlockType::OnPackageStart;
		else if (!String::Compare(TypeToken, "OnReset", true))
			return ScriptBlockType::OnReset;
		else if (!String::Compare(TypeToken, "OnSell", true))
			return ScriptBlockType::OnSell;
		else if (!String::Compare(TypeToken, "OnTrigger", true))
			return ScriptBlockType::OnTrigger;
		else if (!String::Compare(TypeToken, "OnTriggerActor", true))
			return ScriptBlockType::OnTriggerActor;
		else if (!String::Compare(TypeToken, "OnTriggerMob", true))
			return ScriptBlockType::OnTriggerMob;
		else if (!String::Compare(TypeToken, "OnUnequip", true))
			return ScriptBlockType::OnUnequip;
		else if (!String::Compare(TypeToken, "ScriptEffectFinish", true))
			return ScriptBlockType::ScriptEffectFinish;
		else if (!String::Compare(TypeToken, "ScriptEffectStart", true))
			return ScriptBlockType::ScriptEffectStart;
		else if (!String::Compare(TypeToken, "ScriptEffectUpdate", true))
			return ScriptBlockType::ScriptEffectUpdate;
		else if (!String::Compare(TypeToken, "GameMode", true))
			return ScriptBlockType::Function;
		else
			return ScriptBlockType::None;
	}

	bool ObScriptSemanticAnalysis::ScriptBlock::HasCompilerOverride(String^ TypeToken)
	{
		if (TypeToken->Length && TypeToken[0] == '_')
			return true;
		else
			return false;
	}

	ObScriptSemanticAnalysis::AnalysisData::UserMessage::UserMessage(UInt32 Line, String^ Message, bool Critical) :
		Line(Line), Message(Message), Critical(Critical)
	{
	}

	ObScriptSemanticAnalysis::AnalysisData::AnalysisData()
	{
		Name = "Unknown";
		Description = "";
		Variables = gcnew List<Variable^>();
		ControlBlocks = gcnew List<ControlBlock^>();
		MalformedStructure = false;
		FirstStructuralErrorLine = 0;
		HasCriticalIssues = false;
		UDF = false;
		UDFResult = nullptr;
		AnalysisMessages = gcnew List<UserMessage^>();
	}

	void ObScriptSemanticAnalysis::AnalysisData::PerformAnalysis(String^ ScriptText, ScriptType Type, Operation Operations, CheckVariableNameCollision^ Delegate)
	{
		Tokenizer^ Parser = gcnew Tokenizer();
		CSEStringReader^ Reader = gcnew CSEStringReader(ScriptText);
		Stack<ControlBlock::ControlBlockType>^ StructureStack = gcnew Stack<ControlBlock::ControlBlockType>();
		Stack<ControlBlock^>^ BlockStack = gcnew Stack<ControlBlock^>();
		bool SaveDefinitionComments = false;

		Name = "";
		Description = "";

		if (Operations.HasFlag(Operation::FillVariables))
		{
			Variables->Clear();
			UDFResult = nullptr;
		}

		if (Operations.HasFlag(Operation::FillControlBlocks))
		{
			ControlBlocks->Clear();
			UDF = false;
			MalformedStructure = false;
			FirstStructuralErrorLine = 0;
		}

		if (Operations.HasFlag(Operation::PerformBasicValidation))
		{
			AnalysisMessages->Clear();
		}

		StructureStack->Push(ControlBlock::ControlBlockType::None);

		for (String^ ReadLine = Reader->ReadLine(); ReadLine != nullptr; ReadLine = Reader->ReadLine())
		{
			UInt32 CurrentLine = Reader->LineNumber;

			Parser->Tokenize(ReadLine, false);
			if (Parser->Valid)
			{
				if (SaveDefinitionComments && Parser->GetFirstTokenType() == ScriptTokenType::Comment)
					Description += ReadLine->Substring(Parser->Indices[0] + 1) + "\n";

				String^ FirstToken = Parser->Tokens[0];
				String^ SecondToken = (Parser->TokenCount > 1) ? Parser->Tokens[1] : "";
				ScriptTokenType FirstTokenType = Parser->GetFirstTokenType();
				bool EncounteredProblem = false;
				bool RetokenizeCurrentLine = false;

				switch (FirstTokenType)
				{
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::ScriptName:
					SaveDefinitionComments = true;

					if (Operations.HasFlag(Operation::PerformBasicValidation))
					{
						if (Name != "")
							LogCriticalAnalysisMessage(CurrentLine, "Redeclaration of script name."), EncounteredProblem = true;
						else if (Parser->HasIllegalChar(SecondToken, "_", ""))
							LogCriticalAnalysisMessage(CurrentLine, "Script name contains invalid characters."), EncounteredProblem = true;
					}

					if (EncounteredProblem == false)
						Name = SecondToken;

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::Variable:
					SaveDefinitionComments = false;

					if (SecondToken != "")
					{
						String^ Comment = "";
						if (Parser->TokenCount > 2 && Parser->GetCommentTokenIndex(Parser->TokenCount) == 2)
						{
							Comment = ReadLine->Substring(ReadLine->IndexOf(";") + 1);
							Parser->Tokenize(Comment, false);
							RetokenizeCurrentLine = true;
							Comment = (Parser->Indices->Count > 0) ? Comment->Substring(Parser->Indices[0]) : Comment;
						}

						if (Operations.HasFlag(Operation::PerformBasicValidation))
						{
							if (StructureStack->Peek() != ControlBlock::ControlBlockType::None)
								LogCriticalAnalysisMessage(CurrentLine, "Variable declared inside a script block."), EncounteredProblem = true;

							Variable^ Existing = LookupVariable(SecondToken);
							if (Existing != nullptr)
								LogCriticalAnalysisMessage(CurrentLine, "Variable redeclaration. Previous declaration was at line " + Existing->Line), EncounteredProblem = true;

							bool CommandCollision = false, FormCollision = false;
							if (Delegate)
								Delegate(SecondToken, CommandCollision, FormCollision);

							if (Operations.HasFlag(Operation::CheckVariableNameFormCollisions) && FormCollision)
								LogCriticalAnalysisMessage(CurrentLine, "The identifier " + SecondToken + " has already been assigned to a record."), EncounteredProblem = true;

							if (Operations.HasFlag(Operation::CheckVariableNameCommandCollisions) && CommandCollision)
								LogCriticalAnalysisMessage(CurrentLine, "The identifier " + SecondToken + " is reserved for a script command."), EncounteredProblem = true;
						}

						if (EncounteredProblem == false)
							Variables->Add(gcnew Variable(SecondToken, Variable::GetVariableDataType(FirstToken), Comment, CurrentLine));
					}

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::Begin:
					SaveDefinitionComments = false;

					if (Operations.HasFlag(Operation::FillControlBlocks))
					{
						if (StructureStack->Peek() != ControlBlock::ControlBlockType::None)
						{
							EncounteredProblem = true;
							MalformedStructure = true;
							FirstStructuralErrorLine = CurrentLine;

							if (Operations.HasFlag(Operation::PerformBasicValidation))
								LogCriticalAnalysisMessage(CurrentLine, "Nested Begin block.");
						}
						else
						{
							ScriptBlock^ NewBlock = gcnew ScriptBlock(CurrentLine, 1,
																	  ScriptBlock::GetScriptBlockType(SecondToken),
																	  ScriptBlock::HasCompilerOverride(SecondToken));
							StructureStack->Push(ControlBlock::ControlBlockType::ScriptBlock);
							BlockStack->Push(NewBlock);
							ControlBlocks->Add(NewBlock);

							if (Operations.HasFlag(Operation::FillUDFData) && NewBlock->SBType == ScriptBlock::ScriptBlockType::Function)
							{
								int FirstIdx = ReadLine->IndexOf("{");
								int ParamIdx = 0;
								String^ ParamList = ReadLine->Substring(FirstIdx, ReadLine->IndexOf("}") - FirstIdx);
								Parser->Tokenize(ParamList, false);
								RetokenizeCurrentLine = true;

								for each (String^ Itr in Parser->Tokens)
								{
									Variable^ Existing = LookupVariable(Itr);
									if (Existing)
									{
										Existing->UDFParameter = true;
										Existing->ParameterIndex = ParamIdx;
									}
									ParamIdx++;
								}
							}
						}
					}

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::End:
					if (Operations.HasFlag(Operation::FillControlBlocks))
					{
						if (Operations.HasFlag(Operation::PerformBasicValidation) && Parser->TokenCount > 1 && Parser->Tokens[1][0] != ';')
							LogAnalysisMessage(CurrentLine, "Redundant expression beyond block end specifier.");

						if (StructureStack->Peek() != ControlBlock::ControlBlockType::ScriptBlock)
						{
							EncounteredProblem = true;
							MalformedStructure = true;
							FirstStructuralErrorLine = CurrentLine;

							if (Operations.HasFlag(Operation::PerformBasicValidation))
								LogCriticalAnalysisMessage(CurrentLine, "Mismatching block.");
						}
						else
						{
							StructureStack->Pop();
							BlockStack->Pop()->EndLine = CurrentLine;
						}
					}

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::While:
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::ForEach:
					if (Operations.HasFlag(Operation::FillControlBlocks))
					{
						if (StructureStack->Peek() == ControlBlock::ControlBlockType::None)
						{
							EncounteredProblem = true;
							MalformedStructure = true;
							FirstStructuralErrorLine = CurrentLine;

							if (Operations.HasFlag(Operation::PerformBasicValidation))
								LogCriticalAnalysisMessage(CurrentLine, "Control block declared outside script block.");
						}
						else
						{
							ControlBlock^ Parent = BlockStack->Peek();
							ControlBlock^ NewBlock = gcnew ControlBlock(ControlBlock::ControlBlockType::Loop, CurrentLine,
																		Parent->IndentLevel + 1, Parent);

							StructureStack->Push(ControlBlock::ControlBlockType::Loop);
							BlockStack->Push(NewBlock);
							ControlBlocks->Add(NewBlock);
						}
					}

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::Loop:
					if (Operations.HasFlag(Operation::FillControlBlocks))
					{
						if (Operations.HasFlag(Operation::PerformBasicValidation) && Parser->TokenCount > 1 && Parser->Tokens[1][0] != ';')
							LogAnalysisMessage(CurrentLine, "Redundant expression beyond block end specifier.");

						if (StructureStack->Peek() != ControlBlock::ControlBlockType::Loop)
						{
							EncounteredProblem = true;
							MalformedStructure = true;
							FirstStructuralErrorLine = CurrentLine;

							if (Operations.HasFlag(Operation::PerformBasicValidation))
								LogCriticalAnalysisMessage(CurrentLine, "Mismatching block.");
						}
						else
						{
							StructureStack->Pop();
							BlockStack->Pop()->EndLine = CurrentLine;
						}
					}

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::If:
					if (Operations.HasFlag(Operation::FillControlBlocks))
					{
						if (StructureStack->Peek() == ControlBlock::ControlBlockType::None)
						{
							EncounteredProblem = true;
							MalformedStructure = true;
							FirstStructuralErrorLine = CurrentLine;

							if (Operations.HasFlag(Operation::PerformBasicValidation))
								LogCriticalAnalysisMessage(CurrentLine, "Control block declared outside script block.");
						}
						else
						{
							if (Operations.HasFlag(Operation::PerformBasicValidation) && (Parser->TokenCount < 2 || Parser->Tokens[1][0] == ';'))
								LogCriticalAnalysisMessage(CurrentLine, "Invalid condition."), EncounteredProblem = true;

							ControlBlock^ Parent = BlockStack->Peek();
							ControlBlock^ NewBlock = gcnew ControlBlock(ControlBlock::ControlBlockType::If, CurrentLine,
																		Parent->IndentLevel + 1, Parent);

							StructureStack->Push(ControlBlock::ControlBlockType::If);
							BlockStack->Push(NewBlock);
							ControlBlocks->Add(NewBlock);
						}
					}

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::ElseIf:
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::Else:
					if (Operations.HasFlag(Operation::FillControlBlocks))
					{
						if (Operations.HasFlag(Operation::PerformBasicValidation) && FirstTokenType == ScriptTokenType::ElseIf &&
							(Parser->TokenCount < 2 || Parser->Tokens[1][0] == ';'))
						{
							LogCriticalAnalysisMessage(CurrentLine, "Invalid condition.");
							EncounteredProblem = true;
						}

						if (StructureStack->Peek() != ControlBlock::ControlBlockType::If && StructureStack->Peek() != ControlBlock::ControlBlockType::ElseIf)
						{
							EncounteredProblem = true;
							MalformedStructure = true;
							FirstStructuralErrorLine = CurrentLine;

							if (Operations.HasFlag(Operation::PerformBasicValidation))
								LogCriticalAnalysisMessage(CurrentLine, "Mismatching block.");
						}
						else
						{
							StructureStack->Pop();
							ControlBlock^ PrevBlock = BlockStack->Pop();
							PrevBlock->EndLine = CurrentLine;

							ControlBlock::ControlBlockType BlockType = ControlBlock::ControlBlockType::ElseIf;
							if (FirstTokenType == ScriptTokenType::Else)
								BlockType = ControlBlock::ControlBlockType::Else;

							ControlBlock^ NewBlock = gcnew ControlBlock(BlockType, CurrentLine,
																		PrevBlock->IndentLevel, PrevBlock->Parent);

							StructureStack->Push(BlockType);
							BlockStack->Push(NewBlock);
							ControlBlocks->Add(NewBlock);
						}
					}

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::EndIf:
					if (Operations.HasFlag(Operation::FillControlBlocks))
					{
						if (Operations.HasFlag(Operation::PerformBasicValidation) && Parser->TokenCount > 1 && Parser->Tokens[1][0] != ';')
							LogAnalysisMessage(CurrentLine, "Redundant expression beyond block end specifier.");

						if (StructureStack->Peek() != ControlBlock::ControlBlockType::If &&
							StructureStack->Peek() != ControlBlock::ControlBlockType::ElseIf &&
							StructureStack->Peek() != ControlBlock::ControlBlockType::Else)
						{
							EncounteredProblem = true;
							MalformedStructure = true;
							FirstStructuralErrorLine = CurrentLine;

							if (Operations.HasFlag(Operation::PerformBasicValidation))
								LogCriticalAnalysisMessage(CurrentLine, "Mismatching block.");
						}
						else
						{
							StructureStack->Pop();
							BlockStack->Pop()->EndLine = CurrentLine;
						}
					}

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::Return:
					if (Operations.HasFlag(Operation::PerformBasicValidation) && Parser->TokenCount > 1 && Parser->Tokens[1][0] != ';')
						LogAnalysisMessage(CurrentLine, "Redundant expression beyond block end specifier.");

					break;
				case ConstructionSetExtender::ObScriptSemanticAnalysis::ScriptTokenType::SetFunctionValue:
					if (Operations.HasFlag(Operation::FillUDFData))
					{
						Variable^ Existing = LookupVariable(SecondToken);
						if (Existing)
							UDFResult = Existing;
					}

					break;
				default:
					SaveDefinitionComments = false;
					break;
				}

				if (Operations.HasFlag(Operation::PerformBasicValidation) && Operations.HasFlag(Operation::CountVariableReferences))
				{
					UInt32 Count = 0;
					if (FirstTokenType != ScriptTokenType::Variable)
					{
						if (RetokenizeCurrentLine)
							Parser->Tokenize(ReadLine, false);

						for each (String^ Itr in Parser->Tokens)
						{
							Variable^ Existing = LookupVariable(Itr);
							if (Existing)
							{
								if (Count == 0 || Parser->Delimiters[Count - 1] != '.')
								{
									if (Parser->GetCommentTokenIndex(Count) == -1)
										Existing->RefCount++;
								}
							}
							Count++;
						}
					}
				}
			}
		}

		if (Operations.HasFlag(Operation::PerformBasicValidation))
		{
			for each (Variable^ Itr in Variables)
			{
				if (Operations.HasFlag(Operation::CountVariableReferences))
				{
					if (Itr->RefCount == 0 && (Type != ScriptType::Quest || Operations.HasFlag(Operation::SuppressQuestVariableRefCount) == false))
						LogAnalysisMessage(Itr->Line, "Variable " + Itr->Name + " unreferenced in local context.");
				}

				bool InvalidVarName = false;
				try	{
					UInt32 Temp = int::Parse(Itr->Name);
					InvalidVarName = true;
				}
				catch (...) {}

				if (InvalidVarName)
					LogCriticalAnalysisMessage(Itr->Line, "Variable '" + Itr->Name + "' has an all-numeric identifier.");
			}
		}

		if (Operations.HasFlag(Operation::PerformBasicValidation))
		{
			for each (ControlBlock^ Itr in ControlBlocks)
			{
				ScriptBlock^ Block = dynamic_cast<ScriptBlock^>(Itr);
				if (Block && Block->IsMalformed() == false && Block->IsBlockValid(Type) == false)
				{
					LogCriticalAnalysisMessage(Block->StartLine, "Script block not supported by current script type.");
				}
			}
		}
	}

	ObScriptSemanticAnalysis::Variable^ ObScriptSemanticAnalysis::AnalysisData::LookupVariable(String^ VarName)
	{
		for each (Variable^ Itr in Variables)
		{
			if (!String::Compare(Itr->Name, VarName, true))
				return Itr;
		}

		return nullptr;
	}

	void ObScriptSemanticAnalysis::AnalysisData::LogAnalysisMessage(UInt32 Line, String^ Message)
	{
		AnalysisMessages->Add(gcnew UserMessage(Line, Message, false));
	}

	void ObScriptSemanticAnalysis::AnalysisData::LogCriticalAnalysisMessage(UInt32 Line, String^ Message)
	{
		HasCriticalIssues = true;
		AnalysisMessages->Add(gcnew UserMessage(Line, Message, true));
	}
}