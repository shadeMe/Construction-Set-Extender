#pragma once

#include "BGSEEWrappers.h"
#include "BGSEEWorkspaceManager.h"

namespace BGSEditorExtender
{
	class BGSEEGlobalClipboardOperator
	{
	public:
		virtual ~BGSEEGlobalClipboardOperator() = 0
		{
			;//
		}

		virtual bool								GetIsFormTypeReplicable(UInt8 Type) = 0;
		virtual void								DisplayClipboardContents(BGSEEPluginFileWrapper* File) = 0;

		virtual BGSEEFormCollectionSerializer*		GetSerializer(BGSEEFormListT& Forms) = 0;
		virtual BGSEEFormCollectionSerializer*		GetDeserializer(BGSEEPluginFileWrapper* File) = 0;

		virtual void								PreCopyCallback(BGSEEFormListT& CopyForms, BGSEEPluginFileWrapper* File) = 0;		// before serialization begins
		virtual void								PostCopyCallback(bool Successful) = 0;	// after serialization is complete
		virtual void								PrePasteCallback(BGSEEPluginFileWrapper* File) = 0;	// before deserialization begins
		virtual void								PostPasteCallback(bool Successful, BGSEEFormCollectionSerializer* Deserializer) = 0;	// after deserialization is complete
	};

	class BGSEEGlobalClipboard
	{
		static BGSEEGlobalClipboard*		Singleton;

		BGSEEGlobalClipboard();

		static const char*					kClipboardBufferPath;

		BGSEEGlobalClipboardOperator*		Operator;
		BGSEEPluginFileWrapper*				Buffer;

		bool								Initialized;
	public:
		~BGSEEGlobalClipboard();

		static BGSEEGlobalClipboard*		GetSingleton();

		bool								Initialize(BGSEEGlobalClipboardOperator* Operator,
													BGSEEPluginFileWrapper* PluginWrapper);			// takes ownership of the pointers

		bool								Copy(BGSEEFormListT& Forms);							// takes ownership of the pointers
		bool								Paste(bool ClearIfSuccessful = false);
		void								DisplayContents(void);
	};

#define BGSEECLIPBOARD						BGSEditorExtender::BGSEEGlobalClipboard::GetSingleton()
}