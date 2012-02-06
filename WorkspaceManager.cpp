#include "WorkspaceManager.h"
#include "Hooks\TESFile.h"
#include "Achievements.h"

WorkspaceManager					g_WorkspaceManager;

void WorkspaceManager::Initialize(const char *DefaultDirectory)
{
	this->DefaultDirectory = DefaultDirectory;
	this->CurrentDirectory = DefaultDirectory;

	_FILEFINDER->AddSearchPath((this->DefaultDirectory + "Data").c_str());		// add the absolute path to the filefinder's search path collection

	CreateDefaultDirectories(DefaultDirectory);

	HMENU FileMenu = GetMenu(*g_HWND_CSParent); FileMenu = GetSubMenu(FileMenu, 0);
	MENUITEMINFO ItemDataSetWorkspace;
	ItemDataSetWorkspace.cbSize = sizeof(MENUITEMINFO);
	ItemDataSetWorkspace.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemDataSetWorkspace.wID = MAIN_DATA_SETWORKSPACE;
	ItemDataSetWorkspace.fState = MFS_ENABLED;
	ItemDataSetWorkspace.dwTypeData = "Set Workspace";
	ItemDataSetWorkspace.cch = 0;
	InsertMenuItem(FileMenu, 40003, FALSE, &ItemDataSetWorkspace);
}

void WorkspaceManager::ResetLoadedData()
{
	Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteJump();

	for (tList<TESFile>::Iterator Itr = _DATAHANDLER->fileList.Begin(); !Itr.End(); ++Itr)
	{
		if (!Itr.Get())
			break;

		ToggleFlag(&Itr.Get()->fileFlags, TESFile::kFileFlag_Active, false);
		ToggleFlag(&Itr.Get()->fileFlags, TESFile::kFileFlag_Loaded, false);
	}

	SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CD1, 0);

	Hooks::_MemHdlr(AutoLoadActivePluginOnStartup).WriteBuffer();
}

bool WorkspaceManager::SelectWorkspace(const char* Workspace)
{
	char WorkspacePath[MAX_PATH] = {0};

	if (Workspace == NULL)
	{
		BROWSEINFO WorkspaceInfo = {0};
		WorkspaceInfo.hwndOwner = *g_HWND_CSParent;
		WorkspaceInfo.iImage = NULL;
		WorkspaceInfo.pszDisplayName = WorkspacePath;
		WorkspaceInfo.lpszTitle = "Select a vaild workspace inside the 'Oblivion' directory";
		WorkspaceInfo.ulFlags = BIF_NEWDIALOGSTYLE|BIF_RETURNONLYFSDIRS;
		WorkspaceInfo.pidlRoot = NULL;
		WorkspaceInfo.lpfn = NULL;
		WorkspaceInfo.lParam = NULL;

		PIDLIST_ABSOLUTE ReturnPath = SHBrowseForFolder(&WorkspaceInfo);
		if (ReturnPath)
		{
			if (!SHGetPathFromIDList(ReturnPath, WorkspacePath))
			{
				DebugPrint("Couldn't extract workspace path!");
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
		sprintf_s(WorkspacePath, MAX_PATH, "%s", Workspace);

	strcat_s(WorkspacePath, MAX_PATH, "\\");

	if (strstr(WorkspacePath, g_APPPath.c_str()) == WorkspacePath)
	{
		if (_stricmp(CurrentDirectory.c_str(), WorkspacePath))
		{
			ResetLoadedData();
			ReloadModList(std::string(CurrentDirectory + "Data\\").c_str(), true, false);
			SetWorkingDirectory(WorkspacePath);
			CreateDefaultDirectories(WorkspacePath);
			ReloadModList("Data\\", false, true);

			char Buffer[0x200] = {0};
			FORMAT_STR(Buffer, "Current workspace set to '%s'", WorkspacePath);
			MessageBox(*g_HWND_CSParent, Buffer, "CSE", MB_OK|MB_ICONINFORMATION);
			DebugPrint(Buffer);

			Achievements::UnlockAchievement(Achievements::kAchievement_Compartmentalizer);
			return true;
		}
		return false;
	}
	else
	{
		MessageBox(*g_HWND_CSParent, "The new workspace must be inside the 'Oblivion' directory.", "CSE", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
}

void WorkspaceManager::SetWorkingDirectory(const char *WorkspacePath)
{
	// it is not recommended that the SetCurrentDirectory API function be used in multi-threaded applications
	// but it certainly beats patching a ton of locations in the executable
	// since we reset data and background queuing before calling it, it should be relatively safe

	CurrentDirectory = WorkspacePath;
	SetCurrentDirectory(WorkspacePath);
}

void WorkspaceManager::CreateDefaultDirectories(const char* WorkspacePath)
{
	std::string Buffer(WorkspacePath);

	if ((!CreateDirectory(std::string(Buffer + "Data\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Meshes\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Textures\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Textures\\menus\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Textures\\menus\\icons\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Sound\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Sound\\fx\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Sound\\Voice\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Trees\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Scripts\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Scripts\\Preprocessor\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Scripts\\Preprocessor\\STD\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Scripts\\CSAS\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Scripts\\CSAS\\Global Scripts\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Scripts\\Snippets\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Scripts\\Auto-Recovery Cache\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Backup\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS))
	{
		DebugPrint("Couldn't create create all default directories in workspace '%s'", WorkspacePath);
		LogWinAPIErrorMessage(GetLastError());
	}
}

void WorkspaceManager::ReloadModList(const char* WorkspacePath, bool ClearList, bool LoadESPs)
{
	if (ClearList)
		_DATAHANDLER->ClearPluginArray();

	if (LoadESPs == false)
		this->DataHandlerPopulateModList.WriteUInt8(1);

	_DATAHANDLER->PopulatePluginArray(WorkspacePath);

	if (LoadESPs == false)
		this->DataHandlerPopulateModList.WriteUInt8(2);
}