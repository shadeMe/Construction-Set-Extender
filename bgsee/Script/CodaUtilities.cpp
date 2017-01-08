#include "CodaUtilities.h"
#include "CodaAST.h"
#include "Console.h"

namespace bgsee
{
	namespace script
	{
		bool CodaScriptTokenizer::IsIndexStringLiteral(const CodaScriptSourceCodeT& Source, int Index)
		{
			if (Index >= Source.length())
				return false;

			int QuoteStack = 0;
			int Idx = 0;
			for (char Itr : Source)
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

		const CodaScriptSourceCodeT				CodaScriptTokenizer::kWhitespace = " \t";
		const CodaScriptSourceCodeT				CodaScriptTokenizer::kCommentDelimiter = ";";
		const CodaScriptSourceCodeT				CodaScriptTokenizer::kValidDelimiters = kCommentDelimiter + kWhitespace + ".,(){}[]\n";
		const UInt32							CodaScriptTokenizer::kCodaKeywordCount = 15 + 1;
		const CodaScriptSourceCodeT				CodaScriptTokenizer::kCodaKeywordArray[kCodaKeywordCount] =
		{
			"<UNKNOWN>",
			"VAR",
			"BEGIN",
			"END",
			"WHILE",
			"FOREACH",
			"LOOP",
			"IF",
			"ELSEIF",
			"ELSE",
			"ENDIF",
			"RETURN",
			"CALL",
			"CODA",								// syntax: CODA(<ScriptName> [, "PollingInterval"])
			"CONTINUE",
			"BREAK"
		};

		CodaScriptTokenizer::CodaScriptTokenizer()
		{
			;//
		}

		CodaScriptTokenizer::~CodaScriptTokenizer()
		{
			ResetState();
		}

		bool CodaScriptTokenizer::Tokenize(CodaScriptSourceCodeT Source, bool CollectEmptyTokens)
		{
			bool Result = false;
			int StartPos = -1, LastPos = -1;

			ResetState();
			SourceString = Source;
			Source += "\n";

			for (int i = 0; i < Source.length(); i++)
			{
				char Itr = Source[i];
				if (kWhitespace.find(Itr) == std::string::npos)
				{
					StartPos = i;
					break;
				}
			}

			if (StartPos != -1)
			{
				LastPos = StartPos;

				std::string TokenString = Source.substr(StartPos);
				std::string Token, Delimiter;

				for (int Index = TokenString.find_first_of(kValidDelimiters); Index != std::string::npos; Index = TokenString.find_first_of(kValidDelimiters))
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

					if (Delimiter != kCommentDelimiter)
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

		void CodaScriptTokenizer::ResetState()
		{
			Tokens.clear();
			Delimiters.clear();
			Indices.clear();
			SourceString.clear();
		}

		CodaScriptKeywordT CodaScriptTokenizer::GetKeywordType(CodaScriptSourceCodeT& Token)
		{
			CodaScriptKeywordT Result = kTokenType_Invalid;

			for (int i = 1; i < kCodaKeywordCount; i++)
			{
				if (!_stricmp(Token.c_str(), kCodaKeywordArray[i].c_str()))
				{
					Result = 1 << i;
					break;
				}
			}

			return Result;
		}

		void CodaScriptTokenizer::Sanitize(const CodaScriptSourceCodeT& In, CodaScriptSourceCodeT& Out, UInt32 OperationMask)
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

				if ((OperationMask & kSanitizeOps_StripComments))
				{
					int CommentDelimiter = Out.rfind(kCommentDelimiter);
					if (CommentDelimiter != std::string::npos && IsIndexStringLiteral(SourceString, CommentDelimiter) == false)
						Out.erase(CommentDelimiter, Out.length() - CommentDelimiter);
				}
			}
		}

		UInt32 CodaScriptTokenizer::GetParsedTokenCount() const
		{
			return Tokens.size();
		}

		CodaScriptKeywordT CodaScriptTokenizer::GetFirstTokenKeywordType()
		{
			if (GetParsedTokenCount())
				return GetKeywordType(Tokens[0]);
			else
				return kTokenType_Invalid;
		}

		const CodaScriptSourceCodeT& CodaScriptTokenizer::GetKeywordName(CodaScriptKeywordT Keyword)
		{
			for (int i = 1; i < kCodaKeywordCount; i++)
			{
				if (1 << i == Keyword)
					return kCodaKeywordArray[i];
			}

			return kCodaKeywordArray[0];
		}

		CodaScriptException::CodaScriptException(const ICodaScriptExecutableCode* Source, const char* Message, ...)
		{
			ZeroMemory(ErrorString, sizeof(ErrorString));
			char Buffer[0x512] = { 0 };

			va_list Args;
			va_start(Args, Message);
			vsprintf_s(Buffer, sizeof(Buffer), Message, Args);
			va_end(Args);

			if (Source)
				sprintf_s(ErrorString, sizeof(ErrorString), "Line[%d] Type[%s] - %s", Source->GetLine(), Source->GetTypeString(), Buffer);
			else
				sprintf_s(ErrorString, sizeof(ErrorString), "%s", Buffer);
		}

		CodaScriptException::CodaScriptException(const char* Message, ...)
		{
			ZeroMemory(ErrorString, sizeof(ErrorString));
			char Buffer[0x512] = { 0 };

			va_list Args;
			va_start(Args, Message);
			vsprintf_s(Buffer, sizeof(Buffer), Message, Args);
			va_end(Args);

			sprintf_s(ErrorString, sizeof(ErrorString), "%s", Buffer);
		}

		CodaScriptException::~CodaScriptException()
		{
			;//
		}

		const char* CodaScriptException::Get() const
		{
			return ErrorString;
		}

		CodaScriptProfiler::CodaScriptProfiler() :
			Counters()
		{
			;//
		}

		CodaScriptProfiler::~CodaScriptProfiler()
		{
			SME_ASSERT(Counters.size() == 0);
		}

		void CodaScriptProfiler::BeginProfiling(void)
		{
			Counters.push(CodaScriptElapsedTimeCounterT());
		}

		long double CodaScriptProfiler::EndProfiling(void)
		{
			SME_ASSERT(Counters.size());

			CodaScriptElapsedTimeCounterT& Current = Counters.top();
			Current.Update();

			long double ElapsedTime = Current.GetTimePassed();
			Counters.pop();

			return ElapsedTime;
		}

		CodaScriptMessageHandler::CodaScriptMessageHandler(const char* ConsoleContextName) :
			ConsoleContext(nullptr),
			DefaultContextLoggingState(true)
		{
			SME_ASSERT(ConsoleContextName && strlen(ConsoleContextName));
			ConsoleContext = BGSEECONSOLE->RegisterMessageLogContext(ConsoleContextName);
			SME_ASSERT(ConsoleContext);
		}

		CodaScriptMessageHandler::~CodaScriptMessageHandler()
		{
			BGSEECONSOLE->UnregisterMessageLogContext(ConsoleContext);
		}

		void CodaScriptMessageHandler::SuspendDefaultContextLogging(void)
		{
			SME_ASSERT(DefaultContextLoggingState == true);

			DefaultContextLoggingState = false;
		}

		void CodaScriptMessageHandler::ResumeDefaultContextLogging(void)
		{
			SME_ASSERT(DefaultContextLoggingState == false);

			DefaultContextLoggingState = true;
		}

		void CodaScriptMessageHandler::Log(const char* Format, ...)
		{
			va_list Args;
			char Buffer[0x5000] = { 0 };

			va_start(Args, Format);
			vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
			va_end(Args);

			BGSEECONSOLE->PrintToMessageLogContext(ConsoleContext, false, Buffer);
			if (DefaultContextLoggingState)
				BGSEECONSOLE_MESSAGE(Buffer);
		}

		void CodaScriptMessageHandler::Indent()
		{
			BGSEECONSOLE->Indent();
		}

		void CodaScriptMessageHandler::Outdent()
		{
			BGSEECONSOLE->Outdent();
		}

		CodaScriptCommandRegistrar::CodaScriptCommandRegistrar(const char* Category) :
			Commands(),
			Category(Category)
		{
			;//
		}

		CodaScriptCommandRegistrar::~CodaScriptCommandRegistrar()
		{
			Commands.clear();
		}

		void CodaScriptCommandRegistrar::Add(ICodaScriptCommand* Command)
		{
			Commands.push_back(Command);
		}

		const char* CodaScriptCommandRegistrar::GetCategory() const
		{
			return Category.c_str();
		}

		const ICodaScriptCommand::ListT& CodaScriptCommandRegistrar::GetCommands() const
		{
			return Commands;
		}

	}
}