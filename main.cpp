#include "WindowManager.h"
#include "CSInterop.h"
#include "CSEInterfaceManager.h"
#include "VersionInfo.h"

#include "Hooks\Dialog.h"
#include "Hooks\LOD.h"
#include "Hooks\TESFile.h"
#include "Hooks\AssetSelector.h"
#include "Hooks\ScriptEditor.h"
#include "Hooks\Renderer.h"
#include "Hooks\Misc.h"

void OBSEMessageHandler(OBSEMessagingInterface::Message* Msg);

#define CLR_MEMORYPROFILING_ENABLED		0

BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
//		WaitUntilDebuggerAttached();
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

extern "C"
{
	__declspec(dllexport) bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "CSE";
		info->version = PACKED_CSE_VERSION;

		if (!obse->isEditor)
		{
			return false;
		}

		g_APPPath = obse->GetOblivionDirectory();
		g_INIPath = g_APPPath + "Data\\OBSE\\Plugins\\Construction Set Extender.ini";
		g_DLLPath = g_APPPath + "Data\\OBSE\\Plugins\\Construction Set Extender.dll";

		CONSOLE->InitializeLog(g_APPPath.c_str());
		DebugPrint("Construction Set Extender v%d.%d.%d {%08X} Initializing ...",
					(PACKED_CSE_VERSION >> 24) & 0xFF,
					(PACKED_CSE_VERSION >> 16) & 0xFF,
					(PACKED_CSE_VERSION >> 4) & 0xFFF,
					PACKED_CSE_VERSION);
		CONSOLE->Indent();

		g_DLLInstance = (HINSTANCE)GetModuleHandle(g_DLLPath.c_str());
		if (!g_DLLInstance)
		{
			DebugPrint("Couldn't fetch the DLL's handle!");
			return false;
		}

		DebugPrint("Initializing INI Manager");
		CONSOLE->Indent();
		g_INIManager->SetINIPath(g_INIPath);
		g_INIManager->Initialize();
		CONSOLE->Exdent();

		DebugPrint("Checking Versions and Dependencies");
		CONSOLE->Indent();
		if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			DebugPrint("OBSE version too old - v20 or greater required");
			return false;
		}
		else if (obse->editorVersion < CS_VERSION_1_2)
		{
			DebugPrint("Editor version too old - v1.2 required");
			return false;
		}

		OSVERSIONINFO OSInfo = {0};
		OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&OSInfo);
		if (OSInfo.dwMajorVersion < 5)
		{
			DebugPrint("OS version too old - Windows XP or greater required");
			return false;
		}

		HKEY MSVCRedist = NULL;
		bool HasMSVCRedist = false;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\10.0\\VC\\VCRedist\\x86", NULL, KEY_ALL_ACCESS, &MSVCRedist) == ERROR_SUCCESS)
		{
			UInt32 Type = 0, Size = 4, Value = 0;
			if (RegQueryValueEx(MSVCRedist, "Installed", NULL, &Type, (LPBYTE)&Value, &Size) == ERROR_SUCCESS &&
				Value)
			{
				HasMSVCRedist = true;
			}
		}

		if (HasMSVCRedist == false)
		{
			DebugPrint("Visual C++ 2010 x86/x64 Runtime Libraries not installed");
			return false;
		}

		if (LoadLibrary("d3dx9_41.dll") == NULL)
		{
			DebugPrint("DirectX v9.0c Runtime Libraries not installed");
			return false;
		}

		bool HasDotNet = false;
		HMODULE MsCoreEDLL = LoadLibrary("MSCOREE.DLL");	// initialize CLR
		if (MsCoreEDLL)
		{
			void* CLRInterface = NULL;
			if (GetProcAddress(MsCoreEDLL, "GetCORVersion"))
			{
				HRESULT  ( __stdcall *CorBindToRuntimeHostProc)(LPCWSTR, LPCWSTR, LPCWSTR, void*, DWORD, REFCLSID, REFIID, LPVOID FAR *) =
				(HRESULT  ( __stdcall *)(LPCWSTR, LPCWSTR, LPCWSTR, void*, DWORD, REFCLSID, REFIID, LPVOID FAR *))GetProcAddress(MsCoreEDLL, "CorBindToRuntimeHost");

				if (CorBindToRuntimeHostProc)
				{
					DWORD StartupFlags = STARTUP_LOADER_OPTIMIZATION_MASK|STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN;

#if CLR_MEMORYPROFILING_ENABLED	
					StartupFlags |= STARTUP_SERVER_GC;	// for memory profiling, concurrent GC limits the ability of profilers
#else
					StartupFlags |= STARTUP_CONCURRENT_GC;
#endif
					if (SUCCEEDED(CorBindToRuntimeHostProc(L"v4.0.30319", NULL, L"TESConstructionSet.config", 0, StartupFlags, CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, &CLRInterface)))
						HasDotNet = true;
				}
			}

			FreeLibrary(MsCoreEDLL);
		}

		if (HasDotNet == false)
		{
			DebugPrint(".NET Framework too old/not installed - v4.0.30319 (Full and Client Profile) or greater required");
			return false;
		}

		HRESULT COMLibInitState = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (COMLibInitState != S_OK)				// ensure the main thread's COM apartment state is set to STA so that common controls work correctly
		{
			DebugPrint("COM apartment state couldn't be set to STA - Result = %d", COMLibInitState);
			return false;
		}

		INITCOMMONCONTROLSEX icex = {0};
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC  = ICC_LISTVIEW_CLASSES;
		InitCommonControlsEx(&icex);				// ensure that the common control DLL is loaded.
		CONSOLE->Exdent();

		DebugPrint("Initializing OBSE Interfaces");
		CONSOLE->Indent();
		g_msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
		g_commandTableIntfc = (OBSECommandTableInterface*)obse->QueryInterface(kInterface_CommandTable);

		if (!g_msgIntfc || !g_commandTableIntfc)
		{
			DebugPrint("Messaging/CommandTable interface not found");
			return false;
		}

		g_CommandTableData.GetCommandReturnType = g_commandTableIntfc->GetReturnType;
		g_CommandTableData.GetParentPlugin = g_commandTableIntfc->GetParentPlugin;
		g_CommandTableData.GetRequiredOBSEVersion = g_commandTableIntfc->GetRequiredOBSEVersion;
		CONSOLE->Exdent();

		DebugPrint("Initializing Component DLLs");
		CONSOLE->Indent();
		if (!CLIWrapper::ImportInterfaces(obse))
			return false;
		CONSOLE->Exdent();

		return true;
	}

	__declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		g_pluginHandle = obse->GetPluginHandle();

		DebugPrint("Initializing Hooks");
		CONSOLE->Indent();
 		Hooks::PatchEntryPointHooks();
 		Hooks::PatchDialogHooks();
 		Hooks::PatchLODHooks();
 		Hooks::PatchTESFileHooks();
 		Hooks::PatchAssetSelectorHooks();
 		Hooks::PatchScriptEditorHooks();
 		Hooks::PatchRendererHooks();
 		Hooks::PatchMiscHooks();
		Hooks::PatchMessageHanders();
		CONSOLE->Exdent();

		DebugPrint("Registering OBSE Plugin Message Handlers");
		g_msgIntfc->RegisterListener(g_pluginHandle, "OBSE", OBSEMessageHandler);

		return true;
	}
};

void CSEInteropHandler(OBSEMessagingInterface::Message* Msg)
{
	if (Msg->type == 'CSEI')
	{
		DebugPrint("Dispatching Plugin Interop Interface to '%s'", Msg->sender);
		CONSOLE->Indent();
		g_msgIntfc->Dispatch(g_pluginHandle, 'CSEI', CSEInterfaceManager::GetInterface(), 4, Msg->sender);
		CONSOLE->Exdent();
	}
}

void OBSEMessageHandler(OBSEMessagingInterface::Message* Msg)
{
	switch (Msg->type)
	{
	case OBSEMessagingInterface::kMessage_PostLoad:
		g_CommandTableData.CommandTableStart = g_commandTableIntfc->Start();
		g_CommandTableData.CommandTableEnd = g_commandTableIntfc->End();
		g_msgIntfc->RegisterListener(g_pluginHandle, NULL, CSEInteropHandler);
		break;
	case OBSEMessagingInterface::kMessage_PostPostLoad:
		g_PluginPostLoad = true;
		break;
	}
}