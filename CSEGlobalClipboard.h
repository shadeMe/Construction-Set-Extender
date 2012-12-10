#pragma once
#include <BGSEEGlobalClipboard.h>

namespace ConstructionSetExtender
{
	namespace GlobalClipboard
	{
		class CSEGlobalClipboardOperator;

		class CSEFormWrapper : public BGSEditorExtender::BGSEEFormWrapper
		{
		protected:
			TESForm*							WrappedForm;

			friend class CSEGlobalClipboardOperator;
		public:
			CSEFormWrapper(TESForm* Form);
			virtual ~CSEFormWrapper();

			virtual UInt32						GetFormID(void) const;
			virtual const char*					GetEditorID(void) const;
			virtual UInt8						GetType(void) const;
			virtual const char*					GetTypeString(void) const;
			virtual UInt32						GetFlags(void) const;

			virtual bool						GetIsDeleted(void) const;
		};

		class CSEPluginFileWrapper : public BGSEditorExtender::BGSEEPluginFileWrapper
		{
		protected:
			TESFile*							WrappedPlugin;
			std::string							PluginPath;

			friend class CSEGlobalClipboardOperator;

			void								CreateTempFile(void);
		public:
			CSEPluginFileWrapper();
			virtual ~CSEPluginFileWrapper();

			virtual bool						Construct(const char* FileName);
			virtual void						Purge(void);

			virtual bool						Open(bool ForWriting);  
			virtual bool						SaveHeader(void);
			virtual bool						CorrectHeader(UInt32 RecordCount);
			virtual bool						Close(void);

			virtual UInt8						GetRecordType(void);
			virtual bool						GetNextRecord(bool SkipIgnoredRecords);

			virtual int							GetErrorState(void) const;
		};

		class CSEGlobalClipboardOperator : public BGSEditorExtender::BGSEEGlobalClipboardOperator
		{
		protected:
			typedef std::list<TESForm*>			FormListT;

			FormListT							LoadedFormBuffer;		// stores loaded forms for deferred linking

			void								FreeBuffer(void);
		public:
			CSEGlobalClipboardOperator();
			virtual ~CSEGlobalClipboardOperator();

			virtual bool						GetIsFormTypeReplicable(UInt8 Type);
			virtual void						LoadForm(BGSEditorExtender::BGSEEPluginFileWrapper* File);
			virtual void						SaveForm(BGSEditorExtender::BGSEEPluginFileWrapper* File, BGSEditorExtender::BGSEEFormWrapper* Form);
			virtual void						DisplayClipboardContents(BGSEditorExtender::BGSEEPluginFileWrapper* File);
			
			virtual bool						PreSaveCallback(BGSEditorExtender::BGSEEFormListT& SaveForms, BGSEditorExtender::BGSEEPluginFileWrapper* File);
			virtual bool						PostSaveCallback(void);
			virtual bool						PreLoadCallback(void);
			virtual bool						PostLoadCallback(void);	
		};

		class CSEFormListBuilder
		{
			BGSEditorExtender::BGSEEFormListT		FormList;
		public:
			CSEFormListBuilder();
			~CSEFormListBuilder();

			void									Add(TESForm* Form);
			bool									Copy(void);
		};

		void Initialize(void);
	}
}