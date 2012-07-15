#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "BGSEEUIManager.h"
#include "DetectSIMD.h"
#include "BGSEEScript\CodaVM.h"
#include "SME Sundries\INIEditGUI_Res.h"
#include "SME Sundries\VersionInfo.h"

#include <CrashRpt.h>

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

	void BGSEEDaemon::RegisterInitCallback( UInt8 CallbackType, BoolRFunctorBase* Callback )
	{
		SME_ASSERT(CallbackType < 3 && Callback);
		InitCallbacks[CallbackType].push_back(Callback);
	}

	void BGSEEDaemon::RegisterDeinitCallback( BoolRFunctorBase* Callback )
	{
		SME_ASSERT(Callback);
		DeinitCallbacks.push_back(Callback);
	}

	void BGSEEDaemon::RegisterCrashCallback( BoolRFunctorBase* Callback )
	{
		SME_ASSERT(Callback);
		CrashHandlerCallbacks.push_back(Callback);
	}

	bool BGSEEDaemon::ExecuteInitCallbacks( UInt8 CallbackType )
	{
		SME_ASSERT(CallbackType < 3);

		bool Result = true;

		for (DaemonCallbackListT::const_iterator Itr = InitCallbacks[CallbackType].begin(); Itr != InitCallbacks[CallbackType].end(); Itr++)
		{
			if ((*Itr)->operator()() == false)
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
			(*Itr)->operator()();

		return true;
	}

	bool BGSEEDaemon::ExecuteCrashCallbacks( void )
	{
		for (DaemonCallbackListT::const_iterator Itr = CrashHandlerCallbacks.begin(); Itr != CrashHandlerCallbacks.end(); Itr++)
			(*Itr)->operator()();

		return true;
	}

	BGSEEDaemon::~BGSEEDaemon()
	{
		ReleaseCallbacks(InitCallbacks[0]);
		ReleaseCallbacks(InitCallbacks[1]);
		ReleaseCallbacks(InitCallbacks[2]);
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

	void BGSEEINIManagerSetterFunctor::operator()( const char* Key, const char* Section, const char* Value, bool Direct )
	{
		SME_ASSERT(ParentManager);

		if (Direct)
			ParentManager->DirectWrite(Key, Section, Value);
		else
			ParentManager->FetchSetting(Key, Section)->SetValue(Value);
	}

	void BGSEEINIManagerSetterFunctor::operator()( const char* Section, const char* Value )
	{
		SME_ASSERT(ParentManager);

		ParentManager->DirectWrite(Section, Value);
	}

	BGSEEINIManagerGetterFunctor::BGSEEINIManagerGetterFunctor( SME::INI::INIManager* Parent ) :
			ParentManager(Parent)
	{
		;//
	}

	const char* BGSEEINIManagerGetterFunctor::operator()( const char* Key, const char* Section )
	{
		SME_ASSERT(ParentManager);

		return ParentManager->FetchSetting(Key, Section)->GetValueAsString();
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

	BGSEEINIManagerSettingFactory::BGSEEINIManagerSettingFactory( const char* Section ) :
		Section(Section),
		Settings()
	{
		;//
	}

	const BGSEEINIManagerSettingFactory::SettingData* BGSEEINIManagerSettingFactory::Lookup( const char* Key )
	{
		for (SettingListT::const_iterator Itr = Settings.begin(); Itr != Settings.end(); Itr++)
		{
			if (_stricmp((*Itr)->Key, Key))
				return *Itr;
		}

		return NULL;
	}

	const char* BGSEEINIManagerSettingFactory::Get( int Index, BGSEEINIManagerGetterFunctor& Getter )
	{
		if (Settings.size() <= Index)
			return NULL;
		else
			return Getter(Settings[Index]->Key, Section);
	}

	void BGSEEINIManagerSettingFactory::Set( int Index, BGSEEINIManagerSetterFunctor& Setter, const char* Value, ... )
	{
		if (Settings.size() <= Index)
			return;

		char Buffer[0x100] = {0};

		va_list Args;
		va_start(Args, Value);
		vsprintf_s(Buffer, sizeof(Buffer), Value, Args);
		va_end(Args);

		Setter(Settings[Index]->Key, Section, Buffer);
	}

	BGSEEMain::BGSEEINIManager::BGSEEINIManager() : GUI()
	{
		;//
	}

	BGSEEMain::BGSEEINIManager::~BGSEEINIManager()
	{
		;//
	}

	bool BGSEEMain::BGSEEINIManager::RegisterSetting( const char* Key, const char* Section, const char* DefaultValue, const char* Description )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		return SME::INI::INIManager::RegisterSetting(Key, Buffer, DefaultValue, Description);
	}

	SME::INI::INISetting* BGSEEMain::BGSEEINIManager::FetchSetting( const char* Key, const char* Section )
	{
		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s%s", kSectionPrefix, Section);

		SME::INI::INISetting* Result = SME::INI::INIManager::FetchSetting(Key, Buffer);

		return Result;
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

		char Buffer[0x200] = {0};
		SettingFactoryListT* FactoryList = (SettingFactoryListT*)Paramenter;

		for (SettingFactoryListT::const_iterator Itr = FactoryList->begin(); Itr != FactoryList->end(); Itr++)
		{
			for (BGSEEINIManagerSettingFactory::SettingListT::const_iterator ItrX = (*Itr)->Settings.begin(); ItrX != (*Itr)->Settings.end(); ItrX++)
			{
				const BGSEEINIManagerSettingFactory::SettingData* Current = *ItrX;
				RegisterSetting(Current->Key, (*Itr)->Section, Current->DefaultValue, Current->Description);
			}
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

	void BGSEEReleaseNameTable::RegisterRelease( UInt8 Major, UInt8 Minor, UInt8 Revision, const char* Name )
	{
		SME_ASSERT(Name);

		UInt32 Version = (((Major & 0xFF) << 24) | ((Minor & 0xFF) << 16) | (Revision & 0xFFFF));
		if (Table.count(Version) == 0)
		{
			Table.insert(std::make_pair<UInt32, std::string>(Version, Name));
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

	const char* BGSEEReleaseNameTable::LookupRelease( UInt8 Major, UInt8 Minor, UInt8 Revision )
	{
		UInt32 Version = (((Major & 0xFF) << 24) | ((Minor & 0xFF) << 16) | (Revision & 0xFFFF));

		if (Table.count(Version))
			return Table[Version].c_str();
		else
			return NULL;
	}

	BGSEEMain::DefaultInitCallback::DefaultInitCallback() :
		BoolRFunctorBase()
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

	bool BGSEEMain::DefaultInitCallback::operator()()
	{
#ifdef NDEBUG
		const char* ReleaseMode = "";
#else
		const char* ReleaseMode = " DEBUG";
#endif
		BGSEECONSOLE_MESSAGE("%s \"%s\" v%d.%d.%d%s Initializing ...",
							LongName,
							ReleaseName,
							SME_VERSION_MAJOR(ExtenderVersion),
							SME_VERSION_MINOR(ExtenderVersion),
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

		char NativeProgramFilesFolder[MAX_PATH] = {0};
		ExpandEnvironmentStrings("%ProgramW6432%", NativeProgramFilesFolder, ARRAYSIZE(NativeProgramFilesFolder));
		const char* ProgFilesSubstring = strstr(APPPath, NativeProgramFilesFolder);

		if (ProgFilesSubstring != NULL && ProgFilesSubstring == APPPath && OSInfo.dwMajorVersion > 5)
		{
			BGSEECONSOLE_MESSAGE("Editor/game is installed to the Program Files directory - An unprotected directory like 'C:\\Games\\' is recommended");
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
				BGSEECONSOLE_MESSAGE("Editor isn't running with elevated privileges - The CS(E) must be run as an administrator.");
				BGSEECONSOLE_MESSAGE("This can be done by opening the 'File Properties' dialog for the OBSE loader and CS executables and enabling the 'Run this program as an administrator' option from the 'Compatibility' tab.");
				return false;
			}
		}

		if(SECurrentVersion < SEMinVersion)
		{
			BGSEECONSOLE_MESSAGE("Script Extender version too old - v%d.%d.%d or greater required", (SEMinVersion >> 24) & 0xFF,
																								(SEMinVersion >> 16) & 0xFF,
																								(SEMinVersion >> 4) & 0xFFF);
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
		BoolRFunctorBase()
	{
		ParentInstance = Parent;
	}

	BGSEEMain::DefaultDeinitCallback::~DefaultDeinitCallback()
	{
		;//
	}

	bool BGSEEMain::DefaultDeinitCallback::operator()()
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
								UInt32 SECurrentVersion, SettingFactoryListT& INISettingFactoryList,
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

		INISettingFactoryList.push_back(BGSEEScript::CodaScriptBackgrounder::GetINIFactory());
		INISettingFactoryList.push_back(BGSEEScript::CodaScriptExecutive::GetINIFactory());
		INISettingFactoryList.push_back(BGSEEConsole::GetINIFactory());

		ExtenderINIManager = new BGSEEINIManager();
		ExtenderINIManager->Initialize(ExtenderINIPath.c_str(), (void*)&INISettingFactoryList);

		ExtenderConsole = new BGSEEConsole((std::string(APPPath + std::string(LongName) + ".log")).c_str(), ExtenderINIManager, ExtenderINIManager);
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
			CrashRptData.pszEmailTo = NULL;
			CrashRptData.pszUrl = NULL;
			CrashRptData.pfnCrashCallback = BGSEEMain::CrashCallback;
			CrashRptData.dwFlags |= CR_INST_SEH_EXCEPTION_HANDLER|
									CR_INST_PURE_CALL_HANDLER|
									CR_INST_NEW_OPERATOR_ERROR_HANDLER|
									CR_INST_SECURITY_ERROR_HANDLER|
									CR_INST_INVALID_PARAMETER_HANDLER|
									CR_INST_SIGINT_HANDLER|
									CR_INST_SIGTERM_HANDLER|
									CR_INST_SIGABRT_HANDLER;

			CrashRptData.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;
//			CrashRptData.dwFlags |= CR_INST_DONT_SEND_REPORT;
//			CrashRptData.dwFlags |= CR_INST_STORE_ZIP_ARCHIVES;
			CrashRptData.pszPrivacyPolicyURL = NULL;
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

			if (crInstall(&CrashRptData))
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

	BOOL CALLBACK BGSEEMain::CrashCallback( LPVOID lpvState )
	{
		// panic and toss grenades around
		BGSEEMAIN->Daemon()->ExecuteCrashCallbacks();

		SAFEDELETE(BGSEEMAIN->ExtenderConsole);			// flush the log file
		return TRUE;
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