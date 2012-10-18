#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "BGSEEWorkspaceManager.h"
#include "BGSEEUIManager.h"

namespace BGSEditorExtender
{
	BGSEEWorkspaceManager* BGSEEWorkspaceManager::Singleton = NULL;

	BGSEEWorkspaceManager::BGSEEWorkspaceManager()
	{
		WorkspaceResetter = NULL;
		WorkspacePluginReloader = NULL;
		Initialized = false;
	}

	void BGSEEWorkspaceManager::SetWorkingDirectory( const char* WorkspacePath )
	{
		// it is not recommended that the SetCurrentDirectory API function be used in multi-threaded applications
		// but it certainly beats patching a ton of locations in the executable
		// since we reset data and background queuing before calling it, it should be relatively safe
		// heck! even Beth calls the function in their code xD

		CurrentDirectory = WorkspacePath;
		SetCurrentDirectory(WorkspacePath);
	}

	void BGSEEWorkspaceManager::CreateDefaultDirectories( const char* WorkspacePath )
	{
		char Buffer[0x200] = {0};

		for (DirectoryListT::const_iterator Itr = DefaultDirectories.begin(); Itr != DefaultDirectories.end(); Itr++)
		{
			FORMAT_STR(Buffer, "%s%s", WorkspacePath, Itr->c_str());

			if (CreateDirectory(Buffer, NULL) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS)
			{
				BGSEECONSOLE_ERROR("Couldn't create directory '%s'", Buffer);
			}
		}
	}

	BGSEEWorkspaceManager::~BGSEEWorkspaceManager()
	{
		DefaultDirectories.clear();
		SAFEDELETE(WorkspaceResetter);
		SAFEDELETE(WorkspacePluginReloader);

		Initialized = false;

		Singleton = NULL;
	}

	BGSEEWorkspaceManager* BGSEEWorkspaceManager::GetSingleton()
	{
		if (Singleton == NULL)
			Singleton = new BGSEEWorkspaceManager();

		return Singleton;
	}

	bool BGSEEWorkspaceManager::Initialize( const char* DefaultDirectory,
											VoidRFunctorBase* Resetter,
											ReloadPluginsFunctor* Reloader,
											DefaultDirectoryListT& DefaultDirectoryData )
	{
		if (Initialized)
			return false;

		SME_ASSERT(DefaultDirectory && Resetter && Reloader && DefaultDirectoryData.size() > 1);

		this->DefaultDirectory = DefaultDirectory;
		this->CurrentDirectory = DefaultDirectory;
		this->WorkspaceResetter = Resetter;
		this->WorkspacePluginReloader = Reloader;

		for (DefaultDirectoryListT::iterator Itr = DefaultDirectoryData.begin(); Itr != DefaultDirectoryData.end(); Itr++)
			DefaultDirectories.push_back(*Itr);

		CreateDefaultDirectories(DefaultDirectory);

		Initialized = true;
		return Initialized;
	}

	bool BGSEEWorkspaceManager::SelectCurrentWorkspace( const char* DefaultWorkspacePath )
	{
		SME_ASSERT(Initialized);

		char WorkspacePath[MAX_PATH] = {0};

		if (DefaultWorkspacePath == NULL)
		{
			BROWSEINFO WorkspaceInfo = {0};
			WorkspaceInfo.hwndOwner = BGSEEUI->GetMainWindow();
			WorkspaceInfo.iImage = NULL;
			WorkspaceInfo.pszDisplayName = WorkspacePath;
			WorkspaceInfo.lpszTitle = "Select a valid workspace inside the root game directory";
			WorkspaceInfo.ulFlags = BIF_NEWDIALOGSTYLE|BIF_RETURNONLYFSDIRS;
			WorkspaceInfo.pidlRoot = NULL;
			WorkspaceInfo.lpfn = NULL;
			WorkspaceInfo.lParam = NULL;

			PIDLIST_ABSOLUTE ReturnPath = SHBrowseForFolder(&WorkspaceInfo);
			if (ReturnPath)
			{
				if (!SHGetPathFromIDList(ReturnPath, WorkspacePath))
				{
					BGSEEUI->MsgBoxE("Couldn't determine workspace folder path.");
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
			sprintf_s(WorkspacePath, MAX_PATH, "%s", DefaultWorkspacePath);

		strcat_s(WorkspacePath, MAX_PATH, "\\");

		if (strstr(WorkspacePath, DefaultDirectory.c_str()) == WorkspacePath)
		{
			if (_stricmp(CurrentDirectory.c_str(), WorkspacePath))
			{
				WorkspaceResetter->operator()();
				WorkspacePluginReloader->operator()(std::string(CurrentDirectory + "Data\\").c_str(), true, false);
				SetWorkingDirectory(WorkspacePath);
				CreateDefaultDirectories(WorkspacePath);
				WorkspacePluginReloader->operator()("Data\\", false, true);

				BGSEEUI->MsgBoxI("Current workspace set to '%s'.", WorkspacePath);
				return true;
			}

			return false;
		}
		else
		{
			BGSEEUI->MsgBoxW("The new workspace must be inside the root game directory.");
			return false;
		}
	}

	const char* BGSEEWorkspaceManager::GetCurrentWorkspace( void ) const
	{
		SME_ASSERT(Initialized);

		return CurrentDirectory.c_str();
	}

	const char* BGSEEWorkspaceManager::GetDefaultWorkspace( void ) const
	{
		SME_ASSERT(Initialized);

		return DefaultDirectory.c_str();
	}

	const std::string			BGSEEResourceLocation::kBasePath = "Data\\BGSEE\\";

	BGSEEResourceLocation::BGSEEResourceLocation( std::string Path ) :
		RelativePath(Path)
	{
		SME_ASSERT(CheckPath() == true);
	}

	BGSEEResourceLocation::BGSEEResourceLocation() :
		RelativePath("")
	{
		;//
	}

	BGSEEResourceLocation::~BGSEEResourceLocation()
	{
		;//
	}

	std::string BGSEEResourceLocation::GetFullPath() const
	{
		return kBasePath + RelativePath;
	}

	std::string BGSEEResourceLocation::GetRelativePath() const
	{
		return RelativePath;
	}

	BGSEEResourceLocation& BGSEEResourceLocation::operator=( const BGSEEResourceLocation& rhs )
	{
		this->RelativePath = rhs.RelativePath;

		return *this;
	}

	BGSEEResourceLocation& BGSEEResourceLocation::operator=( std::string rhs )
	{
		this->RelativePath = rhs;
		SME_ASSERT(CheckPath() == true);

		return *this;
	}

	std::string BGSEEResourceLocation::operator()()
	{
		return GetFullPath();
	}

	bool BGSEEResourceLocation::CheckPath( void )
	{
		std::string PathB(RelativePath), BaseB(kBasePath);

		SME::StringHelpers::MakeLower(PathB);
		SME::StringHelpers::MakeLower(BaseB);

		return PathB.find(BaseB) == std::string::npos;
	}
}