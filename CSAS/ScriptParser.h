#pragma once

namespace CSAutomationScript
{
	class ScriptParser
	{
		static std::string								Whitespace;
		static std::string								ValidDelimiters;
	public:
		typedef std::vector<std::string>				TokenList;
		typedef std::vector<UInt32>						IndexList;
		typedef std::vector<char>						DelimiterList;

		TokenList										Tokens;
		IndexList										Indices;	// the position of each token relative to its parent line
		DelimiterList									Delimiters;

		enum TokenType
		{
			kTokenType_Invalid				= 1 << 0,
			kTokenType_Variable				= 1 << 1,
			kTokenType_Begin				= 1 << 2,
			kTokenType_End					= 1 << 3,
			kTokenType_While				= 1 << 4,
			kTokenType_ForEach				= 1 << 5,
			kTokenType_Loop					= 1 << 6,
			kTokenType_If					= 1 << 7,
			kTokenType_ElseIf				= 1 << 8,
			kTokenType_Else					= 1 << 9,
			kTokenType_EndIf				= 1 << 10,
			kTokenType_Return				= 1 << 11,
			kTokenType_Call					= 1 << 12,
			kTokenType_ScriptName			= 1 << 13,
			kTokenType_Continue				= 1 << 14,
			kTokenType_Break				= 1 << 15,
		};

		enum
		{
			kSanitizeOps_StripLeadingWhitespace		= 1 << 0,
			kSanitizeOps_StripTabCharacters			= 1 << 1,
			kSanitizeOps_StripComments				= 1 << 2
		};

		~ScriptParser() { Reset(); }

		bool											Tokenize(std::string Source, bool CollectEmptyTokens);		// skips comments
		TokenType										GetTokenType(std::string& Token);
		void											Reset();
		UInt32											GetTokenCount() { return Tokens.size(); }
		TokenType										GetFirstTokenType() { return GetTokenType(Tokens[0]); }
		void											Sanitize(std::string& In, std::string& Out, UInt32 OperationMask);
	};

	typedef ScriptParser::TokenType						CmdToken;
}