#include "Main.h"
#include "Console.h"
#include "WorkspaceManager.h"
#include "UIManager.h"

namespace bgsee
{
	WorkspaceManager* WorkspaceManager::Singleton = nullptr;

	WorkspaceManager::WorkspaceManager(const char* DefaultDirectory, WorkspaceManagerOperator* Operator, DefaultDirectoryArrayT& DefaultDirectoryData)
	{
		SME_ASSERT(Singleton == nullptr);
		Singleton = this;

		SME_ASSERT(DefaultDirectory && Operator && DefaultDirectoryData.size() > 1);

		this->DefaultDirectory = DefaultDirectory;
		this->CurrentDirectory = DefaultDirectory;
		this->Operator = Operator;

		for (DefaultDirectoryArrayT::iterator Itr = DefaultDirectoryData.begin(); Itr != DefaultDirectoryData.end(); Itr++)
			DefaultDirectories.push_back(*Itr);

		CreateDefaultDirectories(DefaultDirectory);

		Initialized = true;
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

	WorkspaceManager* WorkspaceManager::Get()
	{
		return Singleton;
	}

	bool WorkspaceManager::Initialize( const char* DefaultDirectory, WorkspaceManagerOperator* Operator, DefaultDirectoryArrayT& DefaultDirectoryData )
	{
		if (Singleton)
			return false;

		WorkspaceManager* Buffer = new WorkspaceManager(DefaultDirectory, Operator, DefaultDirectoryData);
		return Buffer->Initialized;
	}

	void WorkspaceManager::Deinitialize()
	{
		SME_ASSERT(Singleton);
		delete Singleton;
	}

	bool WorkspaceManager::SelectCurrentWorkspace(const char* DefaultWorkspacePath)
	{
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
			_snprintf_s(WorkspacePath, MAX_PATH, _TRUNCATE, "%s", DefaultWorkspacePath);

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
		return CurrentDirectory.c_str();
	}

	const char* WorkspaceManager::GetDefaultWorkspace( void ) const
	{
		return DefaultDirectory.c_str();
	}
}