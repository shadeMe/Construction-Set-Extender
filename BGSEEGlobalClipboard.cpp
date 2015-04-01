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
				if (Operator->GetIsFormTypeReplicable((*Itr)->GetType()) == false)
				{
					BGSEECONSOLE_MESSAGE("Form type error! Type '%s' is not replicable", (*Itr)->GetTypeString());
					FormCheck = false;
					break;
				}
			}

			if (FormCheck)
			{
				BGSEEFormCollectionSerializer* Serializer = Operator->GetSerializer(Forms);
				if (Serializer)
				{
					Operator->PreCopyCallback(Forms, Buffer);
					Result = Serializer->Serialize(Forms, Buffer);
					Operator->PostCopyCallback(Result);
				}

				if (Result)
					BGSEECONSOLE_MESSAGE("Copied %d forms", Forms.size());
			}

			if (Result == false)
				BGSEEUI->MsgBoxE(NULL, MB_TASKMODAL|MB_SETFOREGROUND, "Global copy operation failed! Check the console for more information.");

			for (BGSEEFormListT::iterator Itr = Forms.begin(); Itr != Forms.end(); Itr++)
				delete *Itr;

			Forms.clear();
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

		BGSEEFormCollectionSerializer* Deserializer = Operator->GetDeserializer(Buffer);
		if (Deserializer == NULL)
			BGSEECONSOLE_MESSAGE("Unrecognized clipboard buffer serializer/deserializer!");
		else
		{
			int Count = 0;
			Operator->PrePasteCallback(Buffer);
			Result = Deserializer->Deserialize(Buffer, Count);
			Operator->PostPasteCallback(Result, Deserializer);

			if (Result)
				BGSEECONSOLE_MESSAGE("Pasted %d forms", Count);
		}

		if (Result && ClearIfSuccessful)
			Buffer->Purge();

		if (Result == false)
			BGSEEUI->MsgBoxE(NULL, MB_TASKMODAL|MB_SETFOREGROUND, "Global paste operation failed! Check the console for more information.");

		BGSEECONSOLE->Exdent();

		return Result;
	}

	void BGSEEGlobalClipboard::DisplayContents( void )
	{
		SME_ASSERT(Initialized);

		Operator->DisplayClipboardContents(Buffer);
	}
}