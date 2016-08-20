#pragma once
#include "..\Main.h"
#include "..\Console.h"
#include "..\WorkspaceManager.h"
#include "CodaDataTypes.h"

namespace bgsee
{
	namespace script
	{
		class CodaScriptCommandRegistry
		{
			friend class CodaScriptVM;

			typedef std::multimap<std::string, ICodaScriptCommand*>		CommandTableMapT;

			CommandTableMapT							Registry;
			std::string									BaseWikiURL;

			void										AppendToStream(std::fstream& Out, const char* Fmt, ...);
			std::string									SanitizeLinkableString(const char* String);
			const char*									StringifyParameterType(UInt8 ParamType);

			void										InitializeExpressionParser(ICodaScriptExpressionParser* Parser);
		public:
			CodaScriptCommandRegistry(const char* WikiURL);
			~CodaScriptCommandRegistry();

			ICodaScriptCommand*							LookupCommand(const char* Name, bool UseAlias = false);
			bool										RegisterCommand(const char* Category, ICodaScriptCommand* Command);		// caller retains ownership of the function prototype
			void										RegisterCommands(const CodaScriptRegistrarListT& Registrars);

			void										Dump(std::string OutPath);
		};

		class CodaScriptCommandRegistrar
		{
			friend class CodaScriptCommandRegistry;
			typedef std::list<ICodaScriptCommand*>		CommandListT;

			CommandListT								Commands;
			std::string									Category;
		public:
			CodaScriptCommandRegistrar(const char* Category);
			~CodaScriptCommandRegistrar();

			void										Add(ICodaScriptCommand* Command);
		};

		class CodaScriptProfiler
		{
			typedef std::stack<CodaScriptElapsedTimeCounterT>		TimeCounterStackT;

			TimeCounterStackT							Counters;
		public:
			CodaScriptProfiler();
			~CodaScriptProfiler();

			void										BeginProfiling(void);
			long double									EndProfiling(void);
		};

		class CodaScriptMessageHandler
		{
			bool										DefaultContextLoggingState;
			void*										ConsoleContext;
		public:
			CodaScriptMessageHandler();
			~CodaScriptMessageHandler();

			void										SuspendDefaultContextLogging(void);
			void										ResumeDefaultContextLogging(void);

			void										Log(const char* Format, ...);
		};

		class CodaScriptExecutive
		{
			static const UInt32							kMaxRecursionLimit;
			static INISetting							kINI_Profiling;

			CodaScriptExecutionContextStackT			ExecutionStack;
			CodaScriptProfiler							Profiler;
			DWORD										OwnerThreadID;
			CodaScriptMessageHandler*					MessageHandler;
		public:
			CodaScriptExecutive(CodaScriptMessageHandler* MsgHdlr);
			~CodaScriptExecutive();

			bool										Execute(CodaScriptExecutionContext* Context, CodaScriptBackingStore* Result, bool& ReturnedResult);
			CodaScriptExecutionContext*					GetExecutingContext(void);

			static void									RegisterINISettings(INISettingDepotT& Depot);
		};

		class CodaScriptBackgrounder
		{
			friend class CodaScriptGlobalDataStore;
			friend class CodaScriptVM;

			static const std::string					kDepotName;
			static INISetting							kINI_Enabled;
			static INISetting							kINI_UpdatePeriod;

			static VOID CALLBACK						CallbackProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			typedef std::vector<CodaScriptExecutionContext*>	CodaScriptBackgroundExecutionCacheT;

			ResourceLocation							SourceDepot;
			CodaScriptBackgroundExecutionCacheT			DepotCache;		// stores the contexts for scripts in the depot
			CodaScriptBackgroundExecutionCacheT			RuntimeCache;	// stores the contexts for regular scripts executing in the background
			bool										State;
			bool										Backgrounding;
			UINT_PTR									TimerID;
			CodaScriptElapsedTimeCounterT				PollingTimeCounter;

			INIManagerGetterFunctor						INISettingGetter;
			INIManagerSetterFunctor						INISettingSetter;

			void										ResetCache(CodaScriptBackgroundExecutionCacheT& Cache, bool Renew = false);
			void										ResetTimer(bool Renew = false);

			void										Execute(CodaScriptBackgroundExecutionCacheT& Cache, CodaScriptExecutive* Executive, double TimePassed);
			void										Tick(CodaScriptExecutive* Executive);
		public:
			CodaScriptBackgrounder(ResourceLocation Source, INIManagerGetterFunctor Getter, INIManagerSetterFunctor Setter);
			~CodaScriptBackgrounder();

			void										Suspend(void);
			void										Resume(void);
			bool										GetState(void) const;		// returns true if enabled, false otherwise

			void										Rebuild(void);				// renews the depot cache

			void										Queue(CodaScriptExecutionContext* Context);

			static void									RegisterINISettings(INISettingDepotT& Depot);
		};

		class CodaScriptGlobalDataStore
		{
			friend class CodaScriptVM;

			static BOOL CALLBACK						EditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			CodaScriptVariableArrayT						Cache;
			INIManagerGetterFunctor						INISettingGetter;
			INIManagerSetterFunctor						INISettingSetter;

			bool										Add(CodaScriptVariable* Variable);
			template<typename T>
			CodaScriptVariable*							Add(const char* Name, T Value, bool& ExistingVar);
			CodaScriptVariable*							Add(const char* Name, CodaScriptStringParameterTypeT Value, bool& ExistingVar);
			CodaScriptVariable*							Add(const char* Name, CodaScriptNumericDataTypeT Value, bool& ExistingVar);
			CodaScriptVariable*							Add(const char* Name, CodaScriptReferenceDataTypeT Value, bool& ExistingVar);

			void										Remove(CodaScriptVariable* Variable);
			CodaScriptVariable*							Lookup(const char* Name);
			bool										Lookup(CodaScriptVariable* Variable, CodaScriptVariableArrayT::iterator& Match);
			void										Clear(void);

			void										INILoadState(void);
			void										INISaveState(void);
		public:
			CodaScriptGlobalDataStore(INIManagerGetterFunctor Getter, INIManagerSetterFunctor Setter);
			~CodaScriptGlobalDataStore();

			void										ShowEditDialog(HINSTANCE ResourceInstance, HWND Parent);
			CodaScriptVariableArrayT&					GetCache(void);
		};

		// A virtual machine? Hah! Fat chance!
		class CodaScriptVM
		{
			friend class								CodaScriptBackgrounder;
			friend class								CodaScriptGlobalDataStore;

			static ConsoleCommandInfo					kDumpCodaDocsConsoleCommandData;
			static void									DumpCodaDocsConsoleCommandHandler(UInt32 ParamCount, const char* Args);

			static CodaScriptVM*						Singleton;

			ResourceLocation							BaseDirectory;
			CodaScriptCommandRegistry*					CommandRegistry;
			CodaScriptMessageHandler*					MessageHandler;
			CodaScriptExecutive*						Executive;
			CodaScriptBackgrounder*						Backgrounder;
			CodaScriptGlobalDataStore*					GlobalStore;
			ICodaScriptExpressionParser*				ExpressionParser;

			bool										Initialized;

			CodaScriptVM();

			CodaScriptExecutionContext*					CreateExecutionContext(std::fstream& Input, CodaScriptMutableDataArrayT* Parameters = nullptr);
		public:
			~CodaScriptVM();

			static const std::string					kSourceExtension;
			static CodaScriptVM*						GetSingleton();

			bool										Initialize(ResourceLocation BasePath,
																const char* WikiURL,
																INIManagerGetterFunctor INIGetter,
																INIManagerSetterFunctor INISetter,
																CodaScriptRegistrarListT& ScriptCommands);

			bool										RunScript(std::string ScriptName,					// script name's the same as filename
																CodaScriptMutableDataArrayT* Parameters,
																CodaScriptBackingStore* Result,
																bool& ReturnedResult,
																bool RunInBackground = false);

			void										ShowGlobalStoreEditDialog(HINSTANCE ResourceInstance, HWND Parent);
			CodaScriptVariable*							GetGlobal(const char* Name);
			CodaScriptVariableArrayT&					GetGlobals(void) const;

			bool										GetBackgrounderState(void) const;
			bool										ToggleBackgrounderState(void);

			CodaScriptMessageHandler*					GetMessageHandler(void);

			void										OpenScriptRepository(void) const;
		};
#define CODAVM											bgsee::script::CodaScriptVM::GetSingleton()

		class CodaScriptObjectFactory
		{
		public:
			enum
			{
				kFactoryType_Invalid	= 0,

				kFactoryType_MUP,

				kFactoryType__MAX
			};

			static ICodaScriptExpressionParser*			BuildExpressionParser(UInt8 Type);
			static ICodaScriptDataStoreOwner*			BuildDataStoreOwner(UInt8 Type);
			static CodaScriptSharedHandleArrayT			BuildArray(UInt8 Type, UInt32 InitialSize = 0);
		};
	}
}