#include "ChangeLogManager.h"
#include "Hooks\VersionControl.h"

namespace VersionControl
{
	ChangeLogManager*		ChangeLogManager::Singleton = NULL;

	ChangeLog::ChangeLog(const char* Path, const char* FileName)
	{
		char Buffer[0x100] = {0};
		sprintf_s(Buffer, sizeof(Buffer), "%s-%08X", FileName, MersenneTwister::genrand_int32());		// add a random suffix to make sure files aren't overwritten due to lousy timer resolution

		FilePath = std::string(Path) + "\\" + std::string(Buffer) + ".log";
		Log = _fsopen(FilePath.c_str(), "w", _SH_DENYNO);

		if (!Log)
		{
			DebugPrint("Couldn't initialize change log '%s'", FilePath.c_str());
			DebugPrint("Error Code = %d", errno);
		}

		ZeroMemory(Buffer, sizeof(Buffer));
	}

	void ChangeLog::WriteChange(const char* Message, bool StampTime, bool Flush)
	{
		if (Log)
		{
			if (StampTime)
			{
				fputs(GetTimeString(Buffer, sizeof(Buffer)), Log);
				fputs("\t", Log);
			}

			fputs(Message, Log);
			fputs("\n", Log);

			if (Flush)
				fflush(Log);
		}
	}

	bool ChangeLog::Copy(const char* DestinationPath, bool Overwrite)
	{
		if (!Log)
			return false;

		fflush(Log);

		if (CopyFile(FilePath.c_str(), DestinationPath, Overwrite))
		{
			return true;
		}
		else
		{
			DebugPrint("Couldn't copy change log '%s' to '%s'", FilePath.c_str(), DestinationPath);
			LogWinAPIErrorMessage(GetLastError());
			return false;
		}
	}

	void ChangeLog::Finalize(void)
	{
		if (Log)
		{
			fflush(Log);
			fclose(Log);
		}
	}

	bool ChangeLog::Delete()
	{
		if (!Log)
			return false;

		Finalize();
		if (DeleteFile(FilePath.c_str()))
		{
	//		DebugPrint("Deleted change log '%s'", FilePath.c_str());
			return true;
		}
		else
		{
			DebugPrint("Couldn't delete change log '%s'", FilePath.c_str());
			LogWinAPIErrorMessage(GetLastError());
			return false;
		}
	}

	void ChangeLog::OpenForViewing()
	{
		ShellExecute(NULL, "open", (LPSTR)FilePath.c_str(), NULL, NULL, SW_SHOW);
	}

	ChangeLogManager*		ChangeLogManager::GetSingleton(void)
	{
		if (!Singleton)
			Singleton = new ChangeLogManager();
		return Singleton;
	}

	ChangeLogManager::ChangeLogManager()
	{
		Initialized = false;

		GUID LogManagerGUID;
		ZeroMemory(&LogManagerGUID, sizeof(GUID));
		ZeroMemory(GUIDString, sizeof(GUIDString));

		char* TempGUIDString = 0;

		RPC_STATUS GUIDReturn = UuidCreate(&LogManagerGUID),
			   GUIDStrReturn = UuidToString(&LogManagerGUID, (RPC_CSTR*)&TempGUIDString);

		if ((GUIDReturn == RPC_S_OK || GUIDReturn == RPC_S_UUID_LOCAL_ONLY) && GUIDStrReturn == RPC_S_OK)
		{
			DebugPrint("LogManager GUID = %s", TempGUIDString);
			sprintf_s(GUIDString, sizeof(GUIDString), "%s", TempGUIDString);
			RpcStringFree((RPC_CSTR*)&TempGUIDString);

			Initialized = true;
		}
		else
		{
			DebugPrint("Couldn't create LogManager GUID");
		}

		if (Initialized)
		{
			if (!GetTempPath(sizeof(TempPath), TempPath))
			{
				DebugPrint("Couldn't get temp path for current user session");
				LogWinAPIErrorMessage(GetLastError());
				Initialized = false;
			}
			else
			{
				DebugPrint("User Session Temp Path = %s", TempPath);
				char Buffer[MAX_PATH] = {0};

				if (!CreateDirectory(GetCurrentTempDirectory(Buffer, sizeof(Buffer)), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
				{
					DebugPrint("Couldn't create temp log directory");
					LogWinAPIErrorMessage(GetLastError());
					Initialized = false;
				}
			}
		}
	}

	const char* ChangeLogManager::GetCurrentTempDirectory( char* OutBuffer, UInt32 BufferSize )
	{
		sprintf_s(OutBuffer, BufferSize, "%s\\%s\\", TempPath, GUIDString);
		return OutBuffer;
	}

	void ChangeLogManager::WriteChangeToLogs(const char* Message, bool StampTime)
	{
		if (!Initialized)
			return;

		if (LogStack.size())
			LogStack.top()->WriteChange(Message, StampTime, true);

		SessionLog->WriteChange(Message, StampTime, true);
	}

	void ChangeLogManager::Initialize()
	{
		if (!Initialized)
			return;

		char Buffer[MAX_PATH] = {0};

		SessionLog = new ChangeLog(GetCurrentTempDirectory(Buffer, sizeof(Buffer)), "CSE Change Log");
		this->PushNewActiveLog();

		WriteChangeToLogs("CS Session Started", true);
		Pad(2);

		Hooks::PatchVersionControlHooks();		// version control hooks are patched here to prevent the premature init of the manager
	}

	void ChangeLogManager::Deinitialize()
	{
		if (!Initialized)
			return;

		Pad(1);
		WriteChangeToLogs("CS Session Ended", true);

		while (LogStack.size())
		{
			LogStack.top()->Delete();
			delete LogStack.top();
			LogStack.pop();
		}

		SessionLog->Delete();
		delete SessionLog;

		char Buffer[MAX_PATH] = {0};

		if (!RemoveDirectory(GetCurrentTempDirectory(Buffer, sizeof(Buffer))))
		{
			DebugPrint("Couldn't remove temp change log files");
			LogWinAPIErrorMessage(GetLastError());
		}
	}

	void ChangeLogManager::PushNewActiveLog()
	{
		if (!Initialized)
			return;

		char Buffer[MAX_PATH] = {0}, TimeString[0x100] = {0};

		if (LogStack.size())
			LogStack.top()->Finalize();

		LogStack.push(new ChangeLog(GetCurrentTempDirectory(Buffer, sizeof(Buffer)), GetTimeString(TimeString, sizeof(TimeString))));
	}

	void ChangeLogManager::RecordChange(const char* Format, ...)
	{
		if (!Initialized)
			return;

		char Buffer[0x4000] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		WriteChangeToLogs(Buffer, true);
	}

	void ChangeLogManager::RecordChange(TESForm* Base, TESForm* New)
	{
		if (!Initialized)
			return;

		assertR(Base->formType == New->formType);

		char Buffer[0x4000] = {0};

		// compare baseformcomponents first
		// follow that with the actual comparison of disjoint members

		WriteChangeToLogs(Buffer, true);
	}

	void ChangeLogManager::RecordFormChange(TESForm* Form, UInt8 ChangeType, UInt32 Value)
	{
		char Buffer[0x512] = {0};

		sprintf_s(Buffer, sizeof(Buffer), "%s\t[%08X]\t%s\t", Form->GetTypeIDString(), Form->formID, Form->editorID.c_str());
		std::string STLBuffer(Buffer);

		switch (ChangeType)
		{
		case kFormChange_SetActive:
			sprintf_s(Buffer, sizeof(Buffer), "Modified: %d", Value);
			break;
		case kFormChange_SetDeleted:
			sprintf_s(Buffer, sizeof(Buffer), "Deleted: %d", Value);
			break;
		case kFormChange_SetFormID:
			sprintf_s(Buffer, sizeof(Buffer), "FormID: %08X", Value);
			break;
		case kFormChange_SetEditorID:
			sprintf_s(Buffer, sizeof(Buffer), "EditorID: %s", (const char*)Value);
			break;
		}

		STLBuffer += Buffer;
		WriteChangeToLogs(STLBuffer.c_str(), true);
	}

	void ChangeLogManager::Pad(UInt32 Size)
	{
		for (int i = 0; i < Size; i++)
			WriteChangeToLogs("\n", false);
	}

	void ChangeLogManager::OpenSessionLog()
	{
		SessionLog->OpenForViewing();
	}

	const char* GetTimeString(char* Out, size_t Size)
	{
		__time32_t TimeData;
		tm LocalTime;

		_time32(&TimeData);
		_localtime32_s(&LocalTime, &TimeData);
		strftime(Out, Size, "%m--%d--%Y %H-%M-%S", &LocalTime);
		return Out;
	}

	void HandlePluginSave(TESFile* SaveFile)
	{
		if (!_stricmp(SaveFile->filePath, "Data\\Backup\\"))		// skip autosaves
			return;

		CHANGELOG->RecordChange("Active plugin %s saved", SaveFile->fileName);
		CHANGELOG->Pad(1);

		if (g_INIManager->GetINIInt("BackupOnSave", "Extender::VersionControl"))
		{
			char TimeString[0x100] = {0}, ExistingPath[MAX_PATH] = {0}, NewPath[MAX_PATH] = {0};
			GetTimeString(TimeString, sizeof(TimeString));

			std::string Name(SaveFile->fileName), Extension(Name.substr(Name.find_last_of(".") + 1, 3));
			Name = Name.substr(0, Name.find_last_of("."));

			sprintf_s(NewPath, sizeof(NewPath), "Data\\Backup\\%s - [%s].%s", Name.c_str(), TimeString, Extension.c_str());
			sprintf_s(ExistingPath, sizeof(ExistingPath), "%s%s", SaveFile->filePath, SaveFile->fileName);

			if (CopyFile(ExistingPath, NewPath, FALSE))
				DebugPrint("Saved active file backup to '%s'", NewPath);
			else
			{
				DebugPrint("Couldn't save active file backup to '%s'", NewPath);
				LogWinAPIErrorMessage(GetLastError());
			}

			char Buffer[0x200] = {0};
			FORMAT_STR(Buffer, "Data\\Backup\\%s - [%s].log", Name.c_str(), TimeString);
			CHANGELOG->CopyActiveLog(Buffer);
		}

		CHANGELOG->PushNewActiveLog();
	}

	void HandlePluginLoad(void)
	{
		if (_DATAHANDLER->activeFile)
			CHANGELOG->RecordChange("Plugins reloaded; Active = %s", _DATAHANDLER->activeFile->fileName);
		else
			CHANGELOG->RecordChange("Plugins reloaded; No Active file");

		CHANGELOG->Pad(1);
		CHANGELOG->PushNewActiveLog();
	}
}