#pragma once
#include <BGSEEGlobalClipboard.h>
#include "CSEWrappers.h"

namespace ConstructionSetExtender
{
	namespace GlobalClipboard
	{
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
			
			virtual void						PreSaveCallback(BGSEditorExtender::BGSEEFormListT& SaveForms, BGSEditorExtender::BGSEEPluginFileWrapper* File);
			virtual void						PostSaveCallback(void);
			virtual void						PreLoadCallback(void);
			virtual void						PostLoadCallback(void);	
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