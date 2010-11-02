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

	DebugPrint("\tError Message: %s", (LPSTR)ErrorMsg); 
	LocalFree(ErrorMsg);
}

Console* Console::Singleton = NULL;

Console::Console()
{
	WindowHandle = NULL;
	EditHandle = NULL;
	DisplayState = false;
	MessageBuffer.reserve(0x3000);
	IndentLevel = 0;
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
}

void Console::InitializeConsole()
{
	if (IsConsoleInitalized())	return;

	WindowHandle = CreateDialog(g_DLLInstance, MAKEINTRESOURCE(DLG_CONSOLE), NULL, (DLGPROC)ConsoleDlgProc);
	EditHandle = GetDlgItem(WindowHandle, EDIT_CONSOLE);
	g_ConsoleEditControlOrgWindowProc = (WNDPROC)SetWindowLong(EditHandle, GWL_WNDPROC, (LONG)ConsoleEditControlSubClassProc);
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

void Console::LoadINISettings()
{
	int Top = g_INIManager->FetchSetting("Top")->GetValueAsInteger(), 
		Left = g_INIManager->FetchSetting("Left")->GetValueAsInteger(),
		Right = g_INIManager->FetchSetting("Right")->GetValueAsInteger(),
		Bottom = g_INIManager->FetchSetting("Bottom")->GetValueAsInteger();

	SetWindowPos(WindowHandle, HWND_NOTOPMOST, Left, Top, Right, Bottom, NULL);
}

void Console::SaveINISettings()
{
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

	for (int i = 0; i < IndentLevel; i++) {
		Message += "\t";
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
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, WM_SETREDRAW, FALSE, 0);
		Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, EM_LINESCROLL, 0, MessageBuffer.length());	
		SendDlgItemMessage(WindowHandle, EDIT_CONSOLE, WM_SETREDRAW, TRUE, 0);
	}
}

void Console::Clear()
{
	MessageBuffer.clear();
	Edit_SetText(EditHandle, (LPCSTR)MessageBuffer.c_str());
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

void DoNop(const NopData* Data)
{
	for (int Offset = 0; Offset < Data->Size; Offset++) {
		SafeWrite8(Data->Address + Offset, 0x90);
	}	
}

// modified to use plugin debugging tools
void CSEDumpClass(void * theClassPtr, UInt32 nIntsToDump)
{
	DebugPrint("DumpClass:");
	UInt32* basePtr = (UInt32*)theClassPtr;

	if (!theClassPtr) return;
	for (UInt32 ix = 0; ix < nIntsToDump; ix++ ) {
		UInt32* curPtr = basePtr+ix;
		const char* curPtrName = NULL;
		UInt32 otherPtr = 0;
		float otherFloat = 0.0;
		const char* otherPtrName = NULL;
		if (curPtr) {
			curPtrName = GetObjectClassName((void*)curPtr);

			__try
			{
				otherPtr = *curPtr;
				otherFloat = *(float*)(curPtr);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				//
			}

			if (otherPtr) {
				otherPtrName = GetObjectClassName((void*)otherPtr);
			}
		}

		DebugPrint("\t%3d +%03X ptr: 0x%08X: %32s *ptr: 0x%08x | %f: %32s", ix, ix*4, curPtr, curPtrName, otherPtr, otherFloat, otherPtrName);
	}
}