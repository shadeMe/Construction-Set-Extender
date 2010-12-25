#pragma once
#include <string>
#include "Rpc.h"

#pragma comment(lib, "Rpcrt4.lib")

// loader derived from OBSE's code. well, duplicated.

class CSInteropManager
{
	static CSInteropManager*		Singleton;

	CSInteropManager();

	PROCESS_INFORMATION				CS10ProcInfo;
	std::string						DLLPath;
	HANDLE							InteropPipeHandle;
	bool							Loaded;
	GUID							PipeGUID;

	void							DoInjectDLL(PROCESS_INFORMATION * info);
	bool							InjectDLL(PROCESS_INFORMATION * info);
	bool							CreateNamedPipeServer(char** GUIDOut);
public:
	static CSInteropManager*		GetSingleton();

	bool							Initialize(const char* DLLPath);
	void							Deinitialize();

	bool							DoGenerateLIPOperation(const char* WAVPath, const char* ResponseText);
};

#define CSIOM						CSInteropManager::GetSingleton()