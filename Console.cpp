#include "Console.h"
#include "resource.h"
#include "CSEInterfaceManager.h"
#include "ConsoleCommands.h"
#include "WindowManager.h"
#include "ChangeLogManager.h"

Console*					Console::Singleton = NULL;
WNDPROC						g_ConsoleWndOrgWindowProc = NULL;
WNDPROC						g_ConsoleEditControlOrgWindowProc = NULL;
WNDPROC						g_ConsoleCmdBoxOrgWindowProc = NULL;

LRESULT CALLBACK ConsoleDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConsoleEditControlSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConsoleCmdBoxSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

Console::Console()
{
	WindowHandle = NULL;
	EditHandle = NULL;
	DisplayState = false;
	IndentLevel = 0;
	DebugLog = NULL;
	UpdateSignalFlag = false;

	DebugLogPath = "";
	MessageBuffer.reserve(0x3000);
	ZeroMemory(Buffer, sizeof(Buffer));

	CSEInterfaceManager::RegisterConsoleCallback((CSEConsoleInterface::ConsolePrintCallback)&ConsoleCommandCallback);
	g_ConsoleCommandTable.InitializeCommandTable();
}

Console* Console::GetSingleton()
{
	if (!Singleton)
		Singleton = new Console();

	return Singleton;
}

void Console::InitializeLog(const char* AppPath)
{
	if (IsLogInitalized())	return;

	DebugLogPath = std::string(AppPath) + "Construction Set Extender.log";
	DebugLog = _fsopen(DebugLogPath.c_str(), "w", _SH_DENYWR);

	if (!DebugLog)
	{
		DebugPrint("Couldn't initialize debug log");
	}
}

void Console::InitializeConsoleWindow()
{
	if (IsConsoleInitalized())	return;

	WindowHandle = CreateDialog(g_DLLInstance, MAKEINTRESOURCE(DLG_CONSOLE), *g_HWND_CSParent, (DLGPROC)ConsoleDlgProc);
	EditHandle = GetDlgItem(WindowHandle, EDIT_CONSOLE);
	g_ConsoleEditControlOrgWindowProc = (WNDPROC)SetWindowLong(EditHandle, GWL_WNDPROC, (LONG)ConsoleEditControlSubClassProc);
	g_ConsoleCmdBoxOrgWindowProc = (WNDPROC)SetWindowLong(GetDlgItem(WindowHandle, EDIT_CMDBOX), GWL_WNDPROC, (LONG)ConsoleCmdBoxSubClassProc);
	SendDlgItemMessage(WindowHandle, EDIT_CMDBOX, WM_INITDIALOG, NULL, NULL);
	Edit_LimitText(EditHandle, sizeof(int));

	SetTimer(EditHandle, CONSOLE_UPDATETIMER, g_INIManager->GetINIInt("UpdatePeriod", "Extender::Console"), NULL);
	g_CustomMainWindowChildrenDialogs.AddHandle(WindowHandle);

	HMENU ViewMenu = GetMenu(*g_HWND_CSParent); ViewMenu = GetSubMenu(ViewMenu, 2);
	MENUITEMINFO ItemViewConsole;
	ItemViewConsole.cbSize = sizeof(MENUITEMINFO);
	ItemViewConsole.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemViewConsole.wID = MAIN_VIEW_CONSOLEWINDOW;
	ItemViewConsole.fState = MFS_ENABLED|MFS_CHECKED;
	ItemViewConsole.dwTypeData = "Console Window";
	ItemViewConsole.cch = 0;
	InsertMenuItem(ViewMenu, 40455, FALSE, &ItemViewConsole);

	if (!g_INIManager->GetINIInt("Visible", "Extender::Console"))
		DisplayState = true;

	ToggleDisplayState();
	LoadINISettings();
}

void Console::Deinitialize()
{
	SaveINISettings();
	if (DebugLog)
		fclose(DebugLog);

	KillTimer(EditHandle, CONSOLE_UPDATETIMER);
	g_CustomMainWindowChildrenDialogs.RemoveHandle(WindowHandle);
	DestroyWindow(WindowHandle);
}

bool Console::ToggleDisplayState()
{
	if (IsConsoleInitalized() == 0)	return false;

	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);

	if (IsHidden())
	{
		Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, EM_LINESCROLL, 0, MessageBuffer.length());

		ShowWindow(WindowHandle, SW_SHOWNA);
		CheckMenuItem(ViewMenu, MAIN_VIEW_CONSOLEWINDOW, MF_CHECKED);
		DisplayState = true;
	}
	else
	{
		ShowWindow(WindowHandle, SW_HIDE);
		CheckMenuItem(ViewMenu, MAIN_VIEW_CONSOLEWINDOW, MF_UNCHECKED);
		DisplayState = false;
	}
	return DisplayState;
}

void Console::LoadINISettings()
{
	if (IsConsoleInitalized() == 0)	return;

	int Top = g_INIManager->FetchSetting("Top", "Extender::Console")->GetValueAsInteger(),
		Left = g_INIManager->FetchSetting("Left", "Extender::Console")->GetValueAsInteger(),
		Right = g_INIManager->FetchSetting("Right", "Extender::Console")->GetValueAsInteger(),
		Bottom = g_INIManager->FetchSetting("Bottom", "Extender::Console")->GetValueAsInteger();

	SetWindowPos(WindowHandle, HWND_NOTOPMOST, Left, Top, Right, Bottom, NULL);
}

void Console::SaveINISettings()
{
	if (IsConsoleInitalized() == 0)	return;

	tagRECT WindowRect;
	GetWindowRect(WindowHandle, &WindowRect);

	char Buffer[0x200] = {0};
	_itoa_s(WindowRect.top, Buffer, sizeof(Buffer), 10);
	g_INIManager->FetchSetting("Top", "Extender::Console")->SetValue(Buffer);

	_itoa_s(WindowRect.left, Buffer, sizeof(Buffer), 10);
	g_INIManager->FetchSetting("Left", "Extender::Console")->SetValue(Buffer);

	_itoa_s(WindowRect.right - WindowRect.left, Buffer, sizeof(Buffer), 10);
	g_INIManager->FetchSetting("Right", "Extender::Console")->SetValue(Buffer);

	_itoa_s(WindowRect.bottom - WindowRect.top, Buffer, sizeof(Buffer), 10);
	g_INIManager->FetchSetting("Bottom", "Extender::Console")->SetValue(Buffer);

	g_INIManager->FetchSetting("Visible", "Extender::Console")->SetValue((DisplayState == true)?"1":"0");
}

void Console::OpenDebugLog(void) 
{
	ShellExecute(NULL, "open", (LPSTR)DebugLogPath.c_str(), NULL, NULL, SW_SHOW);
}

void Console::PrintMessage(std::string& Prefix, const char* MessageStr)
{
	if (MessageBuffer.length() > 16000)
		MessageBuffer.clear();

	std::string Message = "";
	if (Prefix != "")
		Message += "[" + std::string(Prefix) + "]\t";

	for (int i = 0; i < IndentLevel; i++)
		Message += "\t";

	Message += std::string(MessageStr);

	if (IsLogInitalized())
	{
		fputs(Message.c_str(), DebugLog);
		fputs("\n", DebugLog);
		fflush(DebugLog);
	}

	if (Message.rfind("\r\n") != Message.length() - 2)
		MessageBuffer += Message + "\r\n";
	else
		MessageBuffer += Message;

	if (IsConsoleInitalized() && !IsHidden())
	{
		UpdateSignalFlag = true;
		CSEInterfaceManager::HandleConsoleCallback(MessageStr, Prefix.c_str());
	}
}

void Console::LogMessage(UInt8 Source, const char* Format, va_list Args)
{
	vsprintf_s(Buffer, sizeof(Buffer), Format, Args);

	std::string Prefix;
	switch (Source)
	{
	case e_CSE:
		Prefix += "CSE";
		break;
	case e_CS:
		Prefix += "CS";
		break;
	case e_BE:
		Prefix += "BE";
		break;
	case e_UL:
		Prefix += "UL";
		break;
	case e_SE:
		Prefix += "SE";
		break;
	case e_BSA:
		Prefix += "BSA";
		break;
	case e_TAG:
		Prefix += "TAG";
		break;
	}
	PrintMessage(Prefix, Buffer);
}

void Console::LogMessage(const char* Prefix, const char* Format, va_list Args)
{
	vsprintf_s(Buffer, sizeof(Buffer), Format, Args);

	PrintMessage(std::string(Prefix), Buffer);
}

void Console::LogMessage(const char* Prefix, const char* MessageStr)
{
	PrintMessage(std::string(Prefix), MessageStr);
}

void Console::Clear()
{
	if (IsConsoleInitalized() == 0)	return;

	Edit_SetText(EditHandle, 0);
	MessageBuffer.clear();
	SendDlgItemMessage(WindowHandle, EDIT_CMDBOX, CONSOLECMDBOX_CLEARCOMMANDSTACK, NULL, NULL);
}

UInt32 Console::Indent()
{
	if (++IndentLevel > 10)		IndentLevel = 10;
	return IndentLevel;
}

UInt32 Console::Exdent()
{
	if (IndentLevel > 0)		--IndentLevel;
	return IndentLevel;
}

void Console::Pad(UInt32 PaddingCount)
{
	for (int i = 0; i < PaddingCount; i++)
		LogMessage("", "\r\n");
}

void LogWinAPIErrorMessage(DWORD ErrorID)
{
	LPVOID ErrorMsg;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		ErrorID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &ErrorMsg,
		0, NULL );

	DebugPrint("\tError Message: %s", (LPSTR)ErrorMsg);
	LocalFree(ErrorMsg);
}

void DebugPrint(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	CONSOLE->LogMessage(Console::e_CSE, fmt, args);
	va_end(args);
}

void DebugPrint(UInt8 source, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	CONSOLE->LogMessage(source, fmt, args);
	va_end(args);
}

LRESULT CALLBACK ConsoleDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
    case WM_MOVING:
        return g_WindowEdgeSnapper.OnSnapMoving(hWnd, uMsg, wParam, lParam);
    case WM_ENTERSIZEMOVE:
        return g_WindowEdgeSnapper.OnSnapEnterSizeMove(hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
	{
		tagRECT WindowRect, EditRect;
		GetWindowRect(hWnd, &WindowRect);
		MoveWindow(GetDlgItem(hWnd, EDIT_CONSOLE), 0, 0, WindowRect.right - WindowRect.left - 15, WindowRect.bottom - WindowRect.top - 65, TRUE);
		GetWindowRect(GetDlgItem(hWnd, EDIT_CONSOLE), &EditRect);
		SetWindowPos(GetDlgItem(hWnd, EDIT_CMDBOX), HWND_NOTOPMOST, 0, EditRect.bottom - EditRect.top, WindowRect.right - WindowRect.left - 18, 31, SWP_NOZORDER);
		break;
	}
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_ConsoleWndOrgWindowProc);
		break;
	case WM_INITDIALOG:
		HFONT EditFont = CreateFont(20, 0, 0, 0,
                             FW_BOLD, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, 5,		// CLEARTYPE_QUALITY
                             FF_DONTCARE, "Consolas");
		SendMessage(GetDlgItem(hWnd, EDIT_CMDBOX), WM_SETFONT, (WPARAM)EditFont, (LPARAM)TRUE);
		break;
	}

	return CallWindowProc(g_ConsoleWndOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ConsoleEditControlSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_TIMER:
		switch (wParam)
		{
		case CONSOLE_UPDATETIMER:
			if (CONSOLE->GetShouldUpdate())
			{
				SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);
				Edit_SetText(hWnd, (LPCSTR)CONSOLE->GetMessageBuffer());
				SendMessage(hWnd, WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL);
				SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);
			}
			break;
		}
		return TRUE;
	case WM_RBUTTONUP:
	{
		static bool AlwaysOnTopFlag = false;

		RECT Rect;
		POINT Point;

		GetClientRect(hWnd, &Rect);
		Point.x = GET_X_LPARAM(lParam);
        Point.y = GET_Y_LPARAM(lParam);

		if (PtInRect((LPRECT) &Rect, Point))
		{
			HMENU Popup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU1); Popup = GetSubMenu(Popup, 0);

			if (AlwaysOnTopFlag)
				CheckMenuItem(Popup, 1, MF_CHECKED|MF_BYPOSITION);
			else
				CheckMenuItem(Popup, 1, MF_UNCHECKED|MF_BYPOSITION);

			ClientToScreen(hWnd, (LPPOINT) &Point);

			switch (TrackPopupMenu(Popup, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, Point.x, Point.y, 0, hWnd, NULL))
			{
			case CONSOLEMENU_CLEARCONSOLE:
				CONSOLE->Clear();
				break;
			case CONSOLEMENU_ALWAYSONTOP:
				if (AlwaysOnTopFlag)
				{
					SetWindowPos(CONSOLE->GetWindowHandle(), HWND_NOTOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
					AlwaysOnTopFlag = false;
				}
				else
				{
					SetWindowPos(CONSOLE->GetWindowHandle(), HWND_TOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
					AlwaysOnTopFlag = true;
				}
				break;
			case CONSOLEMENU_HIDECONSOLE:
				CONSOLE->ToggleDisplayState();
				break;
			case CONSOLEMENU_OPENDEBUGLOG:
				CONSOLE->OpenDebugLog();
				break;
			case CONSOLEMENU_OPENCHANGELOG:
				VersionControl::CHANGELOG->OpenSessionLog();
				break;
			}
			DestroyMenu(Popup);
		}
		return FALSE;
	}
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_ConsoleEditControlOrgWindowProc);
		break;
	}

	return CallWindowProc(g_ConsoleEditControlOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ConsoleCmdBoxSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static std::stack<std::string> CommandStack, AlternateCommandStack;

	switch (uMsg)
	{
	case CONSOLECMDBOX_RESETCOMMANDSTACK:
		while (AlternateCommandStack.empty() == false)
		{
			CommandStack.push(AlternateCommandStack.top());
			AlternateCommandStack.pop();
		}
		return TRUE;
	case CONSOLECMDBOX_CLEARCOMMANDSTACK:
		while (AlternateCommandStack.empty() == false)
			AlternateCommandStack.pop();
		while (CommandStack.empty() == false)
			CommandStack.pop();
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
				char Buffer[0x200];
				Edit_GetText(hWnd, Buffer, sizeof(Buffer));
				if (strlen(Buffer) > 2)
				{
					CONSOLE->LogMessage("CMD", Buffer);
					SendMessage(hWnd, CONSOLECMDBOX_RESETCOMMANDSTACK, NULL, NULL);
					CommandStack.push(Buffer);
				}
				Edit_SetText(hWnd, NULL);
				return TRUE;
			}
		case VK_UP:
			{
				Edit_SetText(hWnd, NULL);
				if (CommandStack.empty() == false)
				{
					std::string Command(CommandStack.top());
					Edit_SetText(hWnd, Command.c_str());
					CommandStack.pop();
					AlternateCommandStack.push(Command);
				}
				return TRUE;
			}
		case VK_DOWN:
			{
				Edit_SetText(hWnd, NULL);
				if (AlternateCommandStack.empty() == false)
				{
					std::string Command(AlternateCommandStack.top());
					Edit_SetText(hWnd, Command.c_str());
					AlternateCommandStack.pop();
					CommandStack.push(Command);
				}
				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_ConsoleCmdBoxOrgWindowProc);
		break;
	case WM_INITDIALOG:
		return TRUE;
	}

	return CallWindowProc(g_ConsoleCmdBoxOrgWindowProc, hWnd, uMsg, wParam, lParam);
}