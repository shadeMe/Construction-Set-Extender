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

		virtual bool						GetIsFormTypeReplicable(UInt8 Type) = 0;
		virtual void						LoadForm(BGSEEPluginFileWrapper* File) = 0;
		virtual void						SaveForm(BGSEEPluginFileWrapper* File, BGSEEFormWrapper* Form) = 0;
		virtual void						DisplayClipboardContents(BGSEEPluginFileWrapper* File) = 0;

		virtual void						PreSaveCallback(BGSEEFormListT& SaveForms, BGSEEPluginFileWrapper* File) = 0;
		virtual void						PostSaveCallback(void) = 0;
		virtual void						PreLoadCallback(void) = 0;
		virtual void						PostLoadCallback(void) = 0;
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