#pragma once

namespace BGSEditorExtender
{
	class BGSEEINIManagerGetterFunctor;
	class BGSEEINIManagerSetterFunctor;

	class BGSEEToolBox
	{
		static BOOL CALLBACK				GUIDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static const char*					kINISection;

		static BGSEEToolBox*				Singleton;
		BGSEEToolBox();

		class Tool
		{
		public:
			std::string						Title;
			std::string						CommandLine;
			std::string						InitialDir;

			Tool(const char* Title, const char* CommandLine, const char* InitialDir);
			~Tool();

			bool							Run() const;
		};

		struct DlgUserData
		{
			BGSEEToolBox*					Instance;
			LPARAM							UserData;
		};

		Tool*								AddTool(const char* Title, const char* CmdLine, const char* InitDir);
		void								AddTool(Tool* Tool);
		void								RemoveTool(const char* Title, bool ReleaseMemory = false);
		void								RemoveTool(Tool* Tool, bool ReleaseMemory = false);
		void								ClearTools(bool ReleaseMemory = true);

		void								INISaveToolList(void);
		void								INILoadToolList(void);

		typedef std::list<Tool*>			ToolListT;

		ToolListT							RegisteredTools;
		BGSEEINIManagerGetterFunctor*		INIGetter;
		BGSEEINIManagerSetterFunctor*		INISetter;
		bool								Initialized;

		void								EnumerateToolsInListBox(HWND ListBox);
		bool								LookupToolByTitle(const char* Title, ToolListT::iterator& Match);
	public:
		~BGSEEToolBox();

		static BGSEEToolBox*				GetSingleton();
		bool								Initialize(BGSEEINIManagerGetterFunctor* Getter, BGSEEINIManagerSetterFunctor* Setter, bool LoadFromINI = true);	// takes ownership of the pointers

		void								ShowGUI(HINSTANCE ResourceInstance, HWND Parent);
		void								ShowToolListMenu(HINSTANCE ResourceInstance, HWND Parent, LPARAM Coords);
	};

#define BGSEETOOLBOX						BGSEEToolBox::GetSingleton()
}