#include "ExtenderInternals.h"
#include "WindowManager.h"
#include "resource.h"

extern HINSTANCE g_DLLInstance;

void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State)
{
	if (State)	*Flag |= Mask;
	else		*Flag &= ~Mask;
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

	CONSOLE->LogMessage(Console::e_CSE, "\tError Message: %s", (LPSTR)ErrorMsg); 
	LocalFree(ErrorMsg);
}

Console* Console::Singleton = NULL;

Console::Console()
{
	WindowHandle = NULL;
	EditHandle = NULL;
	DisplayState = false;
	MessageBuffer.reserve(0x1000);
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

	DebugLog.open(std::string(std::string(AppPath) + "Construction Set Extender.log").c_str(), std::fstream::out);
	DebugLog << MessageBuffer << std::endl;
}

void Console::InitializeConsole()
{
	if (IsConsoleInitalized())	return;

	WindowHandle = CreateDialog(g_DLLInstance, MAKEINTRESOURCE(DLG_CONSOLE), NULL, (DLGPROC)ConsoleDlgProc);
	EditHandle = GetDlgItem(WindowHandle, EDIT_CONSOLE);
	Edit_LimitText(EditHandle, sizeof(int));
	ToggleDisplayState();
}

bool Console::ToggleDisplayState()
{
	if (IsHidden()) {
		Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, EM_LINESCROLL, 0, MessageBuffer.length());
		ShowWindow(WindowHandle, SW_SHOWNA);
		DisplayState = true;
	} else {
		ShowWindow(WindowHandle, SW_HIDE);
		DisplayState = false;
	}
	return DisplayState;
}

void Console::LoadINISettings(const char* INIPath)
{
	int Top = GetPrivateProfileIntA("Console::General", "Top", 150, (LPCSTR)INIPath), 
		Left = GetPrivateProfileIntA("Console::General", "Left", 150, (LPCSTR)INIPath),
		Right = GetPrivateProfileIntA("Console::General", "Right", 500, (LPCSTR)INIPath),
		Bottom = GetPrivateProfileIntA("Console::General", "Bottom", 350, (LPCSTR)INIPath);

	SetWindowPos(WindowHandle, HWND_NOTOPMOST, Left, Top, Right, Bottom, NULL);
}

void Console::SaveINISettings(const char* INIPath)
{
	tagRECT WindowRect;
	GetWindowRect(WindowHandle, &WindowRect);

	_itoa_s(WindowRect.top, g_Buffer, sizeof(g_Buffer), 10);
	WritePrivateProfileStringA("Console::General", "Top", (LPCSTR)g_Buffer, (LPCSTR)INIPath); 
	_itoa_s(WindowRect.left, g_Buffer, sizeof(g_Buffer), 10);
	WritePrivateProfileStringA("Console::General", "Left", (LPCSTR)g_Buffer, (LPCSTR)INIPath);
	_itoa_s(WindowRect.right - WindowRect.left, g_Buffer, sizeof(g_Buffer), 10);
	WritePrivateProfileStringA("Console::General", "Right", (LPCSTR)g_Buffer, (LPCSTR)INIPath);
	_itoa_s(WindowRect.bottom - WindowRect.top, g_Buffer, sizeof(g_Buffer), 10);
	WritePrivateProfileStringA("Console::General", "Bottom", (LPCSTR)g_Buffer, (LPCSTR)INIPath);
}

void Console::LogMessage(UInt8 Source, const char* Format, ...)
{
	va_list Args;

	va_start(Args, Format);
	vsprintf_s(g_Buffer, sizeof(g_Buffer), Format, Args);
	va_end(Args);

	std::string Message;
	switch (Source)
	{
	case e_CSE:
		Message += "[CSE]\t";
		break;
	case e_CS:
		Message += "[CS]\t";
		break;
	case e_BE:
		Message += "[BE]\t";
		break;
	case e_UL:
		Message += "[UL]\t";
		break;
	case e_SE:
		Message += "[SE]\t";
		break;
	case e_BSA:
		Message += "[BSA]\t";
		break;
	}

	Message += std::string(g_Buffer);
	if (Message.rfind("\r\n") != Message.length() - 2)
		MessageBuffer += Message + "\r\n";
	else
		MessageBuffer += Message;

	if (IsLogInitalized()) {
		DebugLog << Message << std::endl;
	}

	if (IsConsoleInitalized() && !IsHidden()) {
		Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
	//	Edit_SetSel(EditHandle, - 1, - 1);
	//	Edit_ReplaceSel(EditHandle, (LPCSTR)Message.c_str());
	//	SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, EM_REPLACESEL, 0, (LPARAM)Message.c_str());
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, EM_LINESCROLL, 0, MessageBuffer.length());	
	}
}