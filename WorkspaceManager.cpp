#include "WorkspaceManager.h"
#include "Hooks\TESFile.h"

using namespace Hooks;

WorkspaceManager					g_WorkspaceManager;

void WorkspaceManager::Initialize(const char *DefaultDirectory)
{
	this->DefaultDirectory = DefaultDirectory;
	this->CurrentDirectory = DefaultDirectory;

	thisVirtualCall(kVTBL_FileFinder, 0x8, *g_FileFinder, (this->DefaultDirectory + "Data").c_str());	// add the absolute path to the filefinder's search path collection
}

void WorkspaceManager::ResetLoadedData()
{
	kAutoLoadActivePluginOnStartup.WriteJump();

	for (ModEntry* Itr = &(*g_dataHandler)->modList; Itr && Itr->data; Itr = Itr->next)
	{
		ToggleFlag(&Itr->data->flags, ModEntry::Data::kFlag_Active, false);
		ToggleFlag(&Itr->data->flags, ModEntry::Data::kFlag_Loaded, false);
	}

	SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CD1, 0);

	kAutoLoadActivePluginOnStartup.WriteBuffer();
}

bool WorkspaceManager::SelectWorkspace(const char* Workspace)
{
	char WorkspacePath[MAX_PATH] = {0};

	if (Workspace == NULL)
	{
		BROWSEINFO WorkspaceInfo;
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

	PrintToBuffer("%s\\", WorkspacePath);
	sprintf_s(WorkspacePath, MAX_PATH, "%s", g_TextBuffer);

	if (strstr(WorkspacePath, g_AppPath.c_str()) == WorkspacePath)
	{
		if (_stricmp(CurrentDirectory.c_str(), WorkspacePath))
		{
			ResetLoadedData();
			ReloadModList(std::string(CurrentDirectory + "Data\\").c_str(), true, false);
			SetWorkingDirectory(WorkspacePath);
			CreateDefaultDirectories(WorkspacePath);
			ReloadModList("Data\\", false, true);

			PrintToBuffer("Current workspace set to '%s'", WorkspacePath);
			DebugPrint(g_TextBuffer);
			MessageBox(*g_HWND_CSParent, g_TextBuffer, "CSE", MB_OK|MB_ICONINFORMATION);

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
		(!CreateDirectory(std::string(Buffer + "Data\\Scripts\\Standard Preprocessor Directives\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) ||
		(!CreateDirectory(std::string(Buffer + "Data\\Backup\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS))
	{
		DebugPrint("Couldn't create create default directories in workspace '%s'", WorkspacePath);
		LogWinAPIErrorMessage(GetLastError());
	}
}

void WorkspaceManager::ReloadModList(const char* WorkspacePath, bool ClearList, bool LoadESPs)
{
	tList<TESFile*>* ModList = (tList<TESFile*>*)&(*g_dataHandler)->modList;
	if (ClearList)
	{
		for (ModEntry* Itr = &(*g_dataHandler)->modList; Itr && Itr->data; Itr = Itr->next)
		{
			thisCall(kTESFile_Dtor, Itr->data);
			FormHeap_Free(Itr->data);
		}
		ModList->RemoveAll();
	}

	if (LoadESPs == false)
		this->DataHandlerPopulateModList.WriteUInt8(1);

	thisCall(kDataHandler_PopulateModList, *g_dataHandler, WorkspacePath);

	if (LoadESPs == false)
		this->DataHandlerPopulateModList.WriteUInt8(2);
}