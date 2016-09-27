#pragma once

namespace bgsee
{
	class ChangeLogManager;

	class ChangeLog
	{
		friend class ChangeLogManager;

		FILE*							Log;
		std::string						FilePath;

		ChangeLog(const char* Path, const char* FileName);

		void							WriteChange(const char* Message, bool StampTime, bool Flush, void* ConsoleMessageContext = nullptr);
		bool							Copy(const char* DestinationPath, bool Overwrite) const;
		void							Finalize();
	public:
		~ChangeLog();

		void							View() const;
	};

	class ChangeEntry
	{
	public:
		virtual ~ChangeEntry() = 0
		{
			;//
		}

		virtual const char*				Get() const = 0;		// returns the entry to be logged to the active change log
	};

	class ChangeLogManager
	{
		static ChangeLogManager*		Singleton;

		ChangeLogManager();
		~ChangeLogManager();

		std::stack<ChangeLog*>			LogStack;
		ChangeLog*						SessionLog;
		char							GUIDString[0x100];
		char							TempPath[MAX_PATH];
		void*							ConsoleMessageContext;
		bool							Initialized;

		const char*						GetTempDirectory(char* OutBuffer, UInt32 BufferSize);
		void							WriteToLogs(const char* Message, bool StampTime);
	public:
		static ChangeLogManager*		Get();
		static bool						Initialize();
		static void						Deinitialize();

		void							RecordChange(const char* Format, ...);
		void							RecordChange(const ChangeEntry& Entry);

		void							Pad(UInt32 Size);

		bool							CopyActiveLog(const char* DestinationPath);
		void							PushNewActiveLog();
	};

#define BGSEECHANGELOG				bgsee::ChangeLogManager::Get()
}