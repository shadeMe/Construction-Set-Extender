#include "CLIWrapper.h"

namespace ConstructionSetExtender
{
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

				std::string DLLPath = std::string(BGSEEMAIN->GetComponentDLLPath()) + DLLName;

				HMODULE hMod = LoadLibrary(DLLPath.c_str());
				if (hMod == NULL)
				{
					BGSEECONSOLE_ERROR("Couldn't load %s", DLLName.c_str());
					return false;
				}

				ComponentDLLInterface::QueryInterface ExportedProc = (ComponentDLLInterface::QueryInterface)GetProcAddress(hMod, "QueryInterface");
				if (!ExportedProc)
				{
					BGSEECONSOLE_ERROR("Couldn't import interface from %s", DLLName.c_str());
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
}
