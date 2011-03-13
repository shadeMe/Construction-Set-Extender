#pragma once

class Console
{
	static Console*				Singleton;

	Console();

	HWND						WindowHandle;
	HWND						EditHandle;
	bool						DisplayState;
	FILE*						DebugLog;
	UInt32						IndentLevel;
	std::string					MessageBuffer;
	bool						UpdateSignalFlag;

	void						PrintMessage(std::string& Prefix, const char* MessageStr);
public:
	static Console*				GetSingleton();

	enum MessageSource
	{
		e_CSE = 0,
		e_CS,
		e_BE,
		e_UL,
		e_SE,
		e_BSA,
		e_TAG
	};

	void						InitializeConsole();
	void						InitializeLog(const char* AppPath);
	bool						IsHidden() { return DisplayState == 0; }
	bool						IsConsoleInitalized() { return WindowHandle != 0; }
	bool						IsLogInitalized() { return DebugLog != 0; }
	bool						ToggleDisplayState();
	void						LoadINISettings();
	void						SaveINISettings();
	HWND						GetWindowHandle() { return WindowHandle; }	
	void						Deinitialize();
	const char*					GetMessageBuffer() { return MessageBuffer.c_str(); }
	bool						GetShouldUpdate() { bool Result = UpdateSignalFlag; UpdateSignalFlag = false; return Result; }

	void						LogMessage(UInt8 Source, const char* Format, va_list Args);
	void						LogMessage(const char* Prefix,  const char* Format, va_list Args);
	void						LogMessage(const char* Prefix,  const char* MessageStr);
	UInt32						Indent();
	UInt32						Exdent();
	void						ExdentAll() { IndentLevel = 0; }
	void						Clear();
};

#define CONSOLE									Console::GetSingleton()
#define CONSOLECMDBOX_RESETCOMMANDSTACK			9990
#define CONSOLECMDBOX_CLEARCOMMANDSTACK			9991
#define CONSOLE_UPDATETIMER						0x9999
#define CONSOLE_UPDATEPERIOD					g_INIManager->GET_INI_INT("ConsoleUpdatePeriod")

