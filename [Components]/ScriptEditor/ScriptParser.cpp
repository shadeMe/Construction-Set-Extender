#include "ScriptParser.h"
#include "[Common]\NativeWrapper.h"

namespace ConstructionSetExtender
{
	ScriptParser::ScriptParser()
	{
		Variables = gcnew LinkedList<VariableRefCountData^>();
		BlockStack = gcnew Stack<BlockType>();
		Tokens = gcnew List<String^>();
		Indices = gcnew List<UInt32>();
		Delimiters = gcnew List<Char>();
		Valid = true;

		this->ReferenceDelimiters = gcnew String(ScriptTextDelimiters);
		this->ReferenceControlChars = gcnew String(ScriptTextControlChars);
	}

	ScriptParser::ScriptParser( String^ ReferenceDelimiters, String^ ReferenceControlChars )
	{
		Variables = gcnew LinkedList<VariableRefCountData^>();
		BlockStack = gcnew Stack<BlockType>();
		Tokens = gcnew List<String^>();
		Indices = gcnew List<UInt32>();
		Delimiters = gcnew List<Char>();
		Valid = true;

		this->ReferenceDelimiters = gcnew String(ReferenceDelimiters);
		this->ReferenceControlChars = gcnew String(ReferenceControlChars);
	}

	ScriptParser::ScriptParser( String^ ReferenceDelimiters )
	{
		Variables = gcnew LinkedList<VariableRefCountData^>();
		BlockStack = gcnew Stack<BlockType>();
		Tokens = gcnew List<String^>();
		Indices = gcnew List<UInt32>();
		Delimiters = gcnew List<Char>();
		Valid = true;

		this->ReferenceDelimiters = gcnew String(ReferenceDelimiters);
		this->ReferenceControlChars = gcnew String(ScriptTextControlChars);
	}

	bool ScriptParser::GetIsPlayerToken(String^% Source)
	{
		bool Result = false;

		if (!String::Compare(Source, "player", true) || !String::Compare(Source, "playerref", true))
			Result = true;

		return Result;
	}

	bool ScriptParser::GetIsStringLiteral(String^% Source)
	{
		bool Result = false;

		if (Source[0] == '"' && Source[Source->Length - 1] == '"')
			Result = true;

		return Result;
	}

	int ScriptParser::GetCommentTokenIndex( int BookendTokenIndex )
	{
		int Pos = 0;
		int Result = -1;

		for each (String^ Itr in Tokens)
		{
			if (Pos == BookendTokenIndex)
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

	ScriptParser::VariableRefCountData^ ScriptParser::LookupVariableByName(String^% Variable)
	{
		VariableRefCountData^ Result = nullptr;

		for each (VariableRefCountData^ Itr in Variables)
		{
			if (!String::Compare(Itr->Name, Variable, true))
			{
				Result = Itr;
				break;
			}
		}
		return Result;
	}

	bool ScriptParser::Tokenize( String^ Source, bool CollectEmptyTokens )
	{
		if (Source->Length && Source[Source->Length - 1] != '\n')
			Source += "\n";

		Valid = false;
		int StartPos = -1, LastPos = -1;

		Tokens->Clear();
		Delimiters->Clear();
		Indices->Clear();

		for each (Char Itr in Source)
		{
			if (ReferenceControlChars->IndexOf(Itr) == -1)
			{
				StartPos = Source->IndexOf(Itr);
				break;
			}
		}

		if (StartPos == -1)
			return false;

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
			Valid = true;

		return Valid;
	}

	ScriptParser::TokenType ScriptParser::GetTokenType(String^% Token)
	{
		TokenType Result = TokenType::e_Invalid;
		if (Token->Length)
		{
			if (!String::Compare(Token, "scn", true) || !String::Compare(Token, "scriptName", true))
				Result = TokenType::e_ScriptName;
			else if (ScriptParser::GetVariableType(Token) != VariableType::e_Unknown)
				Result = TokenType::e_Variable;
			else if (Token[0] == ';')
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
			else if (!String::Compare(Token, "set", true))
				Result = TokenType::e_Set;
			else if (!String::Compare(Token, "let", true))
				Result = TokenType::e_Let;
			else if (!String::Compare(Token, "call", true))
				Result = TokenType::e_Call;
			else if (!String::Compare(Token, "player", true) || !String::Compare(Token, "playerref", true))
				Result = TokenType::e_Player;
		}

		return Result;
	}

	void ScriptParser::Reset()
	{
		Variables->Clear();
		BlockStack->Clear();
		Tokens->Clear();
		Delimiters->Clear();
		Indices->Clear();
		Valid = true;
	}

	bool ScriptParser::GetIsBlockValidForScriptType(String^% Source, ScriptParser::ScriptType EditorScriptType)
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

	int ScriptParser::GetTokenIndex(String^ Source)
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

	bool ScriptParser::GetContainsIllegalChar(String^% Source, String^ Includes, String^ Excludes)
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

	int ScriptParser::GetLineStartIndex(UInt32 StartPosition, String^% Source)
	{
		int Result = -1;
		for (int i = StartPosition; i > 0; i--)
		{
			if (Source[i] == '\n' || Source[i] == '\r\n')
			{
				Result = i + 1;
				break;
			}
		}

		return Result;
	}
	int ScriptParser::GetLineEndIndex(UInt32 StartPosition, String^% Source)
	{
		int Result = -1;
		for (int i = StartPosition; i < Source->Length; i++)
		{
			if (Source[i] == '\n' || Source[i] == '\r\n')
			{
				Result = i;
				break;
			}
		}

		return Result;
	}
	UInt32 ScriptParser::GetTrailingTabCount(UInt32 StartPosition, String^% Source, String^ CharactersToSkip)
	{
		UInt32 Result = 0;
		for (int i = StartPosition; i < Source->Length; i++)
		{
			if (Source[i] == '\t')
				Result += 1;
			else if (CharactersToSkip != nullptr)
			{
				array<Char>^ SkipArray = CharactersToSkip->ToCharArray();

				for each (Char Itr in SkipArray)
				{
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

	bool ScriptParser::GetIsTokenOperator(String^% Source)
	{
		for each(String^ Itr in OperatorIDs)
		{
			if (!String::Compare(Source, Itr))
				return true;
		}

		return false;
	}

	int ScriptParser::GetTokenAtOffset( int Offset )
	{
		int ExtractOffset = -1;

		for (int i = 0; i < GetCurrentTokenCount(); i++)
		{
			if (i + 1 < GetCurrentTokenCount())
			{
				int LineOffsetA = Indices[i], LineOffsetB = Indices[i + 1];
				if (Offset >= LineOffsetA && Offset < LineOffsetB)
				{
					ExtractOffset = i;
					break;
				}
			}
			else if (Offset < Indices[i])
				ExtractOffset = i;
		}

		return ExtractOffset;
	}

	bool ScriptParser::CompareBlockStack( ScriptParser::BlockType Block )
	{
		if (BlockStack->Count)
			return BlockStack->Peek() == Block;
		else
			return false;
	}

	bool ScriptParser::GetIsDelimiterKey( System::Windows::Input::Key KeyCode )
	{
		bool Result = false;

		for each (System::Windows::Input::Key Itr in ScriptTextDelimiterKeysWPF)
		{
			if (Itr == KeyCode)
			{
				Result = true;
				break;
			}
		}
		return Result;
	}

	bool ScriptParser::GetIsDelimiterKey( Keys KeyCode )
	{
		bool Result = false;

		for each (Keys Itr in ScriptTextDelimiterKeys)
		{
			if (Itr == KeyCode)
			{
				Result = true;
				break;
			}
		}
		return Result;
	}

	ScriptParser::TokenType ScriptParser::GetLeadingTokenType( void )
	{
		if (Valid)
			return GetTokenType(Tokens[0]);
		else
			return TokenType::e_Invalid;
	}

	String^ ScriptParser::GetVariableID( VariableType Type )
	{
		return VariableIDs[(int)Type];
	}

	ScriptParser::VariableType ScriptParser::GetVariableType( String^ Token )
	{
		VariableType DataType = VariableType::e_Unknown;

		if (!String::Compare(Token, "ref", true) || !String::Compare(Token, "reference", true))
			DataType = ScriptParser::VariableType::e_Ref;
		else if	(!String::Compare(Token, "short", true) || !String::Compare(Token, "long", true) || !String::Compare(Token, "int", true))
			DataType = ScriptParser::VariableType::e_Integer;
		else if	(!String::Compare(Token, "float", true))
			DataType = ScriptParser::VariableType::e_Float;
		else if	(!String::Compare(Token, "string_var", true))
			DataType = ScriptParser::VariableType::e_String;
		else if	(!String::Compare(Token, "array_var", true))
			DataType = ScriptParser::VariableType::e_Array;

		return DataType;
	}

	ScriptParser::~ScriptParser()
	{
		Reset();
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
		for each (char Itr in Extract)
		{
			if (Itr != '\t' && Itr != ' ')	break;
			Idx++;
		}

		Extract = Extract->Substring(Idx)->Split('\t', '\r', '\n', ' ')[0];
		if (Extract == "")
			return false;

		if (!String::Compare(Extract, "ref", true) ||
			!String::Compare(Extract, "reference", true) ||
			!String::Compare(Extract, "string_var", true) ||
			!String::Compare(Extract, "array_var", true) ||
			!String::Compare(Extract, "long", true) ||
			!String::Compare(Extract, "short", true) ||
			!String::Compare(Extract, "float", true) ||
			!String::Compare(Extract, "int", true))
		{
			return false;
		}
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
}