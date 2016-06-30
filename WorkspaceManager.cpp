#include "WorkspaceManager.h"
#include "Hooks\Hooks-Plugins.h"
#include "Main.h"

namespace cse
{
	namespace workspaceManager
	{
		WorkspaceManagerOperator::~WorkspaceManagerOperator()
		{
			;//
		}

		void WorkspaceManagerOperator::ResetCurrentWorkspace()
		{
			hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteJump();

			for (tList<TESFile>::Iterator Itr = _DATAHANDLER->fileList.Begin(); Itr.End() == false && Itr.Get(); ++Itr)
			{
				Itr.Get()->SetActive(false);
				Itr.Get()->SetLoaded(false);
			}

			SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, TESCSMain::kToolbar_DataFiles, 0);

			hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteBuffer();
		}

		void WorkspaceManagerOperator::ReloadPlugins( const char* WorkspacePath, bool ResetPluginList, bool LoadESPs )
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
			static bgsee::WorkspaceManager::DefaultDirectoryArrayT kDefaultDirectories;
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

				kDefaultDirectories.push_back(bgsee::ResourceLocation::GetBasePath());
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_CODADEPOT)()));
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_CODABGDEPOT)()));
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_SEDEPOT)()));
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_SEPREPROCDEPOT)()));
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_SEPREPROCSTDDEPOT)()));
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_SESNIPPETDEPOT)()));
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_SEAUTORECDEPOT)()));
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_OPALDEPOT)()));
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_PREFABDEPOT)()));
				kDefaultDirectories.push_back((bgsee::ResourceLocation(CSE_COSAVEDEPOT)()));
			}

			bool ComponentInitialized = BGSEEWORKSPACE->Initialize(BGSEEMAIN->GetAPPPath(), new WorkspaceManagerOperator(), kDefaultDirectories);
			SME_ASSERT(ComponentInitialized);

			_FILEFINDER->AddSearchPath((std::string(std::string(BGSEEWORKSPACE->GetCurrentWorkspace()) + "Data")).c_str());

		}

	}
}