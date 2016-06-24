#include "ChangeLogManager.h"
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

		DEFINE_BASIC_EVENT_SINK(ChangeLogTESForm);
		DEFINE_BASIC_EVENT_SINK_HANDLER(ChangeLogTESForm)
		{
			events::form::TESFormEventData* Args = dynamic_cast<events::form::TESFormEventData*>(Data);
			SME_ASSERT(Args);

			TESForm* Form = Args->Form;
			UInt32 Value = 0;
			UInt8 ChangeType = BasicFormChangeEntry::kFormChange_SetActive;

			switch (Args->EventType)
			{
			case events::form::TESFormEventData::kType_Instantiation:
				BGSEECHANGELOG->RecordChange("%s\t[%08X]\t%s\tInstantiated", Form->GetTypeIDString(), Form->formID, Form->editorID.c_str());
				break;
			case events::form::TESFormEventData::kType_SetActive:
			case events::form::TESFormEventData::kType_SetDeleted:
			case events::form::TESFormEventData::kType_SetFormID:
			case events::form::TESFormEventData::kType_SetEditorID:
				if (Form->IsTemporary() == false && TESDataHandler::PluginLoadSaveInProgress == false)
				{
					switch (Args->EventType)
					{
					case events::form::TESFormEventData::kType_SetActive:
						ChangeType = BasicFormChangeEntry::kFormChange_SetActive;
						Value = Args->ActiveState;
						break;
					case events::form::TESFormEventData::kType_SetDeleted:
						ChangeType = BasicFormChangeEntry::kFormChange_SetDeleted;
						Value = Args->DeletedState;
						break;
					case events::form::TESFormEventData::kType_SetFormID:
						ChangeType = BasicFormChangeEntry::kFormChange_SetFormID;
						Value = Args->NewFormID;
						break;
					case events::form::TESFormEventData::kType_SetEditorID:
						ChangeType = BasicFormChangeEntry::kFormChange_SetEditorID;
						Value = (UInt32)Args->NewEditorID;
						break;
					}

					BGSEECHANGELOG->RecordChange(changeLogManager::BasicFormChangeEntry(ChangeType, Form, Value));
				}

				break;
			}
		}

		DEFINE_BASIC_EVENT_SINK(ChangeLogPreSave);
		DEFINE_BASIC_EVENT_SINK_HANDLER(ChangeLogPreSave)
		{
			events::plugin::TESFileEventData* Args = dynamic_cast<events::plugin::TESFileEventData*>(Data);
			SME_ASSERT(Args);

			TESFile* SaveFile = Args->File;
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

		DEFINE_BASIC_EVENT_SINK(ChangeLogPreLoad);
		DEFINE_BASIC_EVENT_SINK_HANDLER(ChangeLogPreLoad)
		{
			BGSEECHANGELOG->Pad(1);
			BGSEECHANGELOG->RecordChange("Reloading plugins...");
			BGSEECHANGELOG->Pad(1);
		}

		DEFINE_BASIC_EVENT_SINK(ChangeLogPostLoad);
		DEFINE_BASIC_EVENT_SINK_HANDLER(ChangeLogPostLoad)
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

			ADD_BASIC_SINK_TO_SOURCE(ChangeLogTESForm, events::form::kInstantiation);
			ADD_BASIC_SINK_TO_SOURCE(ChangeLogTESForm, events::form::kSetActive);
			ADD_BASIC_SINK_TO_SOURCE(ChangeLogTESForm, events::form::kSetDeleted);
			ADD_BASIC_SINK_TO_SOURCE(ChangeLogTESForm, events::form::kSetEditorID);
			ADD_BASIC_SINK_TO_SOURCE(ChangeLogTESForm, events::form::kSetFormID);

			ADD_BASIC_SINK_TO_SOURCE(ChangeLogPreSave, events::plugin::kPreSave);
			ADD_BASIC_SINK_TO_SOURCE(ChangeLogPreLoad, events::plugin::kPreLoad);
			ADD_BASIC_SINK_TO_SOURCE(ChangeLogPostLoad, events::plugin::kPostLoad);

		}
	}
}