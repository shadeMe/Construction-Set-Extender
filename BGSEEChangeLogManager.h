#pragma once

namespace BGSEditorExtender
{
	class BGSEEChangeLogManager;

	class BGSEEChangeLog
	{
		friend class BGSEEChangeLogManager;

		FILE*							Log;
		std::string						FilePath;

		BGSEEChangeLog(const char* Path, const char* FileName);

		void							WriteChange(const char* Message, bool StampTime, bool Flush, void* ConsoleMessageContext = NULL);
		bool							Copy(const char* DestinationPath, bool Overwrite) const;
		void							Finalize();
	public:
		~BGSEEChangeLog();

		void							View() const;
	};

	class BGSEEChangeEntry
	{
	public:
		virtual ~BGSEEChangeEntry() = 0
		{
			;//
		}

		virtual const char*				Get() const = 0;		// returns the entry to be logged to the active change log
	};

	class BGSEEChangeLogManager
	{
		static BGSEEChangeLogManager*	Singleton;

		BGSEEChangeLogManager();

		std::stack<BGSEEChangeLog*>		LogStack;
		BGSEEChangeLog*					SessionLog;
		char							GUIDString[0x100];
		char							TempPath[MAX_PATH];
		void*							ConsoleMessageContext;
		bool							Initialized;

		const char*						GetTempDirectory(char* OutBuffer, UInt32 BufferSize);
		void							WriteToLogs(const char* Message, bool StampTime);
	public:
		~BGSEEChangeLogManager();

		static BGSEEChangeLogManager*	GetSingleton();

		bool							Initialize();

		void							RecordChange(const char* Format, ...);
		void							RecordChange(const BGSEEChangeEntry& Entry);

		void							Pad(UInt32 Size);

		bool							CopyActiveLog(const char* DestinationPath);
		void							PushNewActiveLog();
	};

#define BGSEECHANGELOG				BGSEditorExtender::BGSEEChangeLogManager::GetSingleton()
}