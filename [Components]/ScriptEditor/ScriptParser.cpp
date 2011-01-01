#include "ScriptParser.h"
#include "Globals.h"
#include "ScriptEditor.h"
#include "[Common]\NativeWrapper.h"


ScriptParser::ScriptParser()
{
	Variables = gcnew LinkedList<VariableInfo^>();
	BlockStack = gcnew Stack<BlockType>();
	Tokens = gcnew List<String^>();
	Indices = gcnew List<UInt32>();
	Delimiters = gcnew List<Char>();
	CurrentLineNo = 0;
	ScriptName = nullptr;
	Valid = true;
	PreviousLineNo = 0;
}

bool ScriptParser::IsPlayer(String^% Source)
{
	bool Result = false;

	if (!String::Compare(Source, "player", true) || !String::Compare(Source, "playerref", true))
		Result = true;

	return Result;
}

bool ScriptParser::IsLiteral(String^% Source)
{
	bool Result = false;

	if (Source[0] == '"' && Source[Source->Length - 1] == '"')
		Result = true;

	return Result;
}

int ScriptParser::IsComment(int Index)
{
	int Pos = 0;
	int Result = -1;

	for each (String^% Itr in Tokens) {
		if (Pos == Index)	break;
		if (Itr->IndexOf(";") != -1) {
			Result = Pos;
			break;
		}
		++Pos;
	}
	return Result;
}

bool ScriptParser::HasAlpha(int Index)
{
	int Pos = 0;
	bool Result = false;

	for each (Char Itr in Tokens[Index]) {
		if (Char::IsLetter(Itr)) {
			Result = true;
			break;
		}
	}
	return Result;
}

ScriptParser::VariableInfo^ ScriptParser::FindVariable(String^% Variable)
{
	VariableInfo^ Result = const_cast<VariableInfo^>(VariableInfo::NullVar);

	for each (VariableInfo^% Itr in Variables) {
		if (!String::Compare(Itr->VarName, Variable, true)) {
			Result = Itr;
			break;
		}
	}
	return Result;
}

void ScriptParser::Tokenize(String^ Source, bool AllowNulls)
{
	Source += "\n";							// appended as we're only using the ReadLine/Lines method, which strips out the newline char
	Valid = false;
	int StartPos = -1, LastPos = -1;
	Tokens->Clear();
	Delimiters->Clear();
	Indices->Clear();

	for each (Char Itr in Source) {
		if (Globals::ControlChars->IndexOf(Itr) == -1) {
			StartPos = Source->IndexOf(Itr);
			break;
		}
	}

	if (StartPos == -1)	{
		return;
	}

	LastPos = StartPos;

	String^ TokenString = Source->Substring(StartPos), ^%DelimiterStr = Globals::Delimiters, ^Token, ^Delimiter;
	while (TokenString->IndexOfAny(DelimiterStr->ToCharArray()) != -1) {
		int Idx = TokenString->IndexOfAny(DelimiterStr->ToCharArray());
		if (TokenString[0] == '\"')
		{
			if (TokenString->IndexOf('\"', 1) != -1)
				Idx = TokenString->IndexOf('\"', 1) + 1;
			else
				Idx = TokenString->Length - 1;
		}

		Token = TokenString->Substring(0, Idx), Delimiter = TokenString->Substring(Idx, 1);

		TokenString = TokenString->Remove(0, Idx + 1);
		if (Token == "" && !AllowNulls)		continue;

		Tokens->Add(Token);
		Indices->Add(LastPos);
		Delimiters->Add(Delimiter[0]);
		LastPos += Token->Length + 1;
	}

	if (Tokens->Count > 0)
		Valid = true;
}

ScriptParser::TokenType ScriptParser::GetTokenType(String^% Token)
{
	TokenType Result = TokenType::e_Invalid;

	if (!String::Compare(Token, "scn", true) || !String::Compare(Token, "scriptName", true))
		Result = TokenType::e_ScriptName;
	else if (!String::Compare(Token, "ref", true) || !String::Compare(Token, "int", true) || !String::Compare(Token, "reference", true) || !String::Compare(Token, "short", true) || !String::Compare(Token, "long", true) || !String::Compare(Token, "float", true) || !String::Compare(Token, "string_var", true) || !String::Compare(Token, "array_var", true))
		Result = TokenType::e_Variable;
	else if (!String::Compare(Token, ";", true))
		Result = TokenType::e_Comment;
	else if (!String::Compare(Token, "begin", true))
		Result = TokenType::e_Begin;
	else if (!String::Compare(Token, "end", true))
		Result = TokenType::e_End;
	else if (!String::Compare(Token, "while", true))
		Result = TokenType::e_While;
	else if (!String::Compare(Token, "foreach", true))
		Result = TokenType::e_ForEach;
	else if (!String::Compare(Token, "loop", true))
		Result = TokenType::e_Loop;
	else if (!String::Compare(Token, "if", true))
		Result = TokenType::e_If;
	else if (!String::Compare(Token, "elseif", true))
		Result = TokenType::e_ElseIf;
	else if (!String::Compare(Token, "else", true))
		Result = TokenType::e_Else;
	else if (!String::Compare(Token, "endif", true))
		Result = TokenType::e_EndIf;
	else if (!String::Compare(Token, "return", true))
		Result = TokenType::e_Return;
	else if (!String::Compare(Token, "setfunctionvalue", true))
		Result = TokenType::e_SetFunctionValue;

	return Result;
}

void ScriptParser::Reset()
{
	ScriptName = nullptr;
	Variables->Clear();
	CurrentLineNo = 0;
	BlockStack->Clear();
	Tokens->Clear();
	Delimiters->Clear();
	Indices->Clear();
	Valid = true;
	PreviousLineNo = 0;
}

bool ScriptParser::IsValidBlock(String^% Source, ScriptParser::ScriptType EditorScriptType)
{
	if (Source->Length < 1)
		return false;

	bool Result = true;

	ScriptBlock ScriptBlockType = ScriptBlock::e_Universal;
	String^ BlockType = gcnew String(Source);

	if (BlockType->Length > 0 && BlockType[0] == '_')
		BlockType = BlockType->Substring(1);

	if (!String::Compare(BlockType->Substring(0, 1)->ToLower(), "o"))
		ScriptBlockType = ScriptBlock::e_ReferenceSpecific;
	else if (!String::Compare(BlockType->Substring(0, 1)->ToLower(), "s"))
		ScriptBlockType = ScriptBlock::e_MagicEffect;

	switch (EditorScriptType)
	{
		case ScriptType::e_MagicEffect:
			if (ScriptBlockType == ScriptBlock::e_ReferenceSpecific)
				Result = false;
			break;
		case ScriptType::e_Object:
			if (ScriptBlockType == ScriptBlock::e_MagicEffect)
				Result = false;
			break;
		case ScriptType::e_Quest:
			if (ScriptBlockType != ScriptBlock::e_Universal)
				Result = false;
			break;
	}
	return Result;
}

int ScriptParser::HasToken(String^ Source)
{
	int Result = -1, Count = 0;

	for each (String^% Itr in Tokens) {
		if (!String::Compare(Itr, Source, true)) {
			Result = Count;
			break;
		}
		++Count;
	}
	return Result;
}

bool ScriptParser::HasStringGotIllegalChar(String^% Source, String^ Includes, String^ Excludes)
{
	bool Result = false;

	if (Source->IndexOfAny(Includes->ToCharArray()) != -1)	Result = true;
	else {
		for each (Char Itr in Source) {
			if (Char::IsSymbol(Itr) && Excludes->IndexOf(Itr) == -1) {
				Result = true;
				break;
			}
		}
	}

	return Result;
}

int ScriptParser::GetLineStartIndex(UInt32 StartPosition, String^% Source)
{
	int Result = -1;
	for (int i = StartPosition; i > 0; i--) {
		if (Source[i] == '\n' || Source[i] == '\r\n') {
			Result = i + 1;
			break;
		}
	}
	return Result;
}
int ScriptParser::GetLineEndIndex(UInt32 StartPosition, String^% Source)
{
	int Result = -1;
	for (int i = StartPosition; i < Source->Length; i++) {
		if (Source[i] == '\n' || Source[i] == '\r\n') {
			Result = i;
			break;
		}
	}
	return Result;
}
UInt32 ScriptParser::GetTrailingTabCount(UInt32 StartPosition, String^% Source, String^ CharactersToSkip)
{
	UInt32 Result = 0;
	for (int i = StartPosition; i < Source->Length; i++) {
		if (Source[i] == '\t')
			Result += 1;
		else if (CharactersToSkip != nullptr)
		{
			array<Char>^ SkipArray = CharactersToSkip->ToCharArray();

			for each (Char Itr in SkipArray) {
				if (Itr == Source[i])
					continue;
			}
			break;
		}
		else
			break;
	}
	return Result;
}

bool ScriptParser::IsOperator(String^% Source)
{
	for each(String^% Itr in Operators)
	{
		if (!String::Compare(Source, Itr))
			return true;
	}
	return false;
}













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
	for each (char Itr in Extract) {
		if (Itr != '\t' && Itr != ' ')	break;
		Idx++;
	}

	Extract = Extract->Substring(Idx)->Split('\t', '\r', '\n', ' ')[0];
	if (Extract == "")		return false;

	if (!String::Compare(Extract, "ref", true) ||
		!String::Compare(Extract, "reference", true) ||
		!String::Compare(Extract, "string_var", true) ||
		!String::Compare(Extract, "array_var", true) ||
		!String::Compare(Extract, "long", true) ||
		!String::Compare(Extract, "short", true) ||
		!String::Compare(Extract, "float", true) ||
		!String::Compare(Extract, "int", true))

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

#define		PREPROCESSOR_UNKNOWN_MACRO_VALUE			"<UNDEF>"

Preprocessor::Preprocessor()
{
	PreprocessMacros = gcnew Dictionary<String^, String^>();
	TextParser = gcnew ScriptParser();
	ErrorOutput = nullptr;
	ErrorFlag = false;
	SuppressOutput = false;
}

Preprocessor^% Preprocessor::GetSingleton()
{
	if (Singleton == nullptr) {
		Singleton = gcnew Preprocessor();
	}
	return Singleton;
}


int Preprocessor::GetMacroIndex(String^% Macro)
{
	int Result = -1, Count = 0;
	
	for each (KeyValuePair<String^, String^>^ Itr in PreprocessMacros) {
		if (!String::Compare(Itr->Key, Macro)) {
			Result = Count;
			break;
		}
		++Count;
	}
	return Result;
}

String^ Preprocessor::GetMacroAtIndex(UInt32 Index)
{
	String^ Result = PREPROCESSOR_UNKNOWN_MACRO_VALUE;
	int Count = 0;
	
	for each (KeyValuePair<String^, String^>^ Itr in PreprocessMacros) {
		if (Index == Count) {
			Result = Itr->Key;
			break;
		}
		++Count;
	}
	return Result;
}

void Preprocessor::LogErrorMessage(String^ Message, bool OverrideSuppression)
{
	if (!SuppressOutput || OverrideSuppression)
		ErrorOutput(Message);
	ErrorFlag = true;
}

void Preprocessor::LogDebugMessage(String^ Message)
{
	if (!SuppressOutput)
		DebugPrint(Message);
}

String^ Preprocessor::ParseImportDirective(String^% Source, PreprocessOp Operation, String^% ReadLine, UInt32 LineStart, UInt32 LineEnd, bool Recursing)
{
	String^ Result = "", ^ImportFileName, ^Dummy = "";

try {
	int ImportDefIdx = TextParser->HasToken("//import"), 
		ImportColIdx = TextParser->HasToken(";<CSEImportSeg>"),
		ImportColStopIdx = TextParser->HasToken(";</CSEImportSeg>");

	array<Char>^ QuoteDelimit = {'"'}, ^LFDelimit = {'\n'};

	if (ImportDefIdx == 0 && TextParser->IsComment(ImportDefIdx) == -1 && ImportDefIdx + 1 < TextParser->Tokens->Count && Operation == PreprocessOp::e_Expand && TextParser->IsLiteral(TextParser->Tokens[ImportDefIdx + 1])) {
		ImportFileName = TextParser->Tokens[ImportDefIdx + 1]->Split(QuoteDelimit)[1];
		try {
			StreamReader^ ImportParser = gcnew StreamReader(String::Format("{0}Data\\Scripts\\{1}.txt",Globals::AppPath, ImportFileName));
			Result = ";<CSEImportSeg>" + "\n";
			Result += DoPreprocess(Operation, ImportParser->ReadToEnd(), Dummy, false);
			Result += ReadLine->Substring(0, TextParser->Indices[0]) + ";</CSEImportSeg> \"" + ImportFileName + "\"\n";
			ImportParser->Close();
		} catch (Exception^ E) {
			LogErrorMessage("Couldn't import from script '" + ImportFileName + "'. Exception: " + E->Message, false);
			Result = ReadLine + "\n";
		}
	}
	else if (ImportColIdx == 0 && Operation == PreprocessOp::e_Collapse) {
		Result = DoPreprocess(Operation, Source->Substring(LineEnd + ((Source->Length > LineEnd + 1)?1:0)), Dummy, true);
	}
	else if (ImportColStopIdx == 0 && Operation == PreprocessOp::e_Collapse) {
		ImportFileName = TextParser->Tokens[ImportColStopIdx + 1]->Split(QuoteDelimit)[1];
		String^ ImportedText = DoPreprocess(Operation, Source->Substring(0, LineStart - 1), Dummy, true)->Replace("\n", "\r\n");		// needs to be preprocessed as source is always a substring of the original script text

		if (!File::Exists(String::Format("{0}Data\\Scripts\\{1}.txt",Globals::AppPath, ImportFileName))) {
			LogDebugMessage("Import segment '" + ImportFileName + "' couldn't be found");
			if (OPTIONS->FetchSettingAsInt("CreateMissingFromSegment")) {
				try {
					StreamWriter^ ImportParser = gcnew StreamWriter(String::Format("{0}Data\\Scripts\\{1}.txt",Globals::AppPath, ImportFileName));
					ImportParser->Write(ImportedText);
					ImportParser->Flush();
					ImportParser->Close();
					LogDebugMessage("\tImport segment '" + ImportFileName + "' created!");
					
				} catch (Exception^ E) {
					LogErrorMessage("Couldn't write import segment to '" + ImportFileName + "'\n\tException: " + E->Message, false);
				}
			}
		}
		Result = ReadLine->Substring(0, TextParser->Indices[0]) + "//import \"" + ImportFileName + "\"\n";
	}
} catch (Exception^ E) {
	String^ Error = "Preprocessing failed on line '" + ReadLine + "'. Check for incorrect syntax";
	LogErrorMessage(Error, false);
	LogDebugMessage(Error + "\n\tException: " + E->Message);
}
	return Result;
}
String^ Preprocessor::ParseDefineDirective(String^% Source, PreprocessOp Operation, String^% ReadLine)
{
	String^ Result = "";
	String^ InvalidChars = "";

try {
	int DefineDefIdx = TextParser->HasToken("//define"), 
		DefineColIdx = TextParser->HasToken(";<CSEMacroDef>"), 
		DefineRstrIdx = TextParser->HasToken(";<CSEMacroRef>");

	if (DefineDefIdx == 0 && TextParser->IsComment(DefineDefIdx) == -1 && DefineDefIdx + 1 < TextParser->Tokens->Count && Operation == PreprocessOp::e_Expand) {
		String^ Macro = TextParser->Tokens[DefineDefIdx + 1];
		String^ Value = ReadLine->Substring(TextParser->Indices[DefineDefIdx + 1] + Macro->Length + 1);

		if (!String::Compare(Macro, Macro->ToUpper())) {
			Result += ";<CSEMacroDef> " + Macro + " " + Value + " </CSEMacroDef>\n";

			if (GetMacroIndex(Macro) != -1) {
				if (OPTIONS->FetchSettingAsInt("AllowRedefinitions"))	PreprocessMacros[Macro] = Value;
				else								LogErrorMessage("Illegal redefinition of preprocessor macro '" + Macro + "'", true);	// this error message needs to be displayed regardless of suppression as we override the associated INI setting during the second pass
			}
			else if (Value == "")
				LogErrorMessage("Preprocessor macro '" + Macro + "' doesn't have a value", false);
			else if (Value->IndexOfAny(InvalidChars->ToCharArray()) != -1)
				LogErrorMessage("Preprocessor macro '" + Macro + "' contains an invalid character", false);
			else 
				PreprocessMacros->Add(Macro, Value);
		} else
			LogErrorMessage("Preprocessor macro '" + Macro + "' is formatted incorrectly", false);
	}
	else if (DefineColIdx == 0 && Operation == PreprocessOp::e_Collapse) {
		String^ Macro = TextParser->Tokens[DefineColIdx + 1];
		String^ Value = ReadLine->Substring(TextParser->Indices[DefineColIdx + 2])->Replace(" </CSEMacroDef>", "");
		Result += ReadLine->Substring(0, TextParser->Indices[0]) + "//define " + Macro + " " + Value + "\n";

		if (GetMacroIndex(Macro) == -1)
			PreprocessMacros->Add(Macro, Value);
		else if (OPTIONS->FetchSettingAsInt("AllowRedefinitions"))
			PreprocessMacros[Macro] = Value;
	}
	else if (DefineRstrIdx != -1 && Operation == PreprocessOp::e_Collapse) {
		TextParser->Tokenize(ReadLine, true);
		DefineRstrIdx = TextParser->HasToken(";<CSEMacroRef>");
		String^ RestoredLine = ReadLine->Substring(0, TextParser->Indices[0]), 
				^BreadCrumb = TextParser->Tokens[DefineRstrIdx + 1],
				^Token, ^Value;
		array<String^>^ Offsets = BreadCrumb->Split(static_cast<array<Char>^>(Globals::PipeDelimit));
		int TokenIdx = 0, MacroIdx = 0, TokenIdxOffset = 0;

		while (TokenIdx < TextParser->Tokens->Count) {
			Token = TextParser->Tokens[TokenIdx];

			for each (String^ Itr in Offsets) {
				if (Itr == "")		continue;
				else				Itr += "|";

				int RstrTokenIdx = Int32::Parse(Itr->Substring(0, Itr->IndexOf(";"))), 
					MacroIdx = Int32::Parse(Itr->Substring(Itr->IndexOf(";") + 1, Itr->IndexOf("|", Itr->IndexOf(";")) - Itr->IndexOf(";") - 1));
				TokenIdxOffset = 0;
				if (TokenIdx == RstrTokenIdx) {
					Token = GetMacroAtIndex(MacroIdx);
					if (Token != PREPROCESSOR_UNKNOWN_MACRO_VALUE)			Value = PreprocessMacros[Token];
					else							Value = "<CSEUnknownMacro>";
					ScriptParser^ TempParser = gcnew ScriptParser();
					TempParser->Tokenize(Value, true);
					TokenIdxOffset = TokenIdx + TempParser->Tokens->Count - 1;
					break;
				}
			}

			RestoredLine += Token;
			if ((!TokenIdxOffset ? TokenIdx : TokenIdxOffset) + 1 < TextParser->Delimiters->Count && TextParser->Tokens[(!TokenIdxOffset ? TokenIdx : TokenIdxOffset) + 1] == ";<CSEMacroRef>")
				break;
			if (TextParser->Delimiters[(!TokenIdxOffset ? TokenIdx : TokenIdxOffset)] != '\n')	
				RestoredLine += TextParser->Delimiters[(!TokenIdxOffset ? TokenIdx : TokenIdxOffset)];
			if (!TokenIdxOffset)	++TokenIdx;
			else					TokenIdx = TokenIdxOffset + 1;
		}

		Result += RestoredLine + "\n";
	}
	else {
		TextParser->Tokenize(ReadLine, true);
		String^ ExpandedLine = ReadLine->Substring(0, TextParser->Indices[0]), ^BreadCrumb = "|";		// gives us the controlchars to prepend
		int TokenIdx = 0, MacroIdx = 0, TokenIdxOffset = 0;
		
		for each (String^% Itr in TextParser->Tokens) {
			MacroIdx = GetMacroIndex(Itr);
			if (MacroIdx != -1 && TextParser->IsComment(TokenIdx) == -1) {
				ExpandedLine += PreprocessMacros[Itr];
				ScriptParser^ TempParser = gcnew ScriptParser();
				TempParser->Tokenize(PreprocessMacros[Itr], true);
				int TokenEncodeIdx = TokenIdx + TokenIdxOffset;								// offset by the token count of the macro's value to allow multiple tokens
				TokenIdxOffset = TempParser->Tokens->Count - 1;
				BreadCrumb += String::Format("{0};{1}|", TokenEncodeIdx, MacroIdx);			// breadcrumb format = |<TokenIndex>;<VarIndex>|...
			}
			else {
				ExpandedLine += Itr;
			}

			
			if (TextParser->Delimiters[TokenIdx] != '\n')		ExpandedLine += TextParser->Delimiters[TokenIdx];
			++TokenIdx;
		}

		Result += ExpandedLine;
		if (BreadCrumb != "|")
			Result += "\t;<CSEMacroRef> " + BreadCrumb + " </CSEMacroRef>\n";
		else
			Result += "\n";
	}
} catch (Exception^ E) {
	String^ Error = "Preprocessing failed on line '" + ReadLine + "'. Check for incorrect syntax";
	LogErrorMessage(Error, false);
	LogDebugMessage(Error + "\n\tException: " + E->Message);
}

	return Result;
}

void Preprocessor::ParseEnumMacros(String^% Items, bool ReportErrors)
{
	TextParser->Tokenize(Items, false);
	int PreviousValue = 0;

	for each (String^% Itr in TextParser->Tokens) {
		String^ Macro, ^ValueStr;
		int Operator = Itr->IndexOf("="), Value = 0;
		if (Operator != -1) {
			Macro = Itr->Substring(0, Operator);
			ValueStr = Itr->Substring(Operator + 1);
		} else {
			Macro = Itr;
			ValueStr = (((!PreviousValue)?-1:PreviousValue) + 1).ToString();
		}
		try {
			Value = Int32::Parse(ValueStr);
		} catch (...) {
			Value = 0;
			if (ReportErrors)	LogErrorMessage("Preprocessor macro '" + Macro + "' contains an invalid character", false);
		}
		PreviousValue = Value;
		
		if (!String::Compare(Macro, Macro->ToUpper())) {
			if (GetMacroIndex(Macro) != -1) {
				if (OPTIONS->FetchSettingAsInt("AllowRedefinitions"))	PreprocessMacros[Macro] = Value.ToString();
				else {		if (ReportErrors)		LogErrorMessage("Illegal redefinition of preprocessor macro '" + Macro + "'", true); }
			}
			else 
				PreprocessMacros->Add(Macro, Value.ToString());
		} else if (ReportErrors)
			LogErrorMessage("Preprocessor macro '" + Macro + "' is formatted incorrectly", false);
	}
}

String^	Preprocessor::ParseEnumDirective(String^% Source, PreprocessOp Operation, String^% ReadLine)
{
	String^ Result = "";

try {
	int EnumDefIdx = TextParser->HasToken("//enum"), 
		EnumColIdx = TextParser->HasToken(";<CSEEnum>");

	if (EnumDefIdx == 0 && TextParser->IsComment(EnumDefIdx) == -1 && EnumDefIdx + 1 < TextParser->Tokens->Count && Operation == PreprocessOp::e_Expand) {
		String^ Enum = TextParser->Tokens[EnumDefIdx + 1];
		String^ Value = ReadLine->Substring(TextParser->Indices[EnumDefIdx + 1] + Enum->Length + 1);

		if (Enum == "" || Value == "")
			throw gcnew CSEGeneralException("Enum name/value is null");

		Result += ";<CSEEnum> " + Enum + " " + Value + " </CSEEnum>\n";
		Value = Value->Replace("{", "")->Replace("}", "");

		ParseEnumMacros(Value, true);
	}
	else if (EnumColIdx == 0 && Operation == PreprocessOp::e_Collapse) {
		String^ Enum = TextParser->Tokens[EnumColIdx + 1];
		String^ Value = ReadLine->Substring(TextParser->Indices[EnumColIdx + 2])->Replace(" </CSEEnum>", "");
		Result += ReadLine->Substring(0, TextParser->Indices[0]) + "//enum " + Enum + " " + Value + "\n";
		Value = Value->Replace("{", "")->Replace("}", "");

		ParseEnumMacros(Value, false);
	}
} catch (Exception^ E) {
	String^ Error = "Preprocessing failed on line '" + ReadLine + "'. Check for incorrect syntax";
	LogErrorMessage(Error, false);
	LogDebugMessage(Error + "\n\tException: " + E->Message);
}

	return Result;
}

void Preprocessor::ParseNestedDirectives(StringReader^% PreprocessParser, String^% ReadLine, UInt32& LineStart, UInt32& LineEnd)
{
	int Counter = 1;										
	LineStart = LineEnd + 1;	
	ReadLine = PreprocessParser->ReadLine();

	while (ReadLine != nullptr && Counter > 0) {
		TextParser->Tokenize(ReadLine, false);
		LineEnd = LineStart + ReadLine->Length + 1;

		if (TextParser->Valid) {
			if (!TextParser->HasToken(";<CSEImportSeg>"))					++Counter;
			else if (!TextParser->HasToken(";</CSEImportSeg>"))				--Counter;
		}

		LineStart = LineEnd + 1;	
		ReadLine = PreprocessParser->ReadLine();
	}
}

String^ Preprocessor::DoPreprocess(PreprocessOp Operation, String^% Source, String^% ExtractedCSEBlock, bool Recursing)
{
	String^ Result = "", ^ReadLine;
	
	StringReader^ PreprocessParser = gcnew StringReader(Source);
	ReadLine = PreprocessParser->ReadLine();
	UInt32 LineStart = 0, LineEnd = 0;								// relative to the source string
	bool ExtractBlock = false;

	while (ReadLine != nullptr) {
		TextParser->Tokenize(ReadLine, false);
		LineEnd = LineStart + ReadLine->Length + 1;

		if (ExtractBlock) {
			if (!TextParser->HasToken(";</CSEBlock>"))
				ExtractBlock = false;
			else
				ExtractedCSEBlock += ReadLine + "\n";
		
			LineStart = LineEnd + 1;	
			ReadLine = PreprocessParser->ReadLine();
			continue;
		}
		
		if (!TextParser->Valid) {
			Result += ReadLine + "\n";
			LineStart = LineEnd + 1;	
			ReadLine = PreprocessParser->ReadLine();
			continue;
		} else if (!TextParser->HasToken(";<CSEBlock>")) {
			ExtractBlock = true;
			LineStart = LineEnd + 1;	
			ReadLine = PreprocessParser->ReadLine();
			continue;
		}

		int ImportDefIdx = TextParser->HasToken("//import"), 
			ImportColIdx = TextParser->HasToken(";<CSEImportSeg>"),
			ImportColStopIdx = TextParser->HasToken(";</CSEImportSeg>"),	
			DefineDefIdx = TextParser->HasToken("//define"), 
			DefineColIdx = TextParser->HasToken(";<CSEVarDef>"),	
			DefineRstrIdx = TextParser->HasToken(";<CSEVarRef>"),
			EnumDefIdx = TextParser->HasToken("//enum"), 
			EnumColIdx = TextParser->HasToken(";<CSEEnum>");	
		
		if (!ImportDefIdx || !ImportColIdx || !ImportColStopIdx) {								
			if (Recursing) {
				if (!ImportColStopIdx) {		
					Result = ParseImportDirective(Source, Operation, ReadLine, LineStart, LineEnd, Recursing);					
					return Result;																// quick return to prevent unnecessary snooping
				}
				else if (!ImportColIdx) {
					Result += ParseImportDirective(Source, Operation, ReadLine, LineStart, LineEnd, Recursing);

					if (Operation == PreprocessOp::e_Collapse) {
						ParseNestedDirectives(PreprocessParser, ReadLine, LineStart, LineEnd);
						continue;
					}
				}
			}
			else {
				Result += ParseImportDirective(Source, Operation, ReadLine, LineStart, LineEnd, Recursing);

				if (Operation == PreprocessOp::e_Collapse) {
					ParseNestedDirectives(PreprocessParser, ReadLine, LineStart, LineEnd);		// skip processed lines
					continue;
				}
			}
		}
		else if (!EnumDefIdx || !EnumColIdx) {
			Result += ParseEnumDirective(Source, Operation, ReadLine);
		}
		else 
			Result += ParseDefineDirective(Source, Operation, ReadLine);						// needs to be the last, as it automatically parses unformatted script lines


		LineStart = LineEnd + 1;	
		ReadLine = PreprocessParser->ReadLine();
	}
	return Result;
}

bool Preprocessor::Preprocess(PreprocessOp Operation, String^% Source, String^% Result, String^% ExtractedCSEBlock, StandardOutputError^ ErrorOutput)
{
	int Cache = OPTIONS->FetchSettingAsInt("AllowRedefinitions");
	String^ Dummy = "";

	PreprocessMacros->Clear();
	TextParser->Reset();
	ErrorFlag = false;
	SuppressOutput = true;
	this->ErrorOutput = ErrorOutput;

	DoPreprocess(Operation, Source, ExtractedCSEBlock, false);
	SuppressOutput = false;
	ProcessStandardDefineDirectives();
	TextParser->Reset();

	OPTIONS->FetchSetting("AllowRedefinitions")->SetValue("1");
	Result = DoPreprocess(Operation, Source, Dummy, false);
	OPTIONS->FetchSetting("AllowRedefinitions")->SetValue(Cache.ToString());

	return (ErrorFlag == 0);
}

void Preprocessor::ProcessStandardDefineDirectives(void)
{
	String^ Path = Globals::AppPath + "Data\\OBSE\\Plugins\\ComponentDLLs\\CSE\\STDPreprocDefs.txt";
	if (File::Exists(Path)) {
		try {
			StreamReader^ STDFileParser = gcnew StreamReader(Path);
			String^ ReadLine;

			ReadLine = STDFileParser->ReadLine();
			while (ReadLine != nullptr) {
				if (ReadLine != "") {
					if (ReadLine[0] != '@') {
						TextParser->Tokenize(ReadLine, false);
						ParseDefineDirective(ReadLine, PreprocessOp::e_Expand, ReadLine);
					}
				}
				ReadLine = STDFileParser->ReadLine();
			}

			STDFileParser->Close();		
		} catch (Exception^ E) {
			LogDebugMessage("Couldn't read from Standard define directives file!\n\tException: " + E->Message);
		}
	} else
		LogDebugMessage("Standard define directives file not found!");
}

