#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "BGSEEChangeLogManager.h"

namespace BGSEditorExtender
{
	BGSEEChangeLogManager*		BGSEEChangeLogManager::Singleton = NULL;

	BGSEEChangeLog::BGSEEChangeLog(const char* Path, const char* FileName)
	{
		char Buffer[0x100] = {0};
		sprintf_s(Buffer, sizeof(Buffer), "%s-%08X", FileName, SME::MersenneTwister::genrand_int32());
		// add a random suffix to make sure files aren't overwritten due to lousy timer resolution

		FilePath = std::string(Path) + "\\" + std::string(Buffer) + ".log";
		Log = _fsopen(FilePath.c_str(), "w", _SH_DENYNO);

		if (Log == NULL)
		{
			BGSEECONSOLE_MESSAGE("Couldn't initialize change log '%s'", FilePath.c_str());
			BGSEECONSOLE_MESSAGE("Error Code = %d", errno);
		}

		ZeroMemory(Buffer, sizeof(Buffer));
	}

	BGSEEChangeLog::~BGSEEChangeLog()
	{
		if (Log)
		{
			Finalize();
			if (DeleteFile(FilePath.c_str()) == 0)
			{
				BGSEECONSOLE_ERROR("Couldn't delete change log '%s'", FilePath.c_str());
			}
		}
	}

	void BGSEEChangeLog::WriteChange(const char* Message, bool StampTime, bool Flush, void* ConsoleMessageContext)
	{
		char Buffer[0x400] = {0};

		if (Log)
		{
			std::stringstream Output;

			if (StampTime)
			{
				Output << SME::MiscGunk::GetTimeString(Buffer, sizeof(Buffer));
				Output << "\t";
			}

			Output << Message << "\n";

			if (ConsoleMessageContext)
				BGSEECONSOLE->PrintToMessageLogContext(ConsoleMessageContext, Output.str().c_str());

			fputs(Output.str().c_str(), Log);

			if (Flush)
				fflush(Log);
		}
	}

	bool BGSEEChangeLog::Copy(const char* DestinationPath, bool Overwrite) const
	{
		if (Log == NULL)
			return false;

		fflush(Log);

		if (CopyFile(FilePath.c_str(), DestinationPath, Overwrite))
		{
			return true;
		}
		else
		{
			BGSEECONSOLE_ERROR("Couldn't copy change log '%s' to '%s'", FilePath.c_str(), DestinationPath);
			return false;
		}
	}

	void BGSEEChangeLog::Finalize(void)
	{
		if (Log)
		{
			fflush(Log);
			fclose(Log);
		}
	}
	
	void BGSEEChangeLog::View() const
	{
		ShellExecute(NULL, "open", (LPSTR)FilePath.c_str(), NULL, NULL, SW_SHOW);
	}

	
	BGSEEChangeLogManager* BGSEEChangeLogManager::GetSingleton(void)
	{
		if (Singleton == NULL)
			Singleton = new BGSEEChangeLogManager();

		return Singleton;
	}

	BGSEEChangeLogManager::BGSEEChangeLogManager() :
		LogStack(),
		SessionLog(NULL),
		ConsoleMessageContext(NULL),
		Initialized(false)
	{
		ZeroMemory(GUIDString, sizeof(GUIDString));
		ZeroMemory(TempPath, sizeof(TempPath));
	}

	BGSEEChangeLogManager::~BGSEEChangeLogManager()
	{
		Pad(1);
		WriteToLogs("CS Session Ended", true);

		while (LogStack.size())
		{
			delete LogStack.top();
			LogStack.pop();
		}

		SAFEDELETE(SessionLog);

		char Buffer[MAX_PATH] = {0};

		if (RemoveDirectory(GetTempDirectory(Buffer, sizeof(Buffer))) == 0)
		{
			BGSEECONSOLE_ERROR("Couldn't remove temp change log files");
		}

		if (ConsoleMessageContext)
		{
			BGSEECONSOLE->UnregisterMessageLogContext(ConsoleMessageContext);
			ConsoleMessageContext = NULL;
		}

		Initialized = false;

		Singleton = NULL;
	}

	const char* BGSEEChangeLogManager::GetTempDirectory( char* OutBuffer, UInt32 BufferSize )
	{
		sprintf_s(OutBuffer, BufferSize, "%s\\%s\\", TempPath, GUIDString);
		return OutBuffer;
	}

	void BGSEEChangeLogManager::WriteToLogs(const char* Message, bool StampTime)
	{
		SME_ASSERT(Initialized);

		if (LogStack.size())
			LogStack.top()->WriteChange(Message, StampTime, true);

		SessionLog->WriteChange(Message, StampTime, true, ConsoleMessageContext);
	}

	bool BGSEEChangeLogManager::Initialize()
	{
		if (Initialized == true)
			return false;

		Initialized = true;

		GUID LogManagerGUID = {0};
		char* TempGUIDString = 0;

		RPC_STATUS GUIDReturn = UuidCreate(&LogManagerGUID),
					GUIDStrReturn = UuidToString(&LogManagerGUID, (RPC_CSTR*)&TempGUIDString);

		if ((GUIDReturn == RPC_S_OK || GUIDReturn == RPC_S_UUID_LOCAL_ONLY) && GUIDStrReturn == RPC_S_OK)
		{
	//		BGSEECONSOLE_MESSAGE("LogManager GUID = %s", TempGUIDString);
			sprintf_s(GUIDString, sizeof(GUIDString), "%s", TempGUIDString);
			RpcStringFree((RPC_CSTR*)&TempGUIDString);
		}
		else
		{
			BGSEECONSOLE_MESSAGE("Couldn't create LogManager GUID");
			Initialized = false;
		}

		if (Initialized)
		{
			if (!GetTempPath(sizeof(TempPath), TempPath))
			{
				BGSEECONSOLE_ERROR("Couldn't get temp path for current user session");
				Initialized = false;
			}
			else
			{
				BGSEECONSOLE_MESSAGE("User Session Temp Path = %s", TempPath);
				char Buffer[MAX_PATH] = {0};

				if (!CreateDirectory(GetTempDirectory(Buffer, sizeof(Buffer)), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
				{
					BGSEECONSOLE_ERROR("Couldn't create temp log directory");
					Initialized = false;
				}
			}
		}

		if (Initialized)
		{
			char Buffer[MAX_PATH] = {0};

			SME_ASSERT(SessionLog == NULL);
			SessionLog = new BGSEEChangeLog(GetTempDirectory(Buffer, sizeof(Buffer)), "CSE Change Log");
			this->PushNewActiveLog();

			WriteToLogs("CS Session Started", true);
			Pad(2);
		}

		ConsoleMessageContext = BGSEECONSOLE->RegisterMessageLogContext("Change Log", SessionLog->FilePath.c_str());
		if (ConsoleMessageContext == NULL)
		{
			BGSEECONSOLE_ERROR("Couldn't register console message context");
			Initialized = false;
		}

		return Initialized;
	}

	void BGSEEChangeLogManager::PushNewActiveLog()
	{
		if (Initialized == false)
			return;

		char Buffer[MAX_PATH] = {0}, TimeString[0x100] = {0};

		if (LogStack.size())
			LogStack.top()->Finalize();

		LogStack.push(new BGSEEChangeLog(GetTempDirectory(Buffer, sizeof(Buffer)),
								SME::MiscGunk::GetTimeString(TimeString, sizeof(TimeString))));
	}

	void BGSEEChangeLogManager::RecordChange(const char* Format, ...)
	{
		if (Initialized == false)
			return;

		char Buffer[0x4000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		WriteToLogs(Buffer, true);
	}

	void BGSEEChangeLogManager::RecordChange( const BGSEEChangeEntry& Entry )
	{
		WriteToLogs(Entry.Get(), true);
	}


	void BGSEEChangeLogManager::Pad(UInt32 Size)
	{
		if (Initialized == false)
			return;

		for (int i = 0; i < Size; i++)
			WriteToLogs("\n", false);
	}

	bool BGSEEChangeLogManager::CopyActiveLog( const char* DestinationPath )
	{
		if (Initialized == false)
			return false;

		return LogStack.top()->Copy(DestinationPath, false);
	}

}