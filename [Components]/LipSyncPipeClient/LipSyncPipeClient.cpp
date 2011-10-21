#include "LipSyncPipeClient.h"
#include "windows.h"
#include "[Common]\CSInteropData.h"
#include "[Libraries]\MemoryHandler\MemoryHandler.h"
#include <fstream>

using namespace SME::MemoryHandler;

MemHdlr				kCrt0EntryPointInitialization			(0x008AE5C0, (UInt32)0, 0, 0);					// prevents CS windows from being shown
NopHdlr				kInitializeWindows						(0x0041D571, 5);
MemHdlr				kShowSplashScreenWindow					(0x0041D3C8, 0x0041D3F6, 0, 0);
MemHdlr				kMainWindowMessageLoop					(0x0041D5C7, MainWindowMessageLoopHook, 0, 0);	// makes the client wait for a message from the server
MemHdlr				kMessageHandlerDebugPrint				(0x009302A0, (UInt32)0, 0, 0);
MemHdlr				kLogOC3AnimFactoryMessagesA				(0x0087FDB9, LogOC3AnimFactoryMessagesHook, 0, 0);
NopHdlr				kLogOC3AnimFactoryMessagesB				(0x0087FD8F, 2);

bool				g_HandleDebugText		=	false;

typedef void*	(__cdecl *_GenerateLIPFileWrapper)(char** Path, const char* ResponseText, UInt8 unk3);
const _GenerateLIPFileWrapper		GenerateLIPFileWrapper = (_GenerateLIPFileWrapper)0x00406010;

typedef UInt16	(__cdecl *_ReplaceSourceFileExtensionWithLIP)(char** SourcePath);
const _ReplaceSourceFileExtensionWithLIP	ReplaceSourceFileExtensionWithLIP = (_ReplaceSourceFileExtensionWithLIP)0x004ABE20;

static const UInt32	kCreateLIPFile = 0x004ABE10;

HANDLE				g_InteropPipeHandle		=	INVALID_HANDLE_VALUE;

FILE*				g_DebugLog				=	NULL;
char				g_TextBuffer[0x200]		=	{0};

extern "C"
{
void LipSyncPipeClient_Initialize()
{
	kCrt0EntryPointInitialization.WriteUInt8(0);
	kInitializeWindows.WriteNop();
	kShowSplashScreenWindow.WriteJump();
	kMainWindowMessageLoop.WriteJump();
	kMessageHandlerDebugPrint.WriteUInt32((UInt32)&HandleDebugText);
	kLogOC3AnimFactoryMessagesA.WriteJump();
	kLogOC3AnimFactoryMessagesB.WriteNop();
}
}

void DebugPrint(const char* fmt, ...)
{
	if (!g_DebugLog)	return;

	va_list args;
	va_start(args, fmt);
	vsprintf_s(g_TextBuffer, sizeof(g_TextBuffer), fmt, args);
	va_end(args);

	fputs(g_TextBuffer, g_DebugLog);
	fputs("\n", g_DebugLog);
	fflush(g_DebugLog);
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

void __stdcall HandleDebugText(const char* Message)
{
	if (!g_HandleDebugText)	return;

	CSECSInteropData InteropDataOut(CSECSInteropData::kMessageType_DebugPrint);
	DWORD BytesReadWriteBuffer = 0;

	sprintf_s(InteropDataOut.StringBufferA, sizeof (InteropDataOut.StringBufferA), "%s", Message);

	PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &BytesReadWriteBuffer);
}

bool GenerateLIPFile(char* FilePath, char* ResponseText)
{
	bool Result = false;
	g_HandleDebugText = true;

	__try
	{
		void* LIPResult = GenerateLIPFileWrapper(&FilePath, ResponseText, 1);
		if (LIPResult)
		{
			ReplaceSourceFileExtensionWithLIP(&FilePath);
			Result = thisCall<UInt16>(kCreateLIPFile, LIPResult, &FilePath);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DebugPrint("GenerateLIP operation raised an unhandled exception!");
		Result = false;
	}

	g_HandleDebugText = false;
	return Result;
}

void ConnectToInteropPipe(void)
{
	const char* PipeGUID = GetCommandLine();
	DebugPrint("Pipe GUID = %s", PipeGUID);

	char PipeName[0x200] = {0};
	sprintf_s(PipeName, 0x200, "\\\\.\\pipe\\{%s}", PipeGUID);

	while (1)
	{
		g_InteropPipeHandle = CreateFile(PipeName,
							GENERIC_READ|GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							0,
							NULL);

		if (g_InteropPipeHandle != INVALID_HANDLE_VALUE)
		{
			DWORD HandleState = PIPE_READMODE_MESSAGE|PIPE_WAIT;

			if (!SetNamedPipeHandleState(g_InteropPipeHandle, &HandleState, NULL, NULL))
			{
				DebugPrint("Couldn't set handle on interop pipe - Terminating!");
				LogWinAPIErrorMessage(GetLastError());
				CloseHandle(g_InteropPipeHandle);
				ExitProcess(0);
			}
			else
				DebugPrint("Lip Sync Pipe Client opened pipe successfully!");

			break;
		}
		else if (GetLastError() == ERROR_PIPE_BUSY)
			continue;
		else
		{
			DebugPrint("Couldn't open interop pipe - Terminating!");
			LogWinAPIErrorMessage(GetLastError());
			ExitProcess(0);
		}
	}
}

void __stdcall ProcessServerMessage(void)
{
	CSECSInteropData InteropDataIn(CSECSInteropData::kMessageType_Wait), InteropDataOut(CSECSInteropData::kMessageType_OperationResult);
	DWORD BytesReadWriteBuffer = 0;

	if (g_InteropPipeHandle == INVALID_HANDLE_VALUE)
	{
		g_DebugLog = _fsopen("LipSyncPipeClient.log", "w", _SH_DENYWR);

		ConnectToInteropPipe();
		PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Write, &InteropDataIn, &BytesReadWriteBuffer);
	}

	while (1)
	{
		if (PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Read, &InteropDataIn, &BytesReadWriteBuffer))
		{
			switch (InteropDataIn.MessageType)
			{
			case CSECSInteropData::kMessageType_Quit:
				DebugPrint("Lip Sync Pipe Client received a Quit message!");

				if (g_InteropPipeHandle != INVALID_HANDLE_VALUE)			CloseHandle(g_InteropPipeHandle);
				ExitProcess(0);
				break;
			case CSECSInteropData::kMessageType_GenerateLIP:
				DebugPrint("Lip Sync Pipe Client received a GenerateLip message!");

				InteropDataOut.MessageType = CSECSInteropData::kMessageType_Wait;
				PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &BytesReadWriteBuffer);

				InteropDataOut.MessageType = CSECSInteropData::kMessageType_OperationResult;
				InteropDataOut.OperationResult = GenerateLIPFile(InteropDataIn.StringBufferA, InteropDataIn.StringBufferB);
				PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &BytesReadWriteBuffer);
				break;
			}
		}
		else if (GetLastError() == ERROR_BROKEN_PIPE)
		{
			DebugPrint("Interop pipe has ended unexpectedly - Terminating!");
			CloseHandle(g_InteropPipeHandle);
			ExitProcess(0);
		}
		else
		{
			DebugPrint("Pipe idle loop operation caused an error!");
			LogWinAPIErrorMessage(GetLastError());
		}
	}
}

void __declspec(naked) MainWindowMessageLoopHook(void)
{
	static UInt32	kMainWindowMessageLoopHookRetnAddr = 0x0041D5CD;
	__asm
	{
		call	ProcessServerMessage

		call	edi
		mov		esi, eax
		xor		eax, eax
		jmp		[kMainWindowMessageLoopHookRetnAddr]
	}
}

void __declspec(naked) LogOC3AnimFactoryMessagesHook(void)
{
	static UInt32	kLogOC3AnimFactoryMessagesHookRetnAddr = 0x0087FDBE;
	static UInt32	kLogOC3AnimFactoryMessagesHookCallAddr = 0x008731E0;
	__asm
	{
		pushad
		push	eax
		call	HandleDebugText
		popad

		call	[kLogOC3AnimFactoryMessagesHookCallAddr]
		jmp		[kLogOC3AnimFactoryMessagesHookRetnAddr]
	}
}