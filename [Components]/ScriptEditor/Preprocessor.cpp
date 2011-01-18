#include "Preprocessor.h"
#include "ScriptParser.h"
#include "Globals.h"
#include "OptionsDialog.h"

using namespace ScriptPreprocessor;

CSEPreprocessorToken::CSEPreprocessorToken(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	if (Token == nullptr || PreprocessorInstance == nullptr || ErrorOutput == nullptr)
		return;																// invoked by CSEPreprocessorDirective

	try
	{
		String^ ExpandedToken = "";
		ScriptParser^ LocalParser = gcnew ScriptParser();

		LocalParser->Tokenize(Token, true);
		ExpandedToken += Token->Substring(0, LocalParser->Indices[0]);		// gives us the controlchars to prepend
		int TokenIndex = 0;
		
		for each (String^% Itr in LocalParser->Tokens)
		{
			DefineDirective^ Macro = PreprocessorInstance->LookupDefineDirectiveByName(Itr);

			if (Macro != nullptr && LocalParser->IsComment(TokenIndex) == -1)
				ExpandedToken += Macro->GetValue();
			else
				ExpandedToken += Itr;
			
			if (LocalParser->Delimiters[TokenIndex] != '\n')
				ExpandedToken += LocalParser->Delimiters[TokenIndex];

			TokenIndex++;
		}

		this->Token = ExpandedToken;
	}
	catch (Exception^ E)
	{
		this->Token = Token;
		ErrorOutput("Failed to parse TOKEN '" + Token + "' - " + E->Message);
	}
}


String^ CSEPreprocessorDirective::GetMultilineValue(StringReader^% TextReader, String^% SliceStart, String^% SliceEnd)
{
	String^ Result = "";
	bool SliceStartFound = false, SliceEndFound = false;
	ScriptParser^ LocalParser = gcnew ScriptParser();

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
				Result += "\n" + ReadLine->Substring(0, FirstTokenIndex) + ReadLine->Substring(FirstTokenIndex + 1);
			}
			else
				Result += "\n" + ReadLine;
		}

		ReadLine = TextReader->ReadLine();
	}
	
	if (SliceStartFound == false || SliceEndFound == false)
		throw gcnew CSEGeneralException("Couldn't extract multiline value.");

	return Result->Substring(1);
}

String^ CSEPreprocessorDirective::ObfuscateToCompiler(String^% Token)
{
	String^ Result = "";
	StringReader^ TextReader = gcnew StringReader(Token);
	
	for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
	{
		Result += "\n" + ";" + ReadLine;
	}

	return Result->Substring(1);
}

bool DefineDirective::IsNameValid(String^% Name)
{
	bool Result = true;

	for each (Char Itr in Name)
	{
		if (Char::IsLetter(Itr) == false && Itr != '_')
		{
			Result = false;
			break;
		}
	}

	return Result;
}

DefineDirective::DefineDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	this->Token = Token;
	this->Type = DirectiveType::e_Define;
	this->Encoding = EncodingType::e_SingleLine;

	ScriptParser^ LocalParser = gcnew ScriptParser();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->GetCurrentTokenCount())
			{
			case 1:
				throw gcnew CSEGeneralException("No name specified.");
			case 2:
				throw gcnew CSEGeneralException("No value specified.");			
			}

			int Index = LocalParser->HasToken(";" + EncodingIdentifier[(int)EncodingType::e_SingleLine] + "define");
			Name = LocalParser->Tokens[Index + 1];
			Value = Token->Substring(LocalParser->Indices[Index + 1] + Name->Length + 1);	

			if (PreprocessorInstance->LookupDefineDirectiveByName(Name) != nullptr && OPTIONS->FetchSettingAsInt("AllowRedefinitions") == 0)
				throw gcnew CSEGeneralException("Invalid redefinition.");
			else if (IsNameValid(Name) == false)
				throw gcnew CSEGeneralException("Invalid character in name.");
			else
				PreprocessorInstance->RegisterDefineDirective(this);
		}
		else
			throw gcnew CSEGeneralException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Name = "INVALID";
		Value = "INVALID";
		ErrorOutput("Failed to parse DEFINE directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

DefineDirective::DefineDirective(String^ Token, StringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	this->Token = Token;
	this->Type = DirectiveType::e_Define;
	this->Encoding = EncodingType::e_MultiLine;

	ScriptParser^ LocalParser = gcnew ScriptParser();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->GetCurrentTokenCount())
			{
			case 1:
				throw gcnew CSEGeneralException("No name specified.");		
			}

			int Index = LocalParser->HasToken(";" + EncodingIdentifier[(int)EncodingType::e_MultiLine] + "define");
			Name = LocalParser->Tokens[Index + 1];
			Value = GetMultilineValue(TextReader, gcnew String(""), gcnew String(""));

			if (PreprocessorInstance->LookupDefineDirectiveByName(Name) != nullptr && OPTIONS->FetchSettingAsInt("AllowRedefinitions") == 0)
				throw gcnew CSEGeneralException("Invalid redefinition.");
			else if (IsNameValid(Name) == false)
				throw gcnew CSEGeneralException("Invalid character in name.");
			else
				PreprocessorInstance->RegisterDefineDirective(this);
		}
		else
			throw gcnew CSEGeneralException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Name = "INVALID";
		Value = "INVALID";
		ErrorOutput("Failed to parse DEFINE directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

DefineDirective^ DefineDirective::CreateCopy()
{
	return dynamic_cast<DefineDirective^>(this->MemberwiseClone());
}

String^ DefineDirective::GetToken()
{
	if (ErrorFlag)
		return Token;

	switch (Encoding)
	{
	case EncodingType::e_SingleLine:
		return Token;
	case EncodingType::e_MultiLine:
		return ObfuscateToCompiler(Token + "\n;{\n" + Value + "\n;}");
	default:
		return Token;
	}
}

ImportDirective::ImportDirective(String ^Token, StandardOutputError ^ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	this->Token = Token;
	this->Type = DirectiveType::e_Import;
	this->Encoding = EncodingType::e_SingleLine;

	ScriptParser^ LocalParser = gcnew ScriptParser();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->GetCurrentTokenCount())
			{
			case 1:
				throw gcnew CSEGeneralException("No value specified.");		
			}

			int Index = LocalParser->HasToken(";" + EncodingIdentifier[(int)EncodingType::e_SingleLine] + "import");
			Filename = LocalParser->Tokens[Index + 1]->Replace("\"", "");
			String^ Source = "";
			String^ Result = "";

			try
			{
				StreamReader^ ImportParser = gcnew StreamReader(String::Format("{0}Data\\Scripts\\{1}.txt", Globals::AppPath, Filename));
				Source = ImportParser->ReadToEnd();
				ImportParser->Close();
			} 
			catch (Exception^ E) {
				throw gcnew CSEGeneralException("Couldn't read from import script - " + E->Message);
			}
	
			if (!PreprocessorInstance->Preprocess(Source, Result, ErrorOutput))
				throw gcnew CSEGeneralException("Errors encountered while processing IMPORT script");
			else
			{
				ImportSegment = Result;
			}
		}
		else
			throw gcnew CSEGeneralException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Filename = "INVALID";
		ImportSegment = "INVALID";
		ErrorOutput("Failed to parse IMPORT directive in '" + Token + "' - " + E->Message);
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

void EnumDirective::ParseComponentDefineDirectives(String^% Source, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
{
	ScriptParser^ LocalParser = gcnew ScriptParser();

	LocalParser->Tokenize(Source, false);
	float PreviousValue = 0;

	for each (String^% Itr in LocalParser->Tokens)
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
		catch (...) { throw gcnew CSEGeneralException("Invalid value assigned to " + Name); }
		PreviousValue = CurrentValue;
		
		String^ DefineToken = ";" + EncodingIdentifier[(int)EncodingType::e_SingleLine] + "define " + Name + " " + ValueString;
		ComponentDefineDirectives->AddLast(gcnew DefineDirective(DefineToken, ErrorOutput, PreprocessorInstance));
	}	
}

EnumDirective::EnumDirective(String^ Token, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	this->Token = Token;
	this->Type = DirectiveType::e_Enum;
	this->Encoding = EncodingType::e_SingleLine;
	ComponentDefineDirectives = gcnew LinkedList<DefineDirective^>();

	ScriptParser^ LocalParser = gcnew ScriptParser();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->GetCurrentTokenCount())
			{
			case 1:
				throw gcnew CSEGeneralException("No name specified.");
			case 2:
				throw gcnew CSEGeneralException("No value specified.");			
			}

			int Index = LocalParser->HasToken(";" + EncodingIdentifier[(int)EncodingType::e_SingleLine] + "enum");
			String^ Value;

			Name = LocalParser->Tokens[Index + 1];
			Value = Token->Substring(LocalParser->Indices[Index + 1] + Name->Length + 1)->Replace("}", "")->Replace("{", "");	

			ParseComponentDefineDirectives(Value, ErrorOutput, PreprocessorInstance);
		}
		else
			throw gcnew CSEGeneralException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Name = "INVALID";
		ComponentDefineDirectives->Clear();
		ErrorOutput("Failed to parse ENUM directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

EnumDirective::EnumDirective(String^ Token, StringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	this->Token = Token;
	this->Type = DirectiveType::e_Enum;
	this->Encoding = EncodingType::e_SingleLine;
	ComponentDefineDirectives = gcnew LinkedList<DefineDirective^>();

	ScriptParser^ LocalParser = gcnew ScriptParser();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->GetCurrentTokenCount())
			{
			case 1:
				throw gcnew CSEGeneralException("No name specified.");	
			}

			int Index = LocalParser->HasToken(";" + EncodingIdentifier[(int)EncodingType::e_MultiLine] + "enum");

			Name = LocalParser->Tokens[Index + 1];
			Value = GetMultilineValue(TextReader, gcnew String(""), gcnew String(""));	

			String^ CondensedValue = "";
			StringReader^ TextReader = gcnew StringReader(Value);
			for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
			{
				CondensedValue += ReadLine + " ";
			}

			ParseComponentDefineDirectives(CondensedValue, ErrorOutput, PreprocessorInstance);
		}
		else
			throw gcnew CSEGeneralException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		Name = "INVALID";
		Value = "INVALID";
		ComponentDefineDirectives->Clear();
		ErrorOutput("Failed to parse ENUM directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}
}

String^ EnumDirective::GetToken()
{
	if (ErrorFlag)
		return Token;

	switch (Encoding)
	{
	case EncodingType::e_SingleLine:
		return Token;
	case EncodingType::e_MultiLine:
		return ObfuscateToCompiler(Token + "\n;{\n" + Value + "\n;}");
	default:
		return Token;
	}
}

IfDirective::Operator^ IfDirective::LookupOperatorByIdentifier(String^% Identifier)
{
	for each (Operator^ Itr in OperatorList)
	{
		if (Itr->GetIdentifier() == Identifier)
			return Itr;
	}
	return nullptr;
}

bool IfDirective::ParseAsInt(String^% Source, int% Result)
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

void IfDirective::ProcessOperands(String^% LHSSource, String^% RHSSource, String^% LHSResult, String^% RHSResult, Preprocessor^% PreprocessorInstance)
{
	DefineDirective^ LHSDirective = PreprocessorInstance->LookupDefineDirectiveByName(LHSSource);
	DefineDirective^ RHSDirective = PreprocessorInstance->LookupDefineDirectiveByName(RHSSource);

	LHSResult = ((LHSDirective)?(LHSDirective->GetValue()):LHSSource);
	RHSResult = ((RHSDirective)?(RHSDirective->GetValue()):RHSSource);
}

bool IfDirective::EqualityOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
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

		if (LHSNumeric && RHSNumeric)
			Result = (LHSInt == RHSInt);
		else if (!LHSNumeric && !RHSNumeric)
			Result = (LHSString == RHSString);
		else
			throw gcnew CSEGeneralException("Mismatching operand types.");
	}
	catch (Exception^ E)
	{
		ErrorOutput("Couldn't evaluate == operator for operands '" + LHS + "' and '" + RHS + "' - " + E->Message); 
		Result = false;
	}
	return Result;
}

bool IfDirective::LessThanOrEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
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

		if (LHSNumeric && RHSNumeric)
			Result = (LHSInt <= RHSInt);
		else
			throw gcnew CSEGeneralException("Mismatching/invalid operand type(s).");
	}
	catch (Exception^ E)
	{
		ErrorOutput("Couldn't evaluate <= operator for operands '" + LHS + "' and '" + RHS + "' - " + E->Message); 
		Result = false;
	}
	return Result;
}

bool IfDirective::GreaterThanOrEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
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

		if (LHSNumeric && RHSNumeric)
			Result = (LHSInt >= RHSInt);
		else
			throw gcnew CSEGeneralException("Mismatching/invalid operand type(s).");
	}
	catch (Exception^ E)
	{
		ErrorOutput("Couldn't evaluate >= operator for operands '" + LHS + "' and '" + RHS + "' - " + E->Message); 
		Result = false;
	}
	return Result;
}

bool IfDirective::LessThanOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
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

		if (LHSNumeric && RHSNumeric)
			Result = (LHSInt < RHSInt);
		else if (!LHSNumeric && !RHSNumeric)
			Result = (String::Compare(LHSString, RHSString) < 0);
		else
			throw gcnew CSEGeneralException("Mismatching/invalid operand type(s).");
	}
	catch (Exception^ E)
	{
		ErrorOutput("Couldn't evaluate < operator for operands '" + LHS + "' and '" + RHS + "' - " + E->Message); 
		Result = false;
	}
	return Result;
}

bool IfDirective::GreaterThanOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
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

		if (LHSNumeric && RHSNumeric)
			Result = (LHSInt > RHSInt);
		else if (!LHSNumeric && !RHSNumeric)
			Result = (String::Compare(LHSString, RHSString) > 0);
		else
			throw gcnew CSEGeneralException("Mismatching/invalid operand type(s).");
	}
	catch (Exception^ E)
	{
		ErrorOutput("Couldn't evaluate > operator for operands '" + LHS + "' and '" + RHS + "' - " + E->Message); 
		Result = false;
	}
	return Result;
}

bool IfDirective::NotEqualOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
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

		if (LHSNumeric && RHSNumeric)
			Result = (LHSInt != RHSInt);
		else if (!LHSNumeric && !RHSNumeric)
			Result = (LHSString != RHSString);
		else
			throw gcnew CSEGeneralException("Mismatching/invalid operand type(s).");
	}
	catch (Exception^ E)
	{
		ErrorOutput("Couldn't evaluate != operator for operands '" + LHS + "' and '" + RHS + "' - " + E->Message); 
		Result = false;
	}
	return Result;
}

bool IfDirective::LogicalAndOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
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

		if (LHSNumeric && RHSNumeric)
			Result = (LHSInt && RHSInt);
		else
			throw gcnew CSEGeneralException("Mismatching/invalid operand type(s).");
	}
	catch (Exception^ E)
	{
		ErrorOutput("Couldn't evaluate && operator for operands '" + LHS + "' and '" + RHS + "' - " + E->Message); 
		Result = false;
	}
	return Result;
}

bool IfDirective::LogicalOrOperatorEvaluator(String^ LHS, String^ RHS, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
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

		if (LHSNumeric && RHSNumeric)
			Result = (LHSInt || RHSInt);
		else
			throw gcnew CSEGeneralException("Mismatching/invalid operand type(s).");
	}
	catch (Exception^ E)
	{
		ErrorOutput("Couldn't evaluate || operator for operands '" + LHS + "' and '" + RHS + "' - " + E->Message); 
		Result = false;
	}
	return Result;
}

bool IfDirective::ConvertInfixExpressionToPostFix(String^% Source, String^% Result, StandardOutputError^ ErrorOutput)
{
	bool OperationResult = false;				// uses the shunting-yard algorithm 

	try
	{
		String^ InfixExpression = Source; // Source->Substring(1, Source->Length - 2);
		String^ PostFixExpression = "";
		Stack<String^>^ ExpressionStack = gcnew Stack<String^>();

		ScriptParser^ LocalParser = gcnew ScriptParser();
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
							throw gcnew CSEGeneralException("Encountered a non-operator while searching for '('.");
					}

					if (FoundSibling)
						ExpressionStack->Pop();
					else
						throw gcnew CSEGeneralException("Mismatching parentheses.");
				}
			}

			while (ExpressionStack->Count)
			{
				String^ TopExpression = ExpressionStack->Peek();
				Operator^ TopOperator = LookupOperatorByIdentifier(TopExpression);

				if (TopExpression == ")" || TopExpression == ")")
					throw gcnew CSEGeneralException("Mismatching parentheses.");
				else if (TopOperator != nullptr)
					PostFixExpression += " " + ExpressionStack->Pop();
				else
					throw gcnew CSEGeneralException("Encountered a non-operator while unwinding expression stack.");
			}

			OperationResult = true;
			Result = PostFixExpression;
		}
		else
			throw gcnew CSEGeneralException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		OperationResult = false;
		ErrorOutput("Infix to postfix converter failed to parse expression '" + Source + "' - " + E->Message); 
	}

	return OperationResult;
}


bool IfDirective::CheckBaseCondition(String^% Base, StandardOutputError^ ErrorOutput, Preprocessor^% PreprocessorInstance)
{
	bool Result = false;
	
	try
	{
		String^ PostFixExpression = "";
		if (ConvertInfixExpressionToPostFix(Base, PostFixExpression, ErrorOutput))
		{
			Stack<String^>^ ExpressionStack = gcnew Stack<String^>();
			ScriptParser^ LocalParser = gcnew ScriptParser();

			LocalParser->Tokenize(PostFixExpression, true);

			if (LocalParser->Valid)
			{
				for (UInt32 Index = 0; Index < LocalParser->Tokens->Count; Index++)
				{
					String^ Token = LocalParser->Tokens[Index];
					DefineDirective^ CurrentDirective = PreprocessorInstance->LookupDefineDirectiveByName(Token);
					Operator^ CurrentOperator = LookupOperatorByIdentifier(Token);
					
					if (CurrentOperator)
					{
						if (ExpressionStack->Count < CurrentOperator->GetOperandCount())
							throw gcnew CSEGeneralException("Incorrect number of operands for operator '" + CurrentOperator->GetIdentifier() + "'.");
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
					else
						throw gcnew CSEGeneralException("Invalid result expression '" + ResultExpression + "'.");
				}
				else
					throw gcnew CSEGeneralException("Too many operands.");

			}
		}
		else
			throw gcnew CSEGeneralException("Couldn't convert infix expression to postfix notation.");
	}
	catch (Exception^ E)
	{
		Result = false;
		ErrorOutput("Failed to evaluate condition - " + E->Message);
	}

	return Result;
}

IfDirective::IfDirective(String^ Token, StringReader^% TextReader, StandardOutputError^ ErrorOutput, Preprocessor^ PreprocessorInstance)
{
	this->Token = Token;
	this->Type = DirectiveType::e_If;
	this->Encoding = EncodingType::e_MultiLine;

	ScriptParser^ LocalParser = gcnew ScriptParser();

	try
	{
		LocalParser->Tokenize(Token, false);
		if (LocalParser->Valid)
		{
			switch (LocalParser->GetCurrentTokenCount())
			{
			case 1:
				throw gcnew CSEGeneralException("No condition specified.");	
			}

			int Index = LocalParser->HasToken(";" + EncodingIdentifier[(int)EncodingType::e_MultiLine] + "if");
			String^ ConditionString = Token->Substring(LocalParser->Indices[Index] + 4);
			Block = GetMultilineValue(TextReader, SliceStart, SliceEnd);

			String^ PreprocessedBlock = "";
			if (!PreprocessorInstance->Preprocess(Block, PreprocessedBlock, ErrorOutput))
				throw gcnew CSEGeneralException("Errors encountered while processing IF block.");
			else
			{
				Block = PreprocessedBlock;
			}

			BaseCondition = ConditionString;
			ValidationResult = CheckBaseCondition(BaseCondition, ErrorOutput, PreprocessorInstance);
		}
		else
			throw gcnew CSEGeneralException("Token parser was in an invalid state.");
	}
	catch (Exception^ E)
	{
		BaseCondition = "INVALID";
		Block = "INVALID";
		ValidationResult = false;
		SliceStart = "";
		SliceEnd = "";
		ErrorOutput("Failed to parse IF directive in '" + Token + "' - " + E->Message);
		ErrorFlag = true;
	}	
}

String^ IfDirective::GetToken()
{
	if (ErrorFlag)
		return Token;

	if (ValidationResult)
		return Token + "\n" + SliceStart + "\n" + Block + "\n" + SliceEnd;
	else
		return Token + "\n" + SliceStart + ";{\n" + ObfuscateToCompiler(Block) + "\n" + SliceEnd + ";}";
}


Preprocessor::Preprocessor()
{
	RegisteredDefineDirectives = gcnew LinkedList<DefineDirective^>();
}

Preprocessor^% Preprocessor::GetSingleton()
{
	if (Singleton == nullptr) {
		Singleton = gcnew Preprocessor();
	}
	return Singleton;
}


void Preprocessor::RegisterDefineDirective(DefineDirective^ Directive)
{
	DefineDirective^ ExistingDirective = LookupDefineDirectiveByName(Directive->GetName());

	if (ExistingDirective != nullptr)
		ExistingDirective->SetValue(Directive->GetValue());
	else
		RegisteredDefineDirectives->AddLast(Directive->CreateCopy());
}

DefineDirective^ Preprocessor::LookupDefineDirectiveByName(String^% Name)
{
	for each (DefineDirective^ Itr in RegisteredDefineDirectives)
	{
		if (!String::Compare(Itr->GetName(), Name))
			return Itr;
	}
	return nullptr;
}

CSEPreprocessorToken^ Preprocessor::CreateDirectiveFromIdentifier(CSEPreprocessorDirective::EncodingType Encoding, String^ Identifier, String^ Token, StringReader^ TextReader, StandardOutputError^ ErrorOutput)
{
	if (!String::Compare(Identifier, "define", true))
	{
		if (Encoding == CSEPreprocessorDirective::EncodingType::e_SingleLine)
			return gcnew DefineDirective(Token, ErrorOutput, this);
		else if (Encoding == CSEPreprocessorDirective::EncodingType::e_MultiLine)
			return gcnew DefineDirective(Token, TextReader, ErrorOutput, this);
	}
	else if (!String::Compare(Identifier, "import", true))
	{
		if (Encoding == CSEPreprocessorDirective::EncodingType::e_SingleLine)
			return gcnew ImportDirective(Token, ErrorOutput, this);
	}
	else if (!String::Compare(Identifier, "enum", true))
	{
		if (Encoding == CSEPreprocessorDirective::EncodingType::e_SingleLine)
			return gcnew EnumDirective(Token, ErrorOutput, this);
		else if (Encoding == CSEPreprocessorDirective::EncodingType::e_MultiLine)
			return gcnew EnumDirective(Token, TextReader, ErrorOutput, this);
	}
	else if (!String::Compare(Identifier, "if", true))
	{
		if (Encoding == CSEPreprocessorDirective::EncodingType::e_MultiLine)
			return gcnew IfDirective(Token, TextReader, ErrorOutput, this);
	}

	return nullptr;
}

bool Preprocessor::Preprocess(String^% Source, String^% Result, StandardOutputError^ ErrorOutput)
{
	bool OperationResult = false;
	String^ PreprocessedText = "";

	try
	{
		LinkedList<CSEPreprocessorToken^>^ TokenList = gcnew LinkedList<CSEPreprocessorToken^>();
		StringReader^ TextReader = gcnew StringReader(Source);
		ScriptParser^ LocalParser = gcnew ScriptParser();

		for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
		{
			LocalParser->Tokenize(ReadLine, false);
			CSEPreprocessorToken^ ThisToken = nullptr;

			if (LocalParser->Valid == false || ReadLine->Length == 0)
				ThisToken = gcnew CSEPreprocessorToken(ReadLine, ErrorOutput, this);
			else
			{
				String^ FirstToken = LocalParser->Tokens[0];
				if (FirstToken[0] == ';' && FirstToken->Length >= 2)
				{
					CSEPreprocessorDirective::EncodingType Encoding = CSEPreprocessorDirective::EncodingType::e_Invalid;
					bool TokenIsDirective = false;

					if (FirstToken[1] == CSEPreprocessorDirective::EncodingIdentifier[(int)CSEPreprocessorDirective::EncodingType::e_SingleLine])
						Encoding = CSEPreprocessorDirective::EncodingType::e_SingleLine;
					else if (FirstToken[1] == CSEPreprocessorDirective::EncodingIdentifier[(int)CSEPreprocessorDirective::EncodingType::e_MultiLine])
						Encoding = CSEPreprocessorDirective::EncodingType::e_MultiLine;

					if (Encoding != CSEPreprocessorDirective::EncodingType::e_Invalid)
					{
						for each (String^ Directive in CSEPreprocessorDirective::DirectiveIdentifier)
						{
							if (LocalParser->HasToken(";" + CSEPreprocessorDirective::EncodingIdentifier[(int)Encoding] + Directive) == 0)
							{
								ThisToken = CreateDirectiveFromIdentifier(Encoding, Directive, ReadLine, TextReader, ErrorOutput);
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
		for each (CSEPreprocessorToken^ Token in TokenList)
		{
			PreprocessedText += "\n" + Token->GetToken();
			CSEPreprocessorDirective^ Directive = dynamic_cast<CSEPreprocessorDirective^>(Token);

			if (Directive != nullptr && Directive->GetErrorFlag())
				OperationResult = false;
		}

		if (PreprocessedText->Length > 1)
			Result = PreprocessedText->Substring(1);
		else
			Result = PreprocessedText;
	}
	catch (Exception^ E)
	{
		OperationResult = false;
		ErrorOutput("Preprocessing failed - " + E->Message);
	}

	return OperationResult;
}

bool Preprocessor::ScriptEditorDelegate(String^% Source, String^% Result, StandardOutputError^ ErrorOutput)
{
	bool OperationResult = false;

	RegisteredDefineDirectives->Clear();
	ProcessStandardDefineDirectives(ErrorOutput);
	OperationResult = Preprocess(Source, Result, ErrorOutput);

	return OperationResult;
}

void Preprocessor::ProcessStandardDefineDirectives(StandardOutputError^ ErrorOutput)
{
	String^ Path = Globals::AppPath + "Data\\OBSE\\Plugins\\ComponentDLLs\\CSE\\STDPreprocDefs.txt";

	if (File::Exists(Path))
	{
		try 
		{
			StreamReader^ TextParser = gcnew StreamReader(Path);
			String^ FileContents = TextParser->ReadToEnd();
			String^ Throwaway = "";
			Preprocess(FileContents, Throwaway, ErrorOutput);
			TextParser->Close();		
		}
		catch (Exception^ E) 
		{
			DebugPrint("Couldn't read from standard define directives file!\n\tException: " + E->Message);
		}
	} 
	else
		DebugPrint("Standard define directives file not found!");
}