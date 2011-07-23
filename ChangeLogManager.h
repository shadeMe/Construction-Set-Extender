#pragma once

namespace VersionControl
{
	class ChangeLog
	{
		FILE*						Log;
		std::string					FilePath;
		char						Buffer[0x200];
	public:
		ChangeLog(const char* Path, const char* FileName);

		void						WriteChange(const char* Message, bool StampTime, bool Flush);
		bool						Copy(const char* DestinationPath, bool Overwrite);
		void						Finalize();
		bool						Delete();
		void						OpenForViewing();
	};

	class ChangeLogManager
	{
		static ChangeLogManager*	Singleton;

		std::stack<ChangeLog*>		LogStack;
		ChangeLog*					SessionLog;
		char						GUIDString[0x100];
		char						TempPath[MAX_PATH];
		bool						Initialized;

		ChangeLogManager();

		const char*					GetCurrentTempDirectory();
		void						WriteChangeToLogs(const char* Message, bool StampTime);
	public:
		static ChangeLogManager*	GetSingleton();

		enum
		{
			kFormChange_SetActive = 0,
			kFormChange_SetDeleted,
			kFormChange_SetFormID,
			kFormChange_SetEditorID
		};

		void						Initialize();
		void						Deinitialize();

		void						RecordChange(const char* Format, ...);
		void						RecordChange(TESForm* Base, TESForm* New);
		void						RecordFormChange(TESForm* Form, UInt8 ChangeType, UInt32 Value);

		void						Pad(UInt32 Size);

		bool						CopyActiveLog(const char* DestinationPath) { return LogStack.top()->Copy(DestinationPath, false); }
		void						PushNewActiveLog();

		void						OpenSessionLog();
	};
	#define CHANGELOG				ChangeLogManager::GetSingleton()

	const char*						GetTimeString(char* Out, size_t Size);
	void							HandlePluginSave(TESFile* SaveFile);
	void							HandlePluginLoad();
}