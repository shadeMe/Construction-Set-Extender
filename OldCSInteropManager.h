#pragma once

namespace cse
{
	// loader derived from OBSE's code. well, duplicated.
	class OldCSInteropManager
	{
		OldCSInteropManager();

		enum class LoadState
		{
			Unloaded,
			Loaded,
			Error
		};

		PROCESS_INFORMATION		CS10ProcInfo;
		std::string				DLLPath;
		HANDLE					InteropPipeHandle;
		LoadState				State;
		GUID					PipeGUID;

		bool	DoInjectDLL();
		bool	InjectDLL(PROCESS_INFORMATION * info);
		bool	CreateNamedPipeServer(char** GUIDOut);
		bool	CreateTempWAVFile(const char* MP3Path, const char* WAVPath);
		void	Initialize();
	public:
		~OldCSInteropManager();

		bool	GenerateLIPSyncFile(const char* InputPath, const char* ResponseText);

		static	OldCSInteropManager		Instance;
	};

#define CSIOM						OldCSInteropManager::Instance
}
