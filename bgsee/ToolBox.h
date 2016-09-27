#pragma once

namespace bgsee
{
	class INIManagerGetterFunctor;
	class INIManagerSetterFunctor;

	class ToolBox
	{
		static const char*					kINISection;

		static BOOL CALLBACK				GUIDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static ToolBox*						Singleton;

		ToolBox(INIManagerGetterFunctor& Getter, INIManagerSetterFunctor& Setter, bool LoadFromINI);
		~ToolBox();

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
			ToolBox*						Instance;
			LPARAM							UserData;
		};

		Tool*								AddTool(const char* Title, const char* CmdLine, const char* InitDir, const char* Params);
		void								AddTool(Tool* Tool);
		void								RemoveTool(const char* Title, bool ReleaseMemory = false);
		void								RemoveTool(Tool* Tool, bool ReleaseMemory = false);
		void								ClearTools(bool ReleaseMemory = true);

		void								INISaveToolList(void);
		void								INILoadToolList(void);

		typedef std::vector<Tool*>			ToolArrayT;

		ToolArrayT							RegisteredTools;
		INIManagerGetterFunctor				INIGetter;
		INIManagerSetterFunctor				INISetter;
		bool								Initialized;

		void								EnumerateToolsInListBox(HWND ListBox);
		bool								LookupToolByTitle(const char* Title, ToolArrayT::iterator& Match);
	public:
		static ToolBox*						Get();
		static bool							Initialize(INIManagerGetterFunctor& Getter, INIManagerSetterFunctor& Setter, bool LoadFromINI = true);
		static void							Deinitialize();

		void								ShowGUI(HINSTANCE ResourceInstance, HWND Parent);
		void								ShowToolListMenu(HINSTANCE ResourceInstance, HWND Parent, POINT* Coords = nullptr);
	};

#define BGSEETOOLBOX						bgsee::ToolBox::Get()
}