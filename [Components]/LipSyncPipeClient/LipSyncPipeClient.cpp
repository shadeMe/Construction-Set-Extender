#include <fstream>
#include "Windows.h"
#include "LipSyncPipeClient.h"
#include "[Common]\OldCSInteropData.h"

bool				g_HandleDebugText		=	false;
HANDLE				g_InteropPipeHandle		=	INVALID_HANDLE_VALUE;
FILE*				g_DebugLog				=	nullptr;

_DefinePatchHdlr(Crt0EntryPointInitialization, 0x008AE5C0);
_DefineNopHdlr(InitializeWindows, 0x0041D571, 5);
_DefineJumpHdlr(ShowSplashScreenWindow, 0x0041D3C8, 0x0041D3F6);
_DefineHookHdlr(MainWindowMessageLoop, 0x0041D5C7);
_DefinePatchHdlr(MessageHandlerDebugPrint, 0x009302A0);
_DefineHookHdlr(LogOC3AnimFactoryMessagesA, 0x0087FDB9);
_DefineNopHdlr(LogOC3AnimFactoryMessagesB, 0x0087FD8F, 2);
_DefinePatchHdlr(AllowMultipleEditors, 0x0041CD97);

BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		_MemHdlr(Crt0EntryPointInitialization).WriteUInt8(0x0);
		_MemHdlr(InitializeWindows).WriteNop();
		_MemHdlr(ShowSplashScreenWindow).WriteJump();
		_MemHdlr(MainWindowMessageLoop).WriteJump();
		_MemHdlr(MessageHandlerDebugPrint).WriteUInt32((UInt32)&HandleDebugText);
		_MemHdlr(LogOC3AnimFactoryMessagesA).WriteJump();
		_MemHdlr(LogOC3AnimFactoryMessagesB).WriteNop();
		_MemHdlr(AllowMultipleEditors).WriteUInt8(0xEB);

		g_DebugLog = _fsopen("LipSyncPipeClient.log", "w", _SH_DENYWR);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

bool GenerateLIPFile(char* FilePath, char* ResponseText)
{
	bool Result = false;
	g_HandleDebugText = true;

	__try
	{
		void* LIPResult = cdeclCall<void*>(0x00406010, &FilePath, ResponseText, 1);
		if (LIPResult)
		{
			cdeclCall<void>(0x004ABE20, &FilePath);
			Result = thisCall<UInt16>(0x004ABE10, LIPResult, &FilePath);
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
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr);

		if (g_InteropPipeHandle != INVALID_HANDLE_VALUE)
		{
			DWORD HandleState = PIPE_READMODE_MESSAGE|PIPE_WAIT;

			if (!SetNamedPipeHandleState(g_InteropPipeHandle, &HandleState, nullptr, nullptr))
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
	OldCSInteropData InteropDataIn(OldCSInteropData::kMessageType_Wait), InteropDataOut(OldCSInteropData::kMessageType_OperationResult);
	DWORD BytesReadWriteBuffer = 0;

	if (g_InteropPipeHandle == INVALID_HANDLE_VALUE)
	{
		ConnectToInteropPipe();
		PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Write, &InteropDataIn, &BytesReadWriteBuffer);
	}

	while (1)
	{
		if (PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Read, &InteropDataIn, &BytesReadWriteBuffer))
		{
			switch (InteropDataIn.MessageType)
			{
			case OldCSInteropData::kMessageType_Quit:
				DebugPrint("Lip Sync Pipe Client received a Quit message!");

				if (g_DebugLog)
				{
					fflush(g_DebugLog);
					fclose(g_DebugLog);
				}
				CloseHandle(g_InteropPipeHandle);
				ExitProcess(0);
				break;
			case OldCSInteropData::kMessageType_GenerateLIP:
				DebugPrint("Lip Sync Pipe Client received a GenerateLip message!");

				InteropDataOut.MessageType = OldCSInteropData::kMessageType_Wait;
				PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &BytesReadWriteBuffer);

				InteropDataOut.MessageType = OldCSInteropData::kMessageType_OperationResult;
				InteropDataOut.OperationResult = GenerateLIPFile(InteropDataIn.StringBufferA, InteropDataIn.StringBufferB);
				PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &BytesReadWriteBuffer);
				break;
			}
		}
		else if (GetLastError() == ERROR_BROKEN_PIPE)
		{
			DebugPrint("Interop pipe has ended unexpectedly - Terminating!");

			if (g_DebugLog)
			{
				fflush(g_DebugLog);
				fclose(g_DebugLog);
			}
			CloseHandle(g_InteropPipeHandle);
			ExitProcess(0);
		}
		else
		{
			DebugPrint("Pipe idle loop operation caused an error!");
			LogWinAPIErrorMessage(GetLastError());
		}

		Sleep(100);
	}
}

#pragma warning(push)
#pragma optimize("", off)
#pragma warning(disable: 4005 4748)

#define _hhName		MainWindowMessageLoop
_hhBegin()
{
	_hhSetVar(Retn, 0x0041D5CD);
	__asm
	{
		pushad
		call	ProcessServerMessage
		popad

		call	edi
		mov		esi, eax
		xor		eax, eax
		jmp		_hhGetVar(Retn)
	}
}

#define _hhName		LogOC3AnimFactoryMessagesA
_hhBegin()
{
	_hhSetVar(Retn, 0x0087FDBE);
	_hhSetVar(Call, 0x008731E0);
	__asm
	{
		pushad
		push	eax
		call	HandleDebugText
		popad

		call	_hhGetVar(Call)
		jmp		_hhGetVar(Retn)
	}
}

#pragma warning(pop)
#pragma optimize("", on)

void __stdcall HandleDebugText(const char* Message)
{
	if (!g_HandleDebugText)
		return;

	OldCSInteropData InteropDataOut(OldCSInteropData::kMessageType_DebugPrint);
	DWORD BytesReadWriteBuffer = 0;

	sprintf_s(InteropDataOut.StringBufferA, sizeof (InteropDataOut.StringBufferA), "%s", Message);
	PerformPipeOperation(g_InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &BytesReadWriteBuffer);
}

void DebugPrint(const char* fmt, ...)
{
	if (!g_DebugLog)
		return;

	char Buffer[0x500] = {0};

	va_list args;
	va_start(args, fmt);
	vsprintf_s(Buffer, sizeof(Buffer), fmt, args);
	va_end(args);

	fputs(Buffer, g_DebugLog);
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
		nullptr,
		ErrorID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &ErrorMsg,
		0, nullptr );

	DebugPrint("\tError Message: %s", (LPSTR)ErrorMsg);
	LocalFree(ErrorMsg);
}