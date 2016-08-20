#include "CLIWrapper.h"

namespace cse
{
	namespace cliWrapper
	{
		namespace interfaces
		{
			componentDLLInterface::ScriptEditorInterface*			SE = nullptr;
			componentDLLInterface::UseInfoListInterface*			USE = nullptr;
			componentDLLInterface::BSAViewerInterface*				BSA = nullptr;
			componentDLLInterface::BatchEditorInterface*			BE = nullptr;
			componentDLLInterface::TagBrowserInterface*				TAG = nullptr;
		}

		componentDLLInterface::QueryInterface SEQueryInterfaceProc = nullptr;
		componentDLLInterface::QueryInterface USEQueryInterfaceProc = nullptr;
		componentDLLInterface::QueryInterface BSAQueryInterfaceProc = nullptr;
		componentDLLInterface::QueryInterface BEQueryInterfaceProc = nullptr;
		componentDLLInterface::QueryInterface TAGQueryInterfaceProc = nullptr;

		bool cliWrapper::ImportInterfaces(const OBSEInterface * obse)
		{
			SetErrorMode(0);
			std::string DLLName = "";
			void** Interface = nullptr;

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
				if (hMod == nullptr)
				{
					BGSEECONSOLE_ERROR("Couldn't load %s", DLLName.c_str());
					return false;
				}

				componentDLLInterface::QueryInterface ExportedProc = (componentDLLInterface::QueryInterface)GetProcAddress(hMod, "QueryInterface");
				if (ExportedProc == nullptr)
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
			interfaces::SE = (componentDLLInterface::ScriptEditorInterface*)SEQueryInterfaceProc();
			interfaces::USE = (componentDLLInterface::UseInfoListInterface*)USEQueryInterfaceProc();
			interfaces::BSA = (componentDLLInterface::BSAViewerInterface*)BSAQueryInterfaceProc();
			interfaces::BE = (componentDLLInterface::BatchEditorInterface*)BEQueryInterfaceProc();
			interfaces::TAG = (componentDLLInterface::TagBrowserInterface*)TAGQueryInterfaceProc();

			SME_ASSERT(interfaces::SE);
			SME_ASSERT(interfaces::USE);
			SME_ASSERT(interfaces::BSA);
			SME_ASSERT(interfaces::BE);
			SME_ASSERT(interfaces::TAG);

			// the script editor is initialized elsewhere
			interfaces::USE->InitializeComponents();
			interfaces::BSA->InitializeComponents();
			interfaces::BE->InitializeComponents();
			interfaces::TAG->InitializeComponents();
		}
	}
}
