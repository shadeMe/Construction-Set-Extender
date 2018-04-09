#pragma once

namespace cse
{
	// loader derived from OBSE's code. well, duplicated.
	class OldCSInteropManager
	{
		static OldCSInteropManager*		Singleton;

		static const char*				kTempMp3Path;
		static const char*				kTempWavPath;
		static const char*				kTempLipPath;

		OldCSInteropManager();

		PROCESS_INFORMATION				CS10ProcInfo;
		std::string						DLLPath;
		HANDLE							InteropPipeHandle;
		bool							Loaded;
		GUID							PipeGUID;

		void							DoInjectDLL(PROCESS_INFORMATION * info);
		bool							InjectDLL(PROCESS_INFORMATION * info);
		bool							CreateNamedPipeServer(char** GUIDOut);
	public:
		~OldCSInteropManager();

		static OldCSInteropManager*		GetSingleton();

		bool							Initialize();
		bool							GenerateLIPSyncFile(const char* InputPath, const char* ResponseText);
		bool							GetInitialized() const;
	};

#define CSIOM						OldCSInteropManager::GetSingleton()
}
