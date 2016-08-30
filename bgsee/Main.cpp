#include "Main.h"
#include "Console.h"
#include "UIManager.h"
#include "DetectSIMD.h"
#include "Script\CodaVM.h"
#include <Tools\VersionInfo.h>

namespace bgsee
{
	Daemon::Daemon() :
		InitCallbacks(),
		DeinitCallbacks(),
		CrashHandlerCallbacks(),
		FullInitComplete(false),
		Deinitializing(false)
	{
		;//
	}

	void Daemon::WaitForDebugger(void)
	{
		BGSEECONSOLE->Indent();
		BGSEECONSOLE_MESSAGE("Waiting For Debugger...");

		while (IsDebuggerPresent() == FALSE)
			Sleep(100);

		BGSEECONSOLE_MESSAGE("Debugger Attached!");
		BGSEECONSOLE->Exdent();
	}

	void Daemon::RegisterInitCallback( UInt8 CallbackType, DaemonCallback* Callback )
	{
		SME_ASSERT(CallbackType < kInitCallback__MAX && Callback);
		InitCallbacks[CallbackType].push_back(Callback);
	}

	void Daemon::RegisterDeinitCallback( DaemonCallback* Callback )
	{
		SME_ASSERT(Callback);
		DeinitCallbacks.push_back(Callback);
	}

	void Daemon::RegisterCrashCallback( DaemonCallback* Callback )
	{
		SME_ASSERT(Callback);
		CrashHandlerCallbacks.push_back(Callback);
	}

	bool Daemon::ExecuteInitCallbacks( UInt8 CallbackType )
	{
		SME_ASSERT(CallbackType < kInitCallback__MAX);

		bool Result = true;

		for (DaemonCallbackArrayT::const_iterator Itr = InitCallbacks[CallbackType].begin(); Itr != InitCallbacks[CallbackType].end(); Itr++)
		{
			if ((*Itr)->Handle() == false)
			{
				Result = false;
				break;
			}
		}

		if (CallbackType == kInitCallback_Epilog)
		{
			FullInitComplete = true;
		}

		return Result;
	}

	bool Daemon::ExecuteDeinitCallbacks( void )
	{
		FlagDeinitialization();

		for (DaemonCallbackArrayT::const_iterator Itr = DeinitCallbacks.begin(); Itr != DeinitCallbacks.end(); Itr++)
			(*Itr)->Handle();

		return true;
	}

	bool Daemon::ExecuteCrashCallbacks( CR_CRASH_CALLBACK_INFO* Data )
	{
		bool Result = false;

		for (DaemonCallbackArrayT::const_iterator Itr = CrashHandlerCallbacks.begin(); Itr != CrashHandlerCallbacks.end(); Itr++)
		{
			if ((*Itr)->Handle(Data) && Result == false)
				Result = true;
		}

		return Result;
	}

	Daemon::~Daemon()
	{
		ReleaseCallbacks(InitCallbacks[kInitCallback_Query]);
		ReleaseCallbacks(InitCallbacks[kInitCallback_Load]);
		ReleaseCallbacks(InitCallbacks[kInitCallback_PostMainWindowInit]);
		ReleaseCallbacks(InitCallbacks[kInitCallback_Epilog]);
		ReleaseCallbacks(DeinitCallbacks);
		ReleaseCallbacks(CrashHandlerCallbacks);
	}

	void Daemon::ReleaseCallbacks( DaemonCallbackArrayT& CallbackList )
	{
		for (DaemonCallbackArrayT::const_iterator Itr = CallbackList.begin(); Itr != CallbackList.end(); Itr++)
			delete (*Itr);

		CallbackList.clear();
	}

	bool Daemon::GetFullInitComplete( void ) const
	{
		return FullInitComplete;
	}

	void Daemon::FlagDeinitialization(void)
	{
		SME_ASSERT(Deinitializing == false);

		Deinitializing = true;
	}

	bool Daemon::IsDeinitializing(void) const
	{
		return Deinitializing;
	}

	const char*		Main::INIManager::kSectionPrefix = "BGSEE::";

	INIManagerSetterFunctor::INIManagerSetterFunctor( SME::INI::INIManager* Parent ) :
			ParentManager(Parent)
	{
		;//
	}

	void INIManagerSetterFunctor::operator()( const char* Section, const char* Value )
	{
		SME_ASSERT(ParentManager);

		ParentManager->DirectWrite(Section, Value);
	}

	void INIManagerSetterFunctor::operator()( const char* Key, const char* Section, const char* Value )
	{
		SME_ASSERT(ParentManager);

		ParentManager->DirectWrite(Key, Section, Value);
	}

	INIManagerGetterFunctor::INIManagerGetterFunctor( SME::INI::INIManager* Parent ) :
			ParentManager(Parent)
	{
		;//
	}

	int INIManagerGetterFunctor::operator()( const char* Key, const char* Section, const char* Default, char* OutBuffer, UInt32 Size )
	{
		SME_ASSERT(ParentManager);

		return ParentManager->DirectRead(Key, Section, Default, OutBuffer, Size);
	}

	int INIManagerGetterFunctor::operator()( const char* Section, char* OutBuffer, UInt32 Size )
	{
		SME_ASSERT(ParentManager);

		return ParentManager->DirectRead(Section, OutBuffer, Size);
	}

	Main::INIManager::INIManager() :
		GUI()
	{
		;//
	}

	Main::INIManager::~INIManager()
	{
		Save();
	}

	bool Main::INIManager::RegisterSetting( INISetting* Setting, bool AutoLoad /*= true*/, bool Dynamic /*= false*/ )
	{
		SME_ASSERT(Setting);

		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Setting->GetSection());
		SetSettingSection(Setting, Buffer);

		return SME::INI::INIManager::RegisterSetting(Setting, AutoLoad, Dynamic);
	}

	SME::INI::INISetting* Main::INIManager::FetchSetting( const char* Key, const char* Section, bool Dynamic /*= false*/ )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::FetchSetting(Key, Buffer, Dynamic);
	}

	void Main::INIManager::Initialize(const char* INIPath, void* Paramenter)
	{
		INIFilePath = INIPath;

		std::fstream INIStream(INIFilePath.c_str(), std::fstream::in);
		bool CreateINI = false;

		if (INIStream.fail())
		{
			CreateINI = true;
		}

		INIStream.close();
		INIStream.clear();

		INISettingDepotT* FactoryList = (INISettingDepotT*)Paramenter;

		for (INISettingDepotT::iterator Itr = FactoryList->begin(); Itr != FactoryList->end(); Itr++)
		{
			bool Result = RegisterSetting((*Itr), true, false);
			SME_ASSERT(Result);
		}

		if (CreateINI)
		{
			Save();
		}
	}

	void Main::INIManager::ShowGUI( HINSTANCE ResourceInstance, HWND Parent )
	{
		GUI.InitializeGUI(ResourceInstance, Parent, this);
	}

	int Main::INIManager::DirectRead( const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::DirectRead(Setting, Buffer, Default, OutBuffer, Size);
	}

	int Main::INIManager::DirectRead( const char* Section, char* OutBuffer, UInt32 Size )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::DirectRead(Buffer, OutBuffer, Size);
	}

	bool Main::INIManager::DirectWrite( const char* Setting, const char* Section, const char* Value )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::DirectWrite(Setting, Buffer, Value);
	}

	bool Main::INIManager::DirectWrite( const char* Section, const char* Value )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::DirectWrite(Buffer, Value);
	}

	void ReleaseNameTable::RegisterRelease( UInt16 Major, UInt8 Minor, const char* Name )
	{
		SME_ASSERT(Name);

		UInt32 Version = (((Major & 0xFF) << 24) | ((Minor & 0xFF) << 16) | 0xFFFF);
		if (Table.count(Version) == 0)
		{
			Table.insert(std::make_pair(Version, Name));
		}
	}

	ReleaseNameTable::ReleaseNameTable() :
		Table()
	{
		;//
	}

	ReleaseNameTable::~ReleaseNameTable()
	{
		;//
	}

	const char* ReleaseNameTable::LookupRelease( UInt16 Major, UInt8 Minor )
	{
		UInt32 Version = (((Major & 0xFF) << 24) | ((Minor & 0xFF) << 16) | 0xFFFF);

		if (Table.count(Version))
			return Table[Version].c_str();
		else
			return "Herr Keuner";
	}

	Main::DefaultInitCallback::DefaultInitCallback() :
		DaemonCallback()
	{
		DisplayName = nullptr;
		ReleaseName = nullptr;
		APPPath = nullptr;
		ExtenderVersion = 0x0;
		ModuleHandle = 0x0;
		EditorSupportedVersion = EditorCurrentVersion = 0x0;
		SEMinVersion = SECurrentVersion = 0x0;
		DotNETFrameworkVersionString = "v4.0.30319";
		EnableCLRMemoryProfiling = false;
		WaitForDebuggerOnStartup = false;
	}

	bool Main::DefaultInitCallback::Handle(void* Parameter)
	{
#ifdef NDEBUG
		const char* ReleaseMode = "";
#else
		const char* ReleaseMode = " DEBUG";
#endif
		BGSEECONSOLE_MESSAGE("%s \"%s\" v%d.%d.%d.%d%s Initializing...",
							DisplayName,
							ReleaseName,
							SME_VERSION_MAJOR(ExtenderVersion),
							SME_VERSION_MINOR(ExtenderVersion),
							SME_VERSION_REVISION(ExtenderVersion),
							SME_VERSION_BUILD(ExtenderVersion),
							ReleaseMode);

		BGSEECONSOLE->Indent();

		if (WaitForDebuggerOnStartup)
		{
			Daemon::WaitForDebugger();
		}

		if (ModuleHandle == nullptr)
		{
			BGSEECONSOLE_MESSAGE("Couldn't fetch module handle");
			return false;
		}

		BGSEECONSOLE_MESSAGE("Checking Versions and Dependencies");
		BGSEECONSOLE->Indent();
		simd_init();
		if (!HASSSE2)
		{
			BGSEECONSOLE_MESSAGE("Processor doesn't support SSE2 SIMD instructions");
			return false;
		}

		OSVERSIONINFOEX OSInfo = {0};
		SYSTEM_INFO SysInfo = {0};

		OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		GetVersionEx((LPOSVERSIONINFO)&OSInfo);
		GetSystemInfo(&SysInfo);

		if (OSInfo.dwMajorVersion < 6 && OSInfo.dwMinorVersion < 1)
		{
			BGSEECONSOLE_MESSAGE("OS version too old - Windows 7 or greater required");
			return false;
		}
		else if (OSInfo.dwMajorVersion >= 6 && OSInfo.dwMinorVersion > 3)
		{
			BGSEECONSOLE_MESSAGE("Your current version of Windows (%d.%d.%d) is not officially supported - Expect general weirdness such as collapsing time vortexes and code cannibalism", OSInfo.dwMajorVersion, OSInfo.dwMinorVersion, OSInfo.dwBuildNumber);
			BGSEECONSOLE_MESSAGE("You may attempt to run the editor in Windows' Compatibility Mode. This can be done by opening the 'File Properties' dialog for the xSE loader and editor executables and enabling the 'Run this program in compatibility mode for:' option from the 'Compatibility' tab and set the option to 'Windows 7 Service Pack 1' ");
		}

		char NativeProgramFilesFolder[MAX_PATH] = {0};
		ExpandEnvironmentStrings("%ProgramW6432%", NativeProgramFilesFolder, ARRAYSIZE(NativeProgramFilesFolder));
		const char* ProgFilesSubstring = strstr(APPPath, NativeProgramFilesFolder);
		IFileStream DirectoryCheckOverrideFile;
		bool DirectoyCheckOverride = DirectoryCheckOverrideFile.Open(std::string(APPPath + std::string("\\BGSEE_DirectoryCheckOverride")).c_str());

		if (DirectoyCheckOverride == false &&
			ProgFilesSubstring != nullptr && ProgFilesSubstring == APPPath &&
			OSInfo.dwMajorVersion > 5)
		{
			BGSEECONSOLE_MESSAGE("Editor/game is installed to the Program Files directory - An unprotected directory like 'C:\\Games\\' is required");
			return false;
		}
		else
		{
			if (DirectoyCheckOverride)
				BGSEECONSOLE_MESSAGE("WARNING - Protected Directory Check was overridden!");

			BOOL IsAdmin = FALSE;
			SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
			PSID AdministratorsGroup;

			IsAdmin = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
											0, 0, 0, 0, 0, 0, &AdministratorsGroup);

			if (IsAdmin)
			{
				if (CheckTokenMembership( nullptr, AdministratorsGroup, &IsAdmin) == FALSE)
					IsAdmin = FALSE;

				FreeSid(AdministratorsGroup);
			}

			if (IsAdmin == FALSE)
			{
				BGSEECONSOLE_MESSAGE("Editor isn't running with elevated privileges - It must be executed through a user account with administrator privileges");
				BGSEECONSOLE_MESSAGE("This can be done by opening the 'File Properties' dialog for the xSE loader and editor executables and enabling the 'Run this program as an administrator' option from the 'Compatibility' tab");

				return false;
			}
		}

		if (SECurrentVersion < SEMinVersion)
		{
			BGSEECONSOLE_MESSAGE("Script Extender version too old - v%d or greater required", SEMinVersion);
			return false;
		}
		else if (EditorCurrentVersion != EditorSupportedVersion)
		{
			BGSEECONSOLE_MESSAGE("Editor version mismatch - v%d.%d.%d required", (EditorSupportedVersion >> 24) & 0xFF,
																			(EditorSupportedVersion >> 16) & 0xFF,
																			(EditorSupportedVersion >> 4) & 0xFFF);
			return false;
		}

		bool HasDotNet = false;
		ICLRMetaHost* MetaHost = nullptr;

		if (SUCCEEDED(CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost,(LPVOID*)&MetaHost)))
		{
			std::wstring VersionString(SME::StringHelpers::FormatWideString(DotNETFrameworkVersionString));
			std::wstring ConfigName(SME::StringHelpers::FormatWideString("%s\\%s.config", APPPath, BGSEEMAIN->ParentEditorGetLongName()));

			ICLRRuntimeInfo* RequiredRuntime = nullptr;
			if (SUCCEEDED(MetaHost->GetRuntime(VersionString.c_str(), IID_ICLRRuntimeInfo, (LPVOID*)&RequiredRuntime)))
			{
				DWORD StartupFlags = STARTUP_LOADER_OPTIMIZATION_MASK|STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN;
				if (EnableCLRMemoryProfiling)
					StartupFlags |= STARTUP_SERVER_GC;	// for memory profiling, concurrent GC limits the ability of profilers
				else
					StartupFlags |= STARTUP_CONCURRENT_GC;

				if (SUCCEEDED(RequiredRuntime->SetDefaultStartupFlags(StartupFlags, ConfigName.c_str())) &&
					SUCCEEDED(RequiredRuntime->BindAsLegacyV2Runtime()))
				{
					ICLRRuntimeHost* CLRHost = nullptr;
					if (SUCCEEDED(RequiredRuntime->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (LPVOID*)&CLRHost)))
					{
						HasDotNet = true;
					}
				}
			}
		}

		if (HasDotNet == false)
		{
			BGSEECONSOLE_MESSAGE(".NET Framework too old/not installed/had trouble initializing - %s (Full & Client Profile) or greater required",
								DotNETFrameworkVersionString);
			return false;
		}

		HRESULT COMLibInitState = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		if (COMLibInitState != S_OK)				// ensure the main thread's COM apartment state is set to STA so that common controls work correctly
		{
			BGSEECONSOLE_MESSAGE("COM apartment state couldn't be set to STA - Result = %d", COMLibInitState);
			return false;
		}

		INITCOMMONCONTROLSEX icex = {0};
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC  = ICC_LISTVIEW_CLASSES;
		if (InitCommonControlsEx(&icex) == FALSE)				// ensure that the common control DLL is loaded
		{
			BGSEECONSOLE_MESSAGE("Couldn't initialize common controls");
			return false;
		}

		IFileStream ENBWrapper;
		if (ENBWrapper.Open(std::string(APPPath + std::string("\\d3d9.dll")).c_str()))
		{
			BGSEECONSOLE_MESSAGE("Unsupported ENBSeries/D3D wrapper based mod detected");
			BGSEECONSOLE_MESSAGE("Remove the \"d3d9.dll\" file from the root Oblivion directory. Alternatively, the file can be renamed before launching the editor (it must remain renamed until the editor is closed)");
			return false;
		}

		SME::MersenneTwister::init_genrand(GetTickCount());

		BGSEECONSOLE->Exdent();

		return true;
	}

	Main::DefaultInitCallback::~DefaultInitCallback()
	{
		;//
	}

	Main::DefaultDeinitCallback::DefaultDeinitCallback( Main* Parent ) :
		DaemonCallback()
	{
		ParentInstance = Parent;
	}

	Main::DefaultDeinitCallback::~DefaultDeinitCallback()
	{
		;//
	}

	bool Main::DefaultDeinitCallback::Handle(void* Parameter)
	{
		BGSEECONSOLE_MESSAGE("Deinitializing UI Manager");
		BGSEECONSOLE->Indent();
		delete BGSEEUI;
		BGSEECONSOLE->Exdent();

		CoUninitialize();
		return true;
	}

	Main*			Main::Singleton = nullptr;

	static const char*	kXSEShortName[] =					// key = ParentEditorID
	{
		"xSE",
		"OBSE",
		"SKSE"
	};

	static const char*	kParentEditorLongName[] =
	{
		"BGSEditor",
		"TESConstructionSet",
		"CreationKit"
	};

	static const char*	kParentEditorShortName[] =
	{
		"BGSE",
		"CS",
		"CK"
	};

	Main::~Main()
	{
		ExtenderDaemon->RegisterDeinitCallback(new DefaultDeinitCallback(this));

		BGSEECONSOLE->Pad(2);
		BGSEECONSOLE_MESSAGE("Deinitializing %s ...", ExtenderLongName.c_str());
		BGSEECONSOLE->Indent();
		ExtenderDaemon->ExecuteDeinitCallbacks();
		BGSEECONSOLE->Exdent();
		BGSEECONSOLE_MESSAGE("%s Deinitialized!", ExtenderLongName.c_str());

		SAFEDELETE(ExtenderConsole);
		SAFEDELETE(ExtenderDaemon);
		SAFEDELETE(ExtenderINIManager);

		Singleton = nullptr;

		if (CrashRptSupport)
		{
			crUninstall();
		}

		ExitProcess(0);
	}

	Main::Main()
	{
		ExtenderLongName = "<shadeMe's Awesome Extender #26942385.98979>";
		FORMAT_STR(ExtenderShortName, "SMAE");
		ExtenderReleaseName = "Lemon Tea";
		ExtenderVersion = 0x57ADE00E;
		ExtenderModuleHandle = nullptr;

		ParentEditorID = kExtenderParentEditor_Unknown;
		ParentEditorName = "Editor";
		ParentEditorSupportedVersion = 0x00000000;
		GameDirectoryPath = "NiBollocksNode";
		ExtenderDLLPath = "SMAE.dll";
		ExtenderINIPath = "SMAE.ini";
		ExtenderComponentDLLPath = "Meh";

		ScriptExtenderPluginHandle = 0xFFFFFFFF;
		ScriptExtenderCurrentVersion = 0x0;

		ExtenderINIManager = nullptr;
		ExtenderConsole = nullptr;
		ExtenderDaemon = nullptr;
		CrashRptSupport = false;

		Initialized = false;
	}

	__declspec(noinline) Main* Main::GetSingleton()
	{
		if (Singleton == nullptr)
			Singleton = new Main();

		return Singleton;
	}

	bool Main::Initialize( const char* LongName, const char* DisplayName, const char* ShortName, const char* ReleaseName, UInt32 Version,
								UInt8 EditorID, UInt32 EditorSupportedVersion, UInt32 EditorCurrentVersion, const char* APPPath,
								UInt32 SEPluginHandle, UInt32 SEMinimumVersion,
								UInt32 SECurrentVersion, INISettingDepotT& INISettings,
								const char* DotNETFrameworkVersion,	bool CLRMemoryProfiling, bool WaitForDebugger,
								bool CrashRptSupport)
	{
		if (Initialized)
			return false;

		SME_ASSERT(LongName && ShortName &&	ReleaseName && APPPath);
		SME_ASSERT(EditorID != kExtenderParentEditor_Unknown && EditorID < kExtenderParentEditor__MAX);
		SME_ASSERT(SEPluginHandle != 0xFFFFFFFF && DotNETFrameworkVersion);

		ExtenderLongName = LongName;
		FORMAT_STR(ExtenderShortName, "%s", ShortName);
		ExtenderReleaseName = ReleaseName;
		ExtenderVersion = Version;
		if (DisplayName == nullptr)
			ExtenderDisplayName = ExtenderLongName;
		else
			ExtenderDisplayName = DisplayName;

		ParentEditorID = EditorID;
		ParentEditorName = kParentEditorLongName[(int)EditorID];
		ParentEditorSupportedVersion = EditorSupportedVersion;

		GameDirectoryPath = APPPath;
		ExtenderDLLPath = std::string(APPPath) + "Data\\" + std::string(kXSEShortName[(int)EditorID]) + "\\Plugins\\" + ExtenderLongName + ".dll";
		ExtenderINIPath = std::string(APPPath) + "Data\\" + std::string(kXSEShortName[(int)EditorID]) + "\\Plugins\\" + ExtenderLongName + ".ini";
		ExtenderComponentDLLPath = std::string(APPPath) + "Data\\" + std::string(kXSEShortName[(int)EditorID]) + "\\Plugins\\" + ExtenderShortName + "\\";

		ScriptExtenderPluginHandle = SEPluginHandle;
		ScriptExtenderCurrentVersion = SECurrentVersion;

		ExtenderModuleHandle = (HINSTANCE)GetModuleHandle(ExtenderDLLPath.c_str());

		script::CodaScriptBackgrounder::RegisterINISettings(INISettings);
		script::CodaScriptExecutive::RegisterINISettings(INISettings);
		Console::RegisterINISettings(INISettings);

		ExtenderINIManager = new INIManager();
		ExtenderINIManager->Initialize(ExtenderINIPath.c_str(), (void*)&INISettings);

		ExtenderConsole = new Console((std::string(APPPath + std::string(LongName) + ".log")).c_str());
		ExtenderDaemon = new Daemon();

		DefaultInitCallback* InitCallback = new DefaultInitCallback();
		InitCallback->DisplayName = ExtenderDisplayName.c_str();
		InitCallback->ReleaseName = ExtenderReleaseName.c_str();
		InitCallback->APPPath = GameDirectoryPath.c_str();
		InitCallback->ExtenderVersion = ExtenderVersion;
		InitCallback->ModuleHandle = ExtenderModuleHandle;
		InitCallback->EditorSupportedVersion = EditorSupportedVersion;
		InitCallback->EditorCurrentVersion = EditorCurrentVersion;
		InitCallback->SEMinVersion = SEMinimumVersion;
		InitCallback->SECurrentVersion = SECurrentVersion;
		InitCallback->DotNETFrameworkVersionString = DotNETFrameworkVersion;
		InitCallback->EnableCLRMemoryProfiling = CLRMemoryProfiling;
		InitCallback->WaitForDebuggerOnStartup = WaitForDebugger;
		ExtenderDaemon->RegisterInitCallback(Daemon::kInitCallback_Query, InitCallback);

		this->CrashRptSupport = CrashRptSupport;
		Initialized = true;

		if (CrashRptSupport)
		{
			CR_INSTALL_INFO CrashRptData = {0};
			CrashRptData.cb = sizeof(CR_INSTALL_INFO);
			CrashRptData.pszAppName = nullptr;
			CrashRptData.pszAppVersion = nullptr;
			CrashRptData.pszEmailSubject = nullptr;
			CrashRptData.pszEmailTo = "shademe.here+bgsee@gmail.com";
			CrashRptData.pszUrl = nullptr;
			CrashRptData.pszPrivacyPolicyURL = nullptr;
			CrashRptData.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;

			CrashRptData.uPriorities[CR_SMAPI] = 1;
			CrashRptData.uPriorities[CR_HTTP] = CR_NEGATIVE_PRIORITY;
			CrashRptData.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY;
			CrashRptData.dwFlags |= CR_INST_HTTP_BINARY_ENCODING|CR_INST_SHOW_ADDITIONAL_INFO_FIELDS|CR_INST_ALLOW_ATTACH_MORE_FILES;
//			CrashRptData.dwFlags |= CR_INST_DONT_SEND_REPORT|CR_INST_STORE_ZIP_ARCHIVES;
			CrashRptData.uMiniDumpType = (MINIDUMP_TYPE)(MiniDumpNormal|
														MiniDumpWithIndirectlyReferencedMemory|
														MiniDumpScanMemory|
														MiniDumpWithThreadInfo|
														MiniDumpWithProcessThreadData|
														MiniDumpWithUnloadedModules|
														MiniDumpWithHandleData|
// 														MiniDumpWithDataSegs|
														MiniDumpWithFullMemoryInfo);
			CrashRptData.pszErrorReportSaveDir = GameDirectoryPath.c_str();

			if (crInstall(&CrashRptData) || crSetCrashCallback(Main::CrashCallback, this))
			{
				TCHAR Buffer[0x200] = {0};
				crGetLastErrorMsg(Buffer, sizeof(Buffer));

				MessageBox(nullptr,
					"Failed to initialize CrashRpt!\n\nCheck the logs for more information.",
					"Bethesda Game Studios Editor Extender",
					MB_TASKMODAL|MB_SETFOREGROUND|MB_ICONERROR|MB_OK);

				BGSEECONSOLE_MESSAGE("CrashRpt failed to initialize; Error Message: %s", Buffer);
				Initialized = false;
			}
			else
			{
				crAddFile2(GetConsole()->GetLogPath(),
					nullptr, "BGSEE Debug Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2(GetINIPath(),
					nullptr, "BGSEE INI File", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2((std::string(GameDirectoryPath + "\\" + std::string(kXSEShortName[(int)EditorID]) + ".log")).c_str(),
					nullptr, "Script Extender Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2((std::string(GameDirectoryPath + "\\" + std::string(kXSEShortName[(int)EditorID]) + "_editor.log")).c_str(),
					nullptr, "Script Extender Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2((std::string(GameDirectoryPath + "\\" + std::string(kXSEShortName[(int)EditorID]) + "_loader.log")).c_str(),
					nullptr, "Script Extender Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2((std::string(GameDirectoryPath + "\\" + std::string(kXSEShortName[(int)EditorID]) + "_steam_loader.log")).c_str(),
					nullptr, "Script Extender Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddScreenshot2(CR_AS_PROCESS_WINDOWS, 0);
			}
		}

		return Initialized;
	}

	const char* Main::ExtenderGetLongName( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderLongName.c_str();
	}

	const char* Main::ExtenderGetShortName( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderShortName;
	}

	const char* Main::ExtenderGetReleaseName( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderReleaseName.c_str();
	}

	UInt32 Main::ExtenderGetVersion( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderVersion;
	}

	const char* Main::ExtenderGetSEName( void ) const
	{
		SME_ASSERT(Initialized);
		return kXSEShortName[(int)ParentEditorID];
	}

	HINSTANCE Main::GetExtenderHandle( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderModuleHandle;
	}

	const char* Main::GetAPPPath( void ) const
	{
		SME_ASSERT(Initialized);
		return GameDirectoryPath.c_str();
	}

	const char* Main::GetDLLPath( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderDLLPath.c_str();
	}

	const char* Main::GetINIPath( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderINIPath.c_str();
	}

	const char* Main::GetComponentDLLPath( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderComponentDLLPath.c_str();
	}

	Console* Main::GetConsole( void ) const
	{
		SME_ASSERT(Initialized && ExtenderConsole);
		return ExtenderConsole;
	}

	Daemon* Main::GetDaemon( void ) const
	{
		SME_ASSERT(Initialized && ExtenderDaemon);
		return ExtenderDaemon;
	}

	const char* Main::ParentEditorGetLongName( void ) const
	{
		SME_ASSERT(Initialized && ParentEditorID);
		return kParentEditorLongName[ParentEditorID];
	}

	const char* Main::ParentEditorGetShortName( void ) const
	{
		SME_ASSERT(Initialized && ParentEditorID);
		return kParentEditorShortName[ParentEditorID];
	}

	UInt32 Main::ParentEditorGetVersion( void ) const
	{
		SME_ASSERT(Initialized);
		return ParentEditorSupportedVersion;
	}

	int CALLBACK Main::CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
	{
		// panic and toss grenades around
		Main* Instance = (Main*)pInfo->pUserParam;

		bool ResumeExecution = Instance->GetDaemon()->ExecuteCrashCallbacks(pInfo);

		if (ResumeExecution)
		{
			pInfo->bContinueExecution = TRUE;
			Instance->GetConsole()->LogMsg(Instance->ExtenderGetShortName(), "Sweeping a fatal crash under the ru... Shoo! Nothing to see here!");
			Instance->GetConsole()->Pad(2);
		}

		Instance->GetConsole()->FlushDebugLog();

		return CR_CB_DODEFAULT;
	}

	void Main::ShowPreferencesGUI( void )
	{
		ExtenderINIManager->ShowGUI(ExtenderModuleHandle, BGSEEUI->GetMainWindow());
	}

	bgsee::INIManagerSetterFunctor Main::INISetter( void )
	{
		SME_ASSERT(Initialized);

		return ExtenderINIManager;
	}

	bgsee::INIManagerGetterFunctor Main::INIGetter( void )
	{
		SME_ASSERT(Initialized);

		return ExtenderINIManager;
	}

	const char* Main::ExtenderGetDisplayName( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderDisplayName.c_str();
	}

	BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
	{
		switch (dwReason)
		{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}

		return TRUE;
	}
}