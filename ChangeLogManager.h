#pragma once

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		enum
		{
			kVersionControl_BackupOnSave = 0,

			kVersionControl__MAX
		};
		extern const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kVersionControlINISettings[kVersionControl__MAX];
		BGSEditorExtender::BGSEEINIManagerSettingFactory*								GetVersionControl(void);
	}

	namespace VersionControl
	{
		class ChangeLogManager;

		class ChangeLog
		{
			friend class ChangeLogManager;

			FILE*						Log;
			std::string					FilePath;
			char						Buffer[0x200];
		public:
			ChangeLog(const char* Path, const char* FileName);

			void						WriteChange(const char* Message, bool StampTime, bool Flush, void* ConsoleMessageContext = NULL);
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
			void*						ConsoleMessageContext;
			bool						Initialized;

			ChangeLogManager();

			const char*					GetCurrentTempDirectory(char* OutBuffer, UInt32 BufferSize);
			void						WriteChangeToLogs(const char* Message, bool StampTime);
		public:
			~ChangeLogManager();

			static ChangeLogManager*	GetSingleton();

			enum
			{
				kFormChange_SetActive = 0,
				kFormChange_SetDeleted,
				kFormChange_SetFormID,
				kFormChange_SetEditorID
			};

			void						Initialize();

			void						RecordChange(const char* Format, ...);
			void						RecordChange(TESForm* Base, TESForm* New);
			void						RecordFormChange(TESForm* Form, UInt8 ChangeType, UInt32 Value);

			void						Pad(UInt32 Size);

			bool						CopyActiveLog(const char* DestinationPath) { return LogStack.top()->Copy(DestinationPath, false); }
			void						PushNewActiveLog();

			void						OpenSessionLog();
		};
#define CHANGELOG				ChangeLogManager::GetSingleton()

		void							HandlePluginSave(TESFile* SaveFile);
		void							HandlePluginLoadProlog();
		void							HandlePluginLoadEpilog();
	}
}