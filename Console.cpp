#include "Console.h"
#include "ExtenderInternals.h"
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
	MessageBuffer.reserve(0x3000);
	IndentLevel = 0;
	DebugLog = NULL;

	CSEInterfaceManager::RegisterConsoleCallback((CSEConsoleInterface::ConsolePrintCallback)&ConsoleCommandCallback);
	g_ConsoleCommandTable.InitializeCommandTable();
}

Console* Console::GetSingleton()
{
	if (!Singleton) {
		Singleton = new Console();
	}
	return Singleton;
}

void Console::InitializeLog(const char* AppPath)
{
	if (IsLogInitalized())	return;

	DebugLog = _fsopen((std::string(std::string(AppPath) + "Construction Set Extender.log").c_str()), "w", _SH_DENYWR);

	if (!DebugLog) 
		DebugPrint("Couldn't initialize debug log.");
}

void Console::InitializeConsole()
{
	if (IsConsoleInitalized())	return;

	WindowHandle = CreateDialog(g_DLLInstance, MAKEINTRESOURCE(DLG_CONSOLE), *g_HWND_CSParent, (DLGPROC)ConsoleDlgProc);
	EditHandle = GetDlgItem(WindowHandle, EDIT_CONSOLE);
	g_ConsoleEditControlOrgWindowProc = (WNDPROC)SetWindowLong(EditHandle, GWL_WNDPROC, (LONG)ConsoleEditControlSubClassProc);
	g_ConsoleCmdBoxOrgWindowProc = (WNDPROC)SetWindowLong(GetDlgItem(WindowHandle, EDIT_CMDBOX), GWL_WNDPROC, (LONG)ConsoleCmdBoxSubClassProc);

	Edit_LimitText(EditHandle, sizeof(int));

	if (g_INIManager->GET_INI_INT("HideOnStartup"))
		DisplayState = true;

	ToggleDisplayState();
}

bool Console::ToggleDisplayState()
{
	if (IsConsoleInitalized() == 0)	return false;

	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
					
	if (IsHidden()) {
		Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, EM_LINESCROLL, 0, MessageBuffer.length());
		ShowWindow(WindowHandle, SW_SHOWNA);
		CheckMenuItem(ViewMenu, 9903, MF_CHECKED);
		
		DisplayState = true;
	} else {
		ShowWindow(WindowHandle, SW_HIDE);
		CheckMenuItem(ViewMenu, 9903, MF_UNCHECKED);
		DisplayState = false;
	}
	return DisplayState;
}

void Console::LoadINISettings()
{
	if (IsConsoleInitalized() == 0)	return;

	int Top = g_INIManager->FetchSetting("Top")->GetValueAsInteger(), 
		Left = g_INIManager->FetchSetting("Left")->GetValueAsInteger(),
		Right = g_INIManager->FetchSetting("Right")->GetValueAsInteger(),
		Bottom = g_INIManager->FetchSetting("Bottom")->GetValueAsInteger();

	SetWindowPos(WindowHandle, HWND_NOTOPMOST, Left, Top, Right, Bottom, NULL);
}

void Console::SaveINISettings()
{
	if (IsConsoleInitalized() == 0)	return;

	tagRECT WindowRect;
	GetWindowRect(WindowHandle, &WindowRect);

	_itoa_s(WindowRect.top, g_Buffer, sizeof(g_Buffer), 10);
	g_INIManager->FetchSetting("Top")->SetValue(g_Buffer);

	_itoa_s(WindowRect.left, g_Buffer, sizeof(g_Buffer), 10);
	g_INIManager->FetchSetting("Left")->SetValue(g_Buffer);

	_itoa_s(WindowRect.right - WindowRect.left, g_Buffer, sizeof(g_Buffer), 10);
	g_INIManager->FetchSetting("Right")->SetValue(g_Buffer);

	_itoa_s(WindowRect.bottom - WindowRect.top, g_Buffer, sizeof(g_Buffer), 10);
	g_INIManager->FetchSetting("Bottom")->SetValue(g_Buffer);
}

void Console::LogMessage(UInt8 Source, const char* Format, va_list Args)
{
	vsprintf_s(g_Buffer, sizeof(g_Buffer), Format, Args);

	std::string Message, Prefix;
	switch (Source)
	{
	case e_CSE:
		Prefix += "[CSE]\t";
		break;
	case e_CS:
		Prefix += "[CS]\t";
		break;
	case e_BE:
		Prefix += "[BE]\t";
		break;
	case e_UL:
		Prefix += "[UL]\t";
		break;
	case e_SE:
		Prefix += "[SE]\t";
		break;
	case e_BSA:
		Prefix += "[BSA]\t";
		break;
	}
	Message += Prefix;

	for (int i = 0; i < IndentLevel; i++) {
		Message += "\t";
	}

	Message += std::string(g_Buffer);
	if (Message.rfind("\r\n") != Message.length() - 2)
		MessageBuffer += Message + "\r\n";
	else
		MessageBuffer += Message;

	if (IsLogInitalized()) {
		fputs(Message.c_str(), DebugLog);
		fputs("\n", DebugLog);
		fflush(DebugLog);
	}

	if (IsConsoleInitalized() && !IsHidden()) {
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, WM_SETREDRAW, FALSE, 0);
		Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, EM_LINESCROLL, 0, MessageBuffer.length());	
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, WM_SETREDRAW, TRUE, 0);

		CSEInterfaceManager::HandleConsoleCallback(g_Buffer, Prefix.c_str());
	}
}

void Console::LogMessage(const char* Prefix, const char* Format, va_list Args)
{
	vsprintf_s(g_Buffer, sizeof(g_Buffer), Format, Args);

	std::string Message("[" + std::string(Prefix) + "]\t");

	for (int i = 0; i < IndentLevel; i++) {
		Message += "\t";
	}

	Message += std::string(g_Buffer);
	if (Message.rfind("\r\n") != Message.length() - 2)
		MessageBuffer += Message + "\r\n";
	else
		MessageBuffer += Message;

	if (IsLogInitalized()) {
		fputs(Message.c_str(), DebugLog);
		fputs("\n", DebugLog);
		fflush(DebugLog);
	}

	if (IsConsoleInitalized() && !IsHidden()) {
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, WM_SETREDRAW, FALSE, 0);
		Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, EM_LINESCROLL, 0, MessageBuffer.length());	
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, WM_SETREDRAW, TRUE, 0);

		CSEInterfaceManager::HandleConsoleCallback(g_Buffer, Prefix);
	}
}

void Console::LogMessage(const char* Prefix, const char* MessageStr)
{
	std::string Message("[" + std::string(Prefix) + "]\t");

	for (int i = 0; i < IndentLevel; i++) {
		Message += "\t";
	}

	Message += std::string(MessageStr);
	if (Message.rfind("\r\n") != Message.length() - 2)
		MessageBuffer += Message + "\r\n";
	else
		MessageBuffer += Message;

	if (IsLogInitalized()) {
		fputs(Message.c_str(), DebugLog);
		fputs("\n", DebugLog);
		fflush(DebugLog);
	}

	if (IsConsoleInitalized() && !IsHidden()) {
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, WM_SETREDRAW, FALSE, 0);
		Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, EM_LINESCROLL, 0, MessageBuffer.length());	
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, WM_SETREDRAW, TRUE, 0);

		CSEInterfaceManager::HandleConsoleCallback(MessageStr, Prefix);
	}
}

void Console::Clear()
{
	if (IsConsoleInitalized() == 0)	return;

	MessageBuffer.clear();
	Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
}

UInt32 Console::Indent()
{
	if (IsConsoleInitalized() == 0)	return 0;

	if (++IndentLevel > 10)		IndentLevel = 10;
	return IndentLevel;
}

UInt32 Console::Exdent()
{
	if (IsConsoleInitalized() == 0)	return 0;

	if (IndentLevel > 0)		--IndentLevel;
	return IndentLevel;
}