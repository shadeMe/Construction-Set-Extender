#include "ScriptParser.h"
#include "ScriptVariables.h"

namespace CSAutomationScript
{
	std::string								ScriptParser::Whitespace = " \t";
	std::string								ScriptParser::ValidDelimiters = ";., (){}[]\t\n";

	bool ScriptParser::Tokenize(std::string Source, bool CollectEmptyTokens)
	{
		Source += "\n";

		bool Result = false;
		int StartPos = -1, LastPos = -1;

		Reset();

		for (int i = 0; i < Source.length(); i++)
		{
			char Itr = Source[i];
			if (Whitespace.find(Itr) == std::string::npos)
			{
				StartPos =i;
				break;
			}
		}

		if (StartPos != -1)
		{
			LastPos = StartPos;

			std::string TokenString = Source.substr(StartPos);
			std::string Token, Delimiter;

			for (int Index = TokenString.find_first_of(ValidDelimiters); Index != std::string::npos; Index = TokenString.find_first_of(ValidDelimiters))
			{
				if (TokenString[0] == '\"')
				{
					if (TokenString.find('\"', 1) != std::string::npos)
						Index = TokenString.find('\"', 1) + 1;
					else
						Index = TokenString.length() - 1;
				}

				Token = TokenString.substr(0, Index);
				Delimiter = TokenString.substr(Index, 1);

				if (Delimiter != ";")
				{
					TokenString = TokenString.erase(0, Index + 1);

					if (Token == "" && !CollectEmptyTokens)
					{
						LastPos++;
						continue;
					}

					Tokens.push_back(Token);
					Indices.push_back(LastPos);
					Delimiters.push_back(Delimiter[0]);
					LastPos += Token.length() + 1;
				}
				else
				{
					if (Token != "")
					{
						Tokens.push_back(Token);
						Indices.push_back(LastPos);
						Delimiters.push_back(Delimiter[0]);
					}
					break;
				}
			}

			if (Tokens.size() > 0)
				Result = true;
		}

		return Result;
	}

	void ScriptParser::Reset()
	{
		Tokens.clear();
		Delimiters.clear();
		Indices.clear();
	}

	ScriptParser::TokenType ScriptParser::GetTokenType(std::string& Token)
	{
		TokenType Result = kTokenType_Invalid;

		if (!_stricmp(Token.c_str(), "var"))
			Result = kTokenType_Variable;
		else if (!_stricmp(Token.c_str(), "begin"))
			Result = kTokenType_Begin;
		else if (!_stricmp(Token.c_str(), "end"))
			Result = kTokenType_End;
		else if (!_stricmp(Token.c_str(), "while"))
			Result = kTokenType_While;
		else if (!_stricmp(Token.c_str(), "foreach"))
			Result = kTokenType_ForEach;
		else if (!_stricmp(Token.c_str(), "loop"))
			Result = kTokenType_Loop;
		else if (!_stricmp(Token.c_str(), "if"))
			Result = kTokenType_If;
		else if (!_stricmp(Token.c_str(), "elseif"))
			Result = kTokenType_ElseIf;
		else if (!_stricmp(Token.c_str(), "else"))
			Result = kTokenType_Else;
		else if (!_stricmp(Token.c_str(), "endif"))
			Result = kTokenType_EndIf;
		else if (!_stricmp(Token.c_str(), "return"))
			Result = kTokenType_Return;
		else if (!_stricmp(Token.c_str(), "call"))
			Result = kTokenType_Call;
		else if (!_stricmp(Token.c_str(), "scn") || !_stricmp(Token.c_str(), "scriptname"))
			Result = kTokenType_ScriptName;
		else if (!_stricmp(Token.c_str(), "continue"))
			Result = kTokenType_Continue;
		else if (!_stricmp(Token.c_str(), "break"))
			Result = kTokenType_Break;

		return Result;
	}

	void ScriptParser::Sanitize(std::string& In, std::string& Out, UInt32 OperationMask)
	{
		Out.clear();

		if (Tokenize(In, false))
		{
			if ((OperationMask & kSanitizeOps_StripLeadingWhitespace))
				Out = In.substr(Indices[0]);
			else
				Out = In;

			if ((OperationMask & kSanitizeOps_StripTabCharacters))
				std::replace(Out.begin(), Out.end(), '\t', ' ');

			int CommentDelimiter = Out.find(";");
			if ((OperationMask & kSanitizeOps_StripComments) && CommentDelimiter != std::string::npos)
			{
				Out.erase(CommentDelimiter, Out.length() - CommentDelimiter);
			}
		}
	}
}