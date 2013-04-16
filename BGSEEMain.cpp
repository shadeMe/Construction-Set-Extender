#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "BGSEEUIManager.h"
#include "DetectSIMD.h"
#include "BGSEEScript\CodaVM.h"
#include "SME Sundries\INIEditGUI_Res.h"
#include "SME Sundries\VersionInfo.h"

namespace BGSEditorExtender
{
	BGSEEDaemon::BGSEEDaemon() :
		InitCallbacks(),
		DeinitCallbacks(),
		CrashHandlerCallbacks(),
		FullInitComplete(false)
	{
		;//
	}

	void BGSEEDaemon::WaitForDebugger(void)
	{
		BGSEECONSOLE->Indent();
		BGSEECONSOLE_MESSAGE("Waiting For Debugger...");

		while (IsDebuggerPresent() == FALSE)
			Sleep(100);

		BGSEECONSOLE_MESSAGE("Debugger Attached!");
		BGSEECONSOLE->Exdent();
	}

	void BGSEEDaemon::RegisterInitCallback( UInt8 CallbackType, BGSEEDaemonCallback* Callback )
	{
		SME_ASSERT(CallbackType < kInitCallback__MAX && Callback);
		InitCallbacks[CallbackType].push_back(Callback);
	}

	void BGSEEDaemon::RegisterDeinitCallback( BGSEEDaemonCallback* Callback )
	{
		SME_ASSERT(Callback);
		DeinitCallbacks.push_back(Callback);
	}

	void BGSEEDaemon::RegisterCrashCallback( BGSEEDaemonCallback* Callback )
	{
		SME_ASSERT(Callback);
		CrashHandlerCallbacks.push_back(Callback);
	}

	bool BGSEEDaemon::ExecuteInitCallbacks( UInt8 CallbackType )
	{
		SME_ASSERT(CallbackType < kInitCallback__MAX);

		bool Result = true;

		for (DaemonCallbackListT::const_iterator Itr = InitCallbacks[CallbackType].begin(); Itr != InitCallbacks[CallbackType].end(); Itr++)
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

	bool BGSEEDaemon::ExecuteDeinitCallbacks( void )
	{
		for (DaemonCallbackListT::const_iterator Itr = DeinitCallbacks.begin(); Itr != DeinitCallbacks.end(); Itr++)
			(*Itr)->Handle();

		return true;
	}

	bool BGSEEDaemon::ExecuteCrashCallbacks( CR_CRASH_CALLBACK_INFO* Data )
	{
		bool Result = false;

		for (DaemonCallbackListT::const_iterator Itr = CrashHandlerCallbacks.begin(); Itr != CrashHandlerCallbacks.end(); Itr++)
		{
			if ((*Itr)->Handle(Data) && Result == false)
				Result = true;
		}

		return Result;
	}

	BGSEEDaemon::~BGSEEDaemon()
	{
		ReleaseCallbacks(InitCallbacks[kInitCallback_Query]);
		ReleaseCallbacks(InitCallbacks[kInitCallback_Load]);
		ReleaseCallbacks(InitCallbacks[kInitCallback_PostMainWindowInit]);
		ReleaseCallbacks(InitCallbacks[kInitCallback_Epilog]);
		ReleaseCallbacks(DeinitCallbacks);
		ReleaseCallbacks(CrashHandlerCallbacks);
	}

	void BGSEEDaemon::ReleaseCallbacks( DaemonCallbackListT& CallbackList )
	{
		for (DaemonCallbackListT::const_iterator Itr = CallbackList.begin(); Itr != CallbackList.end(); Itr++)
			delete (*Itr);

		CallbackList.clear();
	}

	bool BGSEEDaemon::GetFullInitComplete( void ) const
	{
		return FullInitComplete;
	}

	const char*		BGSEEMain::BGSEEINIManager::kSectionPrefix = "BGSEE::";

	BGSEEINIManagerSetterFunctor::BGSEEINIManagerSetterFunctor( SME::INI::INIManager* Parent ) :
			ParentManager(Parent)
	{
		;//
	}

	void BGSEEINIManagerSetterFunctor::operator()( const char* Section, const char* Value )
	{
		SME_ASSERT(ParentManager);

		ParentManager->DirectWrite(Section, Value);
	}

	void BGSEEINIManagerSetterFunctor::operator()( const char* Key, const char* Section, const char* Value )
	{
		SME_ASSERT(ParentManager);

		ParentManager->DirectWrite(Key, Section, Value);
	}

	BGSEEINIManagerGetterFunctor::BGSEEINIManagerGetterFunctor( SME::INI::INIManager* Parent ) :
			ParentManager(Parent)
	{
		;//
	}

	int BGSEEINIManagerGetterFunctor::operator()( const char* Key, const char* Section, const char* Default, char* OutBuffer, UInt32 Size )
	{
		SME_ASSERT(ParentManager);

		return ParentManager->DirectRead(Key, Section, Default, OutBuffer, Size);
	}

	int BGSEEINIManagerGetterFunctor::operator()( const char* Section, char* OutBuffer, UInt32 Size )
	{
		SME_ASSERT(ParentManager);

		return ParentManager->DirectRead(Section, OutBuffer, Size);
	}
	
	BGSEEMain::BGSEEINIManager::BGSEEINIManager() :
		GUI()
	{
		;//
	}

	BGSEEMain::BGSEEINIManager::~BGSEEINIManager()
	{
		;//
	}

	bool BGSEEMain::BGSEEINIManager::RegisterSetting( INISetting* Setting, bool AutoLoad /*= true*/, bool Dynamic /*= false*/ )
	{
		SME_ASSERT(Setting);

		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Setting->GetSection());
		SetSettingSection(Setting, Buffer);

		return SME::INI::INIManager::RegisterSetting(Setting, AutoLoad, Dynamic);
	}

	SME::INI::INISetting* BGSEEMain::BGSEEINIManager::FetchSetting( const char* Key, const char* Section, bool Dynamic /*= false*/ )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::FetchSetting(Key, Buffer, Dynamic);
	}

	void BGSEEMain::BGSEEINIManager::Initialize(const char* INIPath, void* Paramenter)
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

	void BGSEEMain::BGSEEINIManager::ShowGUI( HINSTANCE ResourceInstance, HWND Parent )
	{
		GUI.InitializeGUI(ResourceInstance, Parent, this);
	}

	int BGSEEMain::BGSEEINIManager::DirectRead( const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::DirectRead(Setting, Buffer, Default, OutBuffer, Size);
	}

	int BGSEEMain::BGSEEINIManager::DirectRead( const char* Section, char* OutBuffer, UInt32 Size )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::DirectRead(Buffer, OutBuffer, Size);
	}

	bool BGSEEMain::BGSEEINIManager::DirectWrite( const char* Setting, const char* Section, const char* Value )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::DirectWrite(Setting, Buffer, Value);
	}

	bool BGSEEMain::BGSEEINIManager::DirectWrite( const char* Section, const char* Value )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::DirectWrite(Buffer, Value);
	}

	void BGSEEReleaseNameTable::RegisterRelease( UInt16 Major, UInt8 Minor, const char* Name )
	{
		SME_ASSERT(Name);

		UInt32 Version = (((Major & 0xFF) << 24) | ((Minor & 0xFF) << 16) | 0xFFFF);
		if (Table.count(Version) == 0)
		{
			Table.insert(std::make_pair(Version, Name));
		}
	}

	BGSEEReleaseNameTable::BGSEEReleaseNameTable() :
		Table()
	{
		;//
	}

	BGSEEReleaseNameTable::~BGSEEReleaseNameTable()
	{
		;//
	}

	const char* BGSEEReleaseNameTable::LookupRelease( UInt16 Major, UInt8 Minor )
	{
		UInt32 Version = (((Major & 0xFF) << 24) | ((Minor & 0xFF) << 16) | 0xFFFF);

		if (Table.count(Version))
			return Table[Version].c_str();
		else
			return NULL;
	}

	BGSEEMain::DefaultInitCallback::DefaultInitCallback() :
		BGSEEDaemonCallback()
	{
		LongName = NULL;
		ReleaseName = NULL;
		APPPath = NULL;
		ExtenderVersion = 0x0;
		ModuleHandle = 0x0;
		EditorSupportedVersion = EditorCurrentVersion = 0x0;
		SEMinVersion = SECurrentVersion = 0x0;
		DotNETFrameworkVersionString = "v4.0.30319";
		EnableCLRMemoryProfiling = false;
		WaitForDebuggerOnStartup = false;
	}

	bool BGSEEMain::DefaultInitCallback::Handle(void* Parameter)
	{
#ifdef NDEBUG
		const char* ReleaseMode = "";
#else
		const char* ReleaseMode = " DEBUG";
#endif
		BGSEECONSOLE_MESSAGE("%s \"%s\" v%d.%d.%d.%d%s Initializing...",
							LongName,
							ReleaseName,
							SME_VERSION_MAJOR(ExtenderVersion),
							SME_VERSION_MINOR(ExtenderVersion),
							SME_VERSION_REVISION(ExtenderVersion),
							SME_VERSION_BUILD(ExtenderVersion),
							ReleaseMode);

		BGSEECONSOLE->Indent();

		if (WaitForDebuggerOnStartup)
		{
			BGSEEDaemon::WaitForDebugger();
		}

		if (ModuleHandle == NULL)
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

		if (OSInfo.dwMajorVersion < 5 || (OSInfo.dwMajorVersion == 5 && OSInfo.dwMinorVersion == 1 && OSInfo.wServicePackMajor < 3))
		{
			BGSEECONSOLE_MESSAGE("OS version too old - Windows XP SP3 or greater required");
			return false;
		}
		else if (OSInfo.dwMajorVersion == 5 && OSInfo.dwMinorVersion == 2 && SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		{
			BGSEECONSOLE_MESSAGE("Windows XP 64-bit is not officially supported - Expect general weirdness such as collapsing time vortexes and code cannibalism");
		}
		else if (OSInfo.dwMajorVersion >= 6 && OSInfo.dwMinorVersion > 1)
		{
			BGSEECONSOLE_MESSAGE("Your current version of Windows is not officially supported - Expect general weirdness such as collapsing time vortexes and code cannibalism");
			BGSEECONSOLE_MESSAGE("You may attempt to run the editor in Windows' Compatibility Mode. This can be done by opening the 'File Properties' dialog for the xSE loader and editor executables and enabling the 'Run this program in compatibility mode for:' option from the 'Compatibility' tab and set the option to 'Windows 7 Service Pack 1'. ");
		}

		char NativeProgramFilesFolder[MAX_PATH] = {0};
		ExpandEnvironmentStrings("%ProgramW6432%", NativeProgramFilesFolder, ARRAYSIZE(NativeProgramFilesFolder));
		const char* ProgFilesSubstring = strstr(APPPath, NativeProgramFilesFolder);

		if (ProgFilesSubstring != NULL && ProgFilesSubstring == APPPath && OSInfo.dwMajorVersion > 5)
		{
			BGSEECONSOLE_MESSAGE("Editor/game is installed to the Program Files directory - An unprotected directory like 'C:\\Games\\' is required");
			return false;
		}
		else
		{
			BOOL IsAdmin = FALSE;
			SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
			PSID AdministratorsGroup;

			IsAdmin = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
											0, 0, 0, 0, 0, 0, &AdministratorsGroup);

			if (IsAdmin)
			{
				if (CheckTokenMembership( NULL, AdministratorsGroup, &IsAdmin) == FALSE)
				{
					IsAdmin = FALSE;
				}

				FreeSid(AdministratorsGroup);
			}

			if (IsAdmin == FALSE)
			{
				BGSEECONSOLE_MESSAGE("Editor isn't running with elevated privileges - It must be executed through a user account with administrator privileges.");
				BGSEECONSOLE_MESSAGE("This can be done by opening the 'File Properties' dialog for the xSE loader and editor executables and enabling the 'Run this program as an administrator' option from the 'Compatibility' tab.");
				return false;
			}
		}

		if(SECurrentVersion < SEMinVersion)
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
		ICLRMetaHost* MetaHost = NULL;

		if (SUCCEEDED(CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost,(LPVOID*)&MetaHost)))
		{
			std::wstring VersionString(SME::StringHelpers::FormatWideString(DotNETFrameworkVersionString));
			std::wstring ConfigName(SME::StringHelpers::FormatWideString("%s\\%s.config", APPPath, BGSEEMAIN->ParentEditorGetLongName()));

			ICLRRuntimeInfo* RequiredRuntime = NULL;
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
					ICLRRuntimeHost* CLRHost = NULL;
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

		HRESULT COMLibInitState = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
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
		BGSEECONSOLE->Exdent();

		return true;
	}

	BGSEEMain::DefaultInitCallback::~DefaultInitCallback()
	{
		;//
	}

	BGSEEMain::DefaultDeinitCallback::DefaultDeinitCallback( BGSEEMain* Parent ) :
		BGSEEDaemonCallback()
	{
		ParentInstance = Parent;
	}

	BGSEEMain::DefaultDeinitCallback::~DefaultDeinitCallback()
	{
		;//
	}

	bool BGSEEMain::DefaultDeinitCallback::Handle(void* Parameter)
	{
		BGSEECONSOLE_MESSAGE("Deinitializing UI Manager");
		BGSEECONSOLE->Indent();
		delete BGSEEUI;
		BGSEECONSOLE->Exdent();

		CoUninitialize();
		return true;
	}

	BGSEEMain*			BGSEEMain::Singleton = NULL;

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

	BGSEEMain::~BGSEEMain()
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

		Singleton = NULL;

		if (CrashRptSupport)
		{
			crUninstall();
		}

		ExitProcess(0);
	}

	BGSEEMain::BGSEEMain()
	{
		ExtenderLongName = "<shadeMe's Awesome Extender #26942385.98979>";
		FORMAT_STR(ExtenderShortName, "SMAE");
		ExtenderReleaseName = "Lemon Tea";
		ExtenderVersion = 0x57ADE00E;
		ExtenderModuleHandle = NULL;

		ParentEditorID = kExtenderParentEditor_Unknown;
		ParentEditorName = "Editor";
		ParentEditorSupportedVersion = 0x00000000;
		GameDirectoryPath = "NiBollocksNode";
		ExtenderDLLPath = "SMAE.dll";
		ExtenderINIPath = "SMAE.ini";
		ExtenderComponentDLLPath = "Meh";

		ScriptExtenderPluginHandle = 0xFFFFFFFF;
		ScriptExtenderCurrentVersion = 0x0;

		ExtenderINIManager = NULL;
		ExtenderConsole = NULL;
		ExtenderDaemon = NULL;
		CrashRptSupport = false;

		Initialized = false;
	}

	BGSEEMain* BGSEEMain::GetSingleton()
	{
		if (Singleton == NULL)
			Singleton = new BGSEEMain();

		return Singleton;
	}

	bool BGSEEMain::Initialize( const char* LongName, const char* ShortName, const char* ReleaseName, UInt32 Version,
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

		Initialized = true;

		ExtenderLongName = LongName;
		FORMAT_STR(ExtenderShortName, "%s", ShortName);
		ExtenderReleaseName = ReleaseName;
		ExtenderVersion = Version;

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

		BGSEEScript::CodaScriptBackgrounder::RegisterINISettings(INISettings);
		BGSEEScript::CodaScriptExecutive::RegisterINISettings(INISettings);
		BGSEEConsole::RegisterINISettings(INISettings);

		ExtenderINIManager = new BGSEEINIManager();
		ExtenderINIManager->Initialize(ExtenderINIPath.c_str(), (void*)&INISettings);

		ExtenderConsole = new BGSEEConsole((std::string(APPPath + std::string(LongName) + ".log")).c_str());
		ExtenderDaemon = new BGSEEDaemon();

		DefaultInitCallback* InitCallback = new DefaultInitCallback();
		InitCallback->LongName = ExtenderLongName.c_str();
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
		ExtenderDaemon->RegisterInitCallback(BGSEEDaemon::kInitCallback_Query, InitCallback);

		this->CrashRptSupport = CrashRptSupport;

		if (CrashRptSupport)
		{
			CR_INSTALL_INFO CrashRptData = {0};
			CrashRptData.cb = sizeof(CR_INSTALL_INFO);
			CrashRptData.pszAppName = NULL;
			CrashRptData.pszAppVersion = NULL;
			CrashRptData.pszEmailSubject = NULL;
			CrashRptData.pszEmailTo = "shademe.here+bgsee@gmail.com";
			CrashRptData.pszUrl = NULL;
			CrashRptData.pszPrivacyPolicyURL = NULL;
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

			if (crInstall(&CrashRptData) || crSetCrashCallback(BGSEEMain::CrashCallback, this))
			{
				TCHAR Buffer[0x200] = {0};
				crGetLastErrorMsg(Buffer, sizeof(Buffer));

				MessageBox(NULL,
					"Failed to initialize CrashRpt!\n\nCheck the logs for more information.",
					"Bethesda Game Studios Editor Extender",
					MB_TASKMODAL|MB_SETFOREGROUND|MB_ICONERROR|MB_OK);

				BGSEECONSOLE_MESSAGE("CrashRpt failed to initialize; Error Message: %s", Buffer);
			}
			else
			{
				crAddFile2(Console()->GetLogPath(),
					NULL, "BGSEE Debug Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2(GetINIPath(),
					NULL, "BGSEE INI File", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2((std::string(GameDirectoryPath + "\\" + std::string(kXSEShortName[(int)EditorID]) + ".log")).c_str(),
					NULL, "Script Extender Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2((std::string(GameDirectoryPath + "\\" + std::string(kXSEShortName[(int)EditorID]) + "_editor.log")).c_str(),
					NULL, "Script Extender Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2((std::string(GameDirectoryPath + "\\" + std::string(kXSEShortName[(int)EditorID]) + "_loader.log")).c_str(),
					NULL, "Script Extender Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddFile2((std::string(GameDirectoryPath + "\\" + std::string(kXSEShortName[(int)EditorID]) + "_steam_loader.log")).c_str(),
					NULL, "Script Extender Log", CR_AF_MISSING_FILE_OK | CR_AF_MAKE_FILE_COPY);

				crAddScreenshot2(CR_AS_PROCESS_WINDOWS, NULL);
			}
		}

		return Initialized;
	}

	const char* BGSEEMain::ExtenderGetLongName( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderLongName.c_str();
	}

	const char* BGSEEMain::ExtenderGetShortName( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderShortName;
	}

	const char* BGSEEMain::ExtenderGetReleaseName( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderReleaseName.c_str();
	}

	UInt32 BGSEEMain::ExtenderGetVersion( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderVersion;
	}

	const char* BGSEEMain::ExtenderGetSEName( void ) const
	{
		SME_ASSERT(Initialized);
		return kXSEShortName[(int)ParentEditorID];
	}

	HINSTANCE BGSEEMain::GetExtenderHandle( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderModuleHandle;
	}

	const char* BGSEEMain::GetAPPPath( void ) const
	{
		SME_ASSERT(Initialized);
		return GameDirectoryPath.c_str();
	}

	const char* BGSEEMain::GetDLLPath( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderDLLPath.c_str();
	}

	const char* BGSEEMain::GetINIPath( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderINIPath.c_str();
	}

	const char* BGSEEMain::GetComponentDLLPath( void ) const
	{
		SME_ASSERT(Initialized);
		return ExtenderComponentDLLPath.c_str();
	}

	BGSEEConsole* BGSEEMain::Console( void ) const
	{
		SME_ASSERT(Initialized && ExtenderConsole);
		return ExtenderConsole;
	}

	BGSEEDaemon* BGSEEMain::Daemon( void ) const
	{
		SME_ASSERT(Initialized && ExtenderDaemon);
		return ExtenderDaemon;
	}

	const char* BGSEEMain::ParentEditorGetLongName( void ) const
	{
		SME_ASSERT(Initialized && ParentEditorID);
		return kParentEditorLongName[ParentEditorID];
	}

	const char* BGSEEMain::ParentEditorGetShortName( void ) const
	{
		SME_ASSERT(Initialized && ParentEditorID);
		return kParentEditorShortName[ParentEditorID];
	}

	UInt32 BGSEEMain::ParentEditorGetVersion( void ) const
	{
		SME_ASSERT(Initialized);
		return ParentEditorSupportedVersion;
	}

	int CALLBACK BGSEEMain::CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
	{
		// panic and toss grenades around
		BGSEEMain* Instance = (BGSEEMain*)pInfo->pUserParam;

		bool ResumeExecution = Instance->Daemon()->ExecuteCrashCallbacks(pInfo);

		if (ResumeExecution)
		{
			pInfo->bContinueExecution = TRUE;
			Instance->Console()->LogMsg(Instance->ExtenderGetShortName(), "Sweeping a fatal crash under the ru... Shoo! Nothing to see here!");
			Instance->Console()->Pad(2);
		}

		Instance->Console()->FlushDebugLog();

		return CR_CB_DODEFAULT;
	}

	void BGSEEMain::ShowPreferencesGUI( void )
	{
		ExtenderINIManager->ShowGUI(ExtenderModuleHandle, BGSEEUI->GetMainWindow());
	}

	BGSEditorExtender::BGSEEINIManagerSetterFunctor BGSEEMain::INISetter( void )
	{
		SME_ASSERT(Initialized);

		return ExtenderINIManager;
	}

	BGSEditorExtender::BGSEEINIManagerGetterFunctor BGSEEMain::INIGetter( void )
	{
		SME_ASSERT(Initialized);

		return ExtenderINIManager;
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