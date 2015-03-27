#include "CSEWorkspaceManager.h"
#include "Hooks\Hooks-Plugins.h"
#include "CSEMain.h"

namespace ConstructionSetExtender
{
	namespace WorkspaceManager
	{
		CSEWorkspaceManagerOperator::~CSEWorkspaceManagerOperator()
		{
			;//
		}

		void CSEWorkspaceManagerOperator::ResetCurrentWorkspace()
		{
			Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteJump();

			for (tList<TESFile>::Iterator Itr = _DATAHANDLER->fileList.Begin(); Itr.End() == false && Itr.Get(); ++Itr)
			{
				Itr.Get()->SetActive(false);
				Itr.Get()->SetLoaded(false);
			}

			SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, TESCSMain::kToolbar_DataFiles, 0);

			Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteBuffer();
		}

		void CSEWorkspaceManagerOperator::ReloadPlugins( const char* WorkspacePath, bool ResetPluginList, bool LoadESPs )
		{
			if (ResetPluginList)
				_DATAHANDLER->ClearPluginArray();

			_DefinePatchHdlr(DataHandlerPopulateModList, 0x0047E708 + 2);

			if (LoadESPs == false)
				_MemHdlr(DataHandlerPopulateModList).WriteUInt8(1);

			_DATAHANDLER->PopulatePluginArray(WorkspacePath);

			if (LoadESPs == false)
				_MemHdlr(DataHandlerPopulateModList).WriteUInt8(2);
		}

		void Initialize()
		{
			static BGSEditorExtender::BGSEEWorkspaceManager::DefaultDirectoryListT kDefaultDirectories;
			if (kDefaultDirectories.size() == 0)
			{
				kDefaultDirectories.push_back("Data\\");
				kDefaultDirectories.push_back("Data\\Meshes\\");
				kDefaultDirectories.push_back("Data\\Textures\\");
				kDefaultDirectories.push_back("Data\\Textures\\menus\\");
				kDefaultDirectories.push_back("Data\\Textures\\menus\\icons\\");
				kDefaultDirectories.push_back("Data\\Sound\\");
				kDefaultDirectories.push_back("Data\\Sound\\fx\\");
				kDefaultDirectories.push_back("Data\\Sound\\Voice\\");
				kDefaultDirectories.push_back("Data\\Trees\\");
				kDefaultDirectories.push_back("Data\\Backup\\");

				kDefaultDirectories.push_back(BGSEditorExtender::BGSEEResourceLocation::GetBasePath());
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_CODADEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_CODABGDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SEDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SEPREPROCDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SEPREPROCSTDDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SESNIPPETDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SEAUTORECDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_OPALDEPOT)()));
			}

			bool ComponentInitialized = BGSEEWORKSPACE->Initialize(BGSEEMAIN->GetAPPPath(),
				new CSEWorkspaceManagerOperator(),
				kDefaultDirectories);
			SME_ASSERT(ComponentInitialized);

			_FILEFINDER->AddSearchPath((std::string(std::string(BGSEEWORKSPACE->GetCurrentWorkspace()) + "Data")).c_str());
		}
	}
}