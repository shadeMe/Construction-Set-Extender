#pragma once

namespace BGSEditorExtender
{
	class BGSEEINIManagerGetterFunctor;
	class BGSEEINIManagerSetterFunctor;

	class BGSEEToolBox
	{
		static const char*					kINISection;

		static BOOL CALLBACK				GUIDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static BGSEEToolBox*				Singleton;
		BGSEEToolBox();

		class Tool
		{
		public:
			std::string						Title;
			std::string						CommandLine;
			std::string						InitialDir;
			std::string						Parameters;

			Tool(const char* Title, const char* CommandLine, const char* InitialDir, const char* Paramters);
			~Tool();

			bool							Run() const;
		};

		struct DlgUserData
		{
			BGSEEToolBox*					Instance;
			LPARAM							UserData;
		};

		Tool*								AddTool(const char* Title, const char* CmdLine, const char* InitDir, const char* Params);
		void								AddTool(Tool* Tool);
		void								RemoveTool(const char* Title, bool ReleaseMemory = false);
		void								RemoveTool(Tool* Tool, bool ReleaseMemory = false);
		void								ClearTools(bool ReleaseMemory = true);

		void								INISaveToolList(void);
		void								INILoadToolList(void);

		typedef std::list<Tool*>			ToolListT;

		ToolListT							RegisteredTools;
		BGSEEINIManagerGetterFunctor		INIGetter;
		BGSEEINIManagerSetterFunctor		INISetter;
		bool								Initialized;

		void								EnumerateToolsInListBox(HWND ListBox);
		bool								LookupToolByTitle(const char* Title, ToolListT::iterator& Match);
	public:
		~BGSEEToolBox();

		static BGSEEToolBox*				GetSingleton();
		bool								Initialize(BGSEEINIManagerGetterFunctor& Getter, BGSEEINIManagerSetterFunctor& Setter, bool LoadFromINI = true);

		void								ShowGUI(HINSTANCE ResourceInstance, HWND Parent);
		void								ShowToolListMenu(HINSTANCE ResourceInstance, HWND Parent, POINT* Coords = NULL);
	};

#define BGSEETOOLBOX						BGSEditorExtender::BGSEEToolBox::GetSingleton()
}