#pragma once

class ToolManager
{
public:
	class Tool
	{
		std::string						Title;
		std::string						CommandLine;
		std::string						InitialDir;
		int								Identifier;

		Tool(const char* Title, const char* CommandLine, const char* InitialDir, int ID) :
			Title(Title), CommandLine(CommandLine), InitialDir(InitialDir), Identifier(ID) {}
	public:
		void							Run() const;
		void							SetTitle(const char* Title) { this->Title = Title; }
		void							SetCommandLine(const char* CmdLine) { this->CommandLine = CmdLine; }
		void							SetInitialDir(const char* InitDir) { this->InitialDir = InitDir; }

		const char*						GetTitle() const { return Title.c_str(); }
		const char*						GetCommandLine() const { return CommandLine.c_str(); }
		const char*						GetInitialDir() const { return InitialDir.c_str(); }

		friend class					ToolManager;
	};

	void								PopulateListBoxWithTools(HWND ListBox);
	Tool*								AddTool(const char* Title, const char* CmdLine, const char* InitDir);
	void								AddTool(Tool* Tool) { ToolList.push_back(Tool); }
	void								RemoveTool(const char* Title, bool ReleaseTool);
	void								ClearToolList(bool ReleaseTools);

	void								ReadFromINI(const char* INIPath);
	void								WriteToINI(const char* INIPath);

	void								InitializeToolsMenu(void);
	void								ReloadToolsMenu(void);

	void								RunTool(int Identifier);
private:
	typedef std::vector<Tool*>			_ToolList;
	_ToolList							ToolList;
	HMENU								ToolMenu;
	int									NextMenuIdentifier;

	_ToolList::iterator					GetToolExists(const char* Title);
	Tool*								FetchToolByIdentifier(int Identifier);
};

#define INIToolsSection					"Extender::Tools"
#define ToolMenuIdentifierBase			50000

extern ToolManager						g_ToolManager;