#pragma once

#include "Wrappers.h"
#include "WorkspaceManager.h"

namespace bgsee
{
	class GlobalClipboardOperator
	{
	public:
		virtual ~GlobalClipboardOperator() = 0
		{
			;//
		}

		virtual bool								GetIsFormTypeReplicable(UInt8 Type) = 0;
		virtual void								DisplayClipboardContents(PluginFileWrapper* File) = 0;

		virtual FormCollectionSerializer*			GetSerializer(FormListT& Forms) = 0;
		virtual FormCollectionSerializer*			GetDeserializer(PluginFileWrapper* File) = 0;

		virtual void								PreCopyCallback(FormListT& CopyForms, PluginFileWrapper* File) = 0;		// before serialization begins
		virtual void								PostCopyCallback(bool Successful) = 0;	// after serialization is complete
		virtual void								PrePasteCallback(PluginFileWrapper* File) = 0;	// before deserialization begins
		virtual void								PostPasteCallback(bool Successful, FormCollectionSerializer* Deserializer) = 0;	// after deserialization is complete
	};

	class GlobalClipboard
	{
		static GlobalClipboard*			Singleton;

		GlobalClipboard(GlobalClipboardOperator* Operator, PluginFileWrapper* PluginWrapper);
		~GlobalClipboard();

		static const char*				kClipboardBufferPath;

		GlobalClipboardOperator*		Operator;
		PluginFileWrapper*				Buffer;

		bool							Initialized;
	public:

		static GlobalClipboard*				Get();
		static bool							Initialize(GlobalClipboardOperator* Operator, PluginFileWrapper* PluginWrapper);			// takes ownership of the pointers
		static void							Deinitialize();

		bool								Copy(FormListT& Forms);							// takes ownership of the pointers
		bool								Paste(bool ClearIfSuccessful = false);
		void								DisplayContents(void);
	};

#define BGSEECLIPBOARD						bgsee::GlobalClipboard::Get()
}