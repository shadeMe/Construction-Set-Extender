#include "Console.h"
#include "UIManager.h"
#include "Script\CodaVM.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace bgsee
{
	const char*								Console::kCommandLinePrefix = "CMD";
	const char*								Console::kWindowTitle       = "Console Window";

#define BGSEECONSOLE_INISECTION				"Console"
	SME::INI::INISetting					Console::kINI_Top("Top", BGSEECONSOLE_INISECTION,
																"Dialog Rect Top",
																(SInt32)150);
	SME::INI::INISetting					Console::kINI_Left("Left", BGSEECONSOLE_INISECTION,
																"Dialog Rect Left",
																(SInt32)150);
	SME::INI::INISetting					Console::kINI_Right("Right", BGSEECONSOLE_INISECTION,
																"Dialog Rect Right",
																(SInt32)150);
	SME::INI::INISetting					Console::kINI_Bottom("Bottom", BGSEECONSOLE_INISECTION,
																"Dialog Rect Bottom",
																(SInt32)150);
	SME::INI::INISetting					Console::kINI_Visible("Visible", BGSEECONSOLE_INISECTION,
																"Dialog Visibility State",
																(SInt32)1);

	SME::INI::INISetting					Console::kINI_FontFace("FontFace", BGSEECONSOLE_INISECTION,
																		"Message log font type",
																		"Consolas");
	SME::INI::INISetting					Console::kINI_FontSize("FontSize", BGSEECONSOLE_INISECTION,
																		"Message log font size",
																		(SInt32)13);
	SME::INI::INISetting					Console::kINI_UpdatePeriod("UpdatePeriod", BGSEECONSOLE_INISECTION,
																			"Duration, in milliseconds, between message log updates",
																			(SInt32)1000);
	SME::INI::INISetting					Console::kINI_LogWarnings("LogWarnings", BGSEECONSOLE_INISECTION,
																			"Log editor warnings",
																			(SInt32)1);
	SME::INI::INISetting					Console::kINI_LogAssertions("LogAssertions", BGSEECONSOLE_INISECTION,
																			"Log editor assertions",
																			(SInt32)1);
	SME::INI::INISetting					Console::kINI_LogTimestamps("LogTimestamps", BGSEECONSOLE_INISECTION,
																			"Add timestamps to messages",
																			(SInt32)0);

#define IDM_BGSEE_CONSOLE_COMMANDLINE_RESETCOMMANDSTACK			(WM_USER + 5001)
#define IDM_BGSEE_CONSOLE_CONTEXTTABS_RELOAD					(WM_USER + 5002)

	LRESULT CALLBACK Console::BaseDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return)
	{
		LRESULT DlgProcResult = FALSE;
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		Console* Instance = dynamic_cast<Console*>(UserData->Instance);
		Return = false;

		switch (uMsg)
		{
		case IDM_BGSEE_CONSOLE_CONTEXTTABS_RELOAD:
			{
				HWND TabStrip = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_CONTEXTS_TABLIST);

				ContextArrayT Contexts;
				Contexts.emplace_back(Instance->PrimaryContext);
				Contexts.insert(Contexts.end(), Instance->SecondaryContexts.begin(), Instance->SecondaryContexts.end());

				TabCtrl_DeleteAllItems(TabStrip);

				int Index = 0;
				for (auto Context : Contexts)
				{
					TCITEM NewTabItem;
					NewTabItem.mask = TCIF_STATE | TCIF_TEXT | TCIF_PARAM;
					if (Context == Instance->PrimaryContext)
						NewTabItem.pszText = "DEFAULT";
					else
						NewTabItem.pszText = reinterpret_cast<LPSTR>(const_cast<char*>(Context->GetName()));

					NewTabItem.lParam = reinterpret_cast<LPARAM>(Context);

					TabCtrl_InsertItem(TabStrip, Index, &NewTabItem);
					if (Instance->ActiveContext == Context)
						TabCtrl_SetCurSel(TabStrip, Index);

					++Index;
				}

				Return = true;
				DlgProcResult = TRUE;
			}

			break;
		case WM_NOTIFY:
			{
				auto Data = reinterpret_cast<NMHDR*>(lParam);

				switch (Data->code)
				{
				case TCN_SELCHANGE:
					{
						SME_ASSERT(wParam == IDC_BGSEE_CONSOLE_CONTEXTS_TABLIST);

						HWND TabStrip = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_CONTEXTS_TABLIST);
						int SelectionIndex = TabCtrl_GetCurSel(TabStrip);
						SME_ASSERT(SelectionIndex != -1);

						TCITEM ItemData;
						ItemData.mask = TCIF_PARAM;
						TabCtrl_GetItem(TabStrip, SelectionIndex, &ItemData);

						SME_ASSERT(ItemData.lParam);
						auto Context = reinterpret_cast<MessageLogContext*>(ItemData.lParam);
						Instance->SetActiveContext(Context);

						Return = true;
						DlgProcResult = TRUE;

						break;
					}
				}
			}

			break;
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
						if (Extension && !_stricmp(Extension, CODAVM->GetScriptFileExtension().c_str()))
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
					Instance->LogMsg(BGSEEMAIN->ExtenderGetShortName(), "Executing %d Coda scripts...", DroppedScripts.size());
					Instance->Indent();

					for (int i = 0; i < DroppedScripts.size(); i++)
					{
						script::ICodaScriptVirtualMachine::ExecuteParams Input;
						script::ICodaScriptVirtualMachine::ExecuteResult Output;

						Input.Filepath = DroppedScripts[i];
						CODAVM->RunScript(Input, Output);
					}

					Instance->Outdent();
				}
			}

			break;
		case WM_INITMENUPOPUP:
			{
				if (wParam == (WPARAM)Instance->ContextMenuHandle)
				{
					UIExtraData* xData = (UIExtraData*)UserData->ExtraData;
					SME_ASSERT(xData);

					MENUITEMINFO ContextMenuItem = {0};
					ContextMenuItem.cbSize = sizeof(MENUITEMINFO);
					ContextMenuItem.fMask = MIIM_STATE;

					if (Instance->GetActiveContext()->HasLog())
						ContextMenuItem.fState = MFS_ENABLED;
					else
						ContextMenuItem.fState = MFS_DISABLED;
					SetMenuItemInfo((HMENU)wParam, ID_BGSEE_CONSOLE_CONTEXTMENU_OPENLOG, FALSE, &ContextMenuItem);

					if (Instance->GetActiveContext() != Instance->PrimaryContext)
					{
						ContextMenuItem.fState = MFS_DISABLED;
						SetMenuItemInfo((HMENU)wParam, ID_BGSEE_CONSOLE_CONTEXTMENU_LOGWARNINGS, FALSE, &ContextMenuItem);
						SetMenuItemInfo((HMENU)wParam, ID_BGSEE_CONSOLE_CONTEXTMENU_LOGASSERTIONS, FALSE, &ContextMenuItem);
					}
					else
					{
						ContextMenuItem.fState = MFS_ENABLED;
						if (kINI_LogWarnings().i)
							ContextMenuItem.fState |= MFS_CHECKED;
						else
							ContextMenuItem.fState &= ~MFS_CHECKED;
						SetMenuItemInfo((HMENU)wParam, ID_BGSEE_CONSOLE_CONTEXTMENU_LOGWARNINGS, FALSE, &ContextMenuItem);

						if (kINI_LogAssertions().i)
							ContextMenuItem.fState |= MFS_CHECKED;
						else
							ContextMenuItem.fState &= ~MFS_CHECKED;
						SetMenuItemInfo((HMENU)wParam, ID_BGSEE_CONSOLE_CONTEXTMENU_LOGASSERTIONS, FALSE, &ContextMenuItem);

						if (kINI_LogTimestamps().i)
							ContextMenuItem.fState |= MFS_CHECKED;
						else
							ContextMenuItem.fState &= ~MFS_CHECKED;
						SetMenuItemInfo((HMENU)wParam, ID_BGSEE_CONSOLE_CONTEXTMENU_SHOWTIMESTAMPS, FALSE, &ContextMenuItem);
					}
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
			case ID_BGSEE_CONSOLE_CONTEXTMENU_LOGWARNINGS:
				kINI_LogWarnings.ToggleData();
				break;
			case ID_BGSEE_CONSOLE_CONTEXTMENU_SHOWTIMESTAMPS:
				kINI_LogTimestamps.ToggleData();
				break;
			case ID_BGSEE_CONSOLE_CONTEXTMENU_LOGASSERTIONS:
				kINI_LogAssertions.ToggleData();
				break;
			case ID_BGSEE_CONSOLE_CONTEXTMENU_WARNINGS:
				Instance->GetWarningManager()->ShowGUI(Instance->ResourceInstance, hWnd);
				break;
			}

			break;
		case WM_SIZE:
			{
				RECT CurrentRect = {0};
				HWND MessageLog = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_MESSAGELOG);
				HWND CommandLine = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_COMMANDLINE);
				HWND TabStrip = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_CONTEXTS_TABLIST);

				UIExtraData* xData = (UIExtraData*)UserData->ExtraData;
				SME_ASSERT(xData);

				SetRect(&CurrentRect, 0, 0, LOWORD(lParam), HIWORD(lParam));
				int DeltaDlgWidth = (CurrentRect.right - xData->DialogInitBounds.right);
				int DeltaDlgHeight = (CurrentRect.bottom - xData->DialogInitBounds.bottom);
				int VerticalScrollWidth = GetSystemMetrics(SM_CXVSCROLL) + 4;
				int HorScrollWidth = GetSystemMetrics(SM_CXHSCROLL) + 4;
				HDWP DeferPosData = BeginDeferWindowPos(3);

				DeferWindowPos(DeferPosData, TabStrip, nullptr,
							xData->ContextTabStripInitBounds.left,
							xData->ContextTabStripInitBounds.top,
							DeltaDlgWidth + xData->ContextTabStripInitBounds.right,
							xData->ContextTabStripInitBounds.bottom,
							NULL);

				DeferWindowPos(DeferPosData, MessageLog, nullptr,
							xData->MessageLogInitBounds.left,
							xData->MessageLogInitBounds.top,
							DeltaDlgWidth + xData->MessageLogInitBounds.right + VerticalScrollWidth,
							CurrentRect.bottom + xData->MessageLogInitBounds.bottom - xData->DialogInitBounds.bottom + HorScrollWidth,
							NULL);

				DeferWindowPos(DeferPosData, CommandLine, nullptr,
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
				HWND TabStrip = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_CONTEXTS_TABLIST);

				GetClientRect(hWnd, &xData->DialogInitBounds);
				SME::UIHelpers::GetClientRectInitBounds(MessageLog, hWnd, &xData->MessageLogInitBounds);
				SME::UIHelpers::GetClientRectInitBounds(CommandLine, hWnd, &xData->CommandLineInitBounds);
				SME::UIHelpers::GetClientRectInitBounds(TabStrip, hWnd, &xData->ContextTabStripInitBounds);

				SendDlgItemMessage(hWnd, IDC_BGSEE_CONSOLE_MESSAGELOG, WM_SETFONT, (WPARAM)xData->MessageLogFont, (LPARAM)TRUE);
				SendDlgItemMessage(hWnd, IDC_BGSEE_CONSOLE_COMMANDLINE, WM_SETFONT, (WPARAM)xData->CommandLineFont, (LPARAM)TRUE);

				DragAcceptFiles(hWnd, TRUE);
			}

			break;
		}

		return DlgProcResult;
	}

#define IDC_BGSEE_CONSOLE_MESSAGELOG_REFRESHTIMER		0xC05

	LRESULT CALLBACK Console::MessageLogSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
	{
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(GetAncestor(hWnd, GA_PARENT), GWL_USERDATA);
		Console* Instance = dynamic_cast<Console*>(UserData->Instance);
		LRESULT CallbackResult = FALSE;

		switch (uMsg)
		{
		case WM_TIMER:
			switch (wParam)
			{
			case IDC_BGSEE_CONSOLE_MESSAGELOG_REFRESHTIMER:
				if (Instance->GetActiveContext()->GetState() == MessageLogContext::kState_Default)
					break;

				SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);

				switch (Instance->GetActiveContext()->GetState())
				{
				case MessageLogContext::kState_Reset:
					Edit_SetText(hWnd, nullptr);
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

			SubclassParams->Out.MarkMessageAsHandled = true;
			CallbackResult = TRUE;
			break;
		case WM_CONTEXTMENU:
			SubclassParams->Out.MarkMessageAsHandled = true;
			CallbackResult = SendMessage(GetAncestor(hWnd, GA_PARENT), uMsg, wParam, lParam);

			break;
		}

		return CallbackResult;
	}

	LRESULT CALLBACK Console::CommandLineSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
	{
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(GetAncestor(hWnd, GA_PARENT), GWL_USERDATA);
		Console* Instance = dynamic_cast<Console*>(UserData->Instance);
		LRESULT CallbackResult = FALSE;

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

			CallbackResult = TRUE;
			SubclassParams->Out.MarkMessageAsHandled = true;
			break;
		case WM_CHAR:
			if (wParam == VK_RETURN)
			{
				CallbackResult = TRUE;
				SubclassParams->Out.MarkMessageAsHandled = true;
			}

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

					Edit_SetText(hWnd, nullptr);
				}

				CallbackResult = TRUE;
				SubclassParams->Out.MarkMessageAsHandled = true;
				break;
			case VK_UP:
				if (Instance->CommandLineHistory.empty() == false)
				{
					std::string Command(Instance->CommandLineHistory.top());
					Edit_SetText(hWnd, Command.c_str());
					Edit_SetSel(hWnd, Command.length(), Command.length());
					Instance->CommandLineHistory.pop();
					Instance->CommandLineHistoryAuxiliary.push(Command);

					CallbackResult = TRUE;
					SubclassParams->Out.MarkMessageAsHandled = true;
				}

				break;
			case VK_DOWN:
				if (Instance->CommandLineHistoryAuxiliary.empty() == false)
				{
					std::string Command(Instance->CommandLineHistoryAuxiliary.top());
					Edit_SetText(hWnd, Command.c_str());
					Edit_SetSel(hWnd, Command.length(), Command.length());
					Instance->CommandLineHistoryAuxiliary.pop();
					Instance->CommandLineHistory.push(Command);

					CallbackResult = TRUE;
					SubclassParams->Out.MarkMessageAsHandled = true;
				}

				break;
			}

			break;
		}

		return CallbackResult;
	}

	void Console::MessageLogContext::SetState( UInt8 NewState )
	{
		State = NewState;
	}

	void Console::MessageLogContext::CheckBufferLength(size_t AddendLength)
	{
		if (BackBuffer.length() + AddendLength >= kMessageLogCharLimit)
			ClearBuffer();
	}

	Console::MessageLogContext::MessageLogContext(const char* ContextName, const char* ContextLogPath /*= NULL*/)
	{
		this->Name = ContextName;
		if (ContextLogPath)
			this->LogPath = ContextLogPath;
		else
			this->LogPath = "";
		this->BackBuffer.reserve(kMessageLogCharLimit);
		this->SetState(kState_Default);
	}

	void Console::MessageLogContext::Print( const char* Message, bool AddTimestamp )
	{
		std::string Buffer;
		if (AddTimestamp)
		{
			char TimeBuffer[0x32] = { 0 };
			SME::MiscGunk::GetTimeString(TimeBuffer, sizeof(TimeBuffer), "%H:%M:%S");
			Buffer.append("{").append(TimeBuffer).append("} ");
		}

		Buffer.append(Message).append("\r\n");
		CheckBufferLength(Buffer.length());
		BackBuffer += Buffer;

		SetState(kState_Update);
	}

	void Console::MessageLogContext::Reset()
	{
		ClearBuffer();
		SetState(kState_Reset);
	}

	UInt8 Console::MessageLogContext::GetState() const
	{
		return State;
	}

	void Console::MessageLogContext::OpenLog() const
	{
		if (HasLog())
			ShellExecute(nullptr, "open", (LPSTR)LogPath.c_str(), nullptr, nullptr, SW_SHOW);
	}

	bool Console::MessageLogContext::HasLog() const
	{
		if (LogPath.size())
			return true;
		else
			return false;
	}

	const char* Console::MessageLogContext::GetName() const
	{
		return Name.c_str();
	}

	const char* Console::MessageLogContext::GetBuffer() const
	{
		return BackBuffer.c_str();
	}

	void Console::MessageLogContext::ClearBuffer()
	{
		BackBuffer.clear();
	}

	Console::MessageLogContext::~MessageLogContext()
	{
		;//
	}

	Console::DefaultDebugLogContext::DefaultDebugLogContext( Console* Parent, const char* DebugLogPath ) :
		Console::MessageLogContext("", DebugLogPath),
		DebugLog(nullptr),
		Parent(Parent),
		PrintCallbacks()
	{
		SME_ASSERT(Parent);

		Open(DebugLogPath);
		IndentLevel = 0;
		ExecutingCallbacks = false;
	}

	void Console::DefaultDebugLogContext::Print( const char* Prefix, const char* Message )
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

		CheckBufferLength(Addend.length());
		BackBuffer += Addend;

		SetState(kState_Update);

		if (ExecutingCallbacks == false && strlen(Message) > 0)
			ExecutePrintCallbacks(Prefix, Message);
	}

	void Console::DefaultDebugLogContext::Reset()
	{
		MessageLogContext::Reset();
	}

	UInt32 Console::DefaultDebugLogContext::Indent()
	{
		if (++IndentLevel > Console::kMaxIndentLevel)
			IndentLevel = Console::kMaxIndentLevel;

		return IndentLevel;
	}

	UInt32 Console::DefaultDebugLogContext::Outdent()
	{
		if (IndentLevel > 0)
			--IndentLevel;

		return IndentLevel;
	}

	void Console::DefaultDebugLogContext::OutdentAll()
	{
		IndentLevel = 0;
	}

	void Console::DefaultDebugLogContext::Pad( UInt32 Count )
	{
		for (int i = 0; i < Count; i++)
			Print("", "");
	}

	Console::DefaultDebugLogContext::~DefaultDebugLogContext()
	{
		if (DebugLog)
		{
			Flush();
			Close();
		}

		PrintCallbacks.clear();
	}

	void Console::DefaultDebugLogContext::ExecutePrintCallbacks( const char* Prefix, const char* Message )
	{
		ExecutingCallbacks = true;

		for (PrintCallbackArrayT::const_iterator Itr = PrintCallbacks.begin(); Itr != PrintCallbacks.end(); Itr++)
			(*Itr)(Prefix, Message);

		ExecutingCallbacks = false;
	}

	bool Console::DefaultDebugLogContext::LookupPrintCallback( ConsolePrintCallback Callback, PrintCallbackArrayT::iterator& Match )
	{
		bool Result = false;

		for (PrintCallbackArrayT::iterator Itr = PrintCallbacks.begin(); Itr != PrintCallbacks.end(); Itr++)
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

	bool Console::DefaultDebugLogContext::RegisterPrintCallback( ConsolePrintCallback Callback )
	{
		if (ExecutingCallbacks)
			return false;

		PrintCallbackArrayT::iterator Match;
		if (LookupPrintCallback(Callback, Match) == false)
		{
			PrintCallbacks.push_back(Callback);
			return true;
		}

		return false;
	}

	void Console::DefaultDebugLogContext::UnregisterPrintCallback( ConsolePrintCallback Callback )
	{
		if (ExecutingCallbacks)
			return;

		PrintCallbackArrayT::iterator Match;
		if (LookupPrintCallback(Callback, Match))
			PrintCallbacks.erase(Match);
	}

	void Console::DefaultDebugLogContext::Flush()
	{
		if (DebugLog)
			fflush(DebugLog);
	}

	void Console::DefaultDebugLogContext::Put( const char* String )
	{
		SME_ASSERT(String);

		if (DebugLog)
			fputs(String, DebugLog);
	}

	void Console::DefaultDebugLogContext::Close()
	{
		if (DebugLog)
		{
			fclose(DebugLog);
			DebugLog = nullptr;
		}
	}

	bool Console::DefaultDebugLogContext::Open( const char* Path )
	{
		SME_ASSERT(DebugLog == nullptr && Path);

		DebugLog = _fsopen(Path, "w", _SH_DENYWR);
		return DebugLog != nullptr;
	}

	Console::ConsoleCommandTable::ConsoleCommandTable() : CommandList()
	{
		;//
	}

	Console::ConsoleCommandTable::~ConsoleCommandTable()
	{
		CommandList.clear();
	}

	bool Console::ConsoleCommandTable::LookupCommandByName( const char* Name, ConsoleCommandArrayT::iterator& Match )
	{
		bool Result = false;

		for (ConsoleCommandArrayT::iterator Itr = CommandList.begin(); Itr != CommandList.end(); Itr++)
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

	bool Console::ConsoleCommandTable::LookupCommandByInstance( ConsoleCommandInfo* Command, ConsoleCommandArrayT::iterator& Match )
	{
		bool Result = false;

		for (ConsoleCommandArrayT::iterator Itr = CommandList.begin(); Itr != CommandList.end(); Itr++)
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

	bool Console::ConsoleCommandTable::AddCommand( ConsoleCommandInfo* Command )
	{
		ConsoleCommandArrayT::iterator Match;
		if (LookupCommandByName(Command->Name, Match) == false)
		{
			CommandList.push_back(Command);
			return true;
		}

		return false;
	}

	void Console::ConsoleCommandTable::RemoveCommand( ConsoleCommandInfo* Command )
	{
		ConsoleCommandArrayT::iterator Match;
		SME_ASSERT(LookupCommandByName(Command->Name, Match));

		CommandList.erase(Match);
	}

	ConsoleCommandInfo* Console::ConsoleCommandTable::GetCommand( const char* Name )
	{
		ConsoleCommandArrayT::iterator Match;
		if (LookupCommandByName(Name, Match) == false)
			return nullptr;

		return *Match;
	}

	Console::UIExtraData::UIExtraData() :
		MessageLogFont(nullptr),
		CommandLineFont(nullptr),
		DialogInitBounds(),
		MessageLogInitBounds(),
		CommandLineInitBounds()
	{
		ZeroMemory(&DialogInitBounds, sizeof(RECT));
		ZeroMemory(&MessageLogInitBounds, sizeof(RECT));
		ZeroMemory(&CommandLineInitBounds, sizeof(RECT));
		ZeroMemory(&ContextTabStripInitBounds, sizeof(RECT));

		MessageLogFont = CreateFont(Console::kINI_FontSize.GetData().i, 0, 0, 0,
									FW_REGULAR,
									FALSE,
									FALSE,
									FALSE,
									ANSI_CHARSET,
									OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS,
									CLEARTYPE_QUALITY,
									FF_DONTCARE,
									Console::kINI_FontFace.GetData().s);

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
									Console::kINI_FontFace.GetData().s);
	}

	Console::UIExtraData::~UIExtraData()
	{
		if (MessageLogFont)
			DeleteFont(MessageLogFont);

		if (CommandLineFont)
			DeleteFont(CommandLineFont);
	}

	void Console::ClearMessageLog( void )
	{
		Edit_SetText(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), nullptr);
	}

	void Console::SetTitle( const char* Prefix )
	{
		std::string Buffer(kWindowTitle);

		if (strlen(Prefix))
			Buffer += " [" + std::string(Prefix) + "]";

		SetWindowText(DialogHandle, Buffer.c_str());
	}

	Console::MessageLogContext* Console::GetActiveContext( void ) const
	{
		SME_ASSERT(ActiveContext);
		return ActiveContext;
	}

	void Console::SetActiveContext( MessageLogContext* Context )
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

		EnableWindow(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_COMMANDLINE), ActiveContext == PrimaryContext);
		PostMessage(DialogHandle, IDM_BGSEE_CONSOLE_CONTEXTTABS_RELOAD, 0, 0);
	}

	void Console::ResetActiveContext( void )
	{
		SetActiveContext(PrimaryContext);
	}

	void Console::ExecuteCommand( const char* CommandExpression )
	{
		LogMsg(kCommandLinePrefix, CommandExpression);
		Indent();

		SME::StringHelpers::Tokenizer Tokenizer(CommandExpression, " ,");
		std::string CurrentToken;

		if (Tokenizer.NextToken(CurrentToken) != -1)
		{
			ConsoleCommandInfo* Command = CommandTable.GetCommand(CurrentToken.c_str());

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

		Outdent();
	}

	bool Console::LookupSecondaryContextByName( const char* Name, ContextArrayT::iterator& Match )
	{
		bool Result = false;

		for (ContextArrayT::iterator Itr = SecondaryContexts.begin(); Itr != SecondaryContexts.end(); Itr++)
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

	bool Console::LookupSecondaryContextByInstance( MessageLogContext* Context, ContextArrayT::iterator& Match )
	{
		bool Result = false;

		for (ContextArrayT::iterator Itr = SecondaryContexts.begin(); Itr != SecondaryContexts.end(); Itr++)
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

	void Console::ReleaseSecondaryContexts( void )
	{
		for (ContextArrayT::iterator Itr = SecondaryContexts.begin(); Itr != SecondaryContexts.end(); Itr++)
			delete (*Itr);

		SecondaryContexts.clear();
	}

	Console*		Console::Singleton = nullptr;

	Console::Console(const char* LogPath) :
		GenericModelessDialog()
	{
		SME_ASSERT(Singleton == nullptr);
		Singleton = this;

		OwnerThreadID = GetCurrentThreadId();
		PrimaryContext = new DefaultDebugLogContext(this, LogPath);
		ActiveContext = PrimaryContext;

		DialogTemplateID = IDD_BGSEE_CONSOLE;
		DialogContextMenuID = IDR_BGSEE_CONSOLE_CONTEXTMENU;
		CallbackDlgProc = &Console::BaseDlgProc;
		WarningManager = nullptr;

	}

	Console::~Console()
	{
		ActiveContext = nullptr;
		SAFEDELETE(PrimaryContext);
		ReleaseSecondaryContexts();

		while (CommandLineHistory.size())
			CommandLineHistory.pop();

		while (CommandLineHistoryAuxiliary.size())
			CommandLineHistoryAuxiliary.pop();

		SAFEDELETE(WarningManager);

		INISaveUIState(&kINI_Top, &kINI_Left, &kINI_Right, &kINI_Bottom, &kINI_Visible);
		KillTimer(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), IDC_BGSEE_CONSOLE_MESSAGELOG_REFRESHTIMER);

		Singleton = nullptr;
	}

	void Console::InitializeUI( HWND Parent, HINSTANCE Resource )
	{
		ParentHandle = Parent;
		ResourceInstance = Resource;

		Create(NULL, false, true);

		BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), MessageLogSubclassProc);
		BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_COMMANDLINE), CommandLineSubclassProc);


		Edit_LimitText(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG), sizeof(int));

		// hacky workaround for proper line break support on Windows 10 RS5 or greater
		const auto EditMsgSetExtendedStyle = ECM_FIRST + 10;	// EM_SETEXTENDEDSTYLE
		const auto EditExtStyleLineBreak = 0x0002L;				// ES_EX_ALLOWEOL_LF
		SendDlgItemMessage(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG, EditMsgSetExtendedStyle, EditExtStyleLineBreak, EditExtStyleLineBreak);

		SetTimer(GetDlgItem(DialogHandle, IDC_BGSEE_CONSOLE_MESSAGELOG),
				IDC_BGSEE_CONSOLE_MESSAGELOG_REFRESHTIMER,
				kINI_UpdatePeriod.GetData().i,
				nullptr);

		INILoadUIState(&kINI_Top, &kINI_Left, &kINI_Right, &kINI_Bottom, &kINI_Visible);
	}

	void Console::InitializeWarningManager( INIManagerGetterFunctor Getter, INIManagerSetterFunctor Setter, ConsoleWarningRegistrar& Registrar )
	{
		SME_ASSERT(WarningManager == nullptr);

		WarningManager = new ConsoleWarningManager(Getter, Setter);
		Registrar.operator()(WarningManager);
		WarningManager->INILoadWarnings();
	}

	void Console::LogMsg( std::string Prefix, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		PrimaryContext->Print(Prefix.c_str(), Buffer);
	}

	void Console::LogMsg(const char* Prefix, const char* Format, ...)
	{
		char Buffer[0x1000] = { 0 };

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		PrimaryContext->Print(Prefix, Buffer);
	}

	void Console::LogWindowsError( std::string Prefix, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		PrimaryContext->Print(Prefix.c_str(), Buffer);

		LPVOID ErrorMsg;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
					nullptr,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&ErrorMsg,
					0,
					nullptr);
		FORMAT_STR(Buffer, "Error Message: %s", reinterpret_cast<const char*>(ErrorMsg));
		LocalFree(ErrorMsg);

		Indent();
		PrimaryContext->Print("", Buffer);
		Outdent();

		MessageBeep(MB_ICONWARNING);
	}

	void Console::LogWarning( std::string Prefix, const char* Format, ... )
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

	void Console::LogAssertion( std::string Prefix, const char* Format, ... )
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

	UInt32 Console::Indent()
	{
		return PrimaryContext->Indent();
	}

	UInt32 Console::Outdent()
	{
		return PrimaryContext->Outdent();
	}

	void Console::OutdentAll()
	{
		PrimaryContext->OutdentAll();
	}

	void Console::Pad( UInt32 Count )
	{
		PrimaryContext->Pad(Count);
	}

	void* Console::RegisterMessageLogContext( const char* Name, const char* LogPath /*= NULL*/ )
	{
		ContextArrayT::iterator Match;
		if (LookupSecondaryContextByName(Name, Match))
			return nullptr;

		MessageLogContext* NewContext = new MessageLogContext(Name, LogPath);
		SecondaryContexts.push_back(NewContext);
		std::sort(SecondaryContexts.begin(), SecondaryContexts.end(), [](const auto& a, const auto& b) { return _stricmp(a->GetName(), b->GetName()) < 0; });

		PostMessage(DialogHandle, IDM_BGSEE_CONSOLE_CONTEXTTABS_RELOAD, 0, 0);

		return NewContext;
	}

	void Console::UnregisterMessageLogContext( void* Context )
	{
		ContextArrayT::iterator Match;
		SME_ASSERT(LookupSecondaryContextByInstance((MessageLogContext*)Context, Match));

		if (Context == GetActiveContext())
			ResetActiveContext();

		SecondaryContexts.erase(Match);
		delete Context;
		std::sort(SecondaryContexts.begin(), SecondaryContexts.end(), [](const auto& a, const auto& b) { return _stricmp(a->GetName(), b->GetName()) < 0; });

		PostMessage(DialogHandle, IDM_BGSEE_CONSOLE_CONTEXTTABS_RELOAD, 0, 0);
	}

	void Console::PrintToMessageLogContext( void* Context, bool HideTimestamp, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		std::string Addend(Buffer);
		Addend += "\r\n";

		bool AddTimestamp = HideTimestamp == false && kINI_LogTimestamps.GetData().i;
		((MessageLogContext*)Context)->Print(Buffer, AddTimestamp);
	}

	bool Console::RegisterConsoleCommand( ConsoleCommandInfo* Command )
	{
		return CommandTable.AddCommand(Command);
	}

	void Console::UnregisterConsoleCommand( ConsoleCommandInfo* Command )
	{
		CommandTable.RemoveCommand(Command);
	}

	bool Console::RegisterPrintCallback( ConsolePrintCallback Callback )
	{
		return PrimaryContext->RegisterPrintCallback(Callback);
	}

	void Console::UnregisterPrintCallback( ConsolePrintCallback Callback )
	{
		PrimaryContext->UnregisterPrintCallback(Callback);
	}

	const char* Console::GetLogPath( void ) const
	{
		SME_ASSERT(PrimaryContext);
		return PrimaryContext->LogPath.c_str();
	}

	bool Console::GetLogsWarnings( void )
	{
		return kINI_LogWarnings.GetData().i != 0;
	}

	void Console::OpenDebugLog( void )
	{
		PrimaryContext->OpenLog();
	}

	void Console::FlushDebugLog( void )
	{
		PrimaryContext->Flush();
	}

	void Console::RegisterINISettings( INISettingDepotT& Depot )
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

	Console* Console::Get()
	{
		return Singleton;
	}

	bool Console::Initialize(const char* LogPath)
	{
		if (Singleton)
			return false;

		Console* Buffer = new Console(LogPath);
		return true;
	}

	void Console::Deinitialize()
	{
		SME_ASSERT(Singleton);
		delete Singleton;
	}

	ConsoleWarningManager* Console::GetWarningManager(void) const
	{
		SME_ASSERT(WarningManager);

		return WarningManager;
	}

	void Console::ToggleWarningLogging(bool State)
	{
		kINI_LogWarnings.SetInt(int(State));
	}

	ConsoleWarning::ConsoleWarning( const char* GUID, const char* Desc, UInt32 CallSiteCount, ... ) :
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

	ConsoleWarning::~ConsoleWarning()
	{
		;//
	}

	bool ConsoleWarning::GetEnabled( void ) const
	{
		return Enabled;
	}

	const char* ConsoleWarningManager::kINISection = "ConsoleWarnings";

	BOOL CALLBACK ConsoleWarningManager::GUIDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		HWND ListView = GetDlgItem(hWnd, IDC_BGSEE_CONSOLE_WARNINGS_WARNINGSLIST);

		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		ConsoleWarningManager* Instance = nullptr;

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
							ConsoleWarning* Current = (ConsoleWarning*)DisplayData->item.lParam;

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

						ConsoleWarning* Current = (ConsoleWarning*)Item.lParam;

						Current->Enabled = Current->Enabled == false;
						InvalidateRect(hWnd, nullptr, TRUE);
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

	void ConsoleWarningManager::Clear()
	{
		for (WarningListT::iterator Itr = WarningDepot.begin(); Itr != WarningDepot.end(); Itr++)
			delete *Itr;

		WarningDepot.clear();
	}

	void ConsoleWarningManager::INISaveWarnings( void )
	{
		INISetter(kINISection, nullptr);

		for (WarningListT::iterator Itr = WarningDepot.begin(); Itr != WarningDepot.end(); Itr++)
		{
			INISetter((*Itr)->BaseIDString.c_str(), kINISection, ((*Itr)->Enabled ? "1" : "0"));
		}
	}

	void ConsoleWarningManager::INILoadWarnings( void )
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

				ConsoleWarning* Current = LookupWarning(GUID.c_str());
				SME_ASSERT(Current);

				if (State == "1")
					Current->Enabled = true;
				else
					Current->Enabled = false;
			}
		}
	}

	ConsoleWarning* ConsoleWarningManager::LookupWarning( const char* GUID ) const
	{
		for (WarningListT::const_iterator Itr = WarningDepot.begin(); Itr != WarningDepot.end(); Itr++)
		{
			if (!_stricmp(GUID, (*Itr)->BaseIDString.c_str()))
				return *Itr;
		}

		return nullptr;
	}

	ConsoleWarning* ConsoleWarningManager::LookupWarning( ConsoleWarning::WarningCallSiteT CallSite ) const
	{
		for (WarningListT::const_iterator Itr = WarningDepot.begin(); Itr != WarningDepot.end(); Itr++)
		{
			for (ConsoleWarning::CallSiteListT::iterator ItrEx = (*Itr)->CallSites.begin(); ItrEx != (*Itr)->CallSites.end(); ItrEx++)
			{
				if (*ItrEx == CallSite)
					return *Itr;
			}
		}

		return nullptr;
	}

	ConsoleWarningManager::ConsoleWarningManager( INIManagerGetterFunctor Getter, INIManagerSetterFunctor Setter ) :
		WarningDepot(),
		INIGetter(Getter),
		INISetter(Setter)
	{
		;//
	}

	ConsoleWarningManager::~ConsoleWarningManager()
	{
		INISaveWarnings();
		Clear();
	}

	void ConsoleWarningManager::RegisterWarning( ConsoleWarning* Warning )
	{
		SME_ASSERT(Warning);

		WarningDepot.push_back(Warning);
	}

	bool ConsoleWarningManager::GetWarningEnabled( ConsoleWarning::WarningCallSiteT CallSite ) const
	{
		SME_ASSERT(CallSite);

		bool Result = true;
		ConsoleWarning* Warning = LookupWarning(CallSite);

		if (Warning && Warning->GetEnabled() == false)
			Result = false;

		return Result;
	}

	void ConsoleWarningManager::ShowGUI( HINSTANCE ResourceInstance, HWND Parent )
	{
		DlgUserData* Param = new DlgUserData();
		Param->Instance = this;

		BGSEEUI->ModalDialog(ResourceInstance, MAKEINTRESOURCE(IDD_BGSEE_CONSOLE_WARNINGS), Parent, GUIDlgProc, (LPARAM)Param);
	}

	void ConsoleWarningManager::EnumerateWarningsInListView( HWND ListView ) const
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