#include "CLIWrapper.h"
#include <MSCorEE.h>

namespace CLIWrapper
{
	namespace Interfaces
	{
		ComponentDLLInterface::ScriptEditorInterface*			SE = NULL;
		ComponentDLLInterface::UseInfoListInterface*			USE = NULL;
		ComponentDLLInterface::BSAViewerInterface*				BSA = NULL;
		ComponentDLLInterface::BatchEditorInterface*			BE = NULL;
		ComponentDLLInterface::TagBrowserInterface*				TAG = NULL;
	}

	ComponentDLLInterface::QueryInterface SEQueryInterfaceProc = NULL;
	ComponentDLLInterface::QueryInterface USEQueryInterfaceProc = NULL;
	ComponentDLLInterface::QueryInterface BSAQueryInterfaceProc = NULL;
	ComponentDLLInterface::QueryInterface BEQueryInterfaceProc = NULL;
	ComponentDLLInterface::QueryInterface TAGQueryInterfaceProc = NULL;

	bool CLIWrapper::ImportInterfaces(const OBSEInterface * obse)
	{
		HMODULE MsCoreEDLL = LoadLibrary("MSCOREE.DLL");	// force load the CLR with server mode GC enabled (helps with debugging memory profiling)
		if (MsCoreEDLL)
		{
			void* CLRInterface = NULL;
			HRESULT  ( __stdcall *CorBindToRuntimeExProc)(LPCWSTR, LPCWSTR, DWORD, REFCLSID, REFIID, LPVOID FAR *) = (HRESULT  ( __stdcall *)(LPCWSTR, LPCWSTR, DWORD, REFCLSID, REFIID, LPVOID FAR*))GetProcAddress(MsCoreEDLL, "CorBindToRuntimeEx");
			if (CorBindToRuntimeExProc)
				CorBindToRuntimeExProc(L"v4.0.30319", NULL, STARTUP_SERVER_GC, CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, &CLRInterface);
		}

		SetErrorMode(0);
		std::string DLLName = "";
		void** Interface = NULL;

		for (int i = 0; i < 5; i++)
		{
			switch (i)
			{
			case 0:
				DLLName = "ScriptEditor.dll";
				Interface = (void**)&SEQueryInterfaceProc;
				break;
			case 1:
				DLLName = "UseInfoList.dll";
				Interface = (void**)&USEQueryInterfaceProc;
				break;
			case 2:
				DLLName = "BSAViewer.dll";
				Interface = (void**)&BSAQueryInterfaceProc;
				break;
			case 3:
				DLLName = "BatchEditor.dll";
				Interface = (void**)&BEQueryInterfaceProc;
				break;
			case 4:
				DLLName = "TagBrowser.dll";
				Interface = (void**)&TAGQueryInterfaceProc;
				break;
			}

			HMODULE hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\CSE\\" + DLLName).c_str());
			if (hMod == NULL)
			{
				DebugPrint("Couldn't load %s", DLLName.c_str());
				LogWinAPIErrorMessage(GetLastError());
				return false;
			}

			ComponentDLLInterface::QueryInterface ExportedProc = (ComponentDLLInterface::QueryInterface)GetProcAddress(hMod, "QueryInterface");
			if (!ExportedProc)
			{
				DebugPrint("Couldn't import interface from %s", DLLName.c_str());
				LogWinAPIErrorMessage(GetLastError());
				return false;
			}

			*Interface = ExportedProc;
		}

		return true;
	}

	void QueryInterfaces( void )
	{
		Interfaces::SE = (ComponentDLLInterface::ScriptEditorInterface*)SEQueryInterfaceProc();
		Interfaces::USE = (ComponentDLLInterface::UseInfoListInterface*)USEQueryInterfaceProc();
		Interfaces::BSA = (ComponentDLLInterface::BSAViewerInterface*)BSAQueryInterfaceProc();
		Interfaces::BE = (ComponentDLLInterface::BatchEditorInterface*)BEQueryInterfaceProc();
		Interfaces::TAG = (ComponentDLLInterface::TagBrowserInterface*)TAGQueryInterfaceProc();
	}
}