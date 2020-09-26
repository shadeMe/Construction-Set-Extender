#pragma once
#include "Main.h"
#include "GenericModelessDialog.h"
#include "WindowSubclasser.h"

// Console - Editor console implementation

namespace bgsee
{
	struct ConsoleCommandInfo
	{
		typedef void				(* ConsoleCommandHandler)(UInt32 ParamCount, const char* Args);

		const char*					Name;
		UInt32						ParamCount;
		ConsoleCommandHandler	ExecuteHandler;
	};

#define BGSEECONSOLECMD_ARGS										UInt32 ParamCount, const char* Args
#define DEFINE_BGSEECONSOLECMD(name, paramcount)					\
		bgsee::ConsoleCommandInfo kBGSEEConsoleCmd_##name =			\
		{															\
			#name,													\
			##paramcount,											\
			BGSEEConsoleCmd_ ## name ## _ExecuteHandler				\
		}

	class Console;
	class ConsoleWarningManager;

	class ConsoleWarning
	{
	public:
		typedef UInt32									WarningCallSiteT;
	private:
		friend class ConsoleWarningManager;

		typedef std::vector<WarningCallSiteT>			CallSiteListT;

		UUID						BaseID;
		std::string					BaseIDString;
		std::string					Description;
		CallSiteListT				CallSites;
		bool						Enabled;
	public:
		ConsoleWarning(const char* GUID, const char* Desc, UInt32 CallSiteCount, ...);
		~ConsoleWarning();

		bool						GetEnabled(void) const;
	};

	class ConsoleWarningManager
	{
		friend class Console;

		static const char*					kINISection;

		static BOOL CALLBACK				GUIDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		typedef std::vector<ConsoleWarning*>		WarningListT;

		struct DlgUserData
		{
			ConsoleWarningManager*			Instance;
			LPARAM							UserData;
		};

		WarningListT						WarningDepot;
		INIManagerGetterFunctor				INIGetter;
		INIManagerSetterFunctor				INISetter;

		void								Clear();
		void								INISaveWarnings(void);
		void								INILoadWarnings(void);

		void								EnumerateWarningsInListView(HWND ListView) const;
		ConsoleWarning*						LookupWarning(const char* GUID) const;
		ConsoleWarning*						LookupWarning(ConsoleWarning::WarningCallSiteT CallSite) const;
	public:
		ConsoleWarningManager(INIManagerGetterFunctor Getter, INIManagerSetterFunctor Setter);
		~ConsoleWarningManager();

		void								RegisterWarning(ConsoleWarning* Warning);								// takes ownership of the pointer
		bool								GetWarningEnabled(ConsoleWarning::WarningCallSiteT CallSite) const;	// returns true if enabled, false otherwise

		void								ShowGUI(HINSTANCE ResourceInstance, HWND Parent);
	};

	class ConsoleWarningRegistrar
	{
	public:
		virtual ~ConsoleWarningRegistrar() = 0
		{
			;//
		}

		virtual void						operator()(ConsoleWarningManager* Manager) = 0;
	};

	class Console : public GenericModelessDialog
	{
	public:
		typedef void				(* ConsolePrintCallback)(const char* Prefix, const char* Message);
	protected:
		static LRESULT CALLBACK		BaseDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);
		static LRESULT CALLBACK		MessageLogSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, WindowExtraDataCollection* ExtraData);
		static LRESULT CALLBACK		CommandLineSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, WindowExtraDataCollection* ExtraData);

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
			static const UInt32		kMessageLogCharLimit = 1 * 1024 * 1024;
		protected:
			std::string				Name;
			std::string				LogPath;
			std::string				BackBuffer;
			UInt8					State;

			friend class			Console;

			void					CheckBufferLength(size_t AddendLength);		// clears the buffer preemptively if it's approaching its limit
		public:
			MessageLogContext(const char* ContextName, const char* ContextLogPath = nullptr);
			virtual ~MessageLogContext();

			enum
			{
				kState_Default		=	0,
				kState_Update,
				kState_Reset,
			};

			virtual void			Print(const char* Message, bool AddTimestamp);
			virtual void			Reset();

			UInt8					GetState() const;
			void					SetState(UInt8 NewState);
			void					OpenLog() const;
			bool					HasLog() const;
			const char*				GetName() const;
			const char*				GetBuffer() const;
			void					ClearBuffer();
		};

		class DefaultDebugLogContext : public MessageLogContext
		{
		protected:
			typedef std::vector<ConsolePrintCallback>	PrintCallbackArrayT;

			friend class			Console;

			Console*				Parent;
			FILE*					DebugLog;
			UInt32					IndentLevel;
			PrintCallbackArrayT		PrintCallbacks;
			bool					ExecutingCallbacks;

			void					ExecutePrintCallbacks(const char* Prefix, const char* Message);
			bool					LookupPrintCallback(ConsolePrintCallback Callback, PrintCallbackArrayT::iterator& Match);

			bool					Open(const char* Path);
			void					Close();

			void					Put(const char* String);
			void					Flush();
		public:
			DefaultDebugLogContext(Console* Parent, const char* DebugLogPath);
			virtual ~DefaultDebugLogContext();

			virtual void			Print(const char* Prefix, const char* Message);
			virtual void			Reset();

			UInt32					Indent();
			UInt32					Outdent();
			void					OutdentAll();
			void					Pad(UInt32 Count);

			bool					RegisterPrintCallback(ConsolePrintCallback Callback);
			void					UnregisterPrintCallback(ConsolePrintCallback Callback);
		};

		class ConsoleCommandTable
		{
			typedef std::vector<ConsoleCommandInfo*>		ConsoleCommandArrayT;

			ConsoleCommandArrayT					CommandList;

			bool								LookupCommandByName(const char* Name, ConsoleCommandArrayT::iterator& Match);
			bool								LookupCommandByInstance(ConsoleCommandInfo* Command, ConsoleCommandArrayT::iterator& Match);
		public:
			ConsoleCommandTable();
			~ConsoleCommandTable();

			bool								AddCommand(ConsoleCommandInfo* Command);
			void								RemoveCommand(ConsoleCommandInfo* Command);
			ConsoleCommandInfo*					GetCommand(const char* Name);
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

		typedef std::vector<MessageLogContext*>	ContextArrayT;
		typedef std::stack<std::string>			CommandHistoryStackT;

		friend class				DefaultDebugLogContext;
		friend struct				UIExtraData;

		DWORD						OwnerThreadID;
		MessageLogContext*			ActiveContext;
		DefaultDebugLogContext*		PrimaryContext;
		ContextArrayT				SecondaryContexts;
		ConsoleCommandTable			CommandTable;
		CommandHistoryStackT		CommandLineHistory;
		CommandHistoryStackT		CommandLineHistoryAuxiliary;
		ConsoleWarningManager*		WarningManager;

		void						ClearMessageLog(void);
		void						SetTitle(const char* Prefix);

		MessageLogContext*			GetActiveContext(void) const;
		void						SetActiveContext(MessageLogContext* Context);
		void						ResetActiveContext(void);

		void						ExecuteCommand(const char* CommandExpression);

		bool						LookupSecondaryContextByName(const char* Name, ContextArrayT::iterator& Match);
		bool						LookupSecondaryContextByInstance(MessageLogContext* Context, ContextArrayT::iterator& Match);
		void						ReleaseSecondaryContexts(void);
	private:
		static Console*				Singleton;

		Console(const char* LogPath);
		virtual ~Console();
	public:
		static const UInt32			kMaxIndentLevel = 0x10;


		virtual void				InitializeUI(HWND Parent, HINSTANCE Resource);
		virtual void				InitializeWarningManager(INIManagerGetterFunctor Getter,
															INIManagerSetterFunctor Setter,
															ConsoleWarningRegistrar& Registrar);

		virtual void				LogMsg(std::string Prefix, const char* Format, ...);
		virtual void				LogMsg(const char* Prefix, const char* Format, ...);
		virtual void				LogWindowsError(std::string Prefix, const char* Format, ...);
		virtual void				LogWarning(std::string Prefix, const char* Format, ...);
		virtual void				LogAssertion(std::string Prefix, const char* Format, ...);

		UInt32						Indent();
		UInt32						Outdent();
		void						OutdentAll();
		void						Pad(UInt32 Count);

		void*						RegisterMessageLogContext(const char* Name, const char* LogPath = nullptr);	// returns the context object
		void						UnregisterMessageLogContext(void* Context);									// destroys the pointer
		void						PrintToMessageLogContext(void* Context, bool HideTimestamp, const char* Format, ...);

		bool						RegisterConsoleCommand(ConsoleCommandInfo* Command);
		void						UnregisterConsoleCommand(ConsoleCommandInfo* Command);

		bool						RegisterPrintCallback(ConsolePrintCallback Callback);
		void						UnregisterPrintCallback(ConsolePrintCallback Callback);

		const char*					GetLogPath(void) const;
		void						OpenDebugLog(void);
		void						FlushDebugLog(void);

		bool						GetLogsWarnings(void);
		void						ToggleWarningLogging(bool State);
		ConsoleWarningManager*		GetWarningManager(void) const;

		static void					RegisterINISettings(INISettingDepotT& Depot);

		static Console*				Get();
		static bool					Initialize(const char* LogPath);
		static void					Deinitialize();
	};
}