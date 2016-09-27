#include "Main.h"
#include "Console.h"
#include "UIManager.h"
#include "ToolBox.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace bgsee
{
	ToolBox*						ToolBox::Singleton = nullptr;

	const char*						ToolBox::kINISection = "ToolBox";

#define IDM_BGSEE_TOOLBOX_RESETINPUTFIELDS		(WM_USER + 5002)

	BOOL CALLBACK ToolBox::GUIDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		HWND ToolList = GetDlgItem(hWnd, IDC_BGSEE_TOOLBOX_TOOLLIST);
		HWND TitleBox =  GetDlgItem(hWnd, IDC_BGSEE_TOOLBOX_TITLEBOX);
		HWND CmdLineBox =  GetDlgItem(hWnd, IDC_BGSEE_TOOLBOX_CMDLINEBOX);
		HWND InitDirBox =  GetDlgItem(hWnd, IDC_BGSEE_TOOLBOX_INITDIRBOX);
		HWND ParamBox =  GetDlgItem(hWnd, IDC_BGSEE_TOOLBOX_PARAMBOX);

		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		ToolBox* Instance = nullptr;

		if (UserData)
			Instance = UserData->Instance;

		switch (uMsg)
		{
		case IDM_BGSEE_TOOLBOX_RESETINPUTFIELDS:
			SetWindowText(TitleBox, nullptr);
			SetWindowText(CmdLineBox, nullptr);
			SetWindowText(InitDirBox, nullptr);
			SetWindowText(ParamBox, nullptr);
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
					char TitleBuffer[0x200] = {0}, CmdLineBuffer[MAX_PATH] = {0}, InitDirBuffer[MAX_PATH] = {0}, ParamsBuffer[0x200] = {0};
					GetWindowText(TitleBox, (LPSTR)TitleBuffer, 0x200);
					GetWindowText(CmdLineBox, (LPSTR)CmdLineBuffer, MAX_PATH);
					GetWindowText(InitDirBox, (LPSTR)InitDirBuffer, MAX_PATH);
					GetWindowText(ParamBox, (LPSTR)ParamsBuffer, 0x200);

					if ((strlen(TitleBuffer) < 1 || strlen(CmdLineBuffer) < 1 || strlen(InitDirBuffer) < 1) ||
						((std::string(TitleBuffer)).find_first_of("~=|") != std::string::npos ||
						(std::string(CmdLineBuffer)).find_first_of("~=|") != std::string::npos ||
						(std::string(InitDirBuffer)).find_first_of("~=|") != std::string::npos ||
						(std::string(ParamsBuffer)).find_first_of("~=|") != std::string::npos))
					{
						BGSEEUI->MsgBoxW(hWnd, NULL, "Invalid input. Make sure the strings are non-null and don't contain any of the following reserved characters - ~, =, |");
						break;
					}

					Tool* NewTool = Instance->AddTool(TitleBuffer, CmdLineBuffer, InitDirBuffer, ParamsBuffer);
					if (NewTool == nullptr)
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
						char Buffer[0x200] = {0};

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

						SendMessage(ToolList, LB_SETCURSEL, -1, NULL);
						SendMessage(ToolList, LB_DELETESTRING, Index, NULL);
						SendMessage(ToolList, LB_INSERTSTRING, NewIndex, (LPARAM)Buffer);
						SendMessage(ToolList, LB_SETITEMDATA, NewIndex, (LPARAM)Selection);
						SendMessage(ToolList, LB_SETCURSEL, NewIndex, NULL);
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
					SelectFile.lpstrCustomFilter = nullptr;
					SelectFile.nFilterIndex = 0;
					SelectFile.lpstrFile = FilePath;
					SelectFile.nMaxFile = sizeof(FilePath);
					SelectFile.lpstrFileTitle = nullptr;
					SelectFile.lpstrInitialDir = nullptr;
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
					FolderInfo.iImage = 0;
					FolderInfo.pszDisplayName = FolderPath;
					FolderInfo.lpszTitle = "Select an initial directory for the tool";
					FolderInfo.ulFlags = BIF_NEWDIALOGSTYLE|BIF_RETURNONLYFSDIRS;
					FolderInfo.pidlRoot = nullptr;
					FolderInfo.lpfn = nullptr;
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
						char CmdLineBuffer[MAX_PATH] = {0}, InitDirBuffer[MAX_PATH] = {0}, ParamsBuffer[0x200] = {0};
						GetWindowText(CmdLineBox, (LPSTR)CmdLineBuffer, MAX_PATH);
						GetWindowText(InitDirBox, (LPSTR)InitDirBuffer, MAX_PATH);
						GetWindowText(ParamBox, (LPSTR)ParamsBuffer, 0x200);

						if (strlen(CmdLineBuffer) > 0 && strlen(InitDirBuffer) > 0)
						{
							if 	((std::string(CmdLineBuffer)).find_first_of("~=|") != std::string::npos ||
								(std::string(InitDirBuffer)).find_first_of("~=|") != std::string::npos ||
								(std::string(ParamsBuffer)).find_first_of("~=|") != std::string::npos)
							{
								BGSEEUI->MsgBoxW(hWnd, NULL, "Invalid input. Make sure the strings are non-null and don't contain any of the following reserved characters - ~, =, |");
								break;
							}

							Selection->CommandLine = CmdLineBuffer;
							Selection->InitialDir = InitDirBuffer;
							Selection->Parameters = ParamsBuffer;
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
							SetWindowText(ParamBox, (LPSTR)Selection->Parameters.c_str());
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
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);
			UserData = (DlgUserData*)lParam;
			Instance = UserData->Instance;
			Instance->EnumerateToolsInListBox(ToolList);

			break;
		case WM_DESTROY:
			delete UserData;
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)0);

			break;
		}

		return FALSE;
	}

	ToolBox::ToolBox(INIManagerGetterFunctor& Getter, INIManagerSetterFunctor& Setter, bool LoadFromINI) :
		RegisteredTools(),
		INIGetter(),
		INISetter(),
		Initialized(false)
	{
		SME_ASSERT(Singleton == nullptr);
		Singleton = this;

		INIGetter = Getter;
		INISetter = Setter;
		Initialized = true;

		if (LoadFromINI)
			INILoadToolList();
	}

	ToolBox::Tool::Tool( const char* Title, const char* CommandLine, const char* InitialDir, const char* Parameters ) :
		Title(Title), CommandLine(CommandLine), InitialDir(InitialDir), Parameters(Parameters)
	{
		;//
	}

	ToolBox::Tool::~Tool()
	{
		;//
	}

	bool ToolBox::Tool::Run() const
	{
		DWORD Result = (DWORD)ShellExecute(nullptr,
										"open",
										CommandLine.c_str(),
										(Parameters.length() ? Parameters.c_str() : nullptr),
										InitialDir.c_str(),
										SW_SHOW);

		if (Result <= 32)
		{
			SetLastError(Result);
			BGSEECONSOLE_ERROR("Couldn't execute tool '%s'", Title.c_str());

			return false;
		}

		BGSEECONSOLE_MESSAGE("Executed tool '%s'", Title.c_str());
		return true;
	}

	ToolBox::Tool* ToolBox::AddTool( const char* Title, const char* CmdLine, const char* InitDir, const char* Params )
	{
		ToolArrayT::iterator Match;
		if (LookupToolByTitle(Title, Match))
			return nullptr;

		Tool* NewTool = new Tool(Title, CmdLine, InitDir, Params);
		RegisteredTools.push_back(NewTool);
		return NewTool;
	}

	void ToolBox::AddTool( ToolBox::Tool* Tool )
	{
		ToolArrayT::iterator Match;
		if (LookupToolByTitle(Tool->Title.c_str(), Match))
			return;

		RegisteredTools.push_back(Tool);
	}

	void ToolBox::RemoveTool( const char* Title, bool ReleaseMemory /*= false*/ )
	{
		ToolArrayT::iterator Match;
		if (LookupToolByTitle(Title, Match) == false)
			return;

		if (ReleaseMemory)
			delete *Match;

		RegisteredTools.erase(Match);
	}

	void ToolBox::RemoveTool( ToolBox::Tool* Tool, bool ReleaseMemory /*= false*/ )
	{
		ToolArrayT::iterator Match;
		if (LookupToolByTitle(Tool->Title.c_str(), Match) == false)
			return;

		if (ReleaseMemory)
			delete *Match;

		RegisteredTools.erase(Match);
	}

	void ToolBox::ClearTools( bool ReleaseMemory /*= true*/ )
	{
		if (ReleaseMemory)
		{
			for (ToolArrayT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++)
				delete *Itr;
		}

		RegisteredTools.clear();
	}

	void ToolBox::INISaveToolList( void )
	{
		INISetter(kINISection, nullptr);

		for (ToolArrayT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++)
		{
			INISetter((*Itr)->Title.c_str(),
					kINISection,
					(std::string((*Itr)->CommandLine + "|" + (*Itr)->InitialDir + "~" + (*Itr)->Parameters).c_str()));
		}
	}

	void ToolBox::INILoadToolList( void )
	{
		char SectionBuffer[0x8000] = {0};
		ClearTools(true);

		INIGetter(kINISection, SectionBuffer, sizeof(SectionBuffer));

		for (const char* Itr = SectionBuffer; *Itr != '\0'; Itr += strlen(Itr) + 1)
		{
			std::string SectionData(Itr);
			size_t IndexA = SectionData.find("="), IndexB = SectionData.find("|"), IndexC = SectionData.find("~");

			if (IndexA != std::string::npos && IndexB != std::string::npos && IndexC != std::string::npos)
			{
				std::string Title(SectionData.substr(0, IndexA));
				std::string CmdLine(SectionData.substr(IndexA + 1, IndexB - IndexA - 1));
				std::string InitDir(SectionData.substr(IndexB + 1, IndexC - IndexB - 1));
				std::string Params(SectionData.substr(IndexC + 1));

				AddTool(Title.c_str(), CmdLine.c_str(), InitDir.c_str(), Params.c_str());
				BGSEECONSOLE_MESSAGE("[%s] - {%s}", Title.c_str(), CmdLine.c_str());
			}
		}
	}

	void ToolBox::EnumerateToolsInListBox( HWND ListBox )
	{
		for (ToolArrayT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++)
		{
			int Index = SendMessage(ListBox, LB_INSERTSTRING, -1, (LPARAM)(*Itr)->Title.c_str());
			SendMessage(ListBox, LB_SETITEMDATA, Index, (LPARAM)*Itr);
		}
	}

	bool ToolBox::LookupToolByTitle( const char* Title, ToolArrayT::iterator& Match )
	{
		bool Result = false;

		for (ToolArrayT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++)
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

	ToolBox::~ToolBox()
	{
		INISaveToolList();
		ClearTools(true);

		Initialized = false;

		Singleton = nullptr;
	}

	ToolBox* ToolBox::Get()
	{
		return Singleton;
	}

	bool ToolBox::Initialize( INIManagerGetterFunctor& Getter, INIManagerSetterFunctor& Setter, bool LoadFromINI /*= true*/ )
	{
		if (Singleton)
			return false;

		ToolBox* Buffer = new ToolBox(Getter, Setter, LoadFromINI);
		return Buffer->Initialized;
	}

	void ToolBox::Deinitialize()
	{
		SME_ASSERT(Singleton);
		delete Singleton;
	}

	void ToolBox::ShowGUI(HINSTANCE ResourceInstance, HWND Parent)
	{
		DlgUserData* Param = new DlgUserData();
		Param->Instance = this;

		BGSEEUI->ModalDialog(ResourceInstance, MAKEINTRESOURCE(IDD_BGSEE_TOOLBOX), Parent, GUIDlgProc, (LPARAM)Param);
	}

	void ToolBox::ShowToolListMenu( HINSTANCE ResourceInstance, HWND Parent, POINT* Coords )
	{
		POINT Point;

		if (Coords)
		{
			Point.x = Coords->x;
			Point.y = Coords->y;
		}
		else
			GetCursorPos(&Point);

		HMENU ToolMenu = CreatePopupMenu();
		AppendMenu(ToolMenu, NULL, IDC_BGSEE_TOOLBOX_TOOLMENU_MANAGE, "Manage");
		AppendMenu(ToolMenu, MF_SEPARATOR, NULL, nullptr);

		int i = 1;
		for (ToolArrayT::iterator Itr = RegisteredTools.begin(); Itr != RegisteredTools.end(); Itr++, i++)
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

		int Selection = TrackPopupMenu(ToolMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, Point.x, Point.y, 0, Parent, nullptr);
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