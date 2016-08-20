#pragma once

#include <CrashRpt.h>

// Main - Global singleton that manages all of the basic extender-specific data
// Daemon - Handles basic services like plugin initialization and shutdown

namespace bgsee
{
	class Main;
	class Console;

	class DaemonCallback
	{
	public:
		virtual ~DaemonCallback() = 0
		{
			;//
		}

		virtual bool				Handle(void* Parameter = nullptr) = 0;
	};

	class Daemon
	{
	public:
		enum
		{
			kInitCallback_Query = 0,
			kInitCallback_Load,
			kInitCallback_PostMainWindowInit,		// the WM_INITDIALOG msg will be dispatched to the main window after this event is handled
			kInitCallback_Epilog,

			kInitCallback__MAX
		};
	private:
		Daemon();
		typedef std::vector<DaemonCallback*>	DaemonCallbackArrayT;

		DaemonCallbackArrayT		InitCallbacks[kInitCallback__MAX];
		DaemonCallbackArrayT		DeinitCallbacks;
		DaemonCallbackArrayT		CrashHandlerCallbacks;

		bool						FullInitComplete;

		bool						ExecuteDeinitCallbacks(void);
		bool						ExecuteCrashCallbacks(CR_CRASH_CALLBACK_INFO* Data);
		void						ReleaseCallbacks(DaemonCallbackArrayT& CallbackList);

		friend class				Main;
	public:
		virtual ~Daemon();



		void						RegisterInitCallback(UInt8 CallbackType, DaemonCallback* Callback);		// takes ownership of pointer, no parameter
		void						RegisterDeinitCallback(DaemonCallback* Callback);							// takes ownership of pointer, no parameter
		void						RegisterCrashCallback(DaemonCallback* Callback);							// takes ownership of pointer, parameter = CR_CRASH_CALLBACK_INFO*
																													// if a callback returns true, the editor process is not terminated
		bool						ExecuteInitCallbacks(UInt8 CallbackType);
		bool						GetFullInitComplete(void) const;

		static void					WaitForDebugger(void);
	};

	// for direct writing
	class INIManagerSetterFunctor
	{
		SME::INI::INIManager*		ParentManager;
	public:
		INIManagerSetterFunctor(SME::INI::INIManager* Parent = nullptr);

		void						operator()(const char* Key, const char* Section, const char* Value);
		void						operator()(const char* Section, const char* Value);
	};

	// for direct reading
	class INIManagerGetterFunctor
	{
		SME::INI::INIManager*		ParentManager;
	public:
		INIManagerGetterFunctor(SME::INI::INIManager* Parent = nullptr);

		int							operator()(const char* Key, const char* Section, const char* Default, char* OutBuffer, UInt32 Size);
		int							operator()(const char* Section, char* OutBuffer, UInt32 Size);
	};

	typedef SME::INI::INISettingListT			INISettingDepotT;

	class ReleaseNameTable
	{
	protected:
		typedef std::map<UInt32, std::string>			VersionNameMap;

		VersionNameMap				Table;

		void						RegisterRelease(UInt16 Major, UInt8 Minor, const char* Name);
	public:
		ReleaseNameTable();
		virtual ~ReleaseNameTable() = 0;

		const char*					LookupRelease(UInt16 Major, UInt8 Minor);
	};

	class Main
	{
		static Main*			Singleton;

		class INIManager : public SME::INI::INIManager
		{
		protected:
			static const char*					kSectionPrefix;

			SME::INI::INIEditGUI				GUI;

			virtual bool						RegisterSetting(INISetting* Setting, bool AutoLoad = true, bool Dynamic = false);
		public:
			INIManager();
			virtual ~INIManager();

			virtual void						Initialize(const char* INIPath, void* Paramenter);
			virtual SME::INI::INISetting*		FetchSetting(const char* Key, const char* Section, bool Dynamic = false);

			virtual int							DirectRead(const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size);
			virtual int							DirectRead(const char* Section, char* OutBuffer, UInt32 Size);
			virtual bool						DirectWrite(const char* Setting, const char* Section, const char* Value);
			virtual bool						DirectWrite(const char* Section, const char* Value);

			void								ShowGUI(HINSTANCE ResourceInstance, HWND Parent);
		};

		class DefaultInitCallback : public DaemonCallback
		{
		public:
			const char*				DisplayName;
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

			virtual bool			Handle(void* Parameter = nullptr);
		};

		static int CALLBACK			CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo);

		class DefaultDeinitCallback : public DaemonCallback
		{
			Main*				ParentInstance;
		public:
			DefaultDeinitCallback(Main* Parent);
			virtual ~DefaultDeinitCallback();

			virtual bool			Handle(void* Parameter = nullptr);
		};

		friend class DefaultInitCallback;
		friend class DefaultDeinitCallback;

		Main();

		std::string					ExtenderLongName;
		std::string					ExtenderDisplayName;
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

		INIManager*					ExtenderINIManager;
		Console*					ExtenderConsole;
		Daemon*						ExtenderDaemon;
		bool						CrashRptSupport;

		bool						Initialized;
	public:
		~Main();

		enum
		{
			kExtenderParentEditor_Unknown	=	0,
			kExtenderParentEditor_TES4CS	=	1,
			kExtenderParentEditor_TES5CK	=	2,
			kExtenderParentEditor__MAX
		};

		static Main*							GetSingleton();

		bool									Initialize(const char* LongName, const char* DisplayName, const char* ShortName, const char* ReleaseName,
														UInt32 Version, UInt8 EditorID, UInt32 EditorSupportedVersion, UInt32 EditorCurrentVersion,
														const char* APPPath,
														UInt32 SEPluginHandle, UInt32 SEMinimumVersion, UInt32 SECurrentVersion,
														INISettingDepotT& INISettings,
														const char* DotNETFrameworkVersion, bool CLRMemoryProfiling, bool WaitForDebugger,
														bool CrashRptSupport = true);

		const char*								ExtenderGetLongName(void) const;
		const char*								ExtenderGetDisplayName(void) const;
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

		Console*								GetConsole(void) const;
		Daemon*									GetDaemon(void) const;

		INIManagerGetterFunctor					INIGetter(void);
		INIManagerSetterFunctor					INISetter(void);

		void									ShowPreferencesGUI(void);
	};

#define BGSEEMAIN					bgsee::Main::GetSingleton()
#define BGSEECONSOLE				BGSEEMAIN->GetConsole()
#define BGSEECONSOLE_MESSAGE(...)	BGSEECONSOLE->LogMsg(BGSEEMAIN->ExtenderGetShortName(), __VA_ARGS__)
#define BGSEECONSOLE_ERROR(...)		BGSEECONSOLE->LogWindowsError(BGSEEMAIN->ExtenderGetShortName(), __VA_ARGS__)

#define BGSEE_DEBUGBREAK			bgsee:Daemon::WaitForDebugger(); __asm { int 3 }

#define BGSEEMAIN_EXTENDERLONGNAME		"Bethesda Game Studios Editor Extender"
#define BGSEEMAIN_EXTENDERSHORTNAME		"BGSEE"

#undef BGSEEMAIN_EXTENDERLONGNAME
#undef BGSEEMAIN_EXTENDERSHORTNAME
}

BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved);