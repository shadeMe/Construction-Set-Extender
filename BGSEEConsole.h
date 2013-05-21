#pragma once
#include "BGSEEMain.h"
#include "BGSEEUIManager.h"

// BGSEEConsole - Editor console implementation

namespace BGSEditorExtender
{
	struct BGSEEConsoleCommandInfo
	{
		typedef void				(* BGSEEConsoleCommandHandler)(UInt32 ParamCount, const char* Args);

		const char*					Name;
		UInt32						ParamCount;
		BGSEEConsoleCommandHandler	ExecuteHandler;
	};

#define BGSEECONSOLECMD_ARGS										UInt32 ParamCount, const char* Args
#define DEFINE_BGSEECONSOLECMD(name, paramcount)					\
		BGSEditorExtender::BGSEEConsoleCommandInfo kBGSEEConsoleCmd_##name =			\
		{															\
			#name,													\
			##paramcount,											\
			BGSEEConsoleCmd_ ## name ## _ExecuteHandler				\
		}

	class BGSEEConsole : public BGSEEGenericModelessDialog
	{
	public:
		typedef void				(* BGSEEConsolePrintCallback)(const char* Prefix, const char* Message);
	protected:
		static LRESULT CALLBACK		BaseDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);
		static LRESULT CALLBACK		MessageLogSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK		CommandLineSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static INISetting			kINI_Top;
		static INISetting			kINI_Left;
		static INISetting			kINI_Right;
		static INISetting			kINI_Bottom;
		static INISetting			kINI_Visible;

		static INISetting			kINI_FontFace;
		static INISetting			kINI_FontSize;
		static INISetting			kINI_UpdatePeriod;
		static INISetting			kINI_LogWarnings;
		static INISetting			kINI_LogAssertions;
		static INISetting			kINI_LogTimestamps;

		static const char*			kCommandLinePrefix;
		static const char*			kWindowTitle;

		class MessageLogContext
		{
		protected:
			std::string				Name;
			std::string				LogPath;
			std::string				BackBuffer;
			UInt8					State;

			friend class			BGSEEConsole;
		public:
			MessageLogContext(const char* ContextName, const char* ContextLogPath = NULL);
			virtual ~MessageLogContext();

			enum
			{
				kState_Default		=	0,
				kState_Update,
				kState_Reset,
			};

			virtual void			Print(const char* Message);
			virtual void			Reset();

			UInt8					GetState() const;
			void					SetState(UInt8 NewState);
			void					OpenLog() const;
			const char*				GetName() const;
			const char*				GetBuffer() const;
			void					ClearBuffer();
		};

		class DefaultDebugLogContext : public MessageLogContext
		{
		protected:
			typedef std::list<BGSEEConsolePrintCallback>	PrintCallbackListT;

			friend class			BGSEEConsole;

			BGSEEConsole*			Parent;
			FILE*					DebugLog;
			UInt32					IndentLevel;
			PrintCallbackListT		PrintCallbacks;
			bool					ExecutingCallbacks;

			void					ExecutePrintCallbacks(const char* Prefix, const char* Message);
			bool					LookupPrintCallback(BGSEEConsolePrintCallback Callback, PrintCallbackListT::iterator& Match);

			bool					Open(const char* Path);
			void					Close();

			void					Put(const char* String);
			void					Flush();
		public:
			DefaultDebugLogContext(BGSEEConsole* Parent, const char* DebugLogPath);
			virtual ~DefaultDebugLogContext();

			virtual void			Print(const char* Prefix, const char* Message);
			virtual void			Reset();

			UInt32					Indent();
			UInt32					Exdent();
			void					ExdentAll();
			void					Pad(UInt32 Count);

			bool					RegisterPrintCallback(BGSEEConsolePrintCallback Callback);
			void					UnregisterPrintCallback(BGSEEConsolePrintCallback Callback);
		};

		class ConsoleCommandTable
		{
			typedef std::list<BGSEEConsoleCommandInfo*>		ConsoleCommandListT;

			ConsoleCommandListT					CommandList;

			bool								LookupCommandByName(const char* Name, ConsoleCommandListT::iterator& Match);
			bool								LookupCommandByInstance(BGSEEConsoleCommandInfo* Command, ConsoleCommandListT::iterator& Match);
		public:
			ConsoleCommandTable();
			~ConsoleCommandTable();

			bool								AddCommand(BGSEEConsoleCommandInfo* Command);
			void								RemoveCommand(BGSEEConsoleCommandInfo* Command);
			BGSEEConsoleCommandInfo*			GetCommand(const char* Name);
		};

		struct UIExtraData
		{
			MessageLogContext*					SelectedContext;
			HFONT								MessageLogFont;
			HFONT								CommandLineFont;
			
			RECT								DialogInitBounds;
			RECT								MessageLogInitBounds;
			RECT								CommandLineInitBounds;

			UIExtraData();
			~UIExtraData();
		};

		typedef std::list<MessageLogContext*>	ContextListT;
		typedef std::stack<std::string>			CommandHistoryStackT;

		friend class							DefaultDebugLogContext;
		friend struct							UIExtraData;

		DWORD									OwnerThreadID;
		MessageLogContext*						ActiveContext;
		DefaultDebugLogContext*					PrimaryContext;
		ContextListT							SecondaryContexts;
		ConsoleCommandTable						CommandTable;
		CommandHistoryStackT					CommandLineHistory;
		CommandHistoryStackT					CommandLineHistoryAuxiliary;

		void						ClearMessageLog(void);
		void						SetTitle(const char* Prefix);

		MessageLogContext*			GetActiveContext(void) const;
		void						SetActiveContext(MessageLogContext* Context);
		void						ResetActiveContext(void);

		void						ExecuteCommand(const char* CommandExpression);

		bool						LookupSecondaryContextByName(const char* Name, ContextListT::iterator& Match);
		bool						LookupSecondaryContextByInstance(MessageLogContext* Context, ContextListT::iterator& Match);
		void						ReleaseSecondaryContexts(void);
	public:
		static const UInt32			kMessageLogCharLimit = 0x8000;
		static const UInt32			kMaxIndentLevel = 0x10;

		BGSEEConsole(const char* LogPath);
		virtual ~BGSEEConsole();

		virtual void				InitializeUI(HWND Parent, HINSTANCE Resource);

		virtual void				LogMsg(std::string Prefix, const char* Format, ...);
		virtual void				LogErrorMsg(std::string Prefix, const char* Format, ...);
		virtual void				LogWarning(std::string Prefix, const char* Format, ...);
		virtual void				LogAssertion(std::string Prefix, const char* Format, ...);

		UInt32						Indent();
		UInt32						Exdent();
		void						ExdentAll();
		void						Pad(UInt32 Count);

		void*						RegisterMessageLogContext(const char* Name, const char* LogPath = NULL);	// returns the context object
		void						UnregisterMessageLogContext(void* Context);									// destroys the pointer
		void						PrintToMessageLogContext(void* Context, const char* Format, ...);

		bool						RegisterConsoleCommand(BGSEEConsoleCommandInfo* Command);
		void						UnregisterConsoleCommand(BGSEEConsoleCommandInfo* Command);

		bool						RegisterPrintCallback(BGSEEConsolePrintCallback Callback);
		void						UnregisterPrintCallback(BGSEEConsolePrintCallback Callback);

		const char*					GetLogPath(void) const;
		void						OpenDebugLog(void);
		void						FlushDebugLog(void);

		bool						GetLogsWarnings(void);

		static void					RegisterINISettings(INISettingDepotT& Depot);
	};
}