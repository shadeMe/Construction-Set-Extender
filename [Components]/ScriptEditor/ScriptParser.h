#pragma once

ref class ScriptParser
{
public:
	ref struct											VariableRefCountData
	{
		String^											Name;
		unsigned int									RefCount;

		VariableRefCountData(String^ Name, unsigned int Count) : Name(Name), RefCount(Count) {};
	};

	static enum	class									TokenType
															{
																e_Invalid	= 0,
																e_ScriptName,
																e_Variable,
																e_Comment,
																e_Begin,
																e_End,
																e_While,
																e_ForEach,
																e_Loop,
																e_If,
																e_ElseIf,
																e_Else,
																e_EndIf,
																e_Return,
																e_SetFunctionValue
															};
	static enum	class									BlockType
															{
																e_Invalid	= 0,
																e_ScriptBlock,
																e_Loop,
																e_If,
																e_IfElse
															};
	static enum	class									ScriptBlock
															{
																e_Universal	= 0,
																e_ReferenceSpecific,
																e_MagicEffect
															};
	static enum	class									ScriptType
															{
																e_Object = 0,
																e_Quest,
																e_MagicEffect
															};
	static enum class									VariableType
															{
																e_Integer = 0,
																e_Float,
																e_Ref,
																e_String,
																e_Array
															};

	static array<String^>^								Operators =
															{
																":=", "||", "&&", "+=", "-=", "*=" ,
																"/=", "^=", ":", "::", "==", "!=",
																">", "<", ">=", "<=", "|", "&", "<<",
																">>", "+", "-", "*", "/", "%", "^",
																"$", "#", "*", "!", "->", "<-"
															};

	LinkedList<VariableRefCountData^>^							Variables;
	Stack<BlockType>^									BlockStack;
	List<String^>^										Tokens;
	List<UInt32>^										Indices;											// the position of each token relative to its parent line
	List<Char>^											Delimiters;
	bool												Valid;

	ScriptParser();

	void												Tokenize(String^ Source, bool CollectEmptyTokens);
	TokenType											GetTokenType(String^% Token);
	void												Reset();
	VariableRefCountData^										LookupVariableByName(String^% Variable);
	int													GetCommentTokenIndex(int BookendTokenIndex);		// returns the index of the token that contains the comment delimiter. argument specifies the end token index for the search
	UInt32												GetCurrentTokenCount() { return Tokens->Count; }
	int													GetTokenAtOffset(int Offset);						// returns the index of the token that takes up the passed offset

	bool												GetIsStringLiteral(String^% Source);				// checks if the passed string in enclosed in quotes
	bool												GetIsPlayerToken(String^% Source);
	bool												GetIsBlockValidForScriptType(String^% Source, ScriptType EditorScriptType);
	int													GetTokenIndex(String^ Source);						// returns the token index corresponding to the first match
	bool												GetContainsIllegalChar(String^% Source, String^ Includes, String^ Excludes);
	int													GetLineStartIndex(UInt32 StartPosition, String^% Source);
	int													GetLineEndIndex(UInt32 StartPosition, String^% Source);
	UInt32												GetTrailingTabCount(UInt32 StartPosition, String^% Source, String^ CharactersToSkip);
	bool												GetIsTokenOperator(String^% Source);
	bool												CompareBlockStack(ScriptParser::BlockType Block);	// returns true if the top of the stack is of the passed block type
};

class ByteCodeParser
{
	static UInt32										Read16(Array^% Data, UInt32% CurrentOffset);
	static bool											LineHasData(String^% Line);
public:
	static UInt32										GetOffsetForLine(String^% Line, Array^% Data, UInt32% CurrentOffset);
};