#include "BGSEEConsole.h"
#include "BGSEEScript\CodaVM.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace BGSEditorExtender
{
	const char*								BGSEEConsole::kCommandLinePrefix = "CMD";
	const char*								BGSEEConsole::kWindowTitle       = "Console Window";

#define BGSEECONSOLE_INISECTION				"Console"
	SME::INI::INISetting					BGSEEConsole::kINI_Top("Top", BGSEECONSOLE_INISECTION,
																"Dialog Rect Top",
																(SInt32)150);
	SME::INI::INISetting					BGSEEConsole::kINI_Left("Left", BGSEECONSOLE_INISECTION,
																"Dialog Rect Left",
																(SInt32)150);
	SME::INI::INISetting					BGSEEConsole::kINI_Right("Right", BGSEECONSOLE_INISECTION,
																"Dialog Rect Right",
																(SInt32)150);
	SME::INI::INISetting					BGSEEConsole::kINI_Bottom("Bottom", BGSEECONSOLE_INISECTION,
																"Dialog Rect Bottom",
																(SInt32)150);
	SME::INI::INISetting					BGSEEConsole::kINI_Visible("Visible", BGSEECONSOLE_INISECTION,
																"Dialog Visibility State",
																(SInt32)1);

	SME::INI::INISetting					BGSEEConsole::kINI_FontFace("FontFace", BGSEECONSOLE_INISECTION,
																		"Message log font type",
																		"Consolas");
	SME::INI::INISetting					BGSEEConsole::kINI_FontSize("FontSize", BGSEECONSOLE_INISECTION,
																		"Message log font size",
																		(SInt32)13);
	SME::INI::INISetting					BGSEEConsole::kINI_UpdatePeriod("UpdatePeriod", BGSEECONSOLE_INISECTION,
																			"Duration, in milliseconds, between message log updates",
																			(SInt32)1000);
	SME::INI::INISetting					BGSEEConsole::kINI_LogWarnings("LogWarnings", BGSEECONSOLE_INISECTION,
																			"Log editor warnings",
																			(SInt32)1);
	SME::INI::INISetting					BGSEEConsole::kINI_LogAssertions("LogAssertions", BGSEECONSOLE_INISECTION,
																			"Log editor assertions",
																			(SInt32)1);
	SME::INI::INISetting					BGSEEConsole::kINI_LogTimestamps("LogTimestamps", BGSEECONSOLE_INISECTION,
																			"Add timestamps to messages",
																			(SInt32)0);

#define IDM_BGSEE_CONSOLE_COMMANDLINE_RESETCOMMANDSTACK			(WM_USER + 5001)
#define ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_START		(WM_USER + 8001)
#define ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_END		(WM_USER + 9000)

	LRESULT CALLBACK BGSEEConsole::BaseDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return )
	{
		LRESULT DlgProcResult = FALSE;
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		BGSEEConsole* Instance = dynamic_cast<BGSEEConsole*>(UserData->Instance);
		Return = false;

		switch (uMsg)
		{
		case WM_DROPFILES:
			{
				// drag-drop coda script files to execute them
				HDROP DropData = (HDROP)wParam;

				char Buffer[MAX_PATH] = {0};
				UInt32 FileCount = DragQueryFile(DropData, 0xFFFFFFFF, Buffer, sizeof(Buffer));
				bool BadFile = false;
				std::vector<std::string> DroppedScripts;

				for (int i = 0; i < FileCount; i++)
				{
					if (DragQueryFile(DropData, i, Buffer, sizeof(Buffer)))
					{
						char* Extension = strrchr(Buffer, '.');
						if (Extension && !_stricmp(Extension, BGSEEScript::CodaScriptVM::kSourceExtension.c_str()))
						{
							*Extension = '\0';
							const char* FileName = strrchr(Buffer, '\\');
							if (FileName)
								DroppedScripts.push_back(std::string(++FileName));
						}
						else
							BadFile = true;
					}
				}

				if (BadFile)
					BGSEEUI->MsgBoxE(hWnd, 0, "Only Coda scripts may be dropped into the window.");

				if (DroppedScripts.size())
				{
					BGSEECONSOLE_MESSAGE("Executing %d Coda scripts...", DroppedScripts.size());
					BGSEECONSOLE->Indent();

					bool Throwaway = false;
					for (int i = 0; i < DroppedScripts.size(); i++)
						CODAVM->RunScript(DroppedScripts[i], NULL, NULL, Throwaway);

					BGSEECONSOLE->Exdent();
				}
			}

			break;
		case WM_INITMENUPOPUP:
			{
				if (wParam == (WPARAM)Instance->ContextMenuHandle)
				{
					// this means the contexts menu absolutely needs to be the last item
					HMENU ContextsMenu = GetSubMenu((HMENU)wParam, GetMenuItemCount((HMENU)wParam) - 1);

					if (Instance->GetActiveContext() == Instance->PrimaryContext)
						CheckMenuItem(ContextsMenu, ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_DEFAULT, MFS_CHECKED);
					else
						CheckMenuItem(ContextsMenu, ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_DEFAULT, MFS_UNCHECKED);

					if (Instance->SecondaryContexts.size())
						InsertMenu(ContextsMenu, -1, MF_BYPOSITION|MF_SEPARATOR, NULL, NULL);

					int i = 1;
					for (ContextListT::const_iterator Itr = Instance->SecondaryContexts.begin(); Itr != Instance->SecondaryContexts.end(); Itr++, i++)
					{
						MENUITEMINFO ContextMenuItem = {0};
						ContextMenuItem.cbSize = sizeof(MENUITEMINFO);
						ContextMenuItem.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING|MIIM_DATA;
						ContextMenuItem.wID = ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_START + i;
						if (Instance->GetActiveContext() == *Itr)
							ContextMenuItem.fState = MFS_ENABLED|MFS_CHECKED;
						else
							ContextMenuItem.fState = MFS_ENABLED;
						ContextMenuItem.dwTypeData = (LPSTR)(*Itr)->GetName();
						ContextMenuItem.cch = 0;
						ContextMenuItem.dwItemData = (ULONG_PTR)(*Itr);
						InsertMenuItem(ContextsMenu, GetMenuItemCount(ContextsMenu), TRUE, &ContextMenuItem);

						SME_ASSERT(i < ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_END);
					}

					UIExtraData* xData = (UIExtraData*)UserData->ExtraData;
					SME_ASSERT(xData);

					xData->SelectedContext = NULL;

					MENUITEMINFO ContextMenuItem = {0};
					ContextMenuItem.cbSize = sizeof(MENUITEMINFO);
					ContextMenuItem.fMask = MIIM_STATE;
					if (Instance->GetActiveContext()->HasLog())
						ContextMenuItem.fState = MFS_ENABLED;
					else
						ContextMenuItem.fState = MFS_DISABLED;
					SetMenuItemInfo((HMENU)wParam, ID_BGSEE_CONSOLE_CONTEXTMENU_OPENLOG, FALSE, &ContextMenuItem);
				}
			}

			break;
		case WM_UNINITMENUPOPUP:
			if (wParam == (WPARAM)Instance->ContextMenuHandle)
			{
				HMENU ContextsMenu = GetSubMenu((HMENU)wParam, GetMenuItemCount((HMENU)wParam) - 1);

				while (GetMenuItemCount(ContextsMenu) > 1)
					DeleteMenu(ContextsMenu, GetMenuItemCount(ContextsMenu) - 1, MF_BYPOSITION);
			}

			break;
		case WM_MENUSELECT:
			if (LOWORD(wParam) > ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_START &&
				LOWORD(wParam) < ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_END)
			{
				if ((HIWORD(wParam) & MF_MOUSESELECT))
				{
					MENUITEMINFO ContextMenuItem = {0};
					ContextMenuItem.cbSize = sizeof(MENUITEMINFO);
					ContextMenuItem.fMask = MIIM_DATA;
					GetMenuItemInfo((HMENU)lParam, LOWORD(wParam), FALSE, &ContextMenuItem);
					SME_ASSERT(ContextMenuItem.dwItemData);

					UIExtraData* xData = (UIExtraData*)UserData->ExtraData;
					SME_ASSERT(xData);

					xData->SelectedContext = (MessageLogContext*)ContextMenuItem.dwItemData;
				}
			}

			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case ID_BGSEE_CONSOLE_CONTEXTMENU_CLEAR:
				Instance->GetActiveContext()->Reset();
				break;
			case ID_BGSEE_CONSOLE_CONTEXTMENU_OPENLOG:
				Instance->GetActiveContext()->OpenLog();
				break;
			case ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_DEFAULT:
				Instance->ResetActiveContext();
				break;
			case ID_BGSEE_CONSOLE_CONTEXTMENU_WARNINGS:
				Instance->GetWarningManager()->ShowGUI(Instance->ResourceInstance, hWnd);
				break;
			default:
				if (LOWORD(wParam) > ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_START &&
					LOWORD(wParam) < ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_END)
				{
					UIExtraData* xData = (UIExtraData*)UserData->ExtraData;
					SME_ASSERT(xData);

					Instance->SetActiveContext(xData->SelectedContext);
				}

				break;
			}

			break;
		case WM_SIZE:
			{
				RECT CurrentRect = {0};
				HWND MessageLog = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_MESSAGELOG);
				HWND CommandLine = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_COMMANDLINE);

				UIExtraData* xData = (UIExtraData*)UserData->ExtraData;
				SME_ASSERT(xData);

				SetRect(&CurrentRect, 0, 0, LOWORD(lParam), HIWORD(lParam));
				int DeltaDlgWidth = (CurrentRect.right - xData->DialogInitBounds.right);
				int DeltaDlgHeight = (CurrentRect.bottom - xData->DialogInitBounds.bottom);
				int VerticalScrollWidth = GetSystemMetrics(SM_CXVSCROLL) + 4;
				int HorScrollWidth = GetSystemMetrics(SM_CXHSCROLL) + 4;
				HDWP DeferPosData = BeginDeferWindowPos(2);

				DeferWindowPos(DeferPosData, MessageLog, NULL,
							xData->MessageLogInitBounds.left,
							xData->MessageLogInitBounds.top,
							DeltaDlgWidth + xData->MessageLogInitBounds.right + VerticalScrollWidth,
							CurrentRect.bottom + xData->MessageLogInitBounds.bottom - xData->DialogInitBounds.bottom + HorScrollWidth,
							NULL);

				DeferWindowPos(DeferPosData, CommandLine, NULL,
							xData->CommandLineInitBounds.left,
							DeltaDlgHeight + xData->CommandLineInitBounds.top,
							DeltaDlgWidth + xData->CommandLineInitBounds.right,
							xData->CommandLineInitBounds.bottom,
							NULL);

				EndDeferWindowPos(DeferPosData);
			}

			break;
		case WM_DESTROY:
			{
				delete (UIExtraData*)UserData->ExtraData;
				UserData->ExtraData = NULL;

				DragAcceptFiles(hWnd, FALSE);
			}

			break;
		case WM_INITDIALOG:
			{
				UIExtraData* xData = new UIExtraData();
				UserData->ExtraData = (LPARAM)xData;

				POINT Position = {0};
				RECT Bounds = {0};

				HWND MessageLog = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_MESSAGELOG);
				HWND CommandLine = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_COMMANDLINE);

				GetClientRect(hWnd, &xData->DialogInitBounds);
				SME::UIHelpers::GetClientRectInitBounds(MessageLog, hWnd, &xData->MessageLogInitBounds);
				SME::UIHelpers::GetClientRectInitBounds(CommandLine, hWnd, &xData->CommandLineInitBounds);

				SendDlgItemMessage(hWnd, IDC_BGSEE_CONSOLE_MESSAGELOG, WM_SETFONT, (WPARAM)xData->MessageLogFont, (LPARAM)TRUE);
				SendDlgItemMessage(hWnd, IDC_BGSEE_CONSOLE_COMMANDLINE, WM_SETFONT, (WPARAM)xData->CommandLineFont, (LPARAM)TRUE);

				DragAcceptFiles(hWnd, TRUE);
			}

			break;
		}

		return DlgProcResult;
	}

#define IDC_BGSEE_CONSOLE_MESSAGELOG_REFRESHTIMER		0xC05

	LRESULT CALLBACK BGSEEConsole::MessageLogSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(GetAncestor(hWnd, GA_PARENT), GWL_USERDATA);
		BGSEEConsole* Instance = dynamic_cast<BGSEEConsole*>(UserData->Instance);

		switch (uMsg)
		{
		case WM_TIMER:
			switch (wParam)
			{
			case IDC_BGSEE_CONSOLE_MESSAGELOG_REFRESHTIMER:
				if (Instance->GetActiveContext()->GetState() == MessageLogContext::kState_Default)
					break;

				SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);
				int LogLength = GetWindowTextLength(hWnd);

				if (LogLength > kMessageLogCharLimit)
				{
					Edit_SetText(hWnd, NULL);
					Instance->GetActiveContext()->ClearBuffer();
				}

				switch (Instance->GetActiveContext()->GetState())
				{
				case MessageLogContext::kState_Reset:
					Edit_SetText(hWnd, NULL);
					break;
				case MessageLogContext::kState_Update:
					{
						Edit_SetText(hWnd, Instance->GetActiveContext()->GetBuffer());
						SendMessage(hWnd, WM_VSCROLL, SB_BOTTOM, NULL);
					}

					break;
				}

				SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);
				Instance->GetActiveContext()->SetState(MessageLogContext::kState_Default);

				break;
			}

			return FALSE;
		case WM_CONTEXTMENU:
			return SendMessage(GetAncestor(hWnd, GA_PARENT), uMsg, wParam, lParam);
		case WM_DESTROY:
			break;
		}

		WNDPROC Original = (WNDPROC)GetWindowLongPtr(hWnd, GWL_USERDATA);
		return CallWindowProc(Original, hWnd, uMsg, wParam, lParam);
	}

	LRESULT CALLBACK BGSEEConsole::CommandLineSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(GetAncestor(hWnd, GA_PARENT), GWL_USERDATA);
		BGSEEConsole* Instance = dynamic_cast<BGSEEConsole*>(UserData->Instance);

		switch (uMsg)
		{
		case IDM_BGSEE_CONSOLE_COMMANDLINE_RESETCOMMANDSTACK:
			switch (wParam)
			{
			case 1:
				while (Instance->CommandLineHistoryAuxiliary.empty() == false)
				{
					Instance->CommandLineHistory.push(Instance->CommandLineHistoryAuxiliary.top());
					Instance->CommandLineHistoryAuxiliary.pop();
				}

				break;
			case 2:
				while (Instance->CommandLineHistoryAuxiliary.empty() == false)
					Instance->CommandLineHistoryAuxiliary.pop();

				while (Instance->CommandLineHistory.empty() == false)
					Instance->CommandLineHistory.pop();

				break;
			}

			return TRUE;
		case WM_CHAR:
			if (wParam == VK_RETURN)
				return TRUE;

			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_RETURN:
				{
					char Buffer[0x200] = {0};
					Edit_GetText(hWnd, Buffer, sizeof(Buffer));

					if (strlen(Buffer) > 2)
					{
						Instance->ExecuteCommand(Buffer);
						SendMessage(hWnd, IDM_BGSEE_CONSOLE_COMMANDLINE_RESETCOMMANDSTACK, 1, NULL);
						Instance->CommandLineHistory.push(Buffer);
					}

					Edit_SetText(hWnd, NULL);
				}

				return TRUE;
			case VK_UP:
				if (Instance->CommandLineHistory.empty() == false)
				{
					std::string Command(Instance->CommandLineHistory.top());
					Edit_SetText(hWnd, Command.c_str());
					Edit_SetSel(hWnd, Command.length(), Command.length());
					Instance->CommandLineHistory.pop();
					Instance->CommandLineHistoryAuxiliary.push(Command);
				}

				return TRUE;
			case VK_DOWN:
				if (Instance->CommandLineHistoryAuxiliary.empty() == false)
				{
					std::string Command(Instance->CommandLineHistoryAuxiliary.top());
					Edit_SetText(hWnd, Command.c_str());
					Edit_SetSel(hWnd, Command.length(), Command.length());
					Instance->CommandLineHistoryAuxiliary.pop();
					Instance->CommandLineHistory.push(Command);
				}

				return TRUE;
			}

			break;
		case WM_DESTROY:
			break;
		}

		WNDPROC Original = (WNDPROC)GetWindowLongPtr(hWnd, GWL_USERDATA);
		return CallWindowProc(Original, hWnd, uMsg, wParam, lParam);
	}

	void BGSEEConsole::MessageLogContext::SetState( UInt8 NewState )
	{
		State = NewState;
	}

	BGSEEConsole::MessageLogContext::MessageLogContext( const char* ContextName, const char* ContextLogPath /*= NULL*/ )
	{
		this->Name = ContextName;
		if (ContextLogPath)
			this->LogPath = ContextLogPath;
		else
			this->LogPath = "";
		this->BackBuffer.reserve(BGSEEConsole::kMessageLogCharLimit);
		this->SetState(kState_Default);
	}

	void BGSEEConsole::MessageLogContext::Print( const char* Message )
	{
		std::string Buffer(Message);
		Buffer += "\r\n";
		BackBuffer += Buffer;

		SetState(kState_Update);
	}

	void BGSEEConsole::MessageLogContext::Reset()
	{
		ClearBuffer();
		SetState(kState_Reset);
	}

	UInt8 BGSEEConsole::MessageLogContext::GetState() const
	{
		return State;
	}

	void BGSEEConsole::MessageLogContext::OpenLog() const
	{
		if (HasLog())
		{
			ShellExecute(NULL, "open", (LPSTR)LogPath.c_str(), NULL, NULL, SW_SHOW);
		}
	}

	bool BGSEEConsole::MessageLogContext::HasLog() const
	{
		if (LogPath.size())
			return true;
		else
			return false;
	}

	const char* BGSEEConsole::MessageLogContext::GetName() const
	{
		return Name.c_str();
	}

	const char* BGSEEConsole::MessageLogContext::GetBuffer() const
	{
		return BackBuffer.c_str();
	}

	void BGSEEConsole::MessageLogContext::ClearBuffer()
	{
		BackBuffer.clear();
	}

	BGSEEConsole::MessageLogContext::~MessageLogContext()
	{
		;//
	}

	BGSEEConsole::DefaultDebugLogContext::DefaultDebugLogContext( BGSEEConsole* Parent, const char* DebugLogPath ) :
		BGSEEConsole::MessageLogContext("", DebugLogPath),
		DebugLog(NULL),
		Parent(Parent),
		PrintCallbacks()
	{
		SME_ASSERT(Parent);

		Open(DebugLogPath);
		IndentLevel = 0;
		ExecutingCallbacks = false;
	}

	void BGSEEConsole::DefaultDebugLogContext::Print( const char* Prefix, const char* Message )
	{
		SME_ASSERT(Prefix && Message);

		std::string Addend;

		if (kINI_LogTimestamps.GetData().i && strlen(Prefix))
		{
			char Buffer[0x32] = {0};
			SME::MiscGunk::GetTimeString(Buffer, sizeof(Buffer), "%H:%M:%S");
			Addend += "{" + std::string(Buffer) + "} ";
		}

		if (strlen(Prefix))
		{
			Addend += "[" + std::string(Prefix) + "]\t";

			for (int i = 0; i < IndentLevel; i++)
				Addend += "\t";
		}

		std::string FormattedMessage = Message;
		SME::StringHelpers::Erase(FormattedMessage, '\r');

		Addend += FormattedMessage;
		if (Addend.length() == 0 || Addend[Addend.length() - 1] != '\n')
			Addend += "\n";

		if (DebugLog)
		{
			Put(Addend.c_str());
			Flush();
		}

		SME::StringHelpers::Replace(FormattedMessage, '\n', (char)'\r\n');
		BackBuffer += Addend + "\r\n";

		SetState(kState_Update);

		if (ExecutingCallbacks == false && strlen(Message) > 0)
		{
			ExecutePrintCallbacks(Prefix, Message);
		}
	}

	void BGSEEConsole::DefaultDebugLogContext::Reset()
	{
		MessageLogContext::Reset();
	}

	UInt32 BGSEEConsole::DefaultDebugLogContext::Indent()
	{
		if (++IndentLevel > BGSEEConsole::kMaxIndentLevel)
			IndentLevel = BGSEEConsole::kMaxIndentLevel;

		return IndentLevel;
	}

	UInt32 BGSEEConsole::DefaultDebugLogContext::Exdent()
	{
		if (IndentLevel > 0)
			--IndentLevel;

		return IndentLevel;
	}

	void BGSEEConsole::DefaultDebugLogContext::ExdentAll()
	{
		IndentLevel = 0;
	}

	void BGSEEConsole::DefaultDebugLogContext::Pad( UInt32 Count )
	{
		for (int i = 0; i < Count; i++)
			Print("", "");
	}

	BGSEEConsole::DefaultDebugLogContext::~DefaultDebugLogContext()
	{
		if (DebugLog)
		{
			Flush();
			Close();
		}

		PrintCallbacks.clear();
	}

	void BGSEEConsole::DefaultDebugLogContext::ExecutePrintCallbacks( const char* Prefix, const char* Message )
	{
		ExecutingCallbacks = true;

		for (PrintCallbackListT::const_iterator Itr = PrintCallbacks.begin(); Itr != PrintCallbacks.end(); Itr++)
			(*Itr)(Prefix, Message);

		ExecutingCallbacks = false;
	}

	bool BGSEEConsole::DefaultDebugLogContext::LookupPrintCallback( BGSEEConsolePrintCallback Callback, PrintCallbackListT::iterator& Match )
	{
		bool Result = false;

		for (PrintCallbackListT::iterator Itr = PrintCallbacks.begin(); Itr != PrintCallbacks.end(); Itr++)
		{
			if (*Itr == Callback)
			{
				Match = Itr;
				Result = true;
				break;
			}
		}

		return Result;
	}

	bool BGSEEConsole::DefaultDebugLogContext::RegisterPrintCallback( BGSEEConsolePrintCallback Callback )
	{
		if (ExecutingCallbacks)
			return false;

		PrintCallbackListT::iterator Match;
		if (LookupPrintCallback(Callback, Match) == false)
		{
			PrintCallbacks.push_back(Callback);
			return true;
		}

		return false;
	}

	void BGSEEConsole::DefaultDebugLogContext::UnregisterPrintCallback( BGSEEConsolePrintCallback Callback )
	{
		if (ExecutingCallbacks)
			return;

		PrintCallbackListT::iterator Match;
		if (LookupPrintCallback(Callback, Match))
		{
			PrintCallbacks.erase(Match);
		}
	}

	void BGSEEConsole::DefaultDebugLogContext::Flush()
	{
		if (DebugLog)
			fflush(DebugLog);
	}

	void BGSEEConsole::DefaultDebugLogContext::Put( const char* String )
	{
		SME_ASSERT(String);

		if (DebugLog)
			fputs(String, DebugLog);
	}

	void BGSEEConsole::DefaultDebugLogContext::Close()
	{
		if (DebugLog)
		{
			fclose(DebugLog);
			DebugLog = NULL;
		}
	}

	bool BGSEEConsole::DefaultDebugLogContext::Open( const char* Path )
	{
		SME_ASSERT(DebugLog == NULL && Path);

		DebugLog = _fsopen(Path, "w", _SH_DENYWR);
		return DebugLog != NULL;
	}

	BGSEEConsole::ConsoleCommandTable::ConsoleCommandTable() : CommandList()
	{
		;//
	}

	BGSEEConsole::ConsoleCommandTable::~ConsoleCommandTable()
	{
		CommandList.clear();
	}

	bool BGSEEConsole::ConsoleCommandTable::LookupCommandByName( const char* Name, ConsoleCommandListT::iterator& Match )
	{
		bool Result = false;

		for (ConsoleCommandListT::iterator Itr = CommandList.begin(); Itr != CommandList.end(); Itr++)
		{
			if (!_stricmp((*Itr)->Name, Name))
			{
				Match = Itr;
				Result = true;
				break;
			}
		}

		return Result;
	}

	bool BGSEEConsole::ConsoleCommandTable::LookupCommandByInstance( BGSEEConsoleCommandInfo* Command, ConsoleCommandListT::iterator& Match )
	{
		bool Result = false;

		for (ConsoleCommandListT::iterator Itr = CommandList.begin(); Itr != CommandList.end(); Itr++)
		{
			if ((*Itr) == Command)
			{
				Match = Itr;
				Result = true;
				break;
			}
		}

		return Result;
	}

	bool BGSEEConsole::ConsoleCommandTable::AddCommand( BGSEEConsoleCommandInfo* Command )
	{
		ConsoleCommandListT::iterator Match;
		if (LookupCommandByName(Command->Name, Match) == false)
		{
			CommandList.push_back(Command);
			return true;
		}

		return false;
	}

	void BGSEEConsole::ConsoleCommandTable::RemoveCommand( BGSEEConsoleCommandInfo* Command )
	{
		ConsoleCommandListT::iterator Match;
		SME_ASSERT(LookupCommandByName(Command->Name, Match));

		CommandList.erase(Match);
	}

	BGSEEConsoleCommandInfo* BGSEEConsole::ConsoleCommandTable::GetCommand( const char* Name )
	{
		ConsoleCommandListT::iterator Match;
		if (LookupCommandByName(Name, Match) == false)
			return NULL;

		return *Match;
	}

	BGSEEConsole::UIExtraData::UIExtraData() :
		SelectedContext(NULL),
		MessageLogFont(NULL),
		CommandLineFont(NULL),
		DialogInitBounds(),
		MessageLogInitBounds(),
		CommandLineInitBounds()
	{
		ZeroMemory(&DialogInitBounds, sizeof(RECT));
		ZeroMemory(&MessageLogInitBounds, sizeof(RECT));
		ZeroMemory(&CommandLineInitBounds, sizeof(RECT));

		MessageLogFont = CreateFont(BGSEEConsole::kINI_FontSize.GetData().i, 0, 0, 0,
									FW_REGULAR,
									FALSE,
									FALSE,
									FALSE,
									ANSI_CHARSET,
									OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS,
									CLEARTYPE_QUALITY,
									FF_DONTCARE,
									BGSEEConsole::kINI_FontFace.GetData().s);

		CommandLineFont = CreateFont(24, 0, 0, 0,
									FW_BOLD,
									FALSE,
									FALSE,
									FALSE,
									ANSI_CHARSET,
									OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS,
									CLEARTYPE_QUALITY,
									FF_DONTCARE,
									"Consolas");
	}

	BGSEEConsole::UIExtraData::~UIExtraData()
	{
		if (MessageLogFont)
			DeleteFont(MessageLogFont);

		if (CommandLineFont)
			DeleteFont(CommandLineFont);
	}

	void BGSEEConsole::ClearMessageLog( void )
	{
		Edit_SetText(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), NULL);
	}

	void BGSEEConsole::SetTitle( const char* Prefix )
	{
		std::string Buffer(kWindowTitle);

		if (strlen(Prefix))
		{
			Buffer += " [" + std::string(Prefix) + "]";
		}

		SetWindowText(DialogHandle, Buffer.c_str());
	}

	BGSEEConsole::MessageLogContext* BGSEEConsole::GetActiveContext( void ) const
	{
		SME_ASSERT(ActiveContext);
		return ActiveContext;
	}

	void BGSEEConsole::SetActiveContext( MessageLogContext* Context )
	{
		SME_ASSERT(Context);

		if (ActiveContext == Context)
			return;

		std::string ContextBuffer(ActiveContext->GetBuffer());

		ActiveContext->BackBuffer = ContextBuffer;
		ActiveContext->SetState(MessageLogContext::kState_Default);

		ActiveContext = Context;
		ActiveContext->SetState(MessageLogContext::kState_Update);

		ClearMessageLog();
		SetTitle(ActiveContext->GetName());
	}

	void BGSEEConsole::ResetActiveContext( void )
	{
		SetActiveContext(PrimaryContext);
	}

	void BGSEEConsole::ExecuteCommand( const char* CommandExpression )
	{
		LogMsg(kCommandLinePrefix, CommandExpression);
		Indent();

		SME::StringHelpers::Tokenizer Tokenizer(CommandExpression, " ,");
		std::string CurrentToken;

		if (Tokenizer.NextToken(CurrentToken) != -1)
		{
			BGSEEConsoleCommandInfo* Command = CommandTable.GetCommand(CurrentToken.c_str());

			if (Command)
			{
				UInt32 ArgCount = 0;
				UInt32 Offset = Tokenizer.NextToken(CurrentToken);
				Tokenizer.PrevToken(CurrentToken);

				while (Tokenizer.NextToken(CurrentToken) != -1)
					ArgCount++;

				if (ArgCount < Command->ParamCount)
					LogMsg(BGSEEMAIN->ExtenderGetShortName(), "Too few arguments passed to console command '%s'. Expected %d.", Command->Name, Command->ParamCount);
				else
				{
					std::string Args;
					if (Command->ParamCount)
						Args = (std::string(CommandExpression)).substr(Offset, strlen(CommandExpression));

					Command->ExecuteHandler(Command->ParamCount, Args.c_str());
				}
			}
			else
				LogMsg(BGSEEMAIN->ExtenderGetShortName(), "Unknown command '%s'", CurrentToken.c_str());
		}

		Exdent();
	}

	bool BGSEEConsole::LookupSecondaryContextByName( const char* Name, ContextListT::iterator& Match )
	{
		bool Result = false;

		for (ContextListT::iterator Itr = SecondaryContexts.begin(); Itr != SecondaryContexts.end(); Itr++)
		{
			if (!_stricmp((*Itr)->GetName(), Name))
			{
				Match = Itr;
				Result = true;
				break;
			}
		}

		return Result;
	}

	bool BGSEEConsole::LookupSecondaryContextByInstance( MessageLogContext* Context, ContextListT::iterator& Match )
	{
		bool Result = false;

		for (ContextListT::iterator Itr = SecondaryContexts.begin(); Itr != SecondaryContexts.end(); Itr++)
		{
			if ((*Itr) == Context)
			{
				Match = Itr;
				Result = true;
				break;
			}
		}

		return Result;
	}

	void BGSEEConsole::ReleaseSecondaryContexts( void )
	{
		for (ContextListT::iterator Itr = SecondaryContexts.begin(); Itr != SecondaryContexts.end(); Itr++)
			delete (*Itr);

		SecondaryContexts.clear();
	}

	BGSEEConsole::BGSEEConsole( const char* LogPath ) :
		BGSEEGenericModelessDialog()
	{
		OwnerThreadID = GetCurrentThreadId();
		PrimaryContext = new DefaultDebugLogContext(this, LogPath);
		ActiveContext = PrimaryContext;

		DialogTemplateID = IDD_BGSEE_CONSOLE;
		DialogContextMenuID = IDR_BGSEE_CONSOLE_CONTEXTMENU;
		CallbackDlgProc = &BGSEEConsole::BaseDlgProc;
		WarningManager = NULL;
	}

	BGSEEConsole::~BGSEEConsole()
	{
		ActiveContext = NULL;
		SAFEDELETE(PrimaryContext);
		ReleaseSecondaryContexts();

		while (CommandLineHistory.size())
			CommandLineHistory.pop();

		while (CommandLineHistoryAuxiliary.size())
			CommandLineHistoryAuxiliary.pop();

		SAFEDELETE(WarningManager);

		INISaveUIState(&kINI_Top, &kINI_Left, &kINI_Right, &kINI_Bottom, &kINI_Visible);
		KillTimer(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), IDC_BGSEE_CONSOLE_MESSAGELOG_REFRESHTIMER);

		SetWindowLongPtr(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG),
					GWL_WNDPROC,
					GetWindowLongPtr(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), GWL_USERDATA));

		SetWindowLongPtr(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_COMMANDLINE),
					GWL_WNDPROC,
					GetWindowLongPtr(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_COMMANDLINE), GWL_USERDATA));
	}

	void BGSEEConsole::InitializeUI( HWND Parent, HINSTANCE Resource )
	{
		ParentHandle = Parent;
		ResourceInstance = Resource;

		Create(NULL, false, true);
		LONG OrgWndProc = SetWindowLongPtr(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), GWL_WNDPROC, (LONG)BGSEEConsole::MessageLogSubclassProc);
		SetWindowLongPtr(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), GWL_USERDATA, OrgWndProc);

		OrgWndProc = SetWindowLongPtr(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_COMMANDLINE), GWL_WNDPROC, (LONG)BGSEEConsole::CommandLineSubclassProc);
		SetWindowLongPtr(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_COMMANDLINE), GWL_USERDATA, OrgWndProc);

		Edit_LimitText(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), sizeof(int));

		SetTimer(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG),
				IDC_BGSEE_CONSOLE_MESSAGELOG_REFRESHTIMER,
				kINI_UpdatePeriod.GetData().i,
				NULL);

		INILoadUIState(&kINI_Top, &kINI_Left, &kINI_Right, &kINI_Bottom, &kINI_Visible);
	}

	void BGSEEConsole::InitializeWarningManager( BGSEEINIManagerGetterFunctor Getter, BGSEEINIManagerSetterFunctor Setter, BGSEEConsoleWarningRegistrar& Registrar )
	{
		SME_ASSERT(WarningManager == NULL);

		WarningManager = new BGSEEConsoleWarningManager(Getter, Setter);
		Registrar.operator()(WarningManager);
		WarningManager->INILoadWarnings();
	}

	void BGSEEConsole::LogMsg( std::string Prefix, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		PrimaryContext->Print(Prefix.c_str(), Buffer);
	}

	void BGSEEConsole::LogErrorMsg( std::string Prefix, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		PrimaryContext->Print(Prefix.c_str(), Buffer);

		LPVOID ErrorMsg;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&ErrorMsg,
					0,
					NULL);
		FORMAT_STR(Buffer, "Error Message: %s", ErrorMsg);
		LocalFree(ErrorMsg);

		Indent();
		PrimaryContext->Print("", Buffer);
		Exdent();

		MessageBeep(MB_ICONWARNING);
	}

	void BGSEEConsole::LogWarning( std::string Prefix, const char* Format, ... )
	{
		if (kINI_LogWarnings.GetData().i == 0)
			return;

		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		PrimaryContext->Print(Prefix.c_str(), Buffer);
	}

	void BGSEEConsole::LogAssertion( std::string Prefix, const char* Format, ... )
	{
		if (kINI_LogAssertions.GetData().i == 0)
			return;

		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		PrimaryContext->Print(Prefix.c_str(), Buffer);
	}

	UInt32 BGSEEConsole::Indent()
	{
		return PrimaryContext->Indent();
	}

	UInt32 BGSEEConsole::Exdent()
	{
		return PrimaryContext->Exdent();
	}

	void BGSEEConsole::ExdentAll()
	{
		PrimaryContext->ExdentAll();
	}

	void BGSEEConsole::Pad( UInt32 Count )
	{
		PrimaryContext->Pad(Count);
	}

	void* BGSEEConsole::RegisterMessageLogContext( const char* Name, const char* LogPath /*= NULL*/ )
	{
		ContextListT::iterator Match;
		if (LookupSecondaryContextByName(Name, Match))
			return NULL;

		MessageLogContext* NewContext = new MessageLogContext(Name, LogPath);
		SecondaryContexts.push_back(NewContext);
		return NewContext;
	}

	void BGSEEConsole::UnregisterMessageLogContext( void* Context )
	{
		ContextListT::iterator Match;
		SME_ASSERT(LookupSecondaryContextByInstance((MessageLogContext*)Context, Match));

		if (Context == GetActiveContext())
			ResetActiveContext();

		SecondaryContexts.erase(Match);
		delete Context;
	}

	void BGSEEConsole::PrintToMessageLogContext( void* Context, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		std::string Addend(Buffer);
		Addend += "\r\n";

		((MessageLogContext*)Context)->Print(Buffer);
	}

	bool BGSEEConsole::RegisterConsoleCommand( BGSEEConsoleCommandInfo* Command )
	{
		return CommandTable.AddCommand(Command);
	}

	void BGSEEConsole::UnregisterConsoleCommand( BGSEEConsoleCommandInfo* Command )
	{
		CommandTable.RemoveCommand(Command);
	}

	bool BGSEEConsole::RegisterPrintCallback( BGSEEConsolePrintCallback Callback )
	{
		return PrimaryContext->RegisterPrintCallback(Callback);
	}

	void BGSEEConsole::UnregisterPrintCallback( BGSEEConsolePrintCallback Callback )
	{
		PrimaryContext->UnregisterPrintCallback(Callback);
	}

	const char* BGSEEConsole::GetLogPath( void ) const
	{
		SME_ASSERT(PrimaryContext);
		return PrimaryContext->LogPath.c_str();
	}

	bool BGSEEConsole::GetLogsWarnings( void )
	{
		return kINI_LogWarnings.GetData().i != 0;
	}

	void BGSEEConsole::OpenDebugLog( void )
	{
		PrimaryContext->OpenLog();
	}

	void BGSEEConsole::FlushDebugLog( void )
	{
		PrimaryContext->Flush();
	}

	void BGSEEConsole::RegisterINISettings( INISettingDepotT& Depot )
	{
		Depot.push_back(&kINI_Top);
		Depot.push_back(&kINI_Left);
		Depot.push_back(&kINI_Right);
		Depot.push_back(&kINI_Bottom);
		Depot.push_back(&kINI_Visible);

		Depot.push_back(&kINI_FontFace);
		Depot.push_back(&kINI_FontSize);
		Depot.push_back(&kINI_UpdatePeriod);
		Depot.push_back(&kINI_LogWarnings);
		Depot.push_back(&kINI_LogAssertions);
		Depot.push_back(&kINI_LogTimestamps);
	}

	BGSEEConsoleWarningManager* BGSEEConsole::GetWarningManager( void ) const
	{
		SME_ASSERT(WarningManager);

		return WarningManager;
	}

	void BGSEEConsole::ToggleWarningLogging(bool State)
	{
		kINI_LogWarnings.SetInt(int(State));
	}

	BGSEEConsoleWarning::BGSEEConsoleWarning( const char* GUID, const char* Desc, UInt32 CallSiteCount, ... ) :
		BaseIDString(GUID),
		Description(Desc),
		CallSites(),
		Enabled(true)
	{
		SME_ASSERT(GUID);

		ZeroMemory(&BaseID, sizeof(BaseID));
		RPC_STATUS Result = UuidFromString((RPC_CSTR)GUID, &BaseID);

		SME_ASSERT(Result == RPC_S_OK);

		va_list Args;
		va_start(Args, CallSiteCount);

		for (int i = 0; i < CallSiteCount; i++)
		{
			WarningCallSiteT CurrentArg = va_arg(Args, WarningCallSiteT);
			SME_ASSERT(CurrentArg);

			CallSites.push_back(CurrentArg);
		}

		va_end(Args);
	}

	BGSEEConsoleWarning::~BGSEEConsoleWarning()
	{
		;//
	}

	bool BGSEEConsoleWarning::GetEnabled( void ) const
	{
		return Enabled;
	}

	const char* BGSEEConsoleWarningManager::kINISection = "ConsoleWarnings";

	BOOL CALLBACK BGSEEConsoleWarningManager::GUIDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		HWND ListView = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_WARNINGS_WARNINGSLIST);

		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		BGSEEConsoleWarningManager* Instance = NULL;

		if (UserData)
			Instance = UserData->Instance;

		switch (uMsg)
		{
		case WM_NOTIFY:
			{
				NMHDR* NotificationData = (NMHDR*)lParam;
				switch (NotificationData->code)
				{
				case LVN_GETDISPINFO:
					{
						NMLVDISPINFO* DisplayData = (NMLVDISPINFO*)lParam;

						if ((DisplayData->item.mask & LVIF_TEXT) && DisplayData->item.lParam)
						{
							BGSEEConsoleWarning* Current = (BGSEEConsoleWarning*)DisplayData->item.lParam;

							switch (DisplayData->item.iSubItem)
							{
							case 0:
								sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s", (Current->GetEnabled() ? "Y" : " "));
								break;
							case 1:
								sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s", Current->Description.c_str());
								break;
							}
						}
					}

					break;
				case LVN_ITEMACTIVATE:
					{
						NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;

						LVITEM Item = {0};
						Item.mask = LVIF_PARAM;
						Item.iItem = Data->iItem;
						ListView_GetItem(ListView, &Item);

						BGSEEConsoleWarning* Current = (BGSEEConsoleWarning*)Item.lParam;

						Current->Enabled = Current->Enabled == false;
						InvalidateRect(hWnd, NULL, TRUE);
					}

					break;
				}
			}

			break;
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
		case WM_INITDIALOG:
			{
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);
				UserData = (DlgUserData*)lParam;
				Instance = UserData->Instance;
				ListView = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_WARNINGS_WARNINGSLIST);

				LVCOLUMN ColumnData = {0};
				ColumnData.mask = LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM|LVCF_FMT;
				ColumnData.fmt = LVCFMT_LEFT;

				ColumnData.cx = 55;
				ColumnData.pszText = "Enabled";
				ColumnData.iSubItem = 0;
				ListView_InsertColumn(ListView, ColumnData.iSubItem, &ColumnData);

				ColumnData.cx = 475;
				ColumnData.pszText = "Warning";
				ColumnData.iSubItem = 1;
				ListView_InsertColumn(ListView, ColumnData.iSubItem, &ColumnData);

				ListView_SetExtendedListViewStyle(ListView, LVS_EX_FULLROWSELECT);
				Instance->EnumerateWarningsInListView(ListView);
			}

			break;
		case WM_DESTROY:
			delete UserData;
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)0);

			break;
		}

		return FALSE;
	}

	void BGSEEConsoleWarningManager::Clear()
	{
		for (WarningListT::iterator Itr = WarningDepot.begin(); Itr != WarningDepot.end(); Itr++)
			delete *Itr;

		WarningDepot.clear();
	}

	void BGSEEConsoleWarningManager::INISaveWarnings( void )
	{
		INISetter(kINISection, NULL);

		for (WarningListT::iterator Itr = WarningDepot.begin(); Itr != WarningDepot.end(); Itr++)
		{
			INISetter((*Itr)->BaseIDString.c_str(), kINISection, ((*Itr)->Enabled ? "1" : "0"));
		}
	}

	void BGSEEConsoleWarningManager::INILoadWarnings( void )
	{
		char SectionBuffer[0x8000] = {0};

		INIGetter(kINISection, SectionBuffer, sizeof(SectionBuffer));
		for (const char* Itr = SectionBuffer; *Itr != '\0'; Itr += strlen(Itr) + 1)
		{
			std::string SectionData(Itr);
			size_t Index = SectionData.find("=");

			if (Index != std::string::npos)
			{
				std::string GUID(SectionData.substr(0, Index));
				std::string State(SectionData.substr(Index + 1));

				BGSEEConsoleWarning* Current = LookupWarning(GUID.c_str());
				SME_ASSERT(Current);

				if (State == "1")
					Current->Enabled = true;
				else
					Current->Enabled = false;
			}
		}
	}

	BGSEEConsoleWarning* BGSEEConsoleWarningManager::LookupWarning( const char* GUID ) const
	{
		for (WarningListT::const_iterator Itr = WarningDepot.begin(); Itr != WarningDepot.end(); Itr++)
		{
			if (!_stricmp(GUID, (*Itr)->BaseIDString.c_str()))
				return *Itr;
		}

		return NULL;
	}

	BGSEEConsoleWarning* BGSEEConsoleWarningManager::LookupWarning( BGSEEConsoleWarning::WarningCallSiteT CallSite ) const
	{
		for (WarningListT::const_iterator Itr = WarningDepot.begin(); Itr != WarningDepot.end(); Itr++)
		{
			for (BGSEEConsoleWarning::CallSiteListT::iterator ItrEx = (*Itr)->CallSites.begin(); ItrEx != (*Itr)->CallSites.end(); ItrEx++)
			{
				if (*ItrEx == CallSite)
					return *Itr;
			}
		}

		return NULL;
	}

	BGSEEConsoleWarningManager::BGSEEConsoleWarningManager( BGSEEINIManagerGetterFunctor Getter, BGSEEINIManagerSetterFunctor Setter ) :
		WarningDepot(),
		INIGetter(Getter),
		INISetter(Setter)
	{
		;//
	}

	BGSEEConsoleWarningManager::~BGSEEConsoleWarningManager()
	{
		INISaveWarnings();
		Clear();
	}

	void BGSEEConsoleWarningManager::RegisterWarning( BGSEEConsoleWarning* Warning )
	{
		SME_ASSERT(Warning);

		WarningDepot.push_back(Warning);
	}

	bool BGSEEConsoleWarningManager::GetWarningEnabled( BGSEEConsoleWarning::WarningCallSiteT CallSite ) const
	{
		SME_ASSERT(CallSite);

		bool Result = true;
		BGSEEConsoleWarning* Warning = LookupWarning(CallSite);

		if (Warning && Warning->GetEnabled() == false)
			Result = false;

		return Result;
	}

	void BGSEEConsoleWarningManager::ShowGUI( HINSTANCE ResourceInstance, HWND Parent )
	{
		DlgUserData* Param = new DlgUserData();
		Param->Instance = this;

		BGSEEUI->ModalDialog(ResourceInstance, MAKEINTRESOURCE(IDD_BGSEE_CONSOLE_WARNINGS), Parent, GUIDlgProc, (LPARAM)Param);
	}

	void BGSEEConsoleWarningManager::EnumerateWarningsInListView( HWND ListView ) const
	{
		int Index = 0;
		for (WarningListT::const_iterator Itr = WarningDepot.begin(); Itr != WarningDepot.end(); Itr++)
		{
			LVITEM Item = {0};
			Item.mask = LVIF_PARAM|LVIF_TEXT|LVIF_STATE;
			Item.lParam = (LPARAM)*Itr;
			Item.pszText = LPSTR_TEXTCALLBACK;
			Item.iItem = Index++;

			ListView_InsertItem(ListView, &Item);
		}
	}
}