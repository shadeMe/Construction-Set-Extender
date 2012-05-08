#pragma once
#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "BGSEEWorkspaceManager.h"
#include "CodaDataTypes.h"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		class CodaScriptCommandRegistry : public ICodaScriptObject
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

		class CodaScriptCommandRegistrar : public ICodaScriptObject
		{
			friend class CodaScriptCommandRegistry;
		protected:
			typedef std::list<ICodaScriptCommand*>		CommandListT;

			CommandListT								Commands;
			std::string									Category;
		public:
			CodaScriptCommandRegistrar(const char* Category);
			~CodaScriptCommandRegistrar();

			void										Add(ICodaScriptCommand* Command);
		};

		class CodaScriptProfiler : public ICodaScriptObject
		{
			typedef std::stack<CodaScriptElapsedTimeCounterT>		TimeCounterStackT;

			TimeCounterStackT							Counters;
		public:
			CodaScriptProfiler();
			~CodaScriptProfiler();

			void										BeginProfiling(void);
			long double									EndProfiling(void);
		};

		class CodaScriptMessageHandler : public ICodaScriptObject
		{
			bool										State;
		public:
			CodaScriptMessageHandler();
			~CodaScriptMessageHandler();

			void										Suspend(void);
			void										Resume(void);

			void										LogMsg(const char* Format, ...);
		};

		class CodaScriptExecutive : public ICodaScriptObject
		{
			static const UInt32							kMaxRecursionLimit;

			CodaScriptExecutionContextStackT			ExecutionStack;
			CodaScriptProfiler							Profiler;
			DWORD										OwnerThreadID;
			CodaScriptMessageHandler*					MessageHandler;
		public:
			CodaScriptExecutive(CodaScriptMessageHandler* MsgHdlr);
			~CodaScriptExecutive();

			bool										Execute(CodaScriptExecutionContext* Context, CodaScriptBackingStore* Result, bool& ReturnedResult);
			CodaScriptExecutionContext*					GetExecutingContext(void);
		};

		class CodaScriptBackgrounder : public ICodaScriptObject
		{
			friend class CodaScriptGlobalDataStore;
			friend class CodaScriptVM;

			static const char*											kINISection;
			static const std::string									kDepotName;
			static const BGSEEINIManagerSettingFactory::SettingData		kINISettings[2];
			enum
			{
				kBackgrounderINISetting_Enabled				= 0,
				kBackgrounderINISetting_UpdatePeriod,
			};
			static VOID CALLBACK						CallbackProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			typedef std::list<CodaScriptExecutionContext*>	CodaScriptBackgroundExecutionCacheT;

			BGSEEResourceLocation						SourceDepot;
			CodaScriptBackgroundExecutionCacheT			BackgroundCache;
			bool										State;
			bool										Backgrounding;
			UINT_PTR									TimerID;
			CodaScriptElapsedTimeCounterT				PollingTimeCounter;

			BGSEEINIManagerGetterFunctor				INISettingGetter;
			BGSEEINIManagerSetterFunctor				INISettingSetter;

			void										ResetCache(bool Renew = false);
			void										ResetTimer(bool Renew = false);

			void										Execute(CodaScriptExecutive* Executive);

		public:
			CodaScriptBackgrounder(BGSEEResourceLocation Source, BGSEEINIManagerGetterFunctor Getter, BGSEEINIManagerSetterFunctor Setter);
			~CodaScriptBackgrounder();

			void										Suspend(void);
			void										Resume(void);
			bool										GetState(void) const;

			void										Rebuild(void);		// renews the cache

			static BGSEEINIManagerSettingFactory*		GetINIFactory(void);
		};

		class CodaScriptGlobalDataStore : public ICodaScriptObject
		{
			friend class CodaScriptVM;

			static const char*							kINISection;

			static BOOL CALLBACK						EditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			CodaScriptVariableListT						Cache;
			BGSEEINIManagerGetterFunctor				INISettingGetter;
			BGSEEINIManagerSetterFunctor				INISettingSetter;

			bool										Add(CodaScriptVariable* Variable);
			CodaScriptVariable*							Add(const char* Name, CodaScriptStringParameterTypeT Value);
			CodaScriptVariable*							Add(const char* Name, CodaScriptNumericDataTypeT Value);
			CodaScriptVariable*							Add(const char* Name, CodaScriptReferenceDataTypeT Value);

			void										Remove(CodaScriptVariable* Variable);
			CodaScriptVariable*							Lookup(const char* Name);
			bool										Lookup(CodaScriptVariable* Variable, CodaScriptVariableListT::iterator& Match);
			void										Clear(void);

			void										INILoadState(void);
			void										INISaveState(void);
		public:
			CodaScriptGlobalDataStore(BGSEEINIManagerGetterFunctor Getter, BGSEEINIManagerSetterFunctor Setter);
			~CodaScriptGlobalDataStore();

			void										ShowEditDialog(HINSTANCE ResourceInstance, HWND Parent);
			CodaScriptVariableListT&					GetCache(void);
		};

		class CodaScriptVM : public ICodaScriptObject
		{
			friend class								CodaScriptBackgrounder;
			friend class								CodaScriptGlobalDataStore;

			static BGSEEConsoleCommandInfo				kDumpCodaDocsConsoleCommandData;
			static void									DumpCodaDocsConsoleCommandHandler(UInt32 ParamCount, const char* Args);

			static CodaScriptVM*						Singleton;

			BGSEEResourceLocation						BaseDirectory;
			CodaScriptCommandRegistry*					CommandRegistry;
			CodaScriptMessageHandler*					MessageHandler;
			CodaScriptExecutive*						Executive;
			CodaScriptBackgrounder*						Backgrounder;
			CodaScriptGlobalDataStore*					GlobalStore;
			ICodaScriptExpressionParser*				ExpressionParser;

			bool										Initialized;

			CodaScriptVM();

			CodaScriptExecutionContext*					CreateExecutionContext(std::fstream& Input, CodaScriptMutableDataArrayT* Parameters = NULL);
		public:
			~CodaScriptVM();

			static const std::string					kSourceExtension;
			static CodaScriptVM*						GetSingleton();

			bool										Initialize(BGSEEResourceLocation BasePath,
																const char* WikiURL,
																BGSEEINIManagerGetterFunctor INIGetter,
																BGSEEINIManagerSetterFunctor INISetter,
																CodaScriptRegistrarListT& ScriptCommands);

			bool										RunScript(std::string ScriptName,					// script name's the same as filename
																CodaScriptMutableDataArrayT* Parameters,
																CodaScriptBackingStore* Result,
																bool& ReturnedResult);

			void										ShowGlobalStoreEditDialog(HINSTANCE ResourceInstance, HWND Parent);
			CodaScriptVariable*							GetGlobal(const char* Name);
			CodaScriptVariableListT&					GetGlobals(void) const;

			bool										GetBackgrounderState(void) const;
			bool										ToggleBackgrounderState(void);

			CodaScriptMessageHandler*					MsgHdlr(void);
		};
#define CODAVM											BGSEditorExtender::BGSEEScript::CodaScriptVM::GetSingleton()
#define CODAVM_ENABLEPROFILER							1

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