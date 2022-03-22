#include "ScriptPreprocessor.h"

namespace cse
{


namespace scriptEditor
{


namespace preprocessor
{


void DummyStandardErrorOutput(int Line, String^ Message)
{
	;//
}


PreprocessorParams::PreprocessorParams(String^ DepotPath, String^ StandardDirectivePath, bool AllowMacroRedefinitions, UInt8 NoOfPasses) :
	DepotPath(DepotPath),
	StandardDirectivePath(StandardDirectivePath),
	AllowMacroRedefinitions(AllowMacroRedefinitions),
	NoOfPasses(NoOfPasses),
	ContainsDirectives(false),
	CurrentLine(0)
{
	;//
}

CSEPreprocessorToken::CSEPreprocessorToken(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	if (Token == nullptr || PreprocessorInstance == nullptr || ErrorOutput == nullptr)
		return;																				// invoked by CSEPreprocessorDirective::c'tor

	try
	{
		String^ ExpandedToken = "";
		obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

		LocalParser->Tokenize(Token, true);
		if (!LocalParser->Valid)
			this->Token = Token;
		else
		{
			String^ Prefix = ExpandedToken += Token->Substring(0, LocalParser->Indices[0]);		// gives us the controlchars to prepend
			int TokenIndex = 0;

			for each (String^ Itr in LocalParser->Tokens)
			{
				String^ TokenBuffer = Itr;

				DefineDirective::eAccessoryOperatorType DefineOperator = DefineDirective::GetAccessoryOperatorFromToken(Itr);
				switch (DefineOperator)
				{
				case DefineDirective::eAccessoryOperatorType::None:
					break;
				default:
					TokenBuffer = Itr->Substring(DefineDirective::AccessoryOperatorIdentifier[(int)DefineOperator]->Length);
					break;
				}

				DefineDirective^ Macro = PreprocessorInstance->LookupDefineDirectiveByName(TokenBuffer);

				if (Macro != nullptr && LocalParser->GetCommentTokenIndex(TokenIndex) == -1)
					ExpandedToken += Macro->GetValue(Prefix, DefineOperator);
				else
					ExpandedToken += Itr;

				if (LocalParser->Delimiters[TokenIndex] != '\n')
					ExpandedToken += LocalParser->Delimiters[TokenIndex];

				TokenIndex++;
			}

			this->Token = ExpandedToken;
		}
	}
	catch (Exception^ E)
	{
		this->Token = Token;
		ErrorOutput(PreprocessorInstance->GetInstanceData()->CurrentLine, "Failed to parse TOKEN '" + Token + "' - " + E->Message);
	}
}

System::String^ CSEPreprocessorDirective::GetDirectiveKeyword(eDirectiveType DirectiveType, eEncodingType EncodingType)
{
	return EncodingIdentifier[safe_cast<int>(EncodingType)] + DirectiveIdentifier[safe_cast<int>(DirectiveType)];
}

String^ CSEPreprocessorDirective::GetMultilineValue(LineTrackingStringReader^% TextReader, String^% SliceStart, String^% SliceEnd)
{
	String^ Result = "";
	bool SliceStartFound = false, SliceEndFound = false;
	obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

	String^ ReadLine = TextReader->ReadLine();
	while (ReadLine != nullptr)
	{
		LocalParser->Tokenize(ReadLine, false);
		if (LocalParser->Valid)
		{
			int FirstTokenIndex = LocalParser->Indices[0];
			if (ReadLine->Length >= FirstTokenIndex + 2)
			{
				if (!String::Compare(ReadLine->Substring(FirstTokenIndex, 2), ";{"))
				{
					SliceStartFound = true;
					SliceStart = ReadLine->Substring(0, FirstTokenIndex);
					ReadLine = TextReader->ReadLine();
					continue;
				}
				else if (!String::Compare(ReadLine->Substring(FirstTokenIndex, 2), ";}"))
				{
					SliceEndFound = true;
					SliceEnd = ReadLine->Substring(0, FirstTokenIndex);
					break;
				}
			}
		}

		if (SliceStartFound)
		{
			if (LocalParser->Valid)
			{
				int FirstTokenIndex = LocalParser->Indices[0];

				// make sure the line's commented
				// otherwise the parser will eat up a character for each pass
				if (ReadLine[FirstTokenIndex] != ';')
				{
					ReadLine = ";" + ReadLine;
					LocalParser->Tokenize(ReadLine, false);
					FirstTokenIndex = LocalParser->Indices[0];
				}

				Result += "\n" + ReadLine->Substring(0, FirstTokenIndex) + ReadLine->Substring(FirstTokenIndex + 1);
			}
			else
				Result += "\n" + ReadLine;
		}

		ReadLine = TextReader->ReadLine();
	}

	if (SliceStartFound == false || SliceEndFound == false)
		throw gcnew InvalidOperationException("Couldn't extract multiline value.");

	return Result->Substring(1);
}

String^ CSEPreprocessorDirective::ObfuscateToCompiler(String^ Token)
{
	String^ Result = "";
	LineTrackingStringReader^ TextReader = gcnew LineTrackingStringReader(Token);

	for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
	{
		Result += "\n" + ";" + ReadLine;
	}

	return Result->Substring(1);
}

bool DefineDirective::IsNameValid(String^ Name)
{
	bool Result = true;

	for each (Char Itr in Name)
	{
		if (Char::IsLetter(Itr) == false && Char::IsNumber(Itr) == false && Itr != '_')
		{
			Result = false;
			break;
		}
	}

	return Result;
}

DefineDirective::DefineDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance, UInt32 LineNumber)
{
	this->Token = Token;
	this->Type = eDirectiveType::Define;
	this->Encoding = eEncodingType::SingleLine;

	obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->TokenCount)
			{
			case 1:
				throw gcnew InvalidOperationException("No name specified.");
			case 2:
				throw gcnew InvalidOperationException("No value specified.");
			}

			int Index = LocalParser->GetTokenIndex(";" + EncodingIdentifier[(int)eEncodingType::SingleLine] + DirectiveIdentifier[(int)eDirectiveType::Define]);
			Name = LocalParser->Tokens[Index + 1];
			Value = Token->Substring(LocalParser->Indices[Index + 1] + Name->Length + 1);

			if (PreprocessorInstance->LookupDefineDirectiveByName(Name) != nullptr && PreprocessorInstance->GetInstanceData()->AllowMacroRedefinitions == false)
				throw gcnew InvalidOperationException("Invalid redefinition.");
			else if (IsNameValid(Name) == false)
				throw gcnew InvalidOperationException("Invalid character in name.");
			else
				PreprocessorInstance->RegisterDefineDirective(this);
		}
		else
			throw gcnew InvalidOperationException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Name = "INVALID";
		Value = "INVALID";
		ErrorOutput(LineNumber, "Failed to parse DEFINE directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

DefineDirective::DefineDirective(String^ Token, LineTrackingStringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	this->Token = Token;
	this->Type = eDirectiveType::Define;
	this->Encoding = eEncodingType::MultiLine;

	obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->TokenCount)
			{
			case 1:
				throw gcnew InvalidOperationException("No name specified.");
			}

			int Index = LocalParser->GetTokenIndex(";" + EncodingIdentifier[(int)eEncodingType::MultiLine] + DirectiveIdentifier[(int)eDirectiveType::Define]);
			Name = LocalParser->Tokens[Index + 1];
			Value = GetMultilineValue(TextReader, SliceStart, SliceEnd);

			String^ Result = "";
			if (PreprocessorInstance->LookupDefineDirectiveByName(Name) != nullptr && PreprocessorInstance->GetInstanceData()->AllowMacroRedefinitions == false)
				throw gcnew InvalidOperationException("Invalid redefinition.");
			else if (IsNameValid(Name) == false)
				throw gcnew InvalidOperationException("Invalid character in name.");
			else if (!PreprocessorInstance->Preprocess(Value, Result, ErrorOutput))
				throw gcnew InvalidOperationException("Errors encountered while preprocessing DEFINE block");
			else
			{
				Value = Result;
				PreprocessorInstance->RegisterDefineDirective(this);
			}
		}
		else
			throw gcnew InvalidOperationException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Name = "INVALID";
		Value = "INVALID";
		ErrorOutput(TextReader->LineNumber, "Failed to parse DEFINE directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

String^ DefineDirective::GetValue(String^ Prefix, DefineDirective::eAccessoryOperatorType ActiveOperator)
{
	String^ Result = "";

	if (Encoding == eEncodingType::MultiLine)
	{
		LineTrackingStringReader^ TextReader = gcnew LineTrackingStringReader(this->Value);
		Result += "\n" + TextReader->ReadLine();

		for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
			Result += "\n" + Prefix + ReadLine;

		Result = Result->Substring(1);
	}
	else
		Result += this->Value;

	switch (ActiveOperator)
	{
	case eAccessoryOperatorType::Stringize:
		Result = "\"" + Result + "\"";
		break;
	}

	return Result;
}

DefineDirective^ DefineDirective::CreateCopy()
{
	return dynamic_cast<DefineDirective^>(this->MemberwiseClone());
}

DefineDirective::eAccessoryOperatorType	DefineDirective::GetAccessoryOperatorFromToken(String^ Token)
{
	eAccessoryOperatorType ActiveOperator = eAccessoryOperatorType::None;

	UInt32 Index = 0;
	for each (String^ Itr in DefineDirective::AccessoryOperatorIdentifier)
	{
		if (Token->IndexOf(Itr) == 0 && Index != (UInt32)ActiveOperator)
		{
			ActiveOperator = (eAccessoryOperatorType)Index;
			break;
		}

		Index++;
	}

	return ActiveOperator;
}

String^ DefineDirective::GetToken()
{
	if (ErrorFlag)
		return Token;

	switch (Encoding)
	{
	case eEncodingType::SingleLine:
		return Token;
	case eEncodingType::MultiLine:
		return Token + "\n" + SliceStart + ";{\n" + ObfuscateToCompiler(Value) + "\n" + SliceEnd + ";}";
	default:
		return Token;
	}
}

ImportDirective::ImportDirective(String ^Token, StandardOutputError ^ErrorOutput, Preprocessor^ PreprocessorInstance, UInt32 LineNumber)
{
	this->Token = Token;
	this->Type = eDirectiveType::Import;
	this->Encoding = eEncodingType::SingleLine;

	obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->TokenCount)
			{
			case 1:
				throw gcnew InvalidOperationException("No value specified.");
			}

			int Index = LocalParser->GetTokenIndex(";" + EncodingIdentifier[(int)eEncodingType::SingleLine] + DirectiveIdentifier[(int)eDirectiveType::Import]);
			Filename = LocalParser->Tokens[Index + 1]->Replace("\"", "");
			String^ Source = "";
			String^ Result = "";

			try
			{
				String^ ImportFile = PreprocessorInstance->GetInstanceData()->DepotPath + Filename + ".txt";
				StreamReader^ ImportParser = gcnew StreamReader(ImportFile);
				Source = ImportParser->ReadToEnd();
				ImportParser->Close();
			}
			catch (Exception^ E)
			{
				throw gcnew InvalidOperationException("Couldn't read from IMPORT script - " + E->Message);
			}

			if (!PreprocessorInstance->Preprocess(Source, Result, ErrorOutput))
				throw gcnew InvalidOperationException("Errors encountered while processing IMPORT script");
			else
			{
				ImportSegment = Result;
			}
		}
		else
			throw gcnew InvalidOperationException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Filename = "INVALID";
		ImportSegment = "INVALID";
		ErrorOutput(LineNumber, "Failed to parse IMPORT directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

String^ ImportDirective::GetToken()
{
	if (ErrorFlag)
		return Token;
	else
		return ImportSegment;
}

void EnumDirective::ParseComponentDefineDirectives(String^ Source, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance, UInt32 LineNumber)
{
	obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer(", (){}[]\t\n");		// don't use the decimal separator as a delimiter as we're parsing FP numbers

	LocalParser->Tokenize(Source, false);
	float PreviousValue = 0;

	for each (String^ Itr in LocalParser->Tokens)
	{
		String^ Name;
		String^ ValueString;
		int Operator = Itr->IndexOf("=");
		float CurrentValue = 0;

		if (Operator != -1)
		{
			Name = Itr->Substring(0, Operator);
			ValueString = Itr->Substring(Operator + 1);
		}
		else
		{
			Name = Itr;
			ValueString = (((!PreviousValue)?-1:PreviousValue) + 1).ToString();
		}

		try			{ CurrentValue = float::Parse(ValueString); }
		catch (...) { throw gcnew InvalidOperationException("Invalid value assigned to " + Name); }
		PreviousValue = CurrentValue;

		String^ DefineToken = ";" + EncodingIdentifier[(int)eEncodingType::SingleLine] + DirectiveIdentifier[(int)eDirectiveType::Define] + " " + Name + " " + ValueString;
		ComponentDefineDirectives->Add(gcnew DefineDirective(DefineToken, ErrorOutput, PreprocessorInstance, LineNumber));
	}
}

EnumDirective::EnumDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance, UInt32 LineNumber)
{
	this->Token = Token;
	this->Type = eDirectiveType::Enum;
	this->Encoding = eEncodingType::SingleLine;
	ComponentDefineDirectives = gcnew List<DefineDirective^>();

	obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->TokenCount)
			{
			case 1:
				throw gcnew InvalidOperationException("No name specified.");
			case 2:
				throw gcnew InvalidOperationException("No value specified.");
			}

			int Index = LocalParser->GetTokenIndex(";" + EncodingIdentifier[(int)eEncodingType::SingleLine] + DirectiveIdentifier[(int)eDirectiveType::Enum]);
			String^ Value;

			Name = LocalParser->Tokens[Index + 1];
			Value = Token->Substring(LocalParser->Indices[Index + 1] + Name->Length + 1)->Replace("}", "")->Replace("{", "");

			ParseComponentDefineDirectives(Value, ErrorOutput, PreprocessorInstance, LineNumber);
		}
		else
			throw gcnew InvalidOperationException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Name = "INVALID";
		ComponentDefineDirectives->Clear();
		ErrorOutput(LineNumber, "Failed to parse ENUM directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

EnumDirective::EnumDirective(String^ Token, LineTrackingStringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	this->Token = Token;
	this->Type = eDirectiveType::Enum;
	this->Encoding = eEncodingType::MultiLine;
	ComponentDefineDirectives = gcnew List<DefineDirective^>();

	obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->TokenCount)
			{
			case 1:
				throw gcnew InvalidOperationException("No name specified.");
			}

			int Index = LocalParser->GetTokenIndex(";" + EncodingIdentifier[(int)eEncodingType::MultiLine] + DirectiveIdentifier[(int)eDirectiveType::Enum]);

			Name = LocalParser->Tokens[Index + 1];
			UInt32 LineNumber = TextReader->LineNumber;
			Value = GetMultilineValue(TextReader, SliceStart, SliceEnd);

			String^ CondensedValue = "";
			LineTrackingStringReader^ NewTextReader = gcnew LineTrackingStringReader(Value);
			for (String^ ReadLine = NewTextReader->ReadLine(); ReadLine != nullptr; ReadLine = NewTextReader->ReadLine())
				CondensedValue += ReadLine + " ";

			ParseComponentDefineDirectives(CondensedValue, ErrorOutput, PreprocessorInstance, LineNumber);
		}
		else
			throw gcnew InvalidOperationException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Name = "INVALID";
		Value = "INVALID";
		ComponentDefineDirectives->Clear();
		ErrorOutput(TextReader->LineNumber, "Failed to parse ENUM directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

String^ EnumDirective::GetToken()
{
	if (ErrorFlag)
		return Token;

	switch (Encoding)
	{
	case eEncodingType::SingleLine:
		return Token;
	case eEncodingType::MultiLine:
		return Token + "\n" + SliceStart + ";{\n" + ObfuscateToCompiler(Value) + "\n" + SliceEnd + ";}";
	default:
		return Token;
	}
}

bool IfDirective::Operator::Evaluator( eBuiltInOperators Type, String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance )
{
	bool Result = false;
	try
	{
		String^ LHSString = "";
		String^ RHSString = "";
		int LHSInt = 0.0;
		int RHSInt = 0.0;

		ProcessOperands(LHS, RHS, LHSString, RHSString, PreprocessorInstance);

		bool LHSNumeric = ParseAsInt(LHSString, LHSInt);
		bool RHSNumeric = ParseAsInt(RHSString, RHSInt);

		switch (Type)
		{
		case IfDirective::Operator::eBuiltInOperators::Equal:
			if (LHSNumeric && RHSNumeric)
				Result = (LHSInt == RHSInt);
			else if (!LHSNumeric && !RHSNumeric)
				Result = (LHSString == RHSString);
			else
				throw gcnew InvalidOperationException("Mismatching operand types.");

			break;
		case IfDirective::Operator::eBuiltInOperators::LessThanOrEqual:
			if (LHSNumeric && RHSNumeric)
				Result = (LHSInt <= RHSInt);
			else
				throw gcnew InvalidOperationException("Mismatching/invalid operand type(s).");

			break;
		case IfDirective::Operator::eBuiltInOperators::GreaterThanOrEqual:
			if (LHSNumeric && RHSNumeric)
				Result = (LHSInt >= RHSInt);
			else
				throw gcnew InvalidOperationException("Mismatching/invalid operand type(s).");

			break;
		case IfDirective::Operator::eBuiltInOperators::LessThan:
			if (LHSNumeric && RHSNumeric)
				Result = (LHSInt < RHSInt);
			else if (!LHSNumeric && !RHSNumeric)
				Result = (String::Compare(LHSString, RHSString) < 0);
			else
				throw gcnew InvalidOperationException("Mismatching/invalid operand type(s).");

			break;
		case IfDirective::Operator::eBuiltInOperators::GreaterThan:
			if (LHSNumeric && RHSNumeric)
				Result = (LHSInt > RHSInt);
			else if (!LHSNumeric && !RHSNumeric)
				Result = (String::Compare(LHSString, RHSString) > 0);
			else
				throw gcnew InvalidOperationException("Mismatching/invalid operand type(s).");

			break;
		case IfDirective::Operator::eBuiltInOperators::NotEqual:
			if (LHSNumeric && RHSNumeric)
				Result = (LHSInt != RHSInt);
			else if (!LHSNumeric && !RHSNumeric)
				Result = (LHSString != RHSString);
			else
				throw gcnew InvalidOperationException("Mismatching/invalid operand type(s).");

			break;
		case IfDirective::Operator::eBuiltInOperators::LogicalAND:
			if (LHSNumeric && RHSNumeric)
				Result = (LHSInt && RHSInt);
			else
				throw gcnew InvalidOperationException("Mismatching/invalid operand type(s).");

			break;
		case IfDirective::Operator::eBuiltInOperators::LogicalOR:
			if (LHSNumeric && RHSNumeric)
				Result = (LHSInt || RHSInt);
			else
				throw gcnew InvalidOperationException("Mismatching/invalid operand type(s).");

			break;
		default:
			throw gcnew InvalidOperationException("Unknown operator");

			break;
		}
	}
	catch (Exception^ E)
	{
		ErrorOutput(PreprocessorInstance->GetInstanceData()->CurrentLine,
					"Couldn't evaluate " + BuiltInOperatorsIdentifier[(int)Type] + " operator for operands '" + LHS + "' and '" + RHS + "' - " + E->Message);
		Result = false;
	}

	return Result;
}

IfDirective::Operator^ IfDirective::LookupOperatorByIdentifier(String^ Identifier)
{
	for each (Operator^ Itr in OperatorList)
	{
		if (Itr->GetIdentifier() == Identifier)
			return Itr;
	}
	return nullptr;
}

bool IfDirective::ParseAsInt(String^ Source, int% Result)
{
	int ParseResult = 0;

	try { ParseResult = int::Parse(Source); }
	catch (...)
	{
		return false;
	}

	Result = ParseResult;
	return true;
}

void IfDirective::ProcessOperands(String^ LHSSource, String^ RHSSource, String^% LHSResult, String^% RHSResult, Preprocessor^ PreprocessorInstance)
{
	String^ OperandBuffer = LHSSource;
	DefineDirective::eAccessoryOperatorType DefineOperator = DefineDirective::GetAccessoryOperatorFromToken(OperandBuffer);
	switch (DefineOperator)
	{
	case DefineDirective::eAccessoryOperatorType::None:
		break;
	default:
		OperandBuffer = OperandBuffer->Substring(DefineDirective::AccessoryOperatorIdentifier[(int)DefineOperator]->Length);
		break;
	}
	DefineDirective^ LHSDirective = PreprocessorInstance->LookupDefineDirectiveByName(OperandBuffer);
	LHSResult = ((LHSDirective)?(LHSDirective->GetValue(gcnew String(""), DefineOperator)):LHSSource);

	OperandBuffer = RHSSource;
	DefineOperator = DefineDirective::GetAccessoryOperatorFromToken(OperandBuffer);
	switch (DefineOperator)
	{
	case DefineDirective::eAccessoryOperatorType::None:
		break;
	default:
		OperandBuffer = OperandBuffer->Substring(DefineDirective::AccessoryOperatorIdentifier[(int)DefineOperator]->Length);
		break;
	}
	DefineDirective^ RHSDirective = PreprocessorInstance->LookupDefineDirectiveByName(OperandBuffer);
	RHSResult = ((RHSDirective)?(RHSDirective->GetValue(gcnew String(""), DefineOperator)):RHSSource);
}

bool IfDirective::EqualityOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	return Operator::Evaluator(Operator::eBuiltInOperators::Equal, LHS, RHS, ErrorOutput, PreprocessorInstance);
}

bool IfDirective::LessThanOrEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	return Operator::Evaluator(Operator::eBuiltInOperators::LessThanOrEqual, LHS, RHS, ErrorOutput, PreprocessorInstance);
}

bool IfDirective::GreaterThanOrEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	return Operator::Evaluator(Operator::eBuiltInOperators::GreaterThanOrEqual, LHS, RHS, ErrorOutput, PreprocessorInstance);
}

bool IfDirective::LessThanOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	return Operator::Evaluator(Operator::eBuiltInOperators::LessThan, LHS, RHS, ErrorOutput, PreprocessorInstance);
}

bool IfDirective::GreaterThanOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	return Operator::Evaluator(Operator::eBuiltInOperators::GreaterThan, LHS, RHS, ErrorOutput, PreprocessorInstance);
}

bool IfDirective::NotEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	return Operator::Evaluator(Operator::eBuiltInOperators::NotEqual, LHS, RHS, ErrorOutput, PreprocessorInstance);
}

bool IfDirective::LogicalAndOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	return Operator::Evaluator(Operator::eBuiltInOperators::LogicalAND, LHS, RHS, ErrorOutput, PreprocessorInstance);
}

bool IfDirective::LogicalOrOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	return Operator::Evaluator(Operator::eBuiltInOperators::LogicalOR, LHS, RHS, ErrorOutput, PreprocessorInstance);
}

bool IfDirective::ConvertInfixExpressionToPostFix(String^ Source, String^% Result, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	bool OperationResult = false;				// uses the shunting-yard algorithm

	try
	{
		String^ InfixExpression = Source;
		String^ PostFixExpression = "";
		Stack<String^>^ ExpressionStack = gcnew Stack<String^>();

		obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();
		LocalParser->Tokenize(InfixExpression, true);

		if (LocalParser->Valid)
		{
			for (UInt32 Index = 0; Index < LocalParser->Tokens->Count; Index++)
			{
				String^ Token = LocalParser->Tokens[Index];
				Char Delimiter = LocalParser->Delimiters[Index];

				if (Token != "")
				{
					Operator^ CurrentOperator = LookupOperatorByIdentifier(Token);
					if (CurrentOperator != nullptr)
					{
						while (ExpressionStack->Count)
						{
							String^ TopExpression = ExpressionStack->Peek();
							Operator^ TopOperator = LookupOperatorByIdentifier(TopExpression);
							if (TopOperator != nullptr && CurrentOperator->GetPrecedence() <= TopOperator->GetPrecedence())
								PostFixExpression += " " + ExpressionStack->Pop();
							else
								break;
						}

						ExpressionStack->Push(Token);
					}
					else										// if not an operator, then a macro name
						PostFixExpression += " " + Token;
				}

				if (Delimiter == '(')
					ExpressionStack->Push("" + Delimiter);
				else if (Delimiter == ')')
				{
					bool FoundSibling = false;

					while (ExpressionStack->Count)
					{
						String^ TopExpression = ExpressionStack->Peek();
						Operator^ TopOperator = LookupOperatorByIdentifier(TopExpression);

						if (TopExpression == "(")
						{
							FoundSibling = true;
							break;
						}
						else if (TopOperator != nullptr)
							PostFixExpression += " " + ExpressionStack->Pop();
						else
							throw gcnew InvalidOperationException("Encountered a non-operator while searching for '('.");
					}

					if (FoundSibling)
						ExpressionStack->Pop();
					else
						throw gcnew InvalidOperationException("Mismatching parentheses.");
				}
			}

			while (ExpressionStack->Count)
			{
				String^ TopExpression = ExpressionStack->Peek();
				Operator^ TopOperator = LookupOperatorByIdentifier(TopExpression);

				if (TopExpression == ")" || TopExpression == ")")
					throw gcnew InvalidOperationException("Mismatching parentheses.");
				else if (TopOperator != nullptr)
					PostFixExpression += " " + ExpressionStack->Pop();
				else
					throw gcnew InvalidOperationException("Encountered a non-operator while unwinding expression stack.");
			}

			OperationResult = true;
			Result = PostFixExpression;
		}
		else
			throw gcnew InvalidOperationException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		OperationResult = false;
		ErrorOutput(PreprocessorInstance->GetInstanceData()->CurrentLine,
					"Infix to postfix converter failed to parse expression '" + Source + "' - " + E->Message);
	}

	return OperationResult;
}

bool IfDirective::CheckBaseCondition(String^ Base, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	bool Result = false;

	try
	{
		String^ PostFixExpression = "";
		if (ConvertInfixExpressionToPostFix(Base, PostFixExpression, ErrorOutput, PreprocessorInstance))
		{
			Stack<String^>^ ExpressionStack = gcnew Stack<String^>();
			obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

			LocalParser->Tokenize(PostFixExpression, true);

			if (LocalParser->Valid)
			{
				for (UInt32 Index = 0; Index < LocalParser->Tokens->Count; Index++)
				{
					String^ Token = LocalParser->Tokens[Index];
					Operator^ CurrentOperator = LookupOperatorByIdentifier(Token);

					if (CurrentOperator)
					{
						if (ExpressionStack->Count < CurrentOperator->GetOperandCount())
							throw gcnew InvalidOperationException("Incorrect number of operands for operator '" + CurrentOperator->GetIdentifier() + "'.");
						else
						{
							String^ LHS = ExpressionStack->Pop();
							String^ RHS = ExpressionStack->Pop();

							ExpressionStack->Push(((int)CurrentOperator->Evaluate(LHS, RHS, ErrorOutput, PreprocessorInstance)).ToString());
						}
					}
					else			// a macro or rvalue
					{
						ExpressionStack->Push(Token);
					}
				}

				if (ExpressionStack->Count == 1)
				{
					String^ ResultExpression = ExpressionStack->Pop();
					if (ResultExpression == "1")
						Result = true;
					else if (ResultExpression == "0")
						Result = false;
					else if (PreprocessorInstance->LookupDefineDirectiveByName(ResultExpression) == nullptr)
						Result = false;		// doubles as a #ifdef when called without any operators
					else
						Result = true;

		//				throw gcnew InvalidOperationException("Invalid result expression '" + ResultExpression + "'.");
				}
				else
					throw gcnew InvalidOperationException("Too many operands.");
			}
		}
		else
			throw gcnew InvalidOperationException("Couldn't convert infix expression to postfix notation.");
	}
	catch (Exception^ E)
	{
		Result = false;
		ErrorOutput(PreprocessorInstance->GetInstanceData()->CurrentLine, "Failed to evaluate condition - " + E->Message);
	}

	return Result;
}

IfDirective::IfDirective(String^ Token, LineTrackingStringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	this->Token = Token;
	this->Type = eDirectiveType::If;
	this->Encoding = eEncodingType::MultiLine;

	obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->TokenCount)
			{
			case 1:
				throw gcnew InvalidOperationException("No condition specified.");
			}

			int Index = LocalParser->GetTokenIndex(";" + EncodingIdentifier[(int)eEncodingType::MultiLine] + DirectiveIdentifier[(int)eDirectiveType::If]);
			String^ ConditionString = Token->Substring(LocalParser->Indices[Index] + 4);
			Block = GetMultilineValue(TextReader, SliceStart, SliceEnd);

			String^ PreprocessedBlock = "";
			if (!PreprocessorInstance->Preprocess(Block, PreprocessedBlock, ErrorOutput))
				throw gcnew InvalidOperationException("Errors encountered while processing IF block.");
			else
			{
				Block = PreprocessedBlock;
			}

			BaseCondition = ConditionString;
			ValidationResult = CheckBaseCondition(BaseCondition, ErrorOutput, PreprocessorInstance);
		}
		else
			throw gcnew InvalidOperationException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		BaseCondition = "INVALID";
		Block = "INVALID";
		ValidationResult = false;
		ErrorOutput(TextReader->LineNumber, "Failed to parse IF directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

String^ IfDirective::GetToken()
{
	if (ErrorFlag)
		return Token;

	if (ValidationResult)
		return Token + "\n" + SliceStart + ";{\n" + Block + "\n" + SliceEnd + ";}";
	else
		return Token + "\n" + SliceStart + ";{\n" + ObfuscateToCompiler(Block) + "\n" + SliceEnd + ";}";
}

Preprocessor::Preprocessor()
{
	RegisteredDefineDirectives = gcnew List<DefineDirective^>();
	Busy = false;
}

Preprocessor^ Preprocessor::Get()
{
	if (Singleton == nullptr)
		Singleton = gcnew Preprocessor();

	return Singleton;
}

void Preprocessor::RegisterDefineDirective(DefineDirective^ Directive)
{
	DefineDirective^ ExistingDirective = LookupDefineDirectiveByName(Directive->GetName());

	if (ExistingDirective != nullptr)
		ExistingDirective->SetValue(Directive->GetValue(gcnew String(""), DefineDirective::eAccessoryOperatorType::None));
	else
		RegisteredDefineDirectives->Add(Directive->CreateCopy());
}

DefineDirective^ Preprocessor::LookupDefineDirectiveByName(String^ Name)
{
	for each (DefineDirective^ Itr in RegisteredDefineDirectives)
	{
		if (!String::Compare(Itr->GetName(), Name))
			return Itr;
	}
	return nullptr;
}

CSEPreprocessorToken^ Preprocessor::CreateDirectiveFromIdentifier(CSEPreprocessorDirective::eEncodingType Encoding, String^ Identifier, String^ Token, LineTrackingStringReader^ TextReader, StandardOutputError^ ErrorOutput)
{
	UInt32 LineNumber = 0;
	if (TextReader)
		LineNumber = TextReader->LineNumber;

	if (!String::Compare(Identifier, CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::eDirectiveType::Define], true))
	{
		if (Encoding == CSEPreprocessorDirective::eEncodingType::SingleLine)
			return gcnew DefineDirective(Token, ErrorOutput, this, LineNumber);
		else if (Encoding == CSEPreprocessorDirective::eEncodingType::MultiLine)
			return gcnew DefineDirective(Token, TextReader, ErrorOutput, this);
	}
	else if (!String::Compare(Identifier, CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::eDirectiveType::Import], true))
	{
		if (Encoding == CSEPreprocessorDirective::eEncodingType::SingleLine)
			return gcnew ImportDirective(Token, ErrorOutput, this, LineNumber);
	}
	else if (!String::Compare(Identifier, CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::eDirectiveType::Enum], true))
	{
		if (Encoding == CSEPreprocessorDirective::eEncodingType::SingleLine)
			return gcnew EnumDirective(Token, ErrorOutput, this, LineNumber);
		else if (Encoding == CSEPreprocessorDirective::eEncodingType::MultiLine)
			return gcnew EnumDirective(Token, TextReader, ErrorOutput, this);
	}
	else if (!String::Compare(Identifier, CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::eDirectiveType::If], true))
	{
		if (Encoding == CSEPreprocessorDirective::eEncodingType::MultiLine)
			return gcnew IfDirective(Token, TextReader, ErrorOutput, this);
	}

	return nullptr;
}

bool Preprocessor::Preprocess(String^ Source, String^% Result, StandardOutputError^ ErrorOutput)
{
	bool OperationResult = false;
	DataBuffer->ContainsDirectives = false;
	String^ PreprocessedText = "";

	try
	{
		LinkedList<CSEPreprocessorToken^>^ TokenList = gcnew LinkedList<CSEPreprocessorToken^>();
		LineTrackingStringReader^ TextReader = gcnew LineTrackingStringReader(Source);
		obScriptParsing::LineTokenizer^ LocalParser = gcnew obScriptParsing::LineTokenizer();

		for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
		{
			DataBuffer->CurrentLine = TextReader->LineNumber;
			LocalParser->Tokenize(ReadLine, false);
			CSEPreprocessorToken^ ThisToken = nullptr;

			if (LocalParser->Valid == false || ReadLine->Length == 0)
				ThisToken = gcnew CSEPreprocessorToken(ReadLine, ErrorOutput, this);
			else
			{
				String^ FirstToken = LocalParser->Tokens[0];
				if (FirstToken[0] == ';' && FirstToken->Length >= 2)
				{
					CSEPreprocessorDirective::eEncodingType Encoding = CSEPreprocessorDirective::eEncodingType::Invalid;
					bool TokenIsDirective = false;

					if (FirstToken[1] == CSEPreprocessorDirective::EncodingIdentifier[(int)CSEPreprocessorDirective::eEncodingType::SingleLine])
						Encoding = CSEPreprocessorDirective::eEncodingType::SingleLine;
					else if (FirstToken[1] == CSEPreprocessorDirective::EncodingIdentifier[(int)CSEPreprocessorDirective::eEncodingType::MultiLine])
						Encoding = CSEPreprocessorDirective::eEncodingType::MultiLine;

					if (Encoding != CSEPreprocessorDirective::eEncodingType::Invalid)
					{
						for each (String^ Directive in CSEPreprocessorDirective::DirectiveIdentifier)
						{
							if (LocalParser->GetTokenIndex(";" + CSEPreprocessorDirective::EncodingIdentifier[(int)Encoding] + Directive) == 0)
							{
								ThisToken = CreateDirectiveFromIdentifier(Encoding, Directive, ReadLine, TextReader, ErrorOutput);
								DataBuffer->ContainsDirectives = true;
								TokenIsDirective = true;
								break;
							}
						}
					}

					if (TokenIsDirective == false)
						ThisToken = gcnew CSEPreprocessorToken(ReadLine, ErrorOutput, this);
				}
				else
					ThisToken = gcnew CSEPreprocessorToken(ReadLine, ErrorOutput, this);
			}

			if (ThisToken != nullptr)
				TokenList->AddLast(ThisToken);
		}

		OperationResult = true;
		DataBuffer->CurrentLine = 1;
		for each (CSEPreprocessorToken^ Token in TokenList)
		{
			PreprocessedText += "\n" + Token->GetToken();
			CSEPreprocessorDirective^ Directive = dynamic_cast<CSEPreprocessorDirective^>(Token);

			if (Directive != nullptr && Directive->GetErrorFlag())
				OperationResult = false;

			DataBuffer->CurrentLine++;
		}

		if (PreprocessedText->Length > 1)
			Result = PreprocessedText->Substring(1);
		else
			Result = PreprocessedText;
	}
	catch (Exception^ E)
	{
		OperationResult = false;
		ErrorOutput(DataBuffer->CurrentLine, "Preprocessing failed - " + E->Message);
	}

	return OperationResult;
}

bool Preprocessor::PreprocessScript(String^ Source, String^% Result, StandardOutputError^ ErrorOutput, PreprocessorParams^ Data)
{
	bool OperationResult = false;

	if (Busy)
		ErrorOutput(1, "Preprocessing failed - A previous operation is in progress");
	else
	{
		DataBuffer = Data;
		Busy = true;

		String^ SourceBuffer = gcnew String(Source);
		String^ ResultBuffer = "";

		for (int i = 1; i <= Data->NoOfPasses; i++)
		{
			RegisteredDefineDirectives->Clear();
			ProcessStandardDirectives(Data->StandardDirectivePath, ErrorOutput);
			OperationResult = Preprocess(SourceBuffer, ResultBuffer, ErrorOutput);
			if (!OperationResult)
			{
				ErrorOutput(1, "Preprocessing failed in pass " + i.ToString());
				break;
			}

			SourceBuffer = ResultBuffer;
			ResultBuffer = "";
		}
		if (OperationResult)
			Result = SourceBuffer;

		RegisteredDefineDirectives->Clear();

		delete DataBuffer;

		DataBuffer = nullptr;
		Busy = false;
	}

	return OperationResult;
}

void Preprocessor::ProcessStandardDirectives(String^ Path, StandardOutputError^ ErrorOutput)
{
	if (Directory::Exists(Path))
	{
		DirectoryInfo^ PreprocessorDir = gcnew DirectoryInfo(Path);

		for each (FileInfo^ Itr in PreprocessorDir->GetFiles())
		{
			if (Itr->Extension == ".txt")
			{
				try
				{
					StreamReader^ TextParser = gcnew StreamReader(Itr->FullName);
					String^ FileContents = TextParser->ReadToEnd();
					String^ Throwaway = "";
					Preprocess(FileContents, Throwaway, ErrorOutput);
					TextParser->Close();
				}
				catch (Exception^ E)
				{
					ErrorOutput(1, "Couldn't read from standard preprocessor directives file '" + Itr->Name + "'!\n\tException: " + E->Message);
				}
			}
		}
	}
	else
		ErrorOutput(1, "Standard preprocessor directives folder not found!");
}

bool Preprocessor::GetImportFilePath( String^ Source, String^% Result, PreprocessorParams^ Data )
{
	bool OperationResult = false;

	if (Busy == false)
	{
		DataBuffer = Data;

		CSEPreprocessorToken^ Directive = CreateDirectiveFromIdentifier(CSEPreprocessorDirective::eEncodingType::SingleLine,
											CSEPreprocessorDirective::DirectiveIdentifier[(int)CSEPreprocessorDirective::eDirectiveType::Import],
											Source, nullptr, gcnew StandardOutputError(&DummyStandardErrorOutput));

		if (Directive->GetValid())
		{
			ImportDirective^ Import = dynamic_cast<ImportDirective^>(Directive);

			if (Import != nullptr)
			{
				Result = Data->DepotPath + Import->GetFilename() + ".txt";
				OperationResult = true;
			}
		}

		DataBuffer = nullptr;
	}

	return OperationResult;
}


} // namespace preprocessor


} // namespace scriptEditor


} // namespace cse