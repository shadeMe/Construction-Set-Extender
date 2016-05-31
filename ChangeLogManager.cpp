#include "ChangeLogManager.h"
#include "Hooks\Hooks-VersionControl.h"
#include "Achievements.h"

namespace cse
{
	namespace changeLogManager
	{
		ChangeEntry::ChangeEntry() :
			bgsee::ChangeEntry(),
			Buffer()
		{
			;//
		}

		ChangeEntry::~ChangeEntry()
		{
			;//
		}

		const char* ChangeEntry::Get() const
		{
			return Buffer.c_str();
		}

		BasicFormChangeEntry::BasicFormChangeEntry( UInt8 ChangeType, TESForm* Form, UInt32 Value ) :
			ChangeEntry()
		{
			SME_ASSERT(Form);

			char FormatBuffer[0x512] = {0};

			FORMAT_STR(FormatBuffer, "%s\t[%08X]\t%s\t", Form->GetTypeIDString(), Form->formID, Form->editorID.c_str());
			Buffer = FormatBuffer;

			switch (ChangeType)
			{
			case kFormChange_SetActive:
				FORMAT_STR(FormatBuffer, "Modified: %d", Value);
				break;
			case kFormChange_SetDeleted:
				FORMAT_STR(FormatBuffer, "Deleted: %d", Value);
				break;
			case kFormChange_SetFormID:
				FORMAT_STR(FormatBuffer, "FormID: %08X", Value);
				break;
			case kFormChange_SetEditorID:
				FORMAT_STR(FormatBuffer, "EditorID: %s", (const char*)Value);
				break;
			}

			Buffer += FormatBuffer;
		}

		BasicFormChangeEntry::~BasicFormChangeEntry()
		{
			;//
		}

		void HandlePluginSave(TESFile* SaveFile)
		{
			if (!_stricmp(SaveFile->filePath, "Data\\Backup\\"))		// skip autosaves
				return;

			BGSEECHANGELOG->RecordChange("Active plugin %s saved", SaveFile->fileName);
			BGSEECHANGELOG->Pad(1);

			if (settings::versionControl::kBackupOnSave.GetData().i)
			{
				char TimeString[0x100] = {0}, ExistingPath[MAX_PATH] = {0}, NewPath[MAX_PATH] = {0};
				SME::MiscGunk::GetTimeString(TimeString, sizeof(TimeString), "%m--%d--%Y %H-%M-%S");

				std::string Name(SaveFile->fileName), Extension(Name.substr(Name.rfind(".") + 1, 3));
				Name = Name.substr(0, Name.rfind("."));

				sprintf_s(NewPath, sizeof(NewPath), "Data\\Backup\\%s", Name.c_str());
				if (CreateDirectory(NewPath, NULL) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS)
				{
					BGSEECONSOLE_ERROR("Couldn't create backup directory '%s'", NewPath);
				}
				else
				{
					sprintf_s(NewPath, sizeof(NewPath), "Data\\Backup\\%s\\%s - [%s].%s", Name.c_str(), Name.c_str(), TimeString, Extension.c_str());
					sprintf_s(ExistingPath, sizeof(ExistingPath), "%s%s", SaveFile->filePath, SaveFile->fileName);

					if (CopyFile(ExistingPath, NewPath, FALSE))
						BGSEECONSOLE_MESSAGE("Saved active file backup to '%s'", NewPath);
					else
					{
						BGSEECONSOLE_ERROR("Couldn't save active file backup to '%s'", NewPath);
					}

					char Buffer[0x200] = {0};
					FORMAT_STR(Buffer, "Data\\Backup\\%s\\%s - [%s].log", Name.c_str(), Name.c_str(), TimeString);
					BGSEECHANGELOG->CopyActiveLog(Buffer);
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_LiveChangeLog);
				}
			}

			BGSEECHANGELOG->PushNewActiveLog();
		}

		void HandlePluginLoadProlog()
		{
			BGSEECHANGELOG->Pad(1);
			BGSEECHANGELOG->RecordChange("Reloading plugins...");
			BGSEECHANGELOG->Pad(1);
		}

		void HandlePluginLoadEpilog(void)
		{
			BGSEECHANGELOG->Pad(1);

			if (_DATAHANDLER->activeFile)
				BGSEECHANGELOG->RecordChange("Plugins reloaded; Active = %s", _DATAHANDLER->activeFile->fileName);
			else
				BGSEECHANGELOG->RecordChange("Plugins reloaded; No Active file");

			BGSEECHANGELOG->Pad(1);
			BGSEECHANGELOG->PushNewActiveLog();
		}

		void Initialize( void )
		{
			bool ComponentInitialized = BGSEECHANGELOG->Initialize();
			SME_ASSERT(ComponentInitialized);

			hooks::PatchVersionControlHooks();		// version control hooks are patched here to prevent the premature init of the manager
		}
	}
}