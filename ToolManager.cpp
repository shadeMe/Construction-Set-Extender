#include "ToolManager.h"
#include "resource.h"

ToolManager							g_ToolManager;

void ToolManager::Tool::Run() const
{
	DWORD Result = (DWORD)ShellExecute(NULL, "open", this->CommandLine.c_str(), NULL, this->InitialDir.c_str(), SW_SHOW);
	if (Result > 32)
		DebugPrint("Executed Tool '%s'", this->Title.c_str());
	else
	{
		DebugPrint("Couldn't Execute Tool '%s'", this->Title.c_str());
		LogWinAPIErrorMessage(Result);
	}
}

void ToolManager::PopulateListBoxWithTools(HWND ListBox)
{
	for (std::vector<Tool*>::const_iterator Itr = ToolList.begin(); Itr != ToolList.end(); Itr++)
	{
		int Index = SendMessage(ListBox, LB_INSERTSTRING, -1, (LPARAM)(*Itr)->Title.c_str());
		SendMessage(ListBox, LB_SETITEMDATA, Index, (LPARAM)*Itr);
	}
}

ToolManager::_ToolList::iterator ToolManager::GetToolExists(const char* Title)
{
	for (std::vector<Tool*>::iterator Itr = ToolList.begin(); Itr != ToolList.end(); Itr++)
	{
		if (!_stricmp((*Itr)->Title.c_str(), Title))
			return Itr;
	}
	return ToolList.end();
}

ToolManager::Tool* ToolManager::AddTool(const char* Title, const char* CmdLine, const char* InitDir)
{
	if (GetToolExists(Title) != ToolList.end())
		return NULL;

	Tool* NewTool = new Tool(Title, CmdLine, InitDir, NextMenuIdentifier++);
	ToolList.push_back(NewTool);
	return NewTool;
}

void ToolManager::RemoveTool(const char* Title, bool ReleaseTool)
{
	_ToolList::iterator Match = GetToolExists(Title);
	if (Match == ToolList.end())
		return;
	else
	{
		if (ReleaseTool)
			delete *Match;

		ToolList.erase(Match);
	}
}

void ToolManager::ClearToolList(bool ReleaseTools)
{
	if (ReleaseTools)
	{
		for (std::vector<Tool*>::iterator Itr = ToolList.begin(); Itr != ToolList.end(); Itr++)
			delete *Itr;
	}

	ToolList.clear();
}

void ToolManager::ReadFromINI(const char* INIPath)
{
	char Buffer[0x500] = {0};

	ClearToolList(true);

	std::fstream INIStream(INIPath, std::ios_base::in);
	if (!INIStream.fail())
	{
		bool Parsing = false;
		while (INIStream.eof() == false)
		{
			INIStream.getline(Buffer, sizeof(Buffer));

			if (!Parsing && !_stricmp(Buffer, (std::string("[" + std::string(INIToolsSection) + "]")).c_str()))
			{
				Parsing = true;
				continue;
			}
			else if (Parsing && strlen(Buffer) < 2)
				break;
			else if (Parsing)
			{
				std::string StrBuffer(Buffer);
				size_t IndexA = StrBuffer.find_first_of("="), IndexB = StrBuffer.find_first_of("|");
				if (IndexA != std::string::npos && IndexB != std::string::npos)
				{
					std::string Title(StrBuffer.substr(0, IndexA)),
								CmdLine(StrBuffer.substr(IndexA + 1, IndexB - IndexA - 1)),
								InitDir(StrBuffer.substr(IndexB + 1));

					AddTool(Title.c_str(), CmdLine.c_str(), InitDir.c_str());
					DebugPrint("Added Tool '%s' at '%s'", Title.c_str(), CmdLine.c_str());
				}
			}
		}
	}
}

void ToolManager::WriteToINI(const char* INIPath)
{
	WritePrivateProfileSection(INIToolsSection, NULL, INIPath);
	for (std::vector<Tool*>::iterator Itr = ToolList.begin(); Itr != ToolList.end(); Itr++)
		WritePrivateProfileString(INIToolsSection, (*Itr)->Title.c_str(), (std::string((*Itr)->CommandLine + "|" + (*Itr)->InitialDir).c_str()), INIPath);

	DebugPrint("Saved Tool List to INI");
}

void ToolManager::InitializeToolsMenu()
{
	HMENU ToolsMenu = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU6); ToolsMenu = GetSubMenu(ToolsMenu, 0);
	HMENU MainMenu = GetMenu(*g_HWND_CSParent);

	InsertMenu(MainMenu, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)ToolsMenu, "Tools");
	DrawMenuBar(*g_HWND_CSParent);

	this->ToolMenu = GetSubMenu(MainMenu, GetMenuItemCount(MainMenu) - 1);
	this->NextMenuIdentifier = ToolMenuIdentifierBase + 1;
}

void ToolManager::ReloadToolsMenu()
{
	for (int i = GetMenuItemCount(ToolMenu); i > 2; i--)
		DeleteMenu(ToolMenu, GetMenuItemCount(ToolMenu) - 1, MF_BYPOSITION);

	for (std::vector<Tool*>::iterator Itr = ToolList.begin(); Itr != ToolList.end(); Itr++)
	{
		MENUITEMINFO ToolItem;
		ToolItem.cbSize = sizeof(MENUITEMINFO);
		ToolItem.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING|MIIM_DATA;
		ToolItem.wID = (*Itr)->Identifier;
		ToolItem.fState = MFS_ENABLED;
		ToolItem.dwTypeData = (LPSTR)(*Itr)->Title.c_str();
		ToolItem.cch = 0;
		ToolItem.dwItemData = (ULONG_PTR)*Itr;

		InsertMenuItem(ToolMenu, -1, TRUE, &ToolItem);
	}

	DebugPrint("Reloaded Tools List");
}

ToolManager::Tool* ToolManager::FetchToolByIdentifier(int Identifier)
{
	for (std::vector<Tool*>::iterator Itr = ToolList.begin(); Itr != ToolList.end(); Itr++)
	{
		if ((*Itr)->Identifier == Identifier)
			return *Itr;
	}
	return NULL;
}

void ToolManager::RunTool(int Identifier)
{
	Tool* Selection = FetchToolByIdentifier(Identifier);
	if (Selection)
		Selection->Run();
}