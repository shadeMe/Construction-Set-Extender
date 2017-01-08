#pragma once
#include "..\Main.h"
#include "..\Console.h"
#include "..\WorkspaceManager.h"
#include "CodaCompiler.h"
#include "CodaInterpreter.h"

namespace bgsee
{
	namespace script
	{
		class CodaScriptCommandRegistry
		{
			typedef std::multimap<std::string, ICodaScriptCommand*>		CommandTableMapT;

			CommandTableMapT							Registry;
			std::string									BaseWikiURL;

			void										AppendToStream(std::fstream& Out, const char* Fmt, ...);
			std::string									SanitizeLinkableString(const char* String);
			const char*									StringifyParameterType(UInt8 ParamType);
		public:
			CodaScriptCommandRegistry(const char* WikiURL);
			~CodaScriptCommandRegistry();

			ICodaScriptCommand*							LookupCommand(const char* Name, bool UseAlias = false);
			bool										RegisterCommand(const char* Category, ICodaScriptCommand* Command);		// caller retains ownership of the function prototype
			void										RegisterCommands(const CodaScriptCommandRegistrar::ListT& Registrars);
			void										InitializeExpressionParser(ICodaScriptExpressionParser* Parser);

			void										Dump(std::string OutPath);

			typedef std::unique_ptr<CodaScriptCommandRegistry>		PtrT;
		};

		class CodaScriptProgramCache : public ICodaScriptProgramCache
		{
			typedef std::unordered_map<std::string, ICodaScriptProgram::PtrT>		ProgramMapT;		// key = file path

			ICodaScriptVirtualMachine*		VM;
			ProgramMapT						Store;

			ICodaScriptProgram*				Lookup(std::string Filepath) const;
			void							Remove(std::string Filepath);
			void							Add(std::string Filepath, ICodaScriptProgram::PtrT& Program);
		public:
			CodaScriptProgramCache(ICodaScriptVirtualMachine* VM);
			virtual ~CodaScriptProgramCache();

			virtual ICodaScriptProgram*			Get(std::string Filepath, bool Recompile = false) override;
			virtual void						Invalidate() override;
		};

		class CodaScriptExecutive : public ICodaScriptExecutor
		{
			static INISetting							kINI_Profiling;
			static INISetting							kINI_RecursionLimit;

			typedef std::unordered_map<ICodaScriptProgram*, int>	ProgramCounterMapT;		// value = executing instance counter

			ProgramCounterMapT							ExecutionCounter;
			ICodaScriptExecutionContext::StackT			ExecutingContexts;
			CodaScriptProfiler							Profiler;
			DWORD										OwnerThreadID;
			ICodaScriptVirtualMachine*					VM;

			void										Push(ICodaScriptExecutionContext* Context);
			void										Pop(ICodaScriptExecutionContext* Context);
		public:
			CodaScriptExecutive(ICodaScriptVirtualMachine* VM);
			virtual ~CodaScriptExecutive();

			virtual void								Execute(ICodaScriptExecutionContext* Context,
																ICodaScriptVirtualMachine::ExecuteResult& Out) override;
			virtual bool								IsBusy() const override;
			virtual bool								IsProgramExecuting(ICodaScriptProgram* Program) const override;

			static void									RegisterINISettings(INISettingDepotT& Depot);
		};

		class CodaScriptBackgrounder : public ICodaScriptBackgroundDaemon
		{
			static INISetting					kINI_Enabled;
			static INISetting					kINI_UpdatePeriod;
			static INISetting					kINI_LogToDefaultConsoleContext;

			static VOID CALLBACK				CallbackProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			typedef std::vector<ICodaScriptExecutionContext::PtrT>	ContextArrayT;

			ResourceLocation					SourceDepot;
			ContextArrayT						DepotCache;		// stores the contexts for scripts in the depot
			ContextArrayT						RuntimeCache;	// stores the contexts for regular scripts executing in the background
			bool								State;
			bool								Backgrounding;
			HWND								TimerDummyWindow;
			CodaScriptElapsedTimeCounterT		PollingTimeCounter;
			ICodaScriptVirtualMachine*			VM;

			INIManagerGetterFunctor				INISettingGetter;
			INIManagerSetterFunctor				INISettingSetter;

			void								ResetDepotCache(bool Renew = false);
			void								ResetTimer(bool Renew = false);

			void								Execute(ContextArrayT& Cache, double TimePassed);
			void								Tick();
		public:
			CodaScriptBackgrounder(ICodaScriptVirtualMachine* VM,
								   ResourceLocation Source,
								   INIManagerGetterFunctor Getter,
								   INIManagerSetterFunctor Setter);
			virtual ~CodaScriptBackgrounder();

			static void							RegisterINISettings(INISettingDepotT& Depot);

			virtual const ResourceLocation&		GetBackgroundScriptRepository() const override;

			virtual void						Suspend() override;
			virtual void						Resume() override;
			virtual bool						IsEnabled() const override;
			virtual bool						IsBackgrounding() const override;

			virtual void						Rebuild() override;
			virtual void						Queue(ICodaScriptExecutionContext* Context) override;
		};

		class CodaScriptGlobalDataStore
		{
			static BOOL CALLBACK						EditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			CodaScriptVariable::ArrayT					Cache;
			INIManagerGetterFunctor						INISettingGetter;
			INIManagerSetterFunctor						INISettingSetter;
			ICodaScriptVirtualMachine*					VM;

			bool										Add(CodaScriptVariable* Variable);
			template<typename T>
			CodaScriptVariable*							Add(const char* Name, T Value, bool& ExistingVar);
			CodaScriptVariable*							Add(const char* Name, CodaScriptStringParameterTypeT Value, bool& ExistingVar);
			CodaScriptVariable*							Add(const char* Name, CodaScriptNumericDataTypeT Value, bool& ExistingVar);
			CodaScriptVariable*							Add(const char* Name, CodaScriptReferenceDataTypeT Value, bool& ExistingVar);

			void										Remove(CodaScriptVariable* Variable);
			bool										Lookup(CodaScriptVariable* Variable, CodaScriptVariable::ArrayT::iterator& Match);
			void										Clear(void);

			void										INILoadState(void);
			void										INISaveState(void);
		public:
			CodaScriptGlobalDataStore(ICodaScriptVirtualMachine* VM,
									  INIManagerGetterFunctor Getter,
									  INIManagerSetterFunctor Setter);
			~CodaScriptGlobalDataStore();

			void										ShowEditDialog(HINSTANCE ResourceInstance, HWND Parent);
			CodaScriptVariable::ArrayT&					GetCache(void);
			CodaScriptVariable*							Lookup(const char* Name);

			typedef std::unique_ptr<CodaScriptGlobalDataStore>		PtrT;
		};

		class CodaScriptVM : public ICodaScriptVirtualMachine
		{
			static const std::string					kBackgroundDepotName;
			static const std::string					kSourceExtension;

			static ConsoleCommandInfo					kDumpCodaDocsConsoleCommandData;
			static void									DumpCodaDocsConsoleCommandHandler(UInt32 ParamCount, const char* Args);

			static CodaScriptVM*						Singleton;

			ResourceLocation							BaseDirectory;
			CodaScriptCommandRegistry::PtrT				CommandRegistry;
			CodaScriptMessageHandler::PtrT				MessageHandler;
			CodaScriptProgramCache::PtrT				ProgramCache;
			CodaScriptExecutive::PtrT					Executive;
			CodaScriptBackgrounder::PtrT				Backgrounder;
			CodaScriptGlobalDataStore::PtrT				GlobalStore;
			ICodaScriptExpressionParser::PtrT			ExpressionParser;

			bool										Initialized;

			CodaScriptVM(ResourceLocation BasePath,
						 const char* WikiURL,
						 INIManagerGetterFunctor INIGetter,
						 INIManagerSetterFunctor INISetter,
						 CodaScriptCommandRegistrar::ListT& ScriptCommands);
			virtual ~CodaScriptVM();
		public:
			static CodaScriptVM*						Get();
			static bool									Initialize(ResourceLocation BasePath,
																const char* WikiURL,
																INIManagerGetterFunctor INIGetter,
																INIManagerSetterFunctor INISetter,
																CodaScriptCommandRegistrar::ListT& ScriptCommands);
			static void									Deinitialize();


			void												ShowGlobalStoreEditDialog(HINSTANCE ResourceInstance, HWND Parent);
			void												OpenScriptRepository(void) const;

			virtual ICodaScriptExpressionParser*				BuildExpressionParser() override;
			virtual ICodaScriptDataStoreOwner*					BuildDataStoreOwner() override;
			virtual ICodaScriptArrayDataType::SharedPtrT		BuildArray(UInt32 InitialSize = 0) override;

			virtual const ResourceLocation&						GetScriptRepository() const override;
			virtual const std::string&							GetScriptFileExtension() const override;

			virtual CodaScriptVariable*							GetGlobal(const char* Name) const override;
			virtual const CodaScriptVariable::ArrayT&			GetGlobals() const override;

			virtual CodaScriptMessageHandler*					GetMessageHandler() const override;
			virtual ICodaScriptExpressionParser*				GetParser() const override;
			virtual ICodaScriptExecutor*						GetExecutor() const override;
			virtual ICodaScriptBackgroundDaemon*				GetBackgroundDaemon() const override;
			virtual ICodaScriptProgramCache*					GetProgramCache() const override;

			virtual bool										IsProgramExecuting(ICodaScriptProgram* Program) const override;
			virtual void										RunScript(ExecuteParams& Input, ExecuteResult& Output) override;
		};

#define CODAVM											bgsee::script::CodaScriptVM::Get()
	}
}