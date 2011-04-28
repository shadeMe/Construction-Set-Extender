#pragma once
#include "ExtenderInternals.h"

class WorkspaceManager
{
	std::string							CurrentDirectory;
	std::string							DefaultDirectory;

	MemoryHandler::MemHdlr				DataHandlerPopulateModList;

	void								SetWorkingDirectory(const char* WorkspacePath);
	void								ResetLoadedData(void);
	void								ReloadModList(const char* WorkspacePath, bool ClearList, bool LoadESPs);
	void								CreateDefaultDirectories(const char* WorkspacePath);
public:
	WorkspaceManager() : DataHandlerPopulateModList(0x0047E708 + 2, (UInt32)0, 0, 0),
						CurrentDirectory(g_AppPath),
						DefaultDirectory(g_AppPath) {}

	void								Initialize(const char* DefaultDirectory);
	bool								SelectWorkspace(const char* Workspace);
};

extern WorkspaceManager					g_WorkspaceManager;