#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "BGSEEUIManager.h"
#include "BGSEEToolBox.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace BGSEditorExtender
{
	BGSEEToolBox*			BGSEEToolBox::Singleton = NULL;
	const char*				BGSEEToolBox::kINISection = "ToolBox";

#define IDM_BGSEE_TOOLBOX_RESETINPUTFIELDS		(WM_USER + 5002)

	BOOL CALLBACK BGSEEToolBox::GUIDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		HWND ToolList = GetDlgItem(hWnd, IDC_BGSEE_TOOLBOX_TOOLLIST);
		HWND TitleBox =  GetDlgItem(hWnd, IDC_BGSEE_TOOLBOX_TITLEBOX);
		HWND CmdLineBox =  GetDlgItem(hWnd, IDC_BGSEE_TOOLBOX_CMDLINEBOX);
		HWND InitDirBox =  GetDlgItem(hWnd, IDC_BGSEE_TOOLBOX_INITDIRBOX);

		char Buffer[0x200] = {0};
		DlgUserData* UserData = (DlgUserData*)GetWindowLong(hWnd, GWL_USERDATA);
		BGSEEToolBox* Instance = UserData->Instance;

		switch (uMsg)
		{
		case IDM_BGSEE_TOOLBOX_RESETINPUTFIELDS:
			SetWindowText(TitleBox, NULL);
			SetWindowText(CmdLineBox, NULL);
			SetWindowText(InitDirBox, NULL);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_BGSEE_CLOSE:
				Instance->ClearTools(false);

				for (int i = 0, j = SendMessage(ToolList, LB_GETCOUNT, NULL, NULL); i < j; i++)
				{
					Tool* Data = (Tool*)SendMessage(ToolList, LB_GETITEMDATA, i, NULL);
					Instance->AddTool(Data);
				}

				EndDialog(hWnd, 0);
				return TRUE;
			case IDC_BGSEE_TOOLBOX_ADD:
				{
					char TitleBuffer[0x200] = {0}, CmdLineBuffer[MAX_PATH] = {0}, InitDirBuffer[MAX_PATH] = {0};
					GetWindowText(TitleBox, (LPSTR)TitleBuffer, 0x200);
					GetWindowText(CmdLineBox, (LPSTR)CmdLineBuffer, MAX_PATH);
					GetWindowText(InitDirBox, (LPSTR)InitDirBuffer, MAX_PATH);

					if ((strlen(TitleBuffer) < 1 || strlen(CmdLineBuffer) < 1 || strlen(InitDirBuffer) < 1) ||
						(strstr(TitleBuffer, "=") || strstr(CmdLineBuffer, "=") || strstr(InitDirBuffer, "=")) ||
						(strstr(TitleBuffer, "|") || strstr(CmdLineBuffer, "|") || strstr(InitDirBuffer, "|")))
					{
						BGSEEUI->MsgBoxW(hWnd, NULL, "Invalid input. Make sure the strings are non-null and don't contain a '=' or a '|'.");
						break;
					}

					Tool* NewTool = Instance->AddTool(TitleBuffer, CmdLineBuffer, InitDirBuffer);
					if (NewTool == NULL)
					{
						BGSEEUI->MsgBoxE(hWnd, NULL, "Enter a unique tool title.");
						break;
					}

					int Index = SendMessage(ToolList, LB_INSERTSTRING, -1, (LPARAM)NewTool->Title.c_str());
					SendMessage(ToolList, LB_SETITEMDATA, Index, (LPARAM)NewTool);
				}

				break;
			case IDC_BGSEE_TOOLBOX_REMOVE:
				{
					int Index = SendMessage(ToolList, LB_GETCURSEL, NULL, NULL);
					if (Index != LB_ERR)
					{
						Tool* Selection = (Tool*)ListBox_GetItemData(ToolList, Index);
						if (Selection)
						{
							Instance->RemoveTool(Selection, true);
							SendMessage(ToolList, LB_DELETESTRING, Index, NULL);
							SendMessage(ToolList, LB_SETSEL, FALSE, -1);
							SendMessage(hWnd, IDM_BGSEE_TOOLBOX_RESETINPUTFIELDS, NULL, NULL);
						}
					}
				}

				break;
			case IDC_BGSEE_TOOLBOX_MOVEDOWN:
			case IDC_BGSEE_TOOLBOX_MOVEUP:
				{
					int Index = SendMessage(ToolList, LB_GETCURSEL, NULL, NULL);
					if (Index != LB_ERR)
					{
						SendMessage(ToolList, LB_GETTEXT, Index, (LPARAM)Buffer);
						Tool* Selection = (Tool*)SendMessage(ToolList, LB_GETITEMDATA, Index, NULL);

						int NewIndex = 0;
						if (LOWORD(wParam) == IDC_BGSEE_TOOLBOX_MOVEDOWN)
							NewIndex = Index + 1;
						else
							NewIndex = Index - 1;

						if (NewIndex < 0)
							NewIndex = 0;
						else if (NewIndex == SendMessage(ToolList, LB_GETCOUNT, NULL, NULL))
							NewIndex--;

						SendMessage(ToolList, LB_SETSEL, FALSE, Index);
						SendMessage(ToolList, LB_DELETESTRING, Index, NULL);
						SendMessage(ToolList, LB_INSERTSTRING, NewIndex, (LPARAM)Buffer);
						SendMessage(ToolList, LB_SETITEMDATA, NewIndex, (LPARAM)Selection);
						SendMessage(ToolList, LB_SETSEL, TRUE, NewIndex);
						SetFocus(ToolList);
					}
				}

				break;
			case IDC_BGSEE_TOOLBOX_SELECTCMDLINE:
				{
					char FilePath[MAX_PATH] = {0};

					OPENFILENAME SelectFile = {0};
					SelectFile.lStructSize = sizeof(OPENFILENAME);
					SelectFile.hwndOwner = hWnd;
					SelectFile.lpstrFilter = "All Files\0*.*\0\0";
					SelectFile.lpstrCustomFilter = NULL;
					SelectFile.nFilterIndex = 0;
					SelectFile.lpstrFile = FilePath;
					SelectFile.nMaxFile = sizeof(FilePath);
					SelectFile.lpstrFileTitle = NULL;
					SelectFile.lpstrInitialDir = NULL;
					SelectFile.lpstrTitle = "Select a file";
					SelectFile.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR;

					if (GetOpenFileName(&SelectFile))
					{
						SetWindowText(CmdLineBox, (LPSTR)FilePath);
					}
				}

				break;
			case IDC_BGSEE_TOOLBOX_SELECTINITDIR:
				{
					char FolderPath[MAX_PATH] = {0};

					BROWSEINFO FolderInfo = {0};
					FolderInfo.hwndOwner = hWnd;
					FolderInfo.iImage = NULL;
					FolderInfo.pszDisplayName = FolderPath;
					FolderInfo.lpszTitle = "Select an initial directory for the tool";
					FolderInfo.ulFlags = BIF_NEWDIALOGSTYLE|BIF_RETURNONLYFSDIRS;
					FolderInfo.pidlRoot = NULL;
					FolderInfo.lpfn = NULL;
					FolderInfo.lParam = NULL;

					PIDLIST_ABSOLUTE ReturnPath = SHBrowseForFolder(&FolderInfo);
					if (ReturnPath)
					{
						if (SHGetPathFromIDList(ReturnPath, FolderPath))
						{
							SetWindowText(InitDirBox, (LPSTR)FolderPath);
						}
					}
				}

				break;
			case IDC_BGSEE_APPLY:
				{
					int Index = SendMessage(ToolList, LB_GETCURSEL, NULL, NULL);
					if (Index != LB_ERR)
					{
						Tool* Selection = (Tool*)SendMessage(ToolList, LB_GETITEMDATA, Index, NULL);
						char CmdLineBuffer[MAX_PATH] = {0}, InitDirBuffer[MAX_PATH] = {0};
						GetWindowText(CmdLineBox, (LPSTR)CmdLineBuffer, MAX_PATH);
						GetWindowText(InitDirBox, (LPSTR)InitDirBuffer, MAX_PATH);

						if (strlen(CmdLineBuffer) > 0 && strlen(InitDirBuffer) > 0)
						{
							if ((strstr(CmdLineBuffer, "=") || strstr(InitDirBuffer, "=")) ||
								(strstr(CmdLineBuffer, "|") || strstr(InitDirBuffer, "|")))
							{
								BGSEEUI->MsgBoxW(hWnd, NULL, "Invalid input. Make sure the strings are non-null and don't contain a '=' or a '|'.");
								break;
							}

							Selection->CommandLine = CmdLineBuffer;
							Selection->InitialDir = InitDirBuffer;
						}
					}
				}

				break;
			case IDC_BGSEE_TOOLBOX_TOOLLIST:
				{
					switch (HIWORD(wParam))
					{
					case LBN_SELCHANGE:
						int Index = SendMessage(ToolList, LB_GETCURSEL, NULL, NULL);
						if (Index != LB_ERR)
						{
							Tool* Selection = (Tool*)SendMessage(ToolList, LB_GETITEMDATA, Index, NULL);
							SetWindowText(TitleBox, (LPSTR)Selection->Title.c_str());
							SetWindowText(CmdLineBox, (LPSTR)Selection->CommandLine.c_str());
							SetWindowText(InitDirBox, (LPSTR)Selection->InitialDir.c_str());
						}
						else
							SendMessage(hWnd, IDM_BGSEE_TOOLBOX_RESETINPUTFIELDS, NULL, NULL);

						break;
					}
				}

				break;
			}

			break;
		case WM_INITDIALOG:
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);
			UserData = (DlgUserData*)lParam;
			Instance = UserData->Instance;
			Instance->EnumerateToolsInListBox(ToolList);

			break;
		case WM_DESTROY:
			delete UserData;

			break;
		}

		return FALSE;
	}

	BGSEEToolBox::BGSEEToolBox()
	{
		INIGetter = NULL;
		INISetter = NULL;
		Initialized = false;
	}

	BGSEEToolBox::Tool::Tool( const char* Title, const char* CommandLine, const char* InitialDir ) :
		Title(Title), CommandLine(CommandLine), InitialDir(InitialDir)
	{
		;//
	}

	BGSEEToolBox::Tool::~Tool()
	{
		;//
	}

	bool BGSEEToolBox::Tool::Run() const
	{
		DWORD Result = (DWORD)ShellExecute(NULL, "open", CommandLine.c_str(), NULL, InitialDir.c_str(), SW_SHOW);
		if (Result <= 32)
		{
			SetLastError(Result);
			BGSEECONSOLE_ERROR("Couldn't execute tool '%s'", Title.c_str());

			return false;
		}

		BGSEECONSOLE_MESSAGE("Executed tool '%s'", Title.c_str());
		return true;
	}

	BGSEEToolBox::Tool* BGSEEToolBox::AddTool( const char* Title, const char* CmdLine, const char* InitDir )
	{
		SME_ASSERT(Initialized);

		ToolListT::iterator Match;
		if (LookupToolByTitle(Title, Match))
			return NULL;

		Tool* NewTool = new Tool(Title, CmdLine, InitDir);
		RegisteredTools.push_back(NewTool);
		return NewTool;
	}

	void BGSEEToolBox::AddTool( BGSEEToolBox::Tool* Tool )
	{
		SME_ASSERT(Initialized);

		ToolListT::iterator Match;
		if (LookupToolByTitle(Tool->Title.c_str(), Match))
			return;

		RegisteredTools.push_back(Tool);
	}

	void BGSEEToolBox::RemoveTool( const char* Title, bool ReleaseMemory /*= false*/ )
	{
		SME_ASSERT(Initialized);

		ToolListT::iterator Match;
		if (LookupToolByTitle(Title, Match) == false)
			return;

		if (ReleaseMemory)
			delete *Match;

		RegisteredTools.erase(Match);
	}

	void BGSEEToolBox::RemoveTool( BGSEEToolBox::Tool* Tool, bool ReleaseMemory /*= false*/ )
	{
		SME_ASSERT(Initialized);

		ToolListT::iterator Match;
		if (LookupToolByTitle(Tool->Title.c_str(), Match) == false)
			return;

		if (ReleaseMemory)
			delete *Match;

		RegisteredTools.erase(Match);
	}

	void BGSEEToolBox::ClearTools( bool ReleaseMemory /*= true*/ )
	{
		SME_ASSERT(Initialized);

		if (ReleaseMemory)
		{
			for (ToolListT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++)
				delete *Itr;
		}

		RegisteredTools.clear();
	}

	void BGSEEToolBox::INISaveToolList( void )
	{
		SME_ASSERT(INISetter);

		INISetter->operator()(kINISection, NULL);
		for (ToolListT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++)
		{
			INISetter->operator()((*Itr)->Title.c_str(),
								kINISection,
								(std::string((*Itr)->CommandLine + "|" + (*Itr)->InitialDir).c_str()));
		}
	}

	void BGSEEToolBox::INILoadToolList( void )
	{
		SME_ASSERT(INIGetter);

		char SectionBuffer[0x8000] = {0};
		ClearTools(true);

		INIGetter->operator()(kINISection, SectionBuffer, sizeof(SectionBuffer));

		for (const char* Itr = SectionBuffer; *Itr != '\0'; Itr += strlen(Itr) + 1)
		{
			std::string SectionData(Itr);
			size_t IndexA = SectionData.find("="), IndexB = SectionData.find("|");

			if (IndexA != std::string::npos && IndexB != std::string::npos)
			{
				std::string Title(SectionData.substr(0, IndexA));
				std::string CmdLine(SectionData.substr(IndexA + 1, IndexB - IndexA - 1));
				std::string InitDir(SectionData.substr(IndexB + 1));

				AddTool(Title.c_str(), CmdLine.c_str(), InitDir.c_str());
				BGSEECONSOLE_MESSAGE("[%s] - {%s}", Title.c_str(), CmdLine.c_str());
			}
		}
	}

	void BGSEEToolBox::EnumerateToolsInListBox( HWND ListBox )
	{
		SME_ASSERT(Initialized);

		for (ToolListT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++)
		{
			int Index = SendMessage(ListBox, LB_INSERTSTRING, -1, (LPARAM)(*Itr)->Title.c_str());
			SendMessage(ListBox, LB_SETITEMDATA, Index, (LPARAM)*Itr);
		}
	}

	bool BGSEEToolBox::LookupToolByTitle( const char* Title, ToolListT::iterator& Match )
	{
		SME_ASSERT(Initialized);

		bool Result = false;

		for (ToolListT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++)
		{
			if (!_stricmp((*Itr)->Title.c_str(), Title))
			{
				Match = Itr;
				Result = true;
				break;
			}
		}

		return Result;
	}

	BGSEEToolBox::~BGSEEToolBox()
	{
		Singleton = NULL;

		INISaveToolList();
		SAFEDELETE(INIGetter);
		SAFEDELETE(INISetter);
		ClearTools(true);

		Initialized = false;
	}

	BGSEEToolBox* BGSEEToolBox::GetSingleton()
	{
		if (Singleton == NULL)
			Singleton = new BGSEEToolBox();

		return Singleton;
	}

	bool BGSEEToolBox::Initialize( BGSEEINIManagerGetterFunctor* Getter, BGSEEINIManagerSetterFunctor* Setter, bool LoadFromINI /*= true*/ )
	{
		if (Initialized)
			return false;

		SME_ASSERT(Getter && Setter);

		INIGetter = Getter;
		INISetter = Setter;
		if (LoadFromINI)
			INILoadToolList();

		Initialized = true;
		return Initialized;
	}

	void BGSEEToolBox::ShowGUI( HINSTANCE ResourceInstance, HWND Parent )
	{
		BGSEEUI->ModalDialog(ResourceInstance, MAKEINTRESOURCE(IDD_BGSEE_TOOLBOX), Parent, GUIDlgProc);
	}

	void BGSEEToolBox::ShowToolListMenu( HINSTANCE ResourceInstance, HWND Parent, LPARAM Coords )
	{
		SME_ASSERT(Initialized);

		RECT Rect;
		POINT Point;

		GetClientRect(Parent, &Rect);
		Point.x = GET_X_LPARAM(Coords);
		Point.y = GET_Y_LPARAM(Coords);

		if (PtInRect(&Rect, Point))
		{
			ClientToScreen(Parent, &Point);
			HMENU ToolMenu = LoadMenu(ResourceInstance, (LPCSTR)IDR_BGSEE_TOOLBOX_TOOLMENU);

			int i = 1;
			for (ToolListT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++, i++)
			{
				MENUITEMINFO ToolMenuItem = {0};
				ToolMenuItem.cbSize = sizeof(MENUITEMINFO);
				ToolMenuItem.fMask = MIIM_ID|MIIM_STRING|MIIM_DATA;
				ToolMenuItem.wID = i;
				ToolMenuItem.dwTypeData = (LPSTR)(*Itr)->Title.c_str();
				ToolMenuItem.cch = 0;
				ToolMenuItem.dwItemData = (ULONG_PTR)(*Itr);
				InsertMenuItem(ToolMenu, GetMenuItemCount(ToolMenu) /*-1*/, TRUE, &ToolMenuItem);
			}

			int Selection = TrackPopupMenu(ToolMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, Point.x, Point.y, 0, Parent, NULL);
			switch (Selection)
			{
			case 0:
				break;
			case IDC_BGSEE_TOOLBOX_TOOLMENU_MANAGE:
				ShowGUI(ResourceInstance, Parent);
				break;
			default:
				MENUITEMINFO ToolMenuItem = {0};
				ToolMenuItem.cbSize = sizeof(MENUITEMINFO);
				ToolMenuItem.fMask = MIIM_DATA;
				GetMenuItemInfo(ToolMenu, Selection, FALSE, &ToolMenuItem);

				Tool* SelectedTool = (Tool*)ToolMenuItem.dwItemData;
				SME_ASSERT(SelectedTool);

				SelectedTool->Run();
				break;
			}

			DestroyMenu(ToolMenu);
		}
	}
}