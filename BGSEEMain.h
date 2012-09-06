#pragma once

// BGSEEMain - Global singleton that manages all of the basic extender-specific data
// BGSEEDaemon - Handles basic services like plugin initialization and shutdown

namespace BGSEditorExtender
{
	class BGSEEMain;
	class BGSEEConsole;

	class BGSEEDaemon
	{
		BGSEEDaemon();
		typedef std::list<BoolRFunctorBase*>	DaemonCallbackListT;

		DaemonCallbackListT			InitCallbacks[4];
		DaemonCallbackListT			DeinitCallbacks;
		DaemonCallbackListT			CrashHandlerCallbacks;

		bool						FullInitComplete;

		bool						ExecuteDeinitCallbacks(void);
		bool						ExecuteCrashCallbacks(void);
		void						ReleaseCallbacks(DaemonCallbackListT& CallbackList);

		friend class				BGSEEMain;
	public:
		virtual ~BGSEEDaemon();

		enum
		{
			kInitCallback_Query		=	0,
			kInitCallback_Load,
			kInitCallback_PostMainWindowInit,		// the WM_INITDIALOG msg will be dispatched to the main window after this event is handled
			kInitCallback_Epilog,

			kInitCallback__MAX
		};

		void						RegisterInitCallback(UInt8 CallbackType, BoolRFunctorBase* Callback);		// takes ownership of pointer
		void						RegisterDeinitCallback(BoolRFunctorBase* Callback);							// takes ownership of pointer
		void						RegisterCrashCallback(BoolRFunctorBase* Callback);							// takes ownership of pointer

		bool						ExecuteInitCallbacks(UInt8 CallbackType);
		bool						GetFullInitComplete(void) const;

		static void					WaitForDebugger(void);
	};

	class BGSEEINIManagerSetterFunctor
	{
		SME::INI::INIManager*		ParentManager;
	public:
		BGSEEINIManagerSetterFunctor(SME::INI::INIManager* Parent = NULL);

		void						operator()(const char* Key, const char* Section, const char* Value, bool Direct = false);
		void						operator()(const char* Section, const char* Value);
	};

	class BGSEEINIManagerGetterFunctor
	{
		SME::INI::INIManager*		ParentManager;
	public:
		BGSEEINIManagerGetterFunctor(SME::INI::INIManager* Parent = NULL);

		const char*					operator()(const char* Key, const char* Section);
		int							operator()(const char* Key, const char* Section, const char* Default, char* OutBuffer, UInt32 Size);
		int							operator()(const char* Section, char* OutBuffer, UInt32 Size);
	};

	struct BGSEEINIManagerSettingFactory
	{
		struct SettingData
		{
			const char*				Key;
			const char*				DefaultValue;
			const char*				Description;
		};

		typedef std::vector<const SettingData*>	SettingListT;

		const char*					Section;
		SettingListT				Settings;

		BGSEEINIManagerSettingFactory(const char* Section);

		const SettingData*			Lookup(const char* Key);

		const char*					Get(int Index, BGSEEINIManagerGetterFunctor& Getter);									// order dependent
		void						Set(int Index, BGSEEINIManagerSetterFunctor& Setter, const char* Value, ...);			// order dependent
	};
	typedef std::list<BGSEEINIManagerSettingFactory*>		SettingFactoryListT;

	class BGSEEReleaseNameTable
	{
	protected:
		typedef std::map<UInt32, std::string>			VersionNameMap;

		VersionNameMap				Table;

		void						RegisterRelease(UInt8 Major, UInt8 Minor, UInt8 Revision, const char* Name);
	public:
		BGSEEReleaseNameTable();
		virtual ~BGSEEReleaseNameTable() = 0;

		const char*					LookupRelease(UInt8 Major, UInt8 Minor, UInt8 Revision);
	};

	class BGSEEMain
	{
		static BGSEEMain*			Singleton;

		class BGSEEINIManager : public SME::INI::INIManager
		{
		protected:
			static const char*					kSectionPrefix;

			SME::INI::INIEditGUI				GUI;

			virtual bool						RegisterSetting(const char* Key, const char* Section, const char* DefaultValue, const char* Description);
		public:
			BGSEEINIManager();
			virtual ~BGSEEINIManager();

			virtual void						Initialize(const char* INIPath, void* Paramenter);
			virtual SME::INI::INISetting*		FetchSetting(const char* Key, const char* Section);

			virtual int							DirectRead(const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size);
			virtual int							DirectRead(const char* Section, char* OutBuffer, UInt32 Size);
			virtual bool						DirectWrite(const char* Setting, const char* Section, const char* Value);
			virtual bool						DirectWrite(const char* Section, const char* Value);

			void								ShowGUI(HINSTANCE ResourceInstance, HWND Parent);
		};

		class DefaultInitCallback : public BoolRFunctorBase
		{
		public:
			const char*				LongName;
			const char*				ReleaseName;
			const char*				APPPath;
			HINSTANCE				ModuleHandle;
			UInt32					ExtenderVersion;
			UInt32					EditorSupportedVersion;
			UInt32					EditorCurrentVersion;
			UInt32					SEMinVersion;
			UInt32					SECurrentVersion;
			const char*				DotNETFrameworkVersionString;
			bool					EnableCLRMemoryProfiling;
			bool					WaitForDebuggerOnStartup;

			DefaultInitCallback();
			virtual ~DefaultInitCallback();

			virtual bool			operator()();
		};

		static BOOL CALLBACK		CrashCallback(LPVOID lpvState);

		class DefaultDeinitCallback : public BoolRFunctorBase
		{
			BGSEEMain*				ParentInstance;
		public:
			DefaultDeinitCallback(BGSEEMain* Parent);
			virtual ~DefaultDeinitCallback();

			virtual bool			operator()();
		};

		friend class DefaultInitCallback;
		friend class DefaultDeinitCallback;
	protected:
		BGSEEMain();

		std::string					ExtenderLongName;
		char						ExtenderShortName[0x50];		// needs to be static as xSE caches the c-string pointer
		std::string					ExtenderReleaseName;
		UInt32						ExtenderVersion;
		HINSTANCE					ExtenderModuleHandle;

		UInt8						ParentEditorID;
		std::string					ParentEditorName;				// name of the editor executable
		UInt32						ParentEditorSupportedVersion;

		std::string					GameDirectoryPath;				// path to the game's root directory
		std::string					ExtenderDLLPath;				// derived from the dirpath and long name
		std::string					ExtenderINIPath;				// derived from the dirpath and long name
		std::string					ExtenderComponentDLLPath;

		UInt32						ScriptExtenderPluginHandle;
		UInt32						ScriptExtenderCurrentVersion;

		BGSEEINIManager*			ExtenderINIManager;
		BGSEEConsole*				ExtenderConsole;
		BGSEEDaemon*				ExtenderDaemon;
		bool						CrashRptSupport;

		bool						Initialized;
	public:
		virtual ~BGSEEMain();

		enum
		{
			kExtenderParentEditor_Unknown	=	0,
			kExtenderParentEditor_TES4CS	=	1,
			kExtenderParentEditor_TES5CK	=	2,
			kExtenderParentEditor__MAX
		};

		static BGSEEMain*						GetSingleton();

		bool									Initialize(const char* LongName, const char* ShortName, const char* ReleaseName,
														UInt32 Version, UInt8 EditorID, UInt32 EditorSupportedVersion, UInt32 EditorCurrentVersion,
														const char* APPPath,
														UInt32 SEPluginHandle, UInt32 SEMinimumVersion, UInt32 SECurrentVersion,
														SettingFactoryListT& INISettingFactoryList,
														const char* DotNETFrameworkVersion, bool CLRMemoryProfiling, bool WaitForDebugger,
														bool CrashRptSupport = true);

		const char*								ExtenderGetLongName(void) const;
		const char*								ExtenderGetShortName(void) const;
		const char*								ExtenderGetReleaseName(void) const;
		UInt32									ExtenderGetVersion(void) const;
		const char*								ExtenderGetSEName(void) const;

		const char*								ParentEditorGetLongName(void) const;
		const char*								ParentEditorGetShortName(void) const;
		UInt32									ParentEditorGetVersion(void) const;

		HINSTANCE								GetExtenderHandle(void) const;

		const char*								GetAPPPath(void) const;
		const char*								GetDLLPath(void) const;
		const char*								GetINIPath(void) const;
		const char*								GetComponentDLLPath(void) const;

		BGSEEConsole*							Console(void) const;
		BGSEEDaemon*							Daemon(void) const;

		BGSEEINIManagerGetterFunctor			INIGetter(void);
		BGSEEINIManagerSetterFunctor			INISetter(void);

		void									ShowPreferencesGUI(void);
	};

#define BGSEEMAIN					BGSEditorExtender::BGSEEMain::GetSingleton()
#define BGSEECONSOLE				BGSEEMAIN->Console()
#define BGSEECONSOLE_MESSAGE(...)	BGSEECONSOLE->LogMsg(BGSEEMAIN->ExtenderGetShortName(), __VA_ARGS__)
#define BGSEECONSOLE_ERROR(...)		BGSEECONSOLE->LogErrorMsg(BGSEEMAIN->ExtenderGetShortName(), __VA_ARGS__)

#define BGSEE_DEBUGBREAK			BGSEditorExtender::BGSEEDaemon::WaitForDebugger(); DebugBreak()

#define BGSEEMAIN_EXTENDERLONGNAME		"Bethesda Game Studios Editor Extender"
#define BGSEEMAIN_EXTENDERSHORTNAME		"BGSEE"

#undef BGSEEMAIN_EXTENDERLONGNAME
#undef BGSEEMAIN_EXTENDERSHORTNAME
}

BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved);