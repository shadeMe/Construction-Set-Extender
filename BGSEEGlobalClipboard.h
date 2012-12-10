#pragma once

#include "BGSEEWorkspaceManager.h"

namespace BGSEditorExtender
{
	// wraps around the base form class in BGS games (TESForm mostly)
	class BGSEEFormWrapper
	{
	public:
		virtual ~BGSEEFormWrapper() = 0
		{
			;//
		}

		virtual UInt32						GetFormID(void) const = 0;
		virtual const char*					GetEditorID(void) const = 0;
		virtual UInt8						GetType(void) const = 0;
		virtual const char*					GetTypeString(void) const = 0;
		virtual UInt32						GetFlags(void) const = 0;
		
		virtual bool						GetIsDeleted(void) const = 0;
	};

	typedef std::list<BGSEEFormWrapper*>	BGSEEFormListT;

	// wraps around the BGS plugin file class (TESFile)
	class BGSEEPluginFileWrapper
	{
	public:
		virtual ~BGSEEPluginFileWrapper() = 0
		{
			;//
		}

		virtual bool						Construct(const char* FileName) = 0;			// creates and initializes an instance of the wrapped class
		virtual void						Purge(void) = 0;								// deletes the file

		virtual bool						Open(bool ForWriting) = 0;						// these return false on error
		virtual bool						SaveHeader(void) = 0;
		virtual bool						CorrectHeader(UInt32 RecordCount) = 0;
		virtual bool						Close(void) = 0;

		virtual UInt8						GetRecordType(void) = 0;
		virtual bool						GetNextRecord(bool SkipIgnoredRecords) = 0;

		virtual int							GetErrorState(void) const = 0;
	};

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

		virtual bool						PreSaveCallback(BGSEEFormListT& SaveForms, BGSEEPluginFileWrapper* File) = 0;
		virtual bool						PostSaveCallback(void) = 0;
		virtual bool						PreLoadCallback(void) = 0;
		virtual bool						PostLoadCallback(void) = 0;
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
		virtual ~BGSEEGlobalClipboard();

		static BGSEEGlobalClipboard*		GetSingleton();

		bool								Initialize(BGSEEGlobalClipboardOperator* Operator,
													BGSEEPluginFileWrapper* PluginWrapper);			// takes ownership of the pointers

		bool								Copy(BGSEEFormListT& Forms);							// takes ownership of the pointers
		bool								Paste(bool ClearIfSuccessful = false);
		void								DisplayContents(void);
	};

#define BGSEECLIPBOARD						BGSEditorExtender::BGSEEGlobalClipboard::GetSingleton()
}