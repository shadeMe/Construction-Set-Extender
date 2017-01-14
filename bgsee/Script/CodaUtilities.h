#pragma once
#include "CodaForwardDecls.inl"
#include "CodaBaseInterfaces.inl"

namespace bgsee
{
	namespace script
	{
		typedef SME::MiscGunk::ElapsedTimeCounter			CodaScriptElapsedTimeCounterT;

		class CodaScriptTokenizer
		{
		public:
			typedef std::vector<CodaScriptSourceCodeT>		ParsedTokenListT;
			typedef std::vector<UInt32>						ParsedTokenOffsetListT;
			typedef std::vector<char>						ParsedDelimiterListT;

			ParsedTokenListT								Tokens;
			ParsedTokenOffsetListT							Indices;		// the position of each token relative to its parent line
			ParsedDelimiterListT							Delimiters;

			enum
			{
				kTokenType_Invalid							= 1 << 0,
				kTokenType_Variable							= 1 << 1,
				kTokenType_Begin							= 1 << 2,
				kTokenType_End								= 1 << 3,
				kTokenType_While							= 1 << 4,
				kTokenType_ForEach							= 1 << 5,
				kTokenType_Loop								= 1 << 6,
				kTokenType_If								= 1 << 7,
				kTokenType_ElseIf							= 1 << 8,
				kTokenType_Else								= 1 << 9,
				kTokenType_EndIf							= 1 << 10,
				kTokenType_Return							= 1 << 11,
				kTokenType_Call								= 1 << 12,
				kTokenType_ScriptName						= 1 << 13,
				kTokenType_Continue							= 1 << 14,
				kTokenType_Break							= 1 << 15,
			};

			enum
			{
				kSanitizeOps_StripLeadingWhitespace			= 1 << 0,
				kSanitizeOps_StripTabCharacters				= 1 << 1,
				kSanitizeOps_StripComments					= 1 << 2
			};

			CodaScriptTokenizer();
			~CodaScriptTokenizer();

			bool											Tokenize(CodaScriptSourceCodeT Source, bool CollectEmptyTokens = false);		// returns true if at least one token was parsed, skips comments
			void											ResetState(void);

			UInt32											GetParsedTokenCount() const;
			CodaScriptKeywordT								GetFirstTokenKeywordType();

			void											Sanitize(const CodaScriptSourceCodeT& In, CodaScriptSourceCodeT& Out, UInt32 OperationMask);

			static CodaScriptKeywordT						GetKeywordType(CodaScriptSourceCodeT& Token);
			static const CodaScriptSourceCodeT&				GetKeywordName(CodaScriptKeywordT Keyword);
		private:
			CodaScriptSourceCodeT							SourceString;

			bool											IsIndexStringLiteral(const CodaScriptSourceCodeT& Source, int Index);		// returns true if the index is inside a string literal

			static const CodaScriptSourceCodeT				kWhitespace;
			static const CodaScriptSourceCodeT				kValidDelimiters;
			static const CodaScriptSourceCodeT				kCommentDelimiter;
			static const UInt32								kCodaKeywordCount;
			static const CodaScriptSourceCodeT				kCodaKeywordArray[];
		};

		class CodaScriptException
		{
		protected:
			char											ErrorString[0x768];
		public:
			CodaScriptException(const ICodaScriptExecutableCode* Source, const char* Message, ...);
			CodaScriptException(const char* Message, ...);
			virtual ~CodaScriptException();

			virtual const char*								Get() const;
		};

		struct ScopedFunctor
		{
			enum class Event
			{
				Construction, Destruction
			};

			typedef std::function<void(Event)>		FunctorT;

			ScopedFunctor(FunctorT Functor) : Functor(Functor) { this->Functor(Event::Construction); }
			~ScopedFunctor() { this->Functor(Event::Destruction); }
		private:
			FunctorT		Functor;
		};

		class CodaScriptProfiler
		{
			typedef std::stack<CodaScriptElapsedTimeCounterT>		TimeCounterStackT;

			TimeCounterStackT		Counters;
		public:
			CodaScriptProfiler();
			~CodaScriptProfiler();

			void					BeginProfiling(void);
			long double				EndProfiling(void);
		};

		class CodaScriptMessageHandler
		{
			bool			DefaultContextLoggingState;
			void*			ConsoleContext;
			char			Buffer[0x5000];
		public:
			CodaScriptMessageHandler(const char* ConsoleContextName);
			~CodaScriptMessageHandler();

			void			SuspendDefaultContextLogging(void);
			void			ResumeDefaultContextLogging(void);

			void			Log(const char* Format, ...);
			void			Indent();
			void			Outdent();

			typedef std::unique_ptr<CodaScriptMessageHandler>		PtrT;
		};

		class CodaScriptCommandRegistrar
		{
			ICodaScriptCommand::ListT		Commands;
			std::string						Category;
		public:
			CodaScriptCommandRegistrar(const char* Category);
			~CodaScriptCommandRegistrar();

			void								Add(ICodaScriptCommand* Command);
			const char*							GetCategory() const;
			const ICodaScriptCommand::ListT&	GetCommands() const;

			typedef std::list<CodaScriptCommandRegistrar*>		ListT;
		};

		template<typename T>
		class CodaScriptSimpleInstanceCounter
		{
			const int&	BaseGIC;
			int		InitGIC;
		public:
			CodaScriptSimpleInstanceCounter() : BaseGIC(T::GetGIC()), InitGIC(BaseGIC) {}

			int GetCount(void) const { return BaseGIC - InitGIC; }
		};
	}
}