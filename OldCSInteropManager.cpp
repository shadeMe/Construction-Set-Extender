#include "OldCSInteropManager.h"
#include "Construction Set Extender_Resource.h"
#include "[Common]\OldCSInteropData.h"

namespace cse
{
	OldCSInteropManager	OldCSInteropManager::Instance;


	OldCSInteropManager::OldCSInteropManager()
	{
		State = LoadState::Unloaded;
		DLLPath = "";
		InteropPipeHandle = INVALID_HANDLE_VALUE;

		ZeroMemory(&CS10ProcInfo, sizeof(PROCESS_INFORMATION));
		ZeroMemory(&PipeGUID, sizeof(GUID));
	}

	OldCSInteropManager::~OldCSInteropManager()
	{
		if (State != LoadState::Loaded)
			return;

		OldCSInteropData InteropDataOut(OldCSInteropData::kMessageType_Quit);
		DWORD BytesWritten = 0;

		WriteFile(InteropPipeHandle,
			&InteropDataOut,
			sizeof(OldCSInteropData),
			&BytesWritten,
			nullptr);

		CloseHandle(InteropPipeHandle);
	}

	bool OldCSInteropManager::CreateNamedPipeServer(char** GUIDOut)
	{
		RPC_STATUS GUIDReturn = UuidCreate(&PipeGUID),
			GUIDStrReturn = UuidToString(&PipeGUID, (RPC_CSTR*)GUIDOut);

		if ((GUIDReturn == RPC_S_OK || GUIDReturn == RPC_S_UUID_LOCAL_ONLY) && GUIDStrReturn == RPC_S_OK)
		{
			char PipeName[0x200] = {0};
			sprintf_s(PipeName, 0x200, "\\\\.\\pipe\\{%s}", *GUIDOut);

			InteropPipeHandle = CreateNamedPipe(PipeName,
				PIPE_ACCESS_DUPLEX|FILE_FLAG_FIRST_PIPE_INSTANCE|FILE_FLAG_WRITE_THROUGH|WRITE_OWNER,
				PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,
				1,
				sizeof(OldCSInteropData),
				sizeof(OldCSInteropData),
				50,
				nullptr);

			if (InteropPipeHandle == INVALID_HANDLE_VALUE)
			{
				BGSEECONSOLE_ERROR("Couldn't create interop pipe!");

				return false;
			}

			return true;
		}
		else
		{
			BGSEECONSOLE_MESSAGE("Couldn't get pipe GUID!");
			return false;
		}
	}

	bool OldCSInteropManager::InjectDLL(PROCESS_INFORMATION * info)
	{
		bool	result = false;

		HANDLE	process = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ,
									FALSE,
									info->dwProcessId);

		if (process)
		{
			UInt32	hookBase = (UInt32)VirtualAllocEx(process, nullptr, 8192, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if(hookBase)
			{
				// safe because kernel32 is loaded at the same address in all processes
				// (can change across restarts)
				UInt32	loadLibraryAAddr = (UInt32)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

				UInt32	bytesWritten;
				WriteProcessMemory(process, (LPVOID)(hookBase + 5), DLLPath.c_str(), strlen(DLLPath.c_str()) + 1, &bytesWritten);

				UInt8	hookCode[5];

				hookCode[0] = 0xE9;
				*((UInt32 *)&hookCode[1]) = loadLibraryAAddr - (hookBase + 5);

				WriteProcessMemory(process, (LPVOID)(hookBase), hookCode, sizeof(hookCode), &bytesWritten);

				HANDLE	thread = CreateRemoteThread(process, nullptr, 0, (LPTHREAD_START_ROUTINE)hookBase, (void *)(hookBase + 5), 0, nullptr);
				if(thread)
				{
					switch(WaitForSingleObject(thread, 5000))
					{
					case WAIT_OBJECT_0:
						result = true;
						break;
					case WAIT_ABANDONED:
						BGSEECONSOLE_MESSAGE("Process::InstallHook: waiting for thread = WAIT_ABANDONED");
						break;
					case WAIT_TIMEOUT:
						BGSEECONSOLE_MESSAGE("Process::InstallHook: waiting for thread = WAIT_TIMEOUT");
						break;
					}

					CloseHandle(thread);
				}
				else
					BGSEECONSOLE_ERROR("CreateRemoteThread failed!");

				VirtualFreeEx(process, (LPVOID)hookBase, 8192, MEM_RELEASE);
			}
			else
				BGSEECONSOLE_MESSAGE("Process::InstallHook: couldn't allocate memory in target process");

			CloseHandle(process);
		}
		else
			BGSEECONSOLE_MESSAGE("Process::InstallHook: couldn't get process handle");

		return result;
	}

	bool OldCSInteropManager::DoInjectDLL()
	{
		__try
		{
			return InjectDLL(&CS10ProcInfo);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			;//
		}

		return false;
	}

	void OldCSInteropManager::Initialize()
	{
		if (State == LoadState::Loaded || State == LoadState::Error)
			return;

		BGSEECONSOLE_MESSAGE("Initializing Old CS Interop Manager...");

		char* GUIDStr = 0;
		if (!CreateNamedPipeServer(&GUIDStr))
		{
			State = LoadState::Error;
			return;
		}

		STARTUPINFO			startupInfo = { 0 };

		startupInfo.cb = sizeof(startupInfo);

		const char	* procName = "TESConstructionSetOld.exe";
		this->DLLPath = std::string(BGSEEMAIN->GetComponentDLLPath()) + "LipSyncPipeClient.dll";

		// check to make sure the dll exists
		IFileStream	tempFile;

		if(!tempFile.Open(this->DLLPath.c_str()))
		{
			BGSEECONSOLE_MESSAGE("Couldn't find DLL (%s)!", this->DLLPath.c_str());
			RpcStringFree((RPC_CSTR*)&GUIDStr);

			State = LoadState::Error;
			return;
		}

		auto CreateProcessResult = CreateProcess(procName,
							GUIDStr,	// pass the pipe guid
							nullptr,		// default process security
							nullptr,		// default thread security
							FALSE,		// don't inherit handles
							CREATE_SUSPENDED,
							nullptr,		// no new environment
							nullptr,		// no new cwd
							&startupInfo, &CS10ProcInfo) != 0;

		RpcStringFree((RPC_CSTR*)&GUIDStr);


		if (CreateProcessResult == false)
		{
			BGSEECONSOLE_ERROR("Couldn't load CS 1.0!");

			State = LoadState::Error;
			return;
		}

		State = DoInjectDLL() ? LoadState::Loaded : LoadState::Error;

		if (State == LoadState::Loaded)
			ResumeThread(CS10ProcInfo.hThread);
		else
		{
			BGSEECONSOLE_MESSAGE("DLL injection failed. In most cases, this is caused by an overly paranoid software firewall or antivirus package. Disabling either of these may solve the problem.");

			// kill the partially-created process
			TerminateProcess(CS10ProcInfo.hProcess, 0);
		}

		// clean up
		CloseHandle(CS10ProcInfo.hProcess);
		CloseHandle(CS10ProcInfo.hThread);
	}

	bool OldCSInteropManager::CreateTempWAVFile(const char* MP3Path, const char* WAVPath)
	{
		STARTUPINFO StartupInfo = { 0 };
		PROCESS_INFORMATION ProcInfo = { 0 };
		StartupInfo.cb = sizeof(StartupInfo);

		IFileStream	TempFile;

		if (TempFile.Open(MP3Path) == false)
		{
			BGSEECONSOLE_MESSAGE("Couldn't find source MP3 file!");
			return false;
		}

		std::string DecoderArgs = std::string(BGSEEMAIN->GetAPPPath()) +
								"lame.exe \"" +
								std::string(BGSEEMAIN->GetAPPPath()) + std::string(MP3Path) +
								"\" \"" +
								std::string(BGSEEMAIN->GetAPPPath()) + std::string(WAVPath) +
								"\" --decode";

		bool Result = CreateProcess(nullptr,
									(LPSTR)DecoderArgs.c_str(),
									nullptr,		// default process security
									nullptr,		// default thread security
									FALSE,		// don't inherit handles
									NULL,
									nullptr,		// no new environment
									nullptr,		// no new cwd
									&StartupInfo, &ProcInfo) != 0;

		if (Result == false)
		{
			BGSEECONSOLE_ERROR("Couldn't launch LAME decoder!");
			return false;
		}

		WaitForSingleObject(ProcInfo.hProcess, INFINITE);		// wait till the decoder's done its job

		CloseHandle(ProcInfo.hProcess);
		CloseHandle(ProcInfo.hThread);

		return true;
	}

	bool OldCSInteropManager::GenerateLIPSyncFile(const char* InputPath, const char* ResponseText)
	{
		// Lazy load the old CS
		Initialize();

		if (State == LoadState::Error)
		{
			BGSEECONSOLE_MESSAGE("Interop manager not initialized!");
			return false;
		}

		bool Result = false, ExitLoop = false;
		DWORD ByteCounter = 0;
		std::string MP3Path(InputPath), WAVPath(InputPath);
		MP3Path += ".mp3", WAVPath += ".wav";

		OldCSInteropData InteropDataOut(OldCSInteropData::kMessageType_GenerateLIP), InteropDataIn(OldCSInteropData::kMessageType_Wait);
		sprintf_s(InteropDataOut.StringBufferA, sizeof(InteropDataOut.StringBufferA), "%s", WAVPath.c_str());
		sprintf_s(InteropDataOut.StringBufferB, sizeof(InteropDataOut.StringBufferB), "%s", ResponseText);

		bool HasWAVFile = false;
		IFileStream	TempFile;
		if (TempFile.Open(WAVPath.c_str()))
			HasWAVFile = true;

		BGSEECONSOLE_MESSAGE("Generating LIP file for '%s'...", InputPath);
		BGSEECONSOLE->Indent();

		if (HasWAVFile || CreateTempWAVFile(MP3Path.c_str(), WAVPath.c_str()))
		{
			if (PerformPipeOperation(InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &ByteCounter))
			{
				InteropDataOut.MessageType = OldCSInteropData::kMessageType_Wait;

				while (true)
				{
					if (PerformPipeOperation(InteropPipeHandle, kPipeOperation_Read, &InteropDataIn, &ByteCounter))
					{
						switch (InteropDataIn.MessageType)
						{
						case OldCSInteropData::kMessageType_DebugPrint:
							BGSEECONSOLE_MESSAGE(InteropDataIn.StringBufferA);
							break;
						case OldCSInteropData::kMessageType_OperationResult:
							Result = InteropDataIn.OperationResult;
							ExitLoop = true;
							break;
						}

						if (ExitLoop)
							break;
					}
					else
					{
						BGSEECONSOLE_ERROR("CSInteropManager::GenerateLIPSyncFile - Idle loop encountered an error!");
						break;
					}
				}

				PerformPipeOperation(InteropPipeHandle, kPipeOperation_Write, &InteropDataOut, &ByteCounter);
			}
			else
			{
				BGSEECONSOLE_ERROR("Couldn't communicate with CS v1.0!");
			}
		}
		else
		{
			BGSEECONSOLE_MESSAGE("Couldn't create temporary WAV file for LIP generation!");
		}

		if (HasWAVFile == false &&
			DeleteFile((std::string(BGSEEMAIN->GetAPPPath() + WAVPath)).c_str()) == FALSE &&
			GetLastError() != ERROR_FILE_NOT_FOUND)
		{
			BGSEECONSOLE_ERROR("Couldn't delete temporary WAV file '%s'!", WAVPath.c_str());
		}

		BGSEECONSOLE->Outdent();

		return Result;
	}

	bool OldCSInteropManager::IsAvailable() const
	{
		return State == LoadState::Loaded;
	}


}

