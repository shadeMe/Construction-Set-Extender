#include "SemanticAnalysis.h"

namespace cse
{


namespace scriptEditor
{


namespace obScriptParsing
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

	if (Variable::GetVariableDataType(Extract) != Variable::eDataType::None)
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

LineTokenizer::LineTokenizer()
{
	Tokens = gcnew List<String^>();
	Indices = gcnew List<UInt32>();
	Delimiters = gcnew List<Char>();
	Good = false;

	this->ReferenceDelimiters = gcnew String(DefaultDelimiters);
	this->ReferenceControlChars = gcnew String(DefaultControlChars);
}

LineTokenizer::LineTokenizer(String^ InputDelimiters, String^ InputControlChars)
{
	Tokens = gcnew List<String^>();
	Indices = gcnew List<UInt32>();
	Delimiters = gcnew List<Char>();
	Good = false;

	this->ReferenceDelimiters = gcnew String(InputDelimiters);
	this->ReferenceControlChars = gcnew String(InputControlChars);
}

LineTokenizer::LineTokenizer(String^ InputDelimiters)
{
	Tokens = gcnew List<String^>();
	Indices = gcnew List<UInt32>();
	Delimiters = gcnew List<Char>();
	Good = false;

	this->ReferenceDelimiters = gcnew String(InputDelimiters);
	this->ReferenceControlChars = gcnew String(DefaultControlChars);
}

bool LineTokenizer::Tokenize(String^ Source, bool CollectEmptyTokens)
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

	if (StartPos != -1)
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

void LineTokenizer::ResetState()
{
	Tokens->Clear();
	Delimiters->Clear();
	Indices->Clear();
	Good = false;
}

eScriptTokenType LineTokenizer::GetFirstTokenType(void)
{
	if (Good)
		return GetScriptTokenType(Tokens[0]);
	else
		return eScriptTokenType::None;
}

int LineTokenizer::GetCommentTokenIndex(int SearchEndIndex)
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

int LineTokenizer::GetTokenIndex(String^ Source)
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

bool LineTokenizer::IsIndexInsideString(int Index)
{
	if (!Good)
		return false;

	for (int i = 0; i < TokenCount; ++i)
	{
		String^ Token = Tokens[i];
		int StartIndex = Indices[i];

		if (Token[0] == '\"'
			&& Index > StartIndex
			&& Index < StartIndex + Token->Length - (Token[Token->Length - 1] == '\"' ? 1 : 0))
			return true;
	}

	return false;
}

eScriptTokenType LineTokenizer::GetScriptTokenType(String^ ScriptToken)
{
	eScriptTokenType Result = eScriptTokenType::None;
	if (ScriptToken->Length)
	{
		if (!String::Compare(ScriptToken, "scn", true) || !String::Compare(ScriptToken, "scriptName", true))
			Result = eScriptTokenType::ScriptName;
		else if (Variable::GetVariableDataType(ScriptToken) != Variable::eDataType::None)
			Result = eScriptTokenType::Variable;
		else if (ScriptToken[0] == ';')
			Result = eScriptTokenType::Comment;
		else if (!String::Compare(ScriptToken, "begin", true))
			Result = eScriptTokenType::Begin;
		else if (!String::Compare(ScriptToken, "end", true))
			Result = eScriptTokenType::End;
		else if (!String::Compare(ScriptToken, "while", true))
			Result = eScriptTokenType::While;
		else if (!String::Compare(ScriptToken, "foreach", true))
			Result = eScriptTokenType::ForEach;
		else if (!String::Compare(ScriptToken, "loop", true))
			Result = eScriptTokenType::Loop;
		else if (!String::Compare(ScriptToken, "if", true))
			Result = eScriptTokenType::If;
		else if (!String::Compare(ScriptToken, "elseif", true))
			Result = eScriptTokenType::ElseIf;
		else if (!String::Compare(ScriptToken, "else", true))
			Result = eScriptTokenType::Else;
		else if (!String::Compare(ScriptToken, "endif", true))
			Result = eScriptTokenType::EndIf;
		else if (!String::Compare(ScriptToken, "return", true))
			Result = eScriptTokenType::Return;
		else if (!String::Compare(ScriptToken, "setfunctionvalue", true))
			Result = eScriptTokenType::SetFunctionValue;
		else if (!String::Compare(ScriptToken, "set", true))
			Result = eScriptTokenType::Set;
		else if (!String::Compare(ScriptToken, "let", true))
			Result = eScriptTokenType::Let;
		else if (!String::Compare(ScriptToken, "call", true))
			Result = eScriptTokenType::Call;
		else if (!String::Compare(ScriptToken, "player", true) || !String::Compare(ScriptToken, "playerref", true))
			Result = eScriptTokenType::Player;
		else if (!String::Compare(ScriptToken, "seteventhandler", true))
			Result = eScriptTokenType::SetEventHandler;
		else if (!String::Compare(ScriptToken, "removeeventhandler", true))
			Result = eScriptTokenType::RemoveEventHandler;
		else if (!String::Compare(ScriptToken, "dispatchevent", true))
			Result = eScriptTokenType::DispatchEvent;
	}

	return Result;
}

bool LineTokenizer::GetIndexInsideString(String^ Source, int Index)
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

bool LineTokenizer::HasIllegalChar(String^ Source, String^ Includes, String^ Excludes)
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

Variable::Variable(String^ Name, eDataType Type, String^ Comment, UInt32 Line) :
	Name(Name), Comment(Comment), Type(Type), UDFParameter(false), ParameterIndex(0), Line(Line), RefCount(0)
{
}

Variable::eDataType Variable::GetVariableDataType(String^ TypeToken)
{
	eDataType VarType = eDataType::None;

	if (!String::Compare(TypeToken, "ref", true) || !String::Compare(TypeToken, "reference", true))
		VarType = Variable::eDataType::Ref;
	else if (!String::Compare(TypeToken, "short", true) || !String::Compare(TypeToken, "long", true) || !String::Compare(TypeToken, "int", true))
		VarType = Variable::eDataType::Integer;
	else if (!String::Compare(TypeToken, "float", true))
		VarType = Variable::eDataType::Float;
	else if (!String::Compare(TypeToken, "string_var", true))
		VarType = Variable::eDataType::StringVar;
	else if (!String::Compare(TypeToken, "array_var", true))
		VarType = Variable::eDataType::ArrayVar;

	return VarType;
}

String^ Variable::GetVariableDataTypeToken(Variable::eDataType Type)
{
	switch (Type)
	{
	case eDataType::Integer:
		return "int";
	case eDataType::Float:
		return "float";
	case eDataType::Ref:
		return "ref";
	case eDataType::StringVar:
		return "string_var";
	case eDataType::ArrayVar:
		return "array_var";
	default:
		return "unk";
	}
}

String^ Variable::GetVariableDataTypeDescription(Variable::eDataType Type)
{
	switch (Type)
	{
	case eDataType::Integer:
		return "Integer";
	case eDataType::Float:
		return "Float";
	case eDataType::Ref:
		return "Reference";
	case eDataType::StringVar:
		return "String";
	case eDataType::ArrayVar:
		return "Array";
	default:
		return "Unknown";
	}
}

ControlBlock::ControlBlock(eControlBlockType Type, UInt32 Start, UInt32 Indents, ControlBlock^ Parent) :
	Type(Type), StartLine(Start), EndLine(0), IndentLevel(Indents), Parent(Parent), BasicBlock(true), OuterEndLine(0),
	ChildBlocks(gcnew List<ControlBlock^>)
{
	if (Parent)
		Parent->ChildBlocks->Add(this);
}

bool ControlBlock::IsMalformed()
{
	return EndLine == 0;
}

ScriptBlock::ScriptBlock(UInt32 Start, UInt32 Indents, eScriptBlockType Type, bool Override) :
	ControlBlock(eControlBlockType::ScriptBlock, Start, Indents, nullptr), ScriptBlockAttribute(eScriptBlockAttribute::None), ScriptBlockType(Type), CompilerOverride(Override)
{
	switch (ScriptBlockType)
	{
	case ScriptBlock::eScriptBlockType::OnActivate:
	case ScriptBlock::eScriptBlockType::OnActorEquip:
	case ScriptBlock::eScriptBlockType::OnActorUnequip:
	case ScriptBlock::eScriptBlockType::OnAdd:
	case ScriptBlock::eScriptBlockType::OnAlarm:
	case ScriptBlock::eScriptBlockType::OnAlarmVictim:
	case ScriptBlock::eScriptBlockType::OnDeath:
	case ScriptBlock::eScriptBlockType::OnDrop:
	case ScriptBlock::eScriptBlockType::OnEquip:
	case ScriptBlock::eScriptBlockType::OnHit:
	case ScriptBlock::eScriptBlockType::OnHitWith:
	case ScriptBlock::eScriptBlockType::OnKnockout:
	case ScriptBlock::eScriptBlockType::OnLoad:
	case ScriptBlock::eScriptBlockType::OnMagicEffectHit:
	case ScriptBlock::eScriptBlockType::OnMurder:
	case ScriptBlock::eScriptBlockType::OnPackageChange:
	case ScriptBlock::eScriptBlockType::OnPackageDone:
	case ScriptBlock::eScriptBlockType::OnPackageStart:
	case ScriptBlock::eScriptBlockType::OnReset:
	case ScriptBlock::eScriptBlockType::OnSell:
	case ScriptBlock::eScriptBlockType::OnTrigger:
	case ScriptBlock::eScriptBlockType::OnTriggerActor:
	case ScriptBlock::eScriptBlockType::OnTriggerMob:
	case ScriptBlock::eScriptBlockType::OnUnequip:
		ScriptBlockAttribute = eScriptBlockAttribute::ReferenceSpecific;
		break;
	case ScriptBlock::eScriptBlockType::ScriptEffectFinish:
	case ScriptBlock::eScriptBlockType::ScriptEffectStart:
	case ScriptBlock::eScriptBlockType::ScriptEffectUpdate:
		ScriptBlockAttribute = eScriptBlockAttribute::MagicEffect;
		break;
	default:
		ScriptBlockAttribute = eScriptBlockAttribute::Universal;
		break;
	}
}

bool ScriptBlock::IsBlockValid(eScriptType Type)
{
	bool Result = true;

	switch (Type)
	{
	case eScriptType::MagicEffect:
		if (ScriptBlockAttribute == eScriptBlockAttribute::ReferenceSpecific)
			Result = false;
		break;
	case eScriptType::Object:
		if (ScriptBlockAttribute == eScriptBlockAttribute::MagicEffect)
			Result = false;
		break;
	case eScriptType::Quest:
		if (ScriptBlockAttribute != eScriptBlockAttribute::Universal)
			Result = false;
		break;
	}

	return Result;
}

ScriptBlock::eScriptBlockType ScriptBlock::GetScriptBlockType(String^ TypeToken)
{
	if (TypeToken->Length && TypeToken[0] == '_')
		TypeToken = TypeToken->Substring(1);

	if (!String::Compare(TypeToken, "GameMode", true))
		return eScriptBlockType::GameMode;
	else if (!String::Compare(TypeToken, "MenuMode", true))
		return eScriptBlockType::MenuMode;
	else if (!String::Compare(TypeToken, "OnActivate", true))
		return eScriptBlockType::OnActivate;
	else if (!String::Compare(TypeToken, "OnActorEquip", true))
		return eScriptBlockType::OnActorEquip;
	else if (!String::Compare(TypeToken, "OnActorUnequip", true))
		return eScriptBlockType::OnActorUnequip;
	else if (!String::Compare(TypeToken, "OnAdd", true))
		return eScriptBlockType::OnAdd;
	else if (!String::Compare(TypeToken, "OnAlarm", true))
		return eScriptBlockType::OnAlarm;
	else if (!String::Compare(TypeToken, "OnAlarmVictim", true))
		return eScriptBlockType::OnAlarmVictim;
	else if (!String::Compare(TypeToken, "OnDeath", true))
		return eScriptBlockType::OnDeath;
	else if (!String::Compare(TypeToken, "OnDrop", true))
		return eScriptBlockType::OnDrop;
	else if (!String::Compare(TypeToken, "OnEquip", true))
		return eScriptBlockType::OnEquip;
	else if (!String::Compare(TypeToken, "OnHit", true))
		return eScriptBlockType::OnHit;
	else if (!String::Compare(TypeToken, "OnHitWith", true))
		return eScriptBlockType::OnHitWith;
	else if (!String::Compare(TypeToken, "OnKnockout", true))
		return eScriptBlockType::OnKnockout;
	else if (!String::Compare(TypeToken, "OnLoad", true))
		return eScriptBlockType::OnLoad;
	else if (!String::Compare(TypeToken, "OnMagicEffectHit", true))
		return eScriptBlockType::OnMagicEffectHit;
	else if (!String::Compare(TypeToken, "OnMurder", true))
		return eScriptBlockType::OnMurder;
	else if (!String::Compare(TypeToken, "OnPackageChange", true))
		return eScriptBlockType::OnPackageChange;
	else if (!String::Compare(TypeToken, "OnPackageDone", true))
		return eScriptBlockType::OnPackageDone;
	else if (!String::Compare(TypeToken, "OnPackageStart", true))
		return eScriptBlockType::OnPackageStart;
	else if (!String::Compare(TypeToken, "OnReset", true))
		return eScriptBlockType::OnReset;
	else if (!String::Compare(TypeToken, "OnSell", true))
		return eScriptBlockType::OnSell;
	else if (!String::Compare(TypeToken, "OnTrigger", true))
		return eScriptBlockType::OnTrigger;
	else if (!String::Compare(TypeToken, "OnTriggerActor", true))
		return eScriptBlockType::OnTriggerActor;
	else if (!String::Compare(TypeToken, "OnTriggerMob", true))
		return eScriptBlockType::OnTriggerMob;
	else if (!String::Compare(TypeToken, "OnUnequip", true))
		return eScriptBlockType::OnUnequip;
	else if (!String::Compare(TypeToken, "ScriptEffectFinish", true))
		return eScriptBlockType::ScriptEffectFinish;
	else if (!String::Compare(TypeToken, "ScriptEffectStart", true))
		return eScriptBlockType::ScriptEffectStart;
	else if (!String::Compare(TypeToken, "ScriptEffectUpdate", true))
		return eScriptBlockType::ScriptEffectUpdate;
	else if (!String::Compare(TypeToken, "Function", true))
		return eScriptBlockType::Function;
	else
		return eScriptBlockType::None;
}

bool ScriptBlock::HasCompilerOverride(String^ TypeToken)
{
	if (TypeToken->Length && TypeToken[0] == '_')
		return true;
	else
		return false;
}

String^ ScriptBlock::GetScriptBlockTypeToken(eScriptBlockType Type)
{
	switch (Type)
	{
	case ScriptBlock::eScriptBlockType::GameMode:
		return "GameMode";
	case ScriptBlock::eScriptBlockType::MenuMode:
		return "MenuMode";
	case ScriptBlock::eScriptBlockType::OnActivate:
		return "OnActivate";
	case ScriptBlock::eScriptBlockType::OnActorEquip:
		return "OnActorEquip";
	case ScriptBlock::eScriptBlockType::OnActorUnequip:
		return "OnActorUnequip";
	case ScriptBlock::eScriptBlockType::OnAdd:
		return "OnAdd";
	case ScriptBlock::eScriptBlockType::OnAlarm:
		return "OnAlarm";
	case ScriptBlock::eScriptBlockType::OnAlarmVictim:
		return "OnAlarmVictim";
	case ScriptBlock::eScriptBlockType::OnDeath:
		return "OnDeath";
	case ScriptBlock::eScriptBlockType::OnDrop:
		return "OnDrop";
	case ScriptBlock::eScriptBlockType::OnEquip:
		return "OnEquip";
	case ScriptBlock::eScriptBlockType::OnHit:
		return "OnHit";
	case ScriptBlock::eScriptBlockType::OnHitWith:
		return "OnHitWith";
	case ScriptBlock::eScriptBlockType::OnKnockout:
		return "OnKnockout";
	case ScriptBlock::eScriptBlockType::OnLoad:
		return "OnLoad";
	case ScriptBlock::eScriptBlockType::OnMagicEffectHit:
		return "OnMagicEffectHit";
	case ScriptBlock::eScriptBlockType::OnMurder:
		return "OnMurder";
	case ScriptBlock::eScriptBlockType::OnPackageChange:
		return "OnPackageChange";
	case ScriptBlock::eScriptBlockType::OnPackageDone:
		return "OnPackageDone";
	case ScriptBlock::eScriptBlockType::OnPackageStart:
		return "OnPackageStart";
	case ScriptBlock::eScriptBlockType::OnReset:
		return "OnReset";
	case ScriptBlock::eScriptBlockType::OnSell:
		return "OnSell";
	case ScriptBlock::eScriptBlockType::OnTrigger:
		return "OnTrigger";
	case ScriptBlock::eScriptBlockType::OnTriggerActor:
		return "OnTriggerActor";
	case ScriptBlock::eScriptBlockType::OnTriggerMob:
		return "OnTriggerMob";
	case ScriptBlock::eScriptBlockType::OnUnequip:
		return "OnUnequip";
	case ScriptBlock::eScriptBlockType::ScriptEffectFinish:
		return "ScriptEffectFinish";
	case ScriptBlock::eScriptBlockType::ScriptEffectStart:
		return "ScriptEffectStart";
	case ScriptBlock::eScriptBlockType::ScriptEffectUpdate:
		return "ScriptEffectUpdate";
	case ScriptBlock::eScriptBlockType::Function:
		return "Function";
	default:
		return "None";
	}
}

AnalysisData::UserMessage::UserMessage(UInt32 Line, String^ Message, bool Critical) :
	Line(Line), Message(Message), Critical(Critical)
{
}

AnalysisData::AnalysisData()
{
	Name = "Unknown";
	Description = "";
	Variables = gcnew List<Variable^>();
	NextVariableLine = 0;
	ControlBlocks = gcnew List<ControlBlock^>();
	MalformedStructure = false;
	FirstStructuralErrorLine = 0;
	IsUDF = false;
	UDFResult = nullptr;
	UDFAmbiguousResult = false;
	AnalysisMessages = gcnew List<UserMessage^>();
}

AnalysisData::~AnalysisData()
{
	Variables->Clear();
	ControlBlocks->Clear();
	UDFResult = nullptr;
	AnalysisMessages->Clear();
}

AnalysisData^ AnalysisData::PerformAnalysis(Params^ Parameters)
{
	auto ScriptText = Parameters->ScriptText;
	auto Type = Parameters->Type;
	auto Operations = Parameters->Ops;
	auto ScriptCommandIdentifiers = Parameters->ScriptCommandIdentifiers;
	auto FormIdentifiers = Parameters->FormIdentifiers;

	LineTokenizer^ Parser = gcnew LineTokenizer();
	LineTrackingStringReader^ Reader = gcnew LineTrackingStringReader(ScriptText);
	Stack<ControlBlock::eControlBlockType>^ StructureStack = gcnew Stack<ControlBlock::eControlBlockType>();
	Stack<ControlBlock^>^ BlockStack = gcnew Stack<ControlBlock^>();
	Stack<ControlBlock^>^ IfStack = gcnew Stack<ControlBlock^>();
	bool SaveDefinitionComments = false;

	Name = "";
	Description = "";

#ifndef NDEBUG
	System::Diagnostics::Stopwatch^ Profiler = gcnew System::Diagnostics::Stopwatch();
	Profiler->Start();
#endif // !NDEBUG

	if (Operations.HasFlag(eOperation::FillVariables))
	{
		Variables->Clear();
		UDFResult = nullptr;
		UDFAmbiguousResult = false;
		NextVariableLine = 0;
	}

	if (Operations.HasFlag(eOperation::FillControlBlocks))
	{
		ControlBlocks->Clear();
		IsUDF = false;
		MalformedStructure = false;
		FirstStructuralErrorLine = 0;
	}

	if (Operations.HasFlag(eOperation::PerformBasicValidation))
	{
		AnalysisMessages->Clear();
	}

	StructureStack->Push(ControlBlock::eControlBlockType::None);

	for (String^ ReadLine = Reader->ReadLine(); ReadLine != nullptr; ReadLine = Reader->ReadLine())
	{
		UInt32 CurrentLine = Reader->LineNumber;

		Parser->Tokenize(ReadLine, false);
		if (Parser->Valid)
		{
			if (SaveDefinitionComments && Parser->GetFirstTokenType() == eScriptTokenType::Comment)
			{
				Description += ReadLine->Substring(Parser->Indices[0] + 1)->Trim() + "\n";
				NextVariableLine = CurrentLine + 1;
			}

			String^ FirstToken = Parser->Tokens[0];
			String^ SecondToken = (Parser->TokenCount > 1) ? Parser->Tokens[1] : "";
			eScriptTokenType FirstTokenType = Parser->GetFirstTokenType();
			bool EncounteredProblem = false;
			bool RetokenizeCurrentLine = false;

			switch (FirstTokenType)
			{
			case eScriptTokenType::ScriptName:
				SaveDefinitionComments = true;

				if (Operations.HasFlag(eOperation::PerformBasicValidation))
				{
					if (Name != "")
						LogCriticalAnalysisMessage(CurrentLine, "Redeclaration of script name."), EncounteredProblem = true;
					else if (Parser->HasIllegalChar(SecondToken, "_", ""))
						LogCriticalAnalysisMessage(CurrentLine, "Script name contains invalid characters."), EncounteredProblem = true;
				}

				if (EncounteredProblem == false)
					Name = SecondToken;

				NextVariableLine = CurrentLine + 1;

				break;
			case eScriptTokenType::Variable:
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

					if (Operations.HasFlag(eOperation::PerformBasicValidation))
					{
						if (StructureStack->Peek() != ControlBlock::eControlBlockType::None)
							LogCriticalAnalysisMessage(CurrentLine, "Variable declared inside a script block."), EncounteredProblem = true;

						Variable^ Existing = LookupVariable(SecondToken);
						if (Existing != nullptr)
							LogCriticalAnalysisMessage(CurrentLine, "Variable redeclaration. Previous declaration was at line " + Existing->Line), EncounteredProblem = true;

						if (Operations.HasFlag(eOperation::CheckVariableNameFormCollisions) && FormIdentifiers->Contains(SecondToken))
							LogCriticalAnalysisMessage(CurrentLine, "The identifier " + SecondToken + " has already been assigned to a record."), EncounteredProblem = true;

						if (Operations.HasFlag(eOperation::CheckVariableNameCommandCollisions) && ScriptCommandIdentifiers->Contains(SecondToken))
							LogCriticalAnalysisMessage(CurrentLine, "The identifier " + SecondToken + " is reserved for a script command."), EncounteredProblem = true;
					}

					if (EncounteredProblem == false)
						Variables->Add(gcnew Variable(SecondToken, Variable::GetVariableDataType(FirstToken), Comment, CurrentLine));

					NextVariableLine = CurrentLine + 1;
				}

				break;
			case eScriptTokenType::Begin:
				SaveDefinitionComments = false;
				NextVariableLine = CurrentLine - 1;

				if (Operations.HasFlag(eOperation::FillControlBlocks))
				{
					if (StructureStack->Peek() != ControlBlock::eControlBlockType::None)
					{
						EncounteredProblem = true;
						MalformedStructure = true;
						FirstStructuralErrorLine = CurrentLine;

						if (Operations.HasFlag(eOperation::PerformBasicValidation))
							LogCriticalAnalysisMessage(CurrentLine, "Nested Begin block.");
					}
					else
					{
						ScriptBlock^ NewBlock = gcnew ScriptBlock(CurrentLine, 1,
																	ScriptBlock::GetScriptBlockType(SecondToken),
																	ScriptBlock::HasCompilerOverride(SecondToken));
						StructureStack->Push(ControlBlock::eControlBlockType::ScriptBlock);
						BlockStack->Push(NewBlock);
						ControlBlocks->Add(NewBlock);
						IsUDF = NewBlock->ScriptBlockType == ScriptBlock::eScriptBlockType::Function;

						if (Operations.HasFlag(eOperation::FillUDFData) && IsUDF)
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
			case eScriptTokenType::End:
				if (Operations.HasFlag(eOperation::FillControlBlocks))
				{
					if (Operations.HasFlag(eOperation::PerformBasicValidation) && Parser->TokenCount > 1 && Parser->Tokens[1][0] != ';')
						LogAnalysisMessage(CurrentLine, "Redundant expression beyond block end specifier.");

					if (StructureStack->Peek() != ControlBlock::eControlBlockType::ScriptBlock)
					{
						EncounteredProblem = true;
						MalformedStructure = true;
						FirstStructuralErrorLine = CurrentLine;

						if (Operations.HasFlag(eOperation::PerformBasicValidation))
							LogCriticalAnalysisMessage(CurrentLine, "Mismatching block.");
					}
					else
					{
						StructureStack->Pop();
						BlockStack->Pop()->EndLine = CurrentLine;
					}
				}

				break;
			case eScriptTokenType::While:
			case eScriptTokenType::ForEach:
				if (Operations.HasFlag(eOperation::FillControlBlocks))
				{
					if (StructureStack->Peek() == ControlBlock::eControlBlockType::None)
					{
						EncounteredProblem = true;
						MalformedStructure = true;
						FirstStructuralErrorLine = CurrentLine;

						if (Operations.HasFlag(eOperation::PerformBasicValidation))
							LogCriticalAnalysisMessage(CurrentLine, "Control block declared outside script block.");
					}
					else
					{
						ControlBlock^ Parent = BlockStack->Peek();
						ControlBlock::eControlBlockType BlockType = ControlBlock::eControlBlockType::While;
						if (FirstTokenType == eScriptTokenType::ForEach)
							BlockType = ControlBlock::eControlBlockType::ForEach;

						ControlBlock^ NewBlock = gcnew ControlBlock(BlockType, CurrentLine,
																	Parent->IndentLevel + 1, Parent);

						StructureStack->Push(BlockType);
						BlockStack->Push(NewBlock);
						ControlBlocks->Add(NewBlock);
					}
				}

				break;
			case eScriptTokenType::Loop:
				if (Operations.HasFlag(eOperation::FillControlBlocks))
				{
					if (Operations.HasFlag(eOperation::PerformBasicValidation) && Parser->TokenCount > 1 && Parser->Tokens[1][0] != ';')
						LogAnalysisMessage(CurrentLine, "Redundant expression beyond block end specifier.");

					if (StructureStack->Peek() != ControlBlock::eControlBlockType::While &&
						StructureStack->Peek() != ControlBlock::eControlBlockType::ForEach)
					{
						EncounteredProblem = true;
						MalformedStructure = true;
						FirstStructuralErrorLine = CurrentLine;

						if (Operations.HasFlag(eOperation::PerformBasicValidation))
							LogCriticalAnalysisMessage(CurrentLine, "Mismatching block.");
					}
					else
					{
						StructureStack->Pop();
						BlockStack->Pop()->EndLine = CurrentLine;
					}
				}

				break;
			case eScriptTokenType::If:
				if (Operations.HasFlag(eOperation::FillControlBlocks))
				{
					if (StructureStack->Peek() == ControlBlock::eControlBlockType::None)
					{
						EncounteredProblem = true;
						MalformedStructure = true;
						FirstStructuralErrorLine = CurrentLine;

						if (Operations.HasFlag(eOperation::PerformBasicValidation))
							LogCriticalAnalysisMessage(CurrentLine, "Control block declared outside script block.");
					}
					else
					{
						if (Operations.HasFlag(eOperation::PerformBasicValidation) && (Parser->TokenCount < 2 || Parser->Tokens[1][0] == ';'))
							LogCriticalAnalysisMessage(CurrentLine, "Invalid condition."), EncounteredProblem = true;

						ControlBlock^ Parent = BlockStack->Peek();
						ControlBlock^ NewBlock = gcnew ControlBlock(ControlBlock::eControlBlockType::If, CurrentLine,
																	Parent->IndentLevel + 1, Parent);

						StructureStack->Push(ControlBlock::eControlBlockType::If);
						BlockStack->Push(NewBlock);
						IfStack->Push(NewBlock);
						ControlBlocks->Add(NewBlock);

						if (Operations.HasFlag(eOperation::PerformBasicValidation))
						{
							String^ Condition = ReadLine->Substring(Parser->Indices[0] + Parser->Tokens[0]->Length);
							if (ParseConditionExpression(CurrentLine, Condition) == false)
								EncounteredProblem = true;
						}
					}
				}

				break;
			case eScriptTokenType::ElseIf:
			case eScriptTokenType::Else:
				if (Operations.HasFlag(eOperation::FillControlBlocks))
				{
					if (Operations.HasFlag(eOperation::PerformBasicValidation) && FirstTokenType == eScriptTokenType::ElseIf &&
						(Parser->TokenCount < 2 || Parser->Tokens[1][0] == ';'))
					{
						LogCriticalAnalysisMessage(CurrentLine, "Invalid condition.");
						EncounteredProblem = true;
					}
					if (Operations.HasFlag(eOperation::PerformBasicValidation) && FirstTokenType == eScriptTokenType::Else &&
						(Parser->TokenCount > 1 && Parser->Tokens[1][0] != ';'))
					{
						LogAnalysisMessage(CurrentLine, "Redundant expression beyond Else specifier.");
					}

					if (StructureStack->Peek() != ControlBlock::eControlBlockType::If && StructureStack->Peek() != ControlBlock::eControlBlockType::ElseIf)
					{
						EncounteredProblem = true;
						MalformedStructure = true;
						FirstStructuralErrorLine = CurrentLine;

						if (Operations.HasFlag(eOperation::PerformBasicValidation))
							LogCriticalAnalysisMessage(CurrentLine, "Mismatching block.");
					}
					else
					{
						StructureStack->Pop();
						ControlBlock^ PrevBlock = BlockStack->Pop();
						PrevBlock->EndLine = CurrentLine;
						PrevBlock->BasicBlock = false;

						ControlBlock::eControlBlockType BlockType = ControlBlock::eControlBlockType::ElseIf;
						if (FirstTokenType == eScriptTokenType::Else)
							BlockType = ControlBlock::eControlBlockType::Else;

						ControlBlock^ NewBlock = gcnew ControlBlock(BlockType, CurrentLine,
																	PrevBlock->IndentLevel, PrevBlock->Parent);

						StructureStack->Push(BlockType);
						BlockStack->Push(NewBlock);
						ControlBlocks->Add(NewBlock);

						if (BlockType == ControlBlock::eControlBlockType::ElseIf && Operations.HasFlag(eOperation::PerformBasicValidation))
						{
							String^ Condition = ReadLine->Substring(Parser->Indices[0] + Parser->Tokens[0]->Length);
							if (ParseConditionExpression(CurrentLine, Condition) == false)
								EncounteredProblem = true;
						}
					}
				}

				break;
			case eScriptTokenType::EndIf:
				if (Operations.HasFlag(eOperation::FillControlBlocks))
				{
					if (Operations.HasFlag(eOperation::PerformBasicValidation) && Parser->TokenCount > 1 && Parser->Tokens[1][0] != ';')
						LogAnalysisMessage(CurrentLine, "Redundant expression beyond block end specifier.");

					if (StructureStack->Peek() != ControlBlock::eControlBlockType::If &&
						StructureStack->Peek() != ControlBlock::eControlBlockType::ElseIf &&
						StructureStack->Peek() != ControlBlock::eControlBlockType::Else)
					{
						EncounteredProblem = true;
						MalformedStructure = true;
						FirstStructuralErrorLine = CurrentLine;

						if (Operations.HasFlag(eOperation::PerformBasicValidation))
							LogCriticalAnalysisMessage(CurrentLine, "Mismatching block.");
					}
					else
					{
						ControlBlock^ Block = BlockStack->Pop();
						if (StructureStack->Peek() != ControlBlock::eControlBlockType::If)
							Block->BasicBlock = false;

						StructureStack->Pop();
						Block->EndLine = CurrentLine;

						if (IfStack->Count)
							IfStack->Pop()->OuterEndLine = CurrentLine;
					}
				}

				break;
			case eScriptTokenType::Return:
				if (Operations.HasFlag(eOperation::PerformBasicValidation) && Parser->TokenCount > 1 && Parser->Tokens[1][0] != ';')
					LogAnalysisMessage(CurrentLine, "Redundant expression beyond block end specifier.");

				break;
			case eScriptTokenType::SetFunctionValue:
				if (Operations.HasFlag(eOperation::FillUDFData))
				{
					Variable^ Existing = LookupVariable(SecondToken);
					if (Existing)
						UDFResult = Existing;
					else
						UDFAmbiguousResult = true;
				}

				break;
			case eScriptTokenType::Comment:
				// break early to save the script description
				break;
			default:
				SaveDefinitionComments = false;
				break;
			}

			if (Operations.HasFlag(eOperation::PerformBasicValidation) && Operations.HasFlag(eOperation::CountVariableReferences))
			{
				UInt32 Count = 0;
				if (FirstTokenType != eScriptTokenType::Variable)
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

	for each (ControlBlock^ Itr in ControlBlocks)
	{
		if (Itr->IsMalformed())
		{
			MalformedStructure = true;
			if (Operations.HasFlag(eOperation::PerformBasicValidation))
				LogCriticalAnalysisMessage(Itr->StartLine, "Missing block end specifier.");
		}
	}

	if (Operations.HasFlag(eOperation::PerformBasicValidation))
	{
		for each (Variable^ Itr in Variables)
		{
			if (Operations.HasFlag(eOperation::CountVariableReferences))
			{
				if (Itr->RefCount == 0 && (Type != eScriptType::Quest || Operations.HasFlag(eOperation::SuppressQuestVariableRefCount) == false))
					LogAnalysisMessage(Itr->Line, "Variable " + Itr->Name + " unreferenced in local context.");
			}

			int Throwaway = 0;
			bool InvalidVarName = int::TryParse(Itr->Name, Throwaway);

			if (InvalidVarName)
				LogCriticalAnalysisMessage(Itr->Line, "Variable '" + Itr->Name + "' has an all-numeric identifier.");
		}
	}

	if (Operations.HasFlag(eOperation::PerformBasicValidation))
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

	IfStack->Clear();
#ifndef NDEBUG
	Profiler->Stop();
//	DebugPrint("Analysis of script '" + Name + "' complete. Time = " + Profiler->ElapsedMilliseconds + " ms, Flags = " + Operations.ToString());
#endif // !NDEBUG

	return this;
}

Variable^ AnalysisData::LookupVariable(String^ VarName)
{
	for each (Variable^ Itr in Variables)
	{
		if (!String::Compare(Itr->Name, VarName, true))
			return Itr;
	}

	return nullptr;
}

AnalysisData^ AnalysisData::Clone()
{
	auto Copy = gcnew AnalysisData;

	Copy->Name = gcnew String(this->Name);
	Copy->Description = gcnew String(this->Description);
	Copy->Variables = gcnew List<Variable^>(this->Variables);
	Copy->NextVariableLine = this->NextVariableLine;
	Copy->ControlBlocks = gcnew List<ControlBlock ^>(this->ControlBlocks);
	Copy->MalformedStructure = this->MalformedStructure;
	Copy->FirstStructuralErrorLine = this->FirstStructuralErrorLine;
	Copy->IsUDF = this->IsUDF;
	Copy->UDFResult = this->UDFResult;
	Copy->UDFAmbiguousResult = this->UDFAmbiguousResult;
	Copy->AnalysisMessages = gcnew List<UserMessage ^>(this->AnalysisMessages);

	return Copy;
}

void AnalysisData::LogAnalysisMessage(UInt32 Line, String^ Message)
{
	AnalysisMessages->Add(gcnew UserMessage(Line, Message, false));
}

void AnalysisData::LogCriticalAnalysisMessage(UInt32 Line, String^ Message)
{
	AnalysisMessages->Add(gcnew UserMessage(Line, Message, true));
}

UInt32 AnalysisData::GetLineIndentLevel(UInt32 Line)
{
	int IndentCount = 0;
	for each (ControlBlock^ Itr in ControlBlocks)
	{
		if (Itr->IsMalformed() == false && Line > Itr->EndLine)
			continue;

		if (Itr->IsMalformed() == false && (Line == Itr->StartLine || Line == Itr->EndLine))
			IndentCount = Itr->IndentLevel - 1;
		else if (Line > Itr->StartLine)
			IndentCount = Itr->IndentLevel;
	}

	if (IndentCount < 0)
		IndentCount = 0;

	return IndentCount;
}

ControlBlock^ AnalysisData::GetBlockStartingAt(UInt32 Line)
{
	for each (ControlBlock^ Itr in ControlBlocks)
	{
		if (Itr->StartLine == Line)
			return Itr;
	}

	return nullptr;
}

ControlBlock^ AnalysisData::GetBlockEndingAt(UInt32 Line)
{
	for each (ControlBlock^ Itr in ControlBlocks)
	{
		if ((Itr->BasicBlock && Itr->EndLine == Line) ||
			(Itr->BasicBlock == false && Itr->OuterEndLine == Line))
		{
			return Itr;
		}
	}

	return nullptr;
}

bool AnalysisData::GetHasCriticalMessages()
{
	for each (auto Itr in AnalysisMessages)
	{
		if (Itr->Critical)
			return true;
	}

	return false;
}

bool AnalysisData::ParseConditionExpression(UInt32 Line, String^ Expression)
{
	bool Result = true;

	// catch errors that the vanilla expression parser doesn't handle
	LineTokenizer^ Parser = gcnew LineTokenizer;
	if (Parser->Tokenize(Expression, false))
	{
		int InvalidOperator = Expression->IndexOf("<>");
		int CommentToken = Expression->IndexOf(";");
		if (InvalidOperator != -1 &&
			(CommentToken == -1 || Parser->GetIndexInsideString(Expression, CommentToken)) &&
			Parser->GetIndexInsideString(Expression, InvalidOperator) == false)
		{
			Result = false;
			LogCriticalAnalysisMessage(Line, "Invalid operator '<>'");
		}
	}

	return Result;
}

Sanitizer::Sanitizer(String^ Source) :
	InputText(Source),
	Data(gcnew AnalysisData()),
	SanitizedText("")
{
	auto Params = gcnew AnalysisData::Params;
	Params->ScriptText = InputText;
	Params->Ops = AnalysisData::eOperation::FillControlBlocks;

	Data->PerformAnalysis(Params);
}

bool Sanitizer::SanitizeScriptText(eOperation Operations, GetSanitizedIdentifier^ Delegate)
{
	if (Data->MalformedStructure)
		return false;

	LineTokenizer^ Parser = gcnew LineTokenizer();
	LineTrackingStringReader^ Reader = gcnew LineTrackingStringReader(InputText);

	for (String^ ReadLine = Reader->ReadLine(); ReadLine != nullptr; ReadLine = Reader->ReadLine())
	{
		UInt32 CurrentLine = Reader->LineNumber;
		String^ SanitizedLine = ReadLine;

		// cull empty lines
		if (Parser->Tokenize(SanitizedLine, false) == false)
			SanitizedLine = "";

		if (Operations.HasFlag(eOperation::IndentLines))
		{
			if (Parser->Tokenize(ReadLine, false))
			{
				SanitizedLine = "";
				UInt32 IndentCount = Data->GetLineIndentLevel(CurrentLine);
				for (int i = 0; i < IndentCount; i++)
					SanitizedLine += "\t";

				SanitizedLine += ReadLine->Substring(Parser->Indices[0]);
			}
		}

		ControlBlock^ CurrentBlock = Data->GetBlockStartingAt(CurrentLine);
		if (CurrentBlock && (CurrentBlock->Type == ControlBlock::eControlBlockType::If || CurrentBlock->Type == ControlBlock::eControlBlockType::ElseIf))
		{
			if (Operations.HasFlag(eOperation::EvalifyIfs))
			{
				if (Parser->Tokenize(SanitizedLine, false) && Parser->TokenCount > 1)
				{
					if (String::Compare(Parser->Tokens[1], "eval", true))
					{
						SanitizedLine = SanitizedLine->Substring(0, Parser->Indices[0] + Parser->Tokens[0]->Length) + " eval" +
										SanitizedLine->Substring(Parser->Indices[0] + Parser->Tokens[0]->Length);
					}
				}
			}
		}

		if (Operations.HasFlag(eOperation::CompilerOverrideBlocks))
		{
			if (CurrentBlock && CurrentBlock->Type == ControlBlock::eControlBlockType::ScriptBlock)
			{
				ScriptBlock^ BeginBlock = (ScriptBlock^)CurrentBlock;
				if (BeginBlock->CompilerOverride == false)
				{
					if (Parser->Tokenize(SanitizedLine, false))
					{
						SanitizedLine = SanitizedLine->Substring(0, Parser->Indices[0] + Parser->Tokens[0]->Length) + " _" +
										SanitizedLine->Substring(Parser->Indices[1]);
					}
				}
			}
		}

		if (Operations.HasFlag(eOperation::AnnealCasing))
		{
			Parser->Tokenize(SanitizedLine, true);
			if (Parser->Valid)
			{
				SanitizedLine = SanitizedLine->Substring(0, Parser->Indices[0]);
				for (int i = 0; i < Parser->TokenCount; i++)
				{
					String^ Token = Parser->Tokens[i];
					String^ Delimiter = "" + Parser->Delimiters[i];

					if (Parser->GetCommentTokenIndex(i) == -1)
					{
						Variable^ LocalVar = Data->LookupVariable(Token);
						if (LocalVar)
							SanitizedLine += LocalVar->Name;
						else if (Delegate)
							SanitizedLine += Delegate(Token);
						else
							SanitizedLine += Token;
					}
					else
						SanitizedLine += Token;

					if (Delimiter != "\n" && Delimiter != "\r\n")
						SanitizedLine += Delimiter;
				}
			}
		}

		SanitizedText += SanitizedLine + "\n";
	}

	if (SanitizedText->Length > 0 && SanitizedText[SanitizedText->Length - 1] == '\n')
		SanitizedText = SanitizedText->Substring(0, SanitizedText->Length - 1);

	return true;
}

System::String^ Sanitizer::PerformLocalizedIndenting(String^ Source, UInt32 DefaultIndentLevel)
{
	int IndentCount = DefaultIndentLevel;
	String^ Result = "";
	LineTokenizer^ Parser = gcnew LineTokenizer();
	LineTrackingStringReader^ Reader = gcnew LineTrackingStringReader(Source);

	for (String^ ReadLine = Reader->ReadLine(); ReadLine != nullptr; ReadLine = Reader->ReadLine())
	{
		String^ Indents = "";
		int CurrentIndentCount = IndentCount;

		if (Parser->Tokenize(ReadLine, false))
		{
			switch (Parser->GetFirstTokenType())
			{
			case eScriptTokenType::Begin:
			case eScriptTokenType::If:
			case eScriptTokenType::ForEach:
			case eScriptTokenType::While:
				IndentCount++;
				break;
			case eScriptTokenType::ElseIf:
			case eScriptTokenType::Else:
				CurrentIndentCount--;
				break;
			case eScriptTokenType::End:
			case eScriptTokenType::EndIf:
			case eScriptTokenType::Loop:
				IndentCount--;
				CurrentIndentCount--;
				break;
			}

			if (IndentCount < 0)
				IndentCount = 0;

			if (CurrentIndentCount < 0)
				CurrentIndentCount = 0;

			if (Reader->LineNumber != 1)
			{
				for (int i = 0; i < CurrentIndentCount; i++)
					Indents += "\t";
			}

			ReadLine = ReadLine->Substring(Parser->Indices[0]);
		}

		Result += Indents + ReadLine + "\n";
	}

	Result = Result->TrimEnd();
	return Result;
}

Documenter::Documenter(String^ Source) :
	InputText(Source),
	DocumentedText("")
{
	;//
}

String^ Documenter::GetVariableDescription(String^ Identifier, Dictionary<String^, String^>^ Descriptions)
{
	for each (auto Itr in Descriptions)
	{
		if (String::Compare(Identifier, Itr.Key, true) == 0)
			return Itr.Value;
	}

	return "";
}

void Documenter::Document(String^ ScriptDescription, Dictionary<String^, String^>^ VariableDescriptions)
{
	LineTokenizer^ Parser = gcnew LineTokenizer();
	LineTrackingStringReader^ Reader = gcnew LineTrackingStringReader(InputText);

	String^ Description = "";
	String^ FixedDescription = "";
	String^ ScriptName = "";
	LineTrackingStringReader^ DescriptionReader = gcnew LineTrackingStringReader(ScriptDescription);

	for (String^ ReadLine = DescriptionReader->ReadLine(); ReadLine != nullptr; ReadLine = DescriptionReader->ReadLine())
	{
		if (ReadLine != "")
			FixedDescription += "; " + ReadLine + "\n";
		else
			FixedDescription += "\n";
	}

	bool SkippedDescription = false;
	bool DoneDocumenting = false;

	for (String^ ReadLine = Reader->ReadLine(); ReadLine != nullptr; ReadLine = Reader->ReadLine())
	{
		Parser->Tokenize(ReadLine, false);

		if (DoneDocumenting || Parser->Valid == false)
		{
			DocumentedText += ReadLine + "\n";
			continue;
		}

		String^ FirstToken = Parser->Tokens[0];
		String^ SecondToken = (Parser->Tokens->Count > 1) ? Parser->Tokens[1] : "";

		eScriptTokenType Type = Parser->GetScriptTokenType(FirstToken);

		switch (Type)
		{
		case eScriptTokenType::Variable:
			{
				if (SkippedDescription == false)
				{
					SkippedDescription = true;
					DocumentedText = "scn " + ScriptName + "\n\n" + FixedDescription + "\n";
				}

				String^ VarDesc = GetVariableDescription(SecondToken, VariableDescriptions);
				if (VarDesc != "")
				{
					DocumentedText += FirstToken + " " + SecondToken + "\t ; " + VarDesc->Replace("\n", "")->Replace("\r", "") + "\n";
					continue;
				}
				else
					DocumentedText += ReadLine + "\n";
			}

			break;
		case eScriptTokenType::ScriptName:
			ScriptName = SecondToken;
			break;
		case eScriptTokenType::Comment:
			if (SkippedDescription)
				DocumentedText += ReadLine + "\n";

			break;
		case eScriptTokenType::Begin:
			if (SkippedDescription == false)
			{
				SkippedDescription = true;
				DocumentedText = "scn " + ScriptName + "\n\n" + FixedDescription + "\n";
			}

			DoneDocumenting = true;
			DocumentedText += ReadLine + "\n";
			break;
		default:
			if (SkippedDescription == false)
			{
				SkippedDescription = true;
				DocumentedText = "scn " + ScriptName + "\n\n" + FixedDescription + "\n";
			}

			DocumentedText += ReadLine + "\n";
			break;
		}
	}

	if (SkippedDescription == false)
		DocumentedText = "scn " + ScriptName + "\n\n" + FixedDescription + "\n";

	DocumentedText = DocumentedText->Substring(0, DocumentedText->Length - 1);
}

Structurizer::Node::Node(eNodeType Type, UInt32 StartLine, UInt32 EndLine, String^ Desc) :
	Type(Type), StartLine(StartLine), EndLine(EndLine), Children(gcnew List<Node^>)
{
	Description = Desc;
}

Structurizer::Structurizer(AnalysisData^ Input, GetLineText^ Delegate, UInt32 CurrentLine) :
	InputData(Input),
	ParsedTree(gcnew List<Node^>),
	FetchLineText(Delegate),
	CurrentLine(CurrentLine)
{
	Debug::Assert(FetchLineText != nullptr);
	ParseStructure();
}

Structurizer::Node ^ Structurizer::GetContainingNode(Node ^ Source, UInt32 Line, Node^ LastHit)
{
	Node^ Result = LastHit;
	if (Line >= Source->StartLine && Line <= Source->EndLine)
	{
		if (LastHit == nullptr || Source->StartLine < LastHit->EndLine)
			Result = Source;
	}

	for each (auto Itr in Source->Children)
		Result = GetContainingNode(Itr, Line, Result);

	return Result;
}

Structurizer::Node^ Structurizer::GetContainingNode(UInt32 Line)
{
	Node^ Result = nullptr;
	for each (auto Itr in ParsedTree)
	{
		Result = GetContainingNode(Itr, Line, Result);

		for each (auto Itr in Itr->Children)
			Result = GetContainingNode(Itr, Line, Result);
	}

	return Result;
}

void Structurizer::ParseStructure()
{
	Valid = false;
	CurrentScope = nullptr;
	if (InputData->MalformedStructure == false)
	{
		if (InputData->Variables->Count)
		{
			Variable^ First = InputData->Variables[0];
			Variable^ Last = InputData->Variables[InputData->Variables->Count - 1];
			ParsedTree->Add(gcnew Node(Node::eNodeType::VariableDeclaration, First->Line, Last->Line, "Variable Declarations"));
		}

		for each (auto Itr in InputData->ControlBlocks)
		{
			if (Itr->GetType() != ScriptBlock::typeid)
				continue;

			ScriptBlock^ BeginBlock = (ScriptBlock^)Itr;
			if (BeginBlock)
			{
				Node^ MainBlock = gcnew Node(Node::eNodeType::ScriptBlock,
												BeginBlock->StartLine,
												BeginBlock->EndLine,
												FetchLineText(BeginBlock->StartLine));

				ParsedTree->Add(MainBlock);
				ParseControlBlock(BeginBlock, MainBlock);
			}
		}

		if (ParsedTree->Count)
			Valid = true;
	}
}

void Structurizer::ParseControlBlock(ControlBlock^ Block, Node^ Parent)
{
	Node^ CurrentBlock = nullptr;
	switch (Block->Type)
	{
	case ControlBlock::eControlBlockType::ScriptBlock:
		CurrentBlock = Parent;
		break;
	case ControlBlock::eControlBlockType::If:
	case ControlBlock::eControlBlockType::ElseIf:
	case ControlBlock::eControlBlockType::Else:
		CurrentBlock = gcnew Node(Node::eNodeType::BasicConditionalBlock, Block->StartLine, Block->EndLine, FetchLineText(Block->StartLine));
		Parent->Children->Add(CurrentBlock);
		break;
	case ControlBlock::eControlBlockType::While:
	case ControlBlock::eControlBlockType::ForEach:
		CurrentBlock = gcnew Node(Node::eNodeType::LoopBlock, Block->StartLine, Block->EndLine, FetchLineText(Block->StartLine));
		Parent->Children->Add(CurrentBlock);
		break;
	default:
		break;
	}

	if (CurrentLine >= Block->StartLine && CurrentLine <= Block->EndLine)
		CurrentScope = CurrentBlock;

	for each (auto Itr in Block->ChildBlocks)
		ParseControlBlock(Itr, CurrentBlock);
}


} // namespace obScriptParsing


} // namespace scriptEditor


} // namespace cse