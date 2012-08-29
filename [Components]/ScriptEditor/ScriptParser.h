#pragma once
#include "[Common]\IncludesCLR.h"

namespace ConstructionSetExtender
{
	using namespace System::Runtime::Serialization;

	ref class ScriptParser
	{
	protected:
		static array<Keys>^									ScriptTextDelimiterKeys =
		{
			Keys::OemPeriod,
			Keys::Oemcomma,
			Keys::Space,
			Keys::OemOpenBrackets,
			Keys::OemCloseBrackets,
			Keys::Tab,
			Keys::Enter
		};
		static array<System::Windows::Input::Key>^			ScriptTextDelimiterKeysWPF =
		{
			System::Windows::Input::Key::OemPeriod,
			System::Windows::Input::Key::OemComma,
			System::Windows::Input::Key::Space,
			System::Windows::Input::Key::OemOpenBrackets,
			System::Windows::Input::Key::OemCloseBrackets,
			System::Windows::Input::Key::Tab,
			System::Windows::Input::Key::Enter
		};
	public:
		static String^										ScriptTextDelimiters = "., (){}[]\t\n";
		static String^										ScriptTextControlChars = " \t";

		static bool											GetIsDelimiterKey(Keys KeyCode);
		static bool											GetIsDelimiterKey(System::Windows::Input::Key KeyCode);

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
			e_SetFunctionValue,
			e_Set,
			e_Let,
			e_Call,
			e_Player,
		};

		static TokenType									GetTokenType(String^% Token);

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

		[DataContract]
		static enum class									VariableType
		{
			[EnumMember]
			e_Unknown		= -1,
			[EnumMember]
			e_Integer		= 0,
			[EnumMember]
			e_Float,
			[EnumMember]
			e_Ref,
			[EnumMember]
			e_String,
			[EnumMember]
			e_Array
		};

		static array<String^>^								VariableIDs =
		{
			"???",
			"int",
			"float",
			"ref",
			"string_var",
			"array_var"
		};

		static String^										GetVariableID(VariableType Type);
		static VariableType									GetVariableType(String^ Token);

		static array<String^>^								OperatorIDs =
		{
			":=", "||", "&&", "+=", "-=", "*=" ,
			"/=", "^=", ":", "::", "==", "!=",
			">", "<", ">=", "<=", "|", "&", "<<",
			">>", "+", "-", "*", "/", "%", "^",
			"$", "#", "*", "!", "->", "<-"
		};

		LinkedList<VariableRefCountData^>^					Variables;
		Stack<BlockType>^									BlockStack;
		List<String^>^										Tokens;
		List<UInt32>^										Indices;		// the position of each token relative to its parent line
		List<Char>^											Delimiters;
		bool												Valid;
		String^												ReferenceDelimiters;
		String^												ReferenceControlChars;

		ScriptParser();
		ScriptParser(String^ ReferenceDelimiters);
		ScriptParser(String^ ReferenceDelimiters, String^ ReferenceControlChars);
		virtual ~ScriptParser();

		virtual bool										Tokenize(String^ Source, bool CollectEmptyTokens);
		TokenType											GetLeadingTokenType(void);
		void												Reset();
		VariableRefCountData^								LookupVariableByName(String^% Variable);
		int													GetCommentTokenIndex(int BookendTokenIndex);		// returns the index of the token that contains the comment delimiter. argument specifies the end token index for the search
		UInt32												GetCurrentTokenCount() { return Tokens->Count; }
		int													GetTokenAtOffset(int Offset);						// returns the index of the token that takes up the passed offset

		bool												GetIsStringLiteral(String^% Source);				// checks if the passed string in enclosed in quotes
		virtual bool										GetIsPlayerToken(String^% Source);
		virtual bool										GetIsBlockValidForScriptType(String^% Source, ScriptType EditorScriptType);
		int													GetTokenIndex(String^ Source);						// returns the token index corresponding to the first match
		bool												GetContainsIllegalChar(String^% Source, String^ Includes, String^ Excludes);
		int													GetLineStartIndex(UInt32 StartPosition, String^% Source);
		int													GetLineEndIndex(UInt32 StartPosition, String^% Source);
		UInt32												GetTrailingTabCount(UInt32 StartPosition, String^% Source, String^ CharactersToSkip);
		bool												GetIsTokenOperator(String^% Source);
		virtual bool										CompareBlockStack(ScriptParser::BlockType Block);	// returns true if the top of the stack is of the passed block type
	};

	class ByteCodeParser
	{
	protected:
		static UInt32										Read16(Array^% Data, UInt32% CurrentOffset);
		static bool											LineHasData(String^% Line);
	public:
		static UInt32										GetOffsetForLine(String^% Line, Array^% Data, UInt32% CurrentOffset);
	};
}