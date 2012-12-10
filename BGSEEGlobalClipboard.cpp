#include "BGSEEMain.h"
#include "BGSEEConsole.h"
#include "BGSEEGlobalClipboard.h"

namespace BGSEditorExtender
{
	BGSEEGlobalClipboard*		BGSEEGlobalClipboard::Singleton = NULL;
	const char*					BGSEEGlobalClipboard::kClipboardBufferPath = "bgseegc";

	
	BGSEEGlobalClipboard::BGSEEGlobalClipboard() :
		Operator(NULL),
		Buffer(NULL),
		Initialized(false)
	{
		;//
	}

	BGSEEGlobalClipboard::~BGSEEGlobalClipboard()
	{
		SAFEDELETE(Operator);
		SAFEDELETE(Buffer);

		Initialized = false;

		Singleton = NULL;
	}

	BGSEEGlobalClipboard* BGSEEGlobalClipboard::GetSingleton()
	{
		if (Singleton == NULL)
			Singleton = new BGSEEGlobalClipboard();

		return Singleton;
	}

	bool BGSEEGlobalClipboard::Initialize( BGSEEGlobalClipboardOperator* Operator, BGSEEPluginFileWrapper* PluginWrapper )
	{
		if (Initialized)
			return false;

		SME_ASSERT(Operator && PluginWrapper);

		this->Operator = Operator;
		this->Buffer = PluginWrapper;

		Initialized = Buffer->Construct(kClipboardBufferPath);

		return Initialized;
	}

	bool BGSEEGlobalClipboard::Copy( BGSEEFormListT& Forms )
	{
		SME_ASSERT(Initialized);

		bool Result = false;

		BGSEECONSOLE_MESSAGE("Copying forms to global clipboard...");
		BGSEECONSOLE->Indent();

		if (Forms.size())
		{
			bool FormCheck = true;
			for (BGSEEFormListT::iterator Itr = Forms.begin(); Itr != Forms.end(); Itr++)
			{
				if ((*Itr)->GetEditorID() && strlen((*Itr)->GetEditorID()) < 1)
				{
					BGSEECONSOLE_MESSAGE("Form editorID error! Invalid editorID");
					FormCheck = false;
					break;
				}
				else if (Operator->GetIsFormTypeReplicable((*Itr)->GetType()) == false || 
					(*Itr)->GetType() != (*Forms.begin())->GetType())
				{
					BGSEECONSOLE_MESSAGE("Form type error! Possible reasons: Selection contains multiple types, or type '%s' is not replicable", (*Itr)->GetTypeString());
					FormCheck = false;
					break;
				}
				else if ((*Itr)->GetIsDeleted())
				{
					BGSEECONSOLE_MESSAGE("Attempting to copy deleted form %08X!", (*Itr)->GetFormID());
					FormCheck = false;
					break;
				}
			}

			if (FormCheck)
			{
				bool Failed = false;

				while (true)
				{
					Buffer->Purge();
					
					if (Buffer->Open(true) == false)
					{
						BGSEECONSOLE_MESSAGE("Failed to open buffer!");
						Failed = true;
						break;
					}

					if (Buffer->SaveHeader() == false)
					{
						BGSEECONSOLE_MESSAGE("Failed to save header!");
						Failed = true;
						break;
					}

					Operator->PreSaveCallback(Forms, Buffer);

					// we can skip the form sorting as they are all of a single type
					for (BGSEEFormListT::iterator Itr = Forms.begin(); Itr != Forms.end(); Itr++)
						Operator->SaveForm(Buffer, *Itr);

					Operator->PostSaveCallback();

					if (Buffer->CorrectHeader(Forms.size()) == false)
					{
						BGSEECONSOLE_MESSAGE("Failed to correct header!");
						Failed = true;
						break;
					}

					if (Buffer->Close() == false)
					{
						BGSEECONSOLE_MESSAGE("Failed to close buffer!");
						Failed = true;
						break;
					}

					Result = true;
					break;
				}

				if (Failed)
					BGSEECONSOLE_MESSAGE("Buffer Error state = %d", Buffer->GetErrorState());

				BGSEECONSOLE_MESSAGE("Copied %d forms", Forms.size());

				for (BGSEEFormListT::iterator Itr = Forms.begin(); Itr != Forms.end(); Itr++)
					delete *Itr;

				Forms.clear();
			}

			if (Result == false)
			{
				BGSEEUI->MsgBoxE(NULL, MB_TASKMODAL|MB_SETFOREGROUND, "Global copy operation failed! Check the console for more information.");
			}
		}

		BGSEECONSOLE->Exdent();

		return Result;
	}

	bool BGSEEGlobalClipboard::Paste( bool ClearIfSuccessful /*= false*/ )
	{
		SME_ASSERT(Initialized);

		bool Result = false;

		BGSEECONSOLE_MESSAGE("Pasting forms from global clipboard...");
		BGSEECONSOLE->Indent();

		while (true)
		{
			if (Buffer->Open(false) == false)
			{
				BGSEECONSOLE_MESSAGE("Failed to open buffer!");
				break;
			}

			Operator->PreLoadCallback();

			do
			{
				Operator->LoadForm(Buffer);
			} 
			while (Buffer->GetNextRecord(true));
			
			Operator->PostLoadCallback();

			if (Buffer->Close() == false)
			{
				BGSEECONSOLE_MESSAGE("Failed to close buffer!");
				break;
			}

			Result = true;

			if (ClearIfSuccessful)
				Buffer->Purge();

			break;
		}

		if (Result == false)
		{
			BGSEECONSOLE_MESSAGE("Buffer Error state = %d", Buffer->GetErrorState());
			BGSEEUI->MsgBoxE(NULL, MB_TASKMODAL|MB_SETFOREGROUND, "Global paste operation failed! Check the console for more information.");
		}

		BGSEECONSOLE->Exdent();

		return Result;
	}

	void BGSEEGlobalClipboard::DisplayContents( void )
	{
		SME_ASSERT(Initialized);

		Operator->DisplayClipboardContents(Buffer);
	}
}