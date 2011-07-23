#pragma once

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
	bool							CreateTempWAVFile(const char* MP3Path, const char* WAVPath);
public:
	static CSInteropManager*		GetSingleton();

	bool							Initialize();
	void							Deinitialize();

	bool							DoGenerateLIPOperation(const char* InputPath, const char* ResponseText);
};

#define CSIOM						CSInteropManager::GetSingleton()