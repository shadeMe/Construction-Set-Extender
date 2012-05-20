#include "CSEWorkspaceManager.h"
#include "Hooks\TESFile.h"
#include "CSEMain.h"

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kStartupWorkspaceINISettings[kStartupWorkspace__MAX] =
		{
			{ "SetWorkspace",			"0",		"Set the working directory to a custom path on startup" },
			{ "WorkspacePath",			"",			"Path of the custom workspace directory" }
		};

		BGSEditorExtender::BGSEEINIManagerSettingFactory* GetStartupWorkspace( void )
		{
			static BGSEditorExtender::BGSEEINIManagerSettingFactory	kFactory("Startup");
			if (kFactory.Settings.size() == 0)
			{
				for (int i = 0; i < kStartupWorkspace__MAX; i++)
					kFactory.Settings.push_back(&kStartupWorkspaceINISettings[i]);
			}

			return &kFactory;
		}
	}

	namespace WorkspaceManager
	{
		CSEWorkspaceResetter::~CSEWorkspaceResetter()
		{
			;//
		}

		void CSEWorkspaceResetter::operator()()
		{
			Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteJump();

			for (tList<TESFile>::Iterator Itr = _DATAHANDLER->fileList.Begin(); Itr.End() == false && Itr.Get(); ++Itr)
			{
				Itr.Get()->SetActive(false);
				Itr.Get()->SetLoaded(false);
			}

			SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, 0x9CD1, 0);

			Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteBuffer();
		}

		CSEWorkspaceReloader::~CSEWorkspaceReloader()
		{
			;//
		}

		void CSEWorkspaceReloader::operator()( const char* WorkspacePath, bool ResetPluginList, bool LoadESPs )
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

				kDefaultDirectories.push_back(BGSEditorExtender::BGSEEResourceLocation::kBasePath);
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_CODADEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_CODABGDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SEDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SEPREPROCDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SEPREPROCSTDDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SESNIPPETDEPOT)()));
				kDefaultDirectories.push_back((BGSEditorExtender::BGSEEResourceLocation(CSE_SEAUTORECDEPOT)()));
			}

			bool ComponentInitialized = BGSEEWORKSPACE->Initialize(BGSEEMAIN->GetAPPPath(),
																new CSEWorkspaceResetter(),
																new CSEWorkspaceReloader(),
																kDefaultDirectories);
			SME_ASSERT(ComponentInitialized);

			_FILEFINDER->AddSearchPath((std::string(std::string(BGSEEWORKSPACE->GetCurrentWorkspace()) + "Data")).c_str());
		}
	}
}