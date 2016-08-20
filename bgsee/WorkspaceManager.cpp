#include "Main.h"
#include "Console.h"
#include "WorkspaceManager.h"
#include "UIManager.h"

namespace bgsee
{
	WorkspaceManager* WorkspaceManager::Singleton = nullptr;

	WorkspaceManager::WorkspaceManager()
	{
		Operator = nullptr;
		Initialized = false;
	}

	void WorkspaceManager::SetWorkingDirectory( const char* WorkspacePath )
	{
		// it is not recommended that the SetCurrentDirectory API function be used in multi-threaded applications
		// but it certainly beats patching a ton of locations in the executable
		// since we reset data and background queuing before calling it, it should be relatively safe
		// heck! even Beth calls the function in their code xD

		CurrentDirectory = WorkspacePath;
		SetCurrentDirectory(WorkspacePath);
	}

	void WorkspaceManager::CreateDefaultDirectories( const char* WorkspacePath )
	{
		char Buffer[0x200] = {0};

		for (DirectoryArrayT::const_iterator Itr = DefaultDirectories.begin(); Itr != DefaultDirectories.end(); Itr++)
		{
			FORMAT_STR(Buffer, "%s%s", WorkspacePath, Itr->c_str());

			if (CreateDirectory(Buffer, nullptr) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS)
			{
				BGSEECONSOLE_ERROR("Couldn't create directory '%s'", Buffer);
			}
		}
	}

	WorkspaceManager::~WorkspaceManager()
	{
		DefaultDirectories.clear();
		SAFEDELETE(Operator);

		Initialized = false;

		Singleton = nullptr;
	}

	WorkspaceManager* WorkspaceManager::GetSingleton()
	{
		if (Singleton == nullptr)
			Singleton = new WorkspaceManager();

		return Singleton;
	}

	bool WorkspaceManager::Initialize( const char* DefaultDirectory,
											WorkspaceManagerOperator* Operator,
											DefaultDirectoryArrayT& DefaultDirectoryData )
	{
		if (Initialized)
			return false;

		SME_ASSERT(DefaultDirectory && Operator && DefaultDirectoryData.size() > 1);

		this->DefaultDirectory = DefaultDirectory;
		this->CurrentDirectory = DefaultDirectory;
		this->Operator = Operator;

		for (DefaultDirectoryArrayT::iterator Itr = DefaultDirectoryData.begin(); Itr != DefaultDirectoryData.end(); Itr++)
			DefaultDirectories.push_back(*Itr);

		CreateDefaultDirectories(DefaultDirectory);

		Initialized = true;
		return Initialized;
	}

	bool WorkspaceManager::SelectCurrentWorkspace( const char* DefaultWorkspacePath )
	{
		SME_ASSERT(Initialized);

		char WorkspacePath[MAX_PATH] = {0};

		if (DefaultWorkspacePath == nullptr)
		{
			BROWSEINFO WorkspaceInfo = {0};
			WorkspaceInfo.hwndOwner = BGSEEUI->GetMainWindow();
			WorkspaceInfo.iImage = 0;
			WorkspaceInfo.pszDisplayName = WorkspacePath;
			WorkspaceInfo.lpszTitle = "Select a valid workspace inside the root game directory";
			WorkspaceInfo.ulFlags = BIF_NEWDIALOGSTYLE|BIF_RETURNONLYFSDIRS;
			WorkspaceInfo.pidlRoot = nullptr;
			WorkspaceInfo.lpfn = nullptr;
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
				Operator->ResetCurrentWorkspace();
				Operator->ReloadPlugins(std::string(CurrentDirectory + "Data\\").c_str(), true, false);
				SetWorkingDirectory(WorkspacePath);
				CreateDefaultDirectories(WorkspacePath);
				Operator->ReloadPlugins("Data\\", false, true);

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

	const char* WorkspaceManager::GetCurrentWorkspace( void ) const
	{
		SME_ASSERT(Initialized);

		return CurrentDirectory.c_str();
	}

	const char* WorkspaceManager::GetDefaultWorkspace( void ) const
	{
		SME_ASSERT(Initialized);

		return DefaultDirectory.c_str();
	}

	ResourceLocation::ResourceLocation( std::string Path ) :
		RelativePath(Path)
	{
		SME_ASSERT(CheckPath() == true);
	}

	ResourceLocation::ResourceLocation() :
		RelativePath("")
	{
		;//
	}

	ResourceLocation::~ResourceLocation()
	{
		;//
	}

	std::string ResourceLocation::GetFullPath() const
	{
		return GetBasePath() + RelativePath;
	}

	std::string ResourceLocation::GetRelativePath() const
	{
		return RelativePath;
	}

	ResourceLocation& ResourceLocation::operator=( const ResourceLocation& rhs )
	{
		this->RelativePath = rhs.RelativePath;

		return *this;
	}

	ResourceLocation& ResourceLocation::operator=( std::string rhs )
	{
		this->RelativePath = rhs;
		SME_ASSERT(CheckPath() == true);

		return *this;
	}

	std::string ResourceLocation::operator()() const
	{
		return GetFullPath();
	}

	bool ResourceLocation::CheckPath( void )
	{
		std::string PathB(RelativePath), BaseB(GetBasePath());

		SME::StringHelpers::MakeLower(PathB);
		SME::StringHelpers::MakeLower(BaseB);

		return PathB.find(BaseB) == std::string::npos;
	}

	const std::string& ResourceLocation::GetBasePath(void)
	{
		// initialized here to ensure statically allocated BGSEEResourceLocation instances never trigger assertions inside CRTMain
		static const std::string kBasePath = "Data\\BGSEE\\";
		return kBasePath;
	}
}