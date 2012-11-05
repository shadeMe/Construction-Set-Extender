#include "BGSEEConsole.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace BGSEditorExtender
{
	const char*								BGSEEConsole::kCommandLinePrefix = "CMD";
	const char*								BGSEEConsole::kWindowTitle = "Console Window";
	const char*								BGSEEConsole::kINISection = "Console";

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
		case WM_INITMENUPOPUP:
			{
				if (wParam == (WPARAM)Instance->ContextMenuHandle)
				{
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

					UserData->UserData = NULL;
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

					UserData->UserData = (LPARAM)ContextMenuItem.dwItemData;
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
			default:
				if (LOWORD(wParam) > ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_START &&
					LOWORD(wParam) < ID_BGSEE_CONSOLE_CONTEXTMENU_CONTEXTS_CUSTOM_END)
				{
					Instance->SetActiveContext((MessageLogContext*)UserData->UserData);
				}

				break;
			}

			break;
		case WM_SIZE:
			{
				RECT DialogRect, MessageLogRect;
				GetClientRect(hWnd, &DialogRect);
				GetClientRect(GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_MESSAGELOG), &MessageLogRect);

				MoveWindow(GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_MESSAGELOG),
							0,
							0,
							DialogRect.right,
							DialogRect.bottom - 35,
							TRUE);

				MoveWindow(GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_COMMANDLINE),
							0,
							MessageLogRect.bottom + 22,
							DialogRect.right,
							DialogRect.bottom - MessageLogRect.bottom - 22,
							TRUE);
			}

			break;
		case WM_DESTROY:
			{
				HFONT CommandLineFont = (HFONT)SendDlgItemMessage(hWnd, IDC_BGSEE_CONSOLE_COMMANDLINE, WM_GETFONT, NULL, NULL);
				DeleteObject(CommandLineFont);
			}

			break;
		case WM_INITDIALOG:
			{
				HFONT CommandLineFont = CreateFont(24,
													0,
													0,
													0,
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

				SendDlgItemMessage(hWnd, IDC_BGSEE_CONSOLE_COMMANDLINE, WM_SETFONT, (WPARAM)CommandLineFont, (LPARAM)TRUE);
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
		this->LogPath = ContextLogPath;
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
		if (LogPath.size())
		{
			ShellExecute(NULL, "open", (LPSTR)LogPath.c_str(), NULL, NULL, SW_SHOW);
		}
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

		if (atoi(Parent->INISettingGetter(Parent->kConsoleSpecificINISettings[BGSEEConsole::kConsoleSpecificINISetting_LogTimestamps].Key, Parent->kINISection)) &&
			strlen(Prefix))
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

	const BGSEEINIManagerSettingFactory::SettingData		BGSEEConsole::kConsoleSpecificINISettings[4] =
	{
		{ "UpdatePeriod",		"1000",		"Duration, in milliseconds, between message log updates" },
		{ "LogWarnings",		"1",		"Log editor warnings" },
		{ "LogAssertions",		"1",		"Log editor assertions" },
		{ "LogTimestamps",		"0",		"Add timestamps to messages" },
	};

	BGSEEConsole::BGSEEConsole( const char* LogPath, BGSEEINIManagerGetterFunctor Getter, BGSEEINIManagerSetterFunctor Setter ) :
			BGSEEGenericModelessDialog(), INISettingGetter(Getter), INISettingSetter(Setter)
	{
		OwnerThreadID = GetCurrentThreadId();
		PrimaryContext = new DefaultDebugLogContext(this, LogPath);
		ActiveContext = PrimaryContext;

		DialogTemplateID = IDD_BGSEE_CONSOLE;
		DialogContextMenuID = IDR_BGSEE_CONSOLE_CONTEXTMENU;
		CallbackDlgProc = &BGSEEConsole::BaseDlgProc;
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

		INISaveUIState(&INISettingSetter, kINISection);
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
				atoi(INISettingGetter(kConsoleSpecificINISettings[kConsoleSpecificINISetting_UpdatePeriod].Key, kINISection)),
				NULL);

		INILoadUIState(&INISettingGetter, kINISection);
		SendMessage(DialogHandle, WM_SIZE, NULL, NULL);
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
		if (atoi(INISettingGetter(kConsoleSpecificINISettings[kConsoleSpecificINISetting_LogWarnings].Key, kINISection)) == 0)
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
		if (atoi(INISettingGetter(kConsoleSpecificINISettings[kConsoleSpecificINISetting_LogAssertions].Key, kINISection)) == 0)
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

	BGSEEINIManagerSettingFactory* BGSEEConsole::GetINIFactory( void )
	{
		static BGSEEINIManagerSettingFactory kFactory(kINISection);
		if (kFactory.Settings.size() == 0)
		{
			kFactory.Settings.push_back(&kDefaultINISettings[kDefaultINISetting_Top]);
			kFactory.Settings.push_back(&kDefaultINISettings[kDefaultINISetting_Left]);
			kFactory.Settings.push_back(&kDefaultINISettings[kDefaultINISetting_Right]);
			kFactory.Settings.push_back(&kDefaultINISettings[kDefaultINISetting_Bottom]);
			kFactory.Settings.push_back(&kDefaultINISettings[kDefaultINISetting_Visible]);

			kFactory.Settings.push_back(&kConsoleSpecificINISettings[kConsoleSpecificINISetting_UpdatePeriod]);
			kFactory.Settings.push_back(&kConsoleSpecificINISettings[kConsoleSpecificINISetting_LogWarnings]);
			kFactory.Settings.push_back(&kConsoleSpecificINISettings[kConsoleSpecificINISetting_LogAssertions]);
			kFactory.Settings.push_back(&kConsoleSpecificINISettings[kConsoleSpecificINISetting_LogTimestamps]);
		}

		return &kFactory;
	}

	bool BGSEEConsole::GetLogsWarnings( void )
	{
		return atoi(INISettingGetter(kConsoleSpecificINISettings[kConsoleSpecificINISetting_LogWarnings].Key, kINISection)) != 0;
	}

	void BGSEEConsole::OpenDebugLog( void )
	{
		PrimaryContext->OpenLog();
	}

	void BGSEEConsole::FlushDebugLog( void )
	{
		PrimaryContext->Flush();
	}
}