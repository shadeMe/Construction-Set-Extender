#include "CSInterop.h"
#include "CSInteropData.h"
#include "ExtenderInternals.h"
#include "common/IFileStream.h"

#define INJECT_TIMEOUT			5000

CSInteropManager* CSInteropManager::Singleton = NULL;

CSInteropManager* CSInteropManager::GetSingleton()
{
	if (!Singleton)
		Singleton = new CSInteropManager();
	return Singleton;
}

CSInteropManager::CSInteropManager()
{
	Loaded = false;
	DLLPath = g_AppPath;
	InteropPipeHandle = INVALID_HANDLE_VALUE;

	ZeroMemory(&CS10ProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&PipeGUID, sizeof(GUID));
}

bool CSInteropManager::CreateNamedPipeServer(char** GUIDOut)
{
//	WaitUntilDebuggerAttached();

	RPC_STATUS GUIDReturn = UuidCreate(&PipeGUID),
			   GUIDStrReturn = UuidToString(&PipeGUID, (RPC_CSTR*)GUIDOut);

	if ((GUIDReturn == RPC_S_OK || GUIDReturn == RPC_S_UUID_LOCAL_ONLY) && GUIDStrReturn == RPC_S_OK)
	{
		DebugPrint("Pipe GUID = %s", *GUIDOut);
		char PipeName[0x200] = {0};
		sprintf_s(PipeName, 0x200, "\\\\.\\pipe\\{%s}", *GUIDOut);

		InteropPipeHandle = CreateNamedPipe(PipeName, 
								PIPE_ACCESS_DUPLEX|FILE_FLAG_FIRST_PIPE_INSTANCE|FILE_FLAG_WRITE_THROUGH|WRITE_OWNER,
								PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,
								1,
								sizeof(CSECSInteropData),
								sizeof(CSECSInteropData),
								50,
								NULL);

		if (InteropPipeHandle == INVALID_HANDLE_VALUE)
		{
			DebugPrint("Couldn't create interop pipe!");
			LogWinAPIErrorMessage(GetLastError());
			return false;
		}

		return true;
	}
	else
	{
		DebugPrint("Couldn't get pipe GUID!");
		return false;
	}
}

void CSInteropManager::Deinitialize()
{
	if (InteropPipeHandle == INVALID_HANDLE_VALUE)
		return;

	CSECSInteropData InteropDataOut(CSECSInteropData::kMessageType_Quit);
	DWORD BytesWritten = 0;

	if (!WriteFile(InteropPipeHandle,
			&InteropDataOut,
			sizeof(CSECSInteropData),
			&BytesWritten,
			NULL))
	{
		DebugPrint("Couldn't write exit message to interop pipe!");
		LogWinAPIErrorMessage(GetLastError());
	}

	CloseHandle(InteropPipeHandle);
}

bool CSInteropManager::InjectDLL(PROCESS_INFORMATION * info)
{
	bool	result = false;

	HANDLE	process = OpenProcess(
		PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, info->dwProcessId);
	if(process)
	{
		UInt32	hookBase = (UInt32)VirtualAllocEx(process, NULL, 8192, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if(hookBase)
		{
			// safe because kernel32 is loaded at the same address in all processes
			// (can change across restarts)
			UInt32	loadLibraryAAddr = (UInt32)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

			DebugPrint("HookBase = %08X", hookBase);
			DebugPrint("LoadLibraryAAddr = %08X", loadLibraryAAddr);

			UInt32	bytesWritten;
			WriteProcessMemory(process, (LPVOID)(hookBase + 5), DLLPath.c_str(), strlen(DLLPath.c_str()) + 1, &bytesWritten);

			UInt8	hookCode[5];

			hookCode[0] = 0xE9;
			*((UInt32 *)&hookCode[1]) = loadLibraryAAddr - (hookBase + 5);

			WriteProcessMemory(process, (LPVOID)(hookBase), hookCode, sizeof(hookCode), &bytesWritten);

			HANDLE	thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)hookBase, (void *)(hookBase + 5), 0, NULL);
			if(thread)
			{
				switch(WaitForSingleObject(thread, INJECT_TIMEOUT))
				{
					case WAIT_OBJECT_0:
						DebugPrint("Hook thread complete");
						result = true;
						break;
					case WAIT_ABANDONED:
						DebugPrint("Process::InstallHook: waiting for thread = WAIT_ABANDONED");
						break;
					case WAIT_TIMEOUT:
						DebugPrint("Process::InstallHook: waiting for thread = WAIT_TIMEOUT");
						break;
				}
				CloseHandle(thread);
			}
			else
				DebugPrint("CreateRemoteThread failed (%d)", GetLastError());

			VirtualFreeEx(process, (LPVOID)hookBase, 8192, MEM_RELEASE);
		}
		else
			DebugPrint("Process::InstallHook: couldn't allocate memory in target process");

		CloseHandle(process);
	}
	else
		DebugPrint("Process::InstallHook: couldn't get process handle");

	return result;
}

void CSInteropManager::DoInjectDLL(PROCESS_INFORMATION * info)
{
	Loaded = false;
	__try {
		Loaded = InjectDLL(&CS10ProcInfo);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DebugPrint("DLL injection failed. In most cases, this is caused by an overly paranoid software firewall or antivirus package. Disabling either of these may solve the problem.");
	}
}

bool CSInteropManager::Initialize(const char *DLLPath)
{
	if (Loaded)	return true;

	DebugPrint("Initializing CSInteropManager");
	CONSOLE->Indent();

	char* GUIDStr = 0;
	if (!CreateNamedPipeServer(&GUIDStr))
	{
		CONSOLE->Exdent();
		return false;
	}

	STARTUPINFO			startupInfo = { 0 };

	startupInfo.cb = sizeof(startupInfo);

	const char	* procName = "TESConstructionSetOld.exe";
	this->DLLPath += DLLPath;

	DebugPrint("Dll = %s", this->DLLPath.c_str());

	// check to make sure the dll exists
	IFileStream	tempFile;

	if(!tempFile.Open(this->DLLPath.c_str()))
	{
		DebugPrint("Couldn't find DLL (%s)!", this->DLLPath.c_str());
		CONSOLE->Exdent();
		return false;
	}

	Loaded = CreateProcess(
		procName,
		GUIDStr,	// pass the pipe guid
		NULL,		// default process security
		NULL,		// default thread security
		TRUE,		// don't inherit handles
		CREATE_SUSPENDED,
		NULL,		// no new environment
		NULL,		// no new cwd
		&startupInfo, &CS10ProcInfo) != 0;

	RpcStringFree((RPC_CSTR*)&GUIDStr);

	// check for Vista failing to create the process due to elevation requirements
	if(!Loaded && (GetLastError() == ERROR_ELEVATION_REQUIRED))
	{
		// in theory we could figure out how to UAC-prompt for this process and then run CreateProcess again, but I have no way to test code for that
		DebugPrint("Vista has decided that launching the CS 1.0 requires UAC privilege elevation. There is no good reason for this to happen, but to fix it, right-click on obse_loader.exe, go to Properties, pick the Compatibility tab, then turn on \"Run this program as an administrator\".");
		CONSOLE->Exdent();
		return Loaded;
	}
	
	if (!Loaded)
	{
		DebugPrint("Couldn't load DLL (%s)!", this->DLLPath.c_str());
		LogWinAPIErrorMessage(GetLastError());
		CONSOLE->Exdent();
		return Loaded;
	}

	DoInjectDLL(&CS10ProcInfo);
		
	if(Loaded)
	{
		ResumeThread(CS10ProcInfo.hThread);
	}
	else
	{
		// kill the partially-created process
		TerminateProcess(CS10ProcInfo.hProcess, 0);
		Deinitialize();
	}

	// clean up
	CloseHandle(CS10ProcInfo.hProcess);
	CloseHandle(CS10ProcInfo.hThread);

	CONSOLE->Exdent();
	return Loaded;
}

bool CSInteropManager::DoGenerateLIPOperation(const char* WAVPath, const char* ResponseText)
{
	if (!Loaded)
	{
		DebugPrint("Interop manager not initialized!");
		return false;
	}

	bool Result = false, ExitLoop = false;
	DWORD ByteCounter = 0;
	CSECSInteropData InteropDataOut(CSECSInteropData::kMessageType_GenerateLIP), InteropDataIn(CSECSInteropData::kMessageType_Wait);
	sprintf_s(InteropDataOut.StringBufferA, sizeof(InteropDataOut.StringBufferA), "%s", WAVPath);
	sprintf_s(InteropDataOut.StringBufferB, sizeof(InteropDataOut.StringBufferB), "%s", ResponseText);

	HWND IdleWindow = CreateDialogParam(g_DLLInstance, MAKEINTRESOURCE(DLG_IDLE), *g_HWND_CSParent, NULL, NULL);

	DebugPrint("Generating LIP file for '%s'...", WAVPath);
	CONSOLE->Indent();

	if (PerformPipeOperation(InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &ByteCounter))
	{
		InteropDataOut.MessageType = CSECSInteropData::kMessageType_Wait;

		while (1)
		{
			if (PerformPipeOperation(InteropPipeHandle, kPipeOperation_Read, &InteropDataIn, &ByteCounter))
			{
				switch (InteropDataIn.MessageType)
				{
				case CSECSInteropData::kMessageType_DebugPrint:
					DebugPrint(InteropDataIn.StringBufferA);
					break;
				case CSECSInteropData::kMessageType_OperationResult:
					Result = InteropDataIn.OperationResult;
					ExitLoop = true;
					break;
				}

				if (ExitLoop)
					break;
			}
			else
			{
				DebugPrint("GenerateLIP Operation idle loop encountered an error!");
				LogWinAPIErrorMessage(GetLastError());
			}
		}
		PerformPipeOperation(InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &ByteCounter);
	}

	DestroyWindow(IdleWindow);
	
	CONSOLE->Exdent();
	return Result;
}