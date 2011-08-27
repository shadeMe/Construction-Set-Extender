#include "Console.h"
#include "WindowManager.h"
#include "resource.h"
#include "CSEInterfaceManager.h"
#include "ConsoleCommands.h"

Console*					Console::Singleton = NULL;

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
		DebugPrint("Couldn't initialize debug log");
}

void Console::InitializeConsole()
{
	if (IsConsoleInitalized())	return;

	WindowHandle = CreateDialog(g_DLLInstance, MAKEINTRESOURCE(DLG_CONSOLE), *g_HWND_CSParent, (DLGPROC)ConsoleDlgProc);
	EditHandle = GetDlgItem(WindowHandle, EDIT_CONSOLE);
	g_ConsoleEditControlOrgWindowProc = (WNDPROC)SetWindowLong(EditHandle, GWL_WNDPROC, (LONG)ConsoleEditControlSubClassProc);
	g_ConsoleCmdBoxOrgWindowProc = (WNDPROC)SetWindowLong(GetDlgItem(WindowHandle, EDIT_CMDBOX), GWL_WNDPROC, (LONG)ConsoleCmdBoxSubClassProc);
	SendDlgItemMessage(WindowHandle, EDIT_CMDBOX, WM_INITDIALOG, NULL, NULL);

	Edit_LimitText(EditHandle, sizeof(int));

	if (g_INIManager->GetINIInt("HideOnStartup", "Extender::Console"))
		DisplayState = true;

	ToggleDisplayState();
	SetTimer(EditHandle, CONSOLE_UPDATETIMER, g_INIManager->GetINIInt("UpdatePeriod", "Extender::Console"), NULL);
	g_CustomMainWindowChildrenDialogs.AddHandle(WindowHandle);
}

void Console::Deinitialize()
{
	SaveINISettings();
	if (DebugLog)
		fclose(DebugLog);

	KillTimer(EditHandle, CONSOLE_UPDATETIMER);
	g_CustomMainWindowChildrenDialogs.RemoveHandle(WindowHandle);
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

	_itoa_s(WindowRect.top, g_TextBuffer, sizeof(g_TextBuffer), 10);
	g_INIManager->FetchSetting("Top", "Extender::Console")->SetValue(g_TextBuffer);

	_itoa_s(WindowRect.left, g_TextBuffer, sizeof(g_TextBuffer), 10);
	g_INIManager->FetchSetting("Left", "Extender::Console")->SetValue(g_TextBuffer);

	_itoa_s(WindowRect.right - WindowRect.left, g_TextBuffer, sizeof(g_TextBuffer), 10);
	g_INIManager->FetchSetting("Right", "Extender::Console")->SetValue(g_TextBuffer);

	_itoa_s(WindowRect.bottom - WindowRect.top, g_TextBuffer, sizeof(g_TextBuffer), 10);
	g_INIManager->FetchSetting("Bottom", "Extender::Console")->SetValue(g_TextBuffer);
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