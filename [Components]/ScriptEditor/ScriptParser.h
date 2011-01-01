#pragma once

#include "[Common]\Includes.h"

public ref class ScriptParser
{						
public:	
	ref struct												VariableInfo 
	{
		String^												VarName;
		unsigned int										RefCount;
															
															VariableInfo(String^ Name, unsigned int Count) : VarName(Name), RefCount(Count) {};

		static const VariableInfo^							NullVar = gcnew ScriptParser::VariableInfo("@nullptr", 0);

		bool												IsValid() { return VarName != "@nullptr"; }
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
																e_If
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

	static array<String^>^								Operators =
															{
																":=", "||", "&&", "+=", "-=", "*=" , 
																"/=", "^=", ":", "::", "==", "!=",
																">", "<", ">=", "<=", "|", "&", "<<",
																">>", "+", "-", "*", "/", "%", "^",
																"$", "#", "*", "!", "->", "<-"
															};


	void												Tokenize(String^ Source, bool AllowNulls);
	TokenType											GetTokenType(String^% Token);
	void												Reset();
	VariableInfo^										FindVariable(String^% Variable);	
	int													IsComment(int Index);					// returns the index of the token that contains the comment delimiter. argument specifies the end token index for the search
	bool												IsLiteral(String^% Source);				// checks if the passed string in enclosed in quotes
	bool												HasAlpha(int Index);
	bool												IsPlayer(String^% Source);
	bool												IsValidBlock(String^% Source, ScriptType EditorScriptType);
	int													HasToken(String^ Source);				// returns the token index
	bool												HasStringGotIllegalChar(String^% Source, String^ Includes, String^ Excludes);

	int													GetLineStartIndex(UInt32 StartPosition, String^% Source);
	int													GetLineEndIndex(UInt32 StartPosition, String^% Source);
	UInt32												GetTrailingTabCount(UInt32 StartPosition, String^% Source, String^ CharactersToSkip);
	bool												IsOperator(String^% Source);

	LinkedList<VariableInfo^>^							Variables;
	Stack<BlockType>^									BlockStack;
	List<String^>^										Tokens;
	List<UInt32>^										Indices;								// the position of each token relative to its parent line
	List<Char>^											Delimiters;

	unsigned int										CurrentLineNo;
	unsigned int										PreviousLineNo;
	String^												ScriptName;
	bool												Valid;

														ScriptParser();
};

class ByteCodeParser
{
	static UInt32										Read16(Array^% Data, UInt32% CurrentOffset);
	static bool											LineHasData(String^% Line);
public:
	static UInt32										GetOffsetForLine(String^% Line, Array^% Data, UInt32% CurrentOffset);
};

public ref class Preprocessor
{
public:
	static enum	class									PreprocessOp
															{
																e_Expand = 0,				// serialize
																e_Collapse					// deserialize
															};

	delegate void										StandardOutputError(String^ Message);
private:
	static Preprocessor^								Singleton = nullptr;
	Preprocessor();

	Dictionary<String^, String^>^						PreprocessMacros;					// key = macro name, value = macro value
	ScriptParser^										TextParser;
	StandardOutputError^								ErrorOutput;
	bool												ErrorFlag;
	bool												SuppressOutput;

	void												LogErrorMessage(String^ Message, bool OverrideSuppression);
	void												LogDebugMessage(String^ Message);

	int													GetMacroIndex(String^% Macro);		// returns the index of the tracked macro, in the order of definition
	String^												GetMacroAtIndex(UInt32 Index);		// returns the identifier of the macro at the passed index

	void												ProcessStandardDefineDirectives(void);

	void												ParseEnumMacros(String^% Items, bool ReportErrors);

	void												ParseNestedDirectives(StringReader^% PreprocessParser, String^% ReadLine, UInt32& LineStart, UInt32& LineEnd);
	String^												ParseImportDirective(String^% Source, PreprocessOp Operation, String^% ReadLine, UInt32 LineStart, UInt32 LineEnd, bool Recursing);
	String^												ParseDefineDirective(String^% Source, PreprocessOp Operation, String^% ReadLine);
	String^												ParseEnumDirective(String^% Source, PreprocessOp Operation, String^% ReadLine);

	String^												DoPreprocess(PreprocessOp Operation, String^% Source, String^% ExtractedCSEBlock, bool Recursing);
public:
	static Preprocessor^%								GetSingleton();

	bool												Preprocess(PreprocessOp Operation, String^% Source, String^% Result, String^% ExtractedCSEBlock, StandardOutputError^ ErrorOutput);
};

#define PREPROC											Preprocessor::GetSingleton()