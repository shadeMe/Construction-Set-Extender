#pragma once
#include <BGSEEGlobalClipboard.h>
#include "CSEWrappers.h"

namespace ConstructionSetExtender
{
	namespace GlobalClipboard
	{
		class CSEGlobalCopyBuffer
		{
			BGSEditorExtender::BGSEEFormListT		FormList;
		public:
			CSEGlobalCopyBuffer();
			~CSEGlobalCopyBuffer();

			void									Add(TESForm* Form);
			bool									Copy(void);
		};

		class CSEGlobalClipboardOperator : public BGSEditorExtender::BGSEEGlobalClipboardOperator
		{
		protected:
			CSEDefaultFormCollectionSerializer*			DefaultFormSerializer;
			CSEObjectRefCollectionSerializer*			ObjectRefSerializer;
		public:
			CSEGlobalClipboardOperator();
			virtual ~CSEGlobalClipboardOperator();

			virtual bool								GetIsFormTypeReplicable(UInt8 Type);
			virtual void								DisplayClipboardContents(BGSEditorExtender::BGSEEPluginFileWrapper* File);

			virtual BGSEditorExtender::BGSEEFormCollectionSerializer*		GetSerializer(BGSEditorExtender::BGSEEFormListT& Forms);
			virtual BGSEditorExtender::BGSEEFormCollectionSerializer*		GetDeserializer(BGSEditorExtender::BGSEEPluginFileWrapper* File);

			virtual void								PreCopyCallback(BGSEditorExtender::BGSEEFormListT& CopyForms,
																		BGSEditorExtender::BGSEEPluginFileWrapper* File);
			virtual void								PostCopyCallback(bool Successful);
			virtual void								PrePasteCallback(BGSEditorExtender::BGSEEPluginFileWrapper* File);
			virtual void								PostPasteCallback(bool Successful, BGSEditorExtender::BGSEEFormCollectionSerializer* Deserializer);
		};

		void Initialize(void);
	}
}