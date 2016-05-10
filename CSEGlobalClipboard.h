#pragma once
#include <BGSEEGlobalClipboard.h>
#include "CSEWrappers.h"

namespace ConstructionSetExtender
{
	namespace GlobalClipboard
	{
		class CSEGlobalCopyBuffer
		{
			bgsee::BGSEEFormListT		FormList;
		public:
			CSEGlobalCopyBuffer();
			~CSEGlobalCopyBuffer();

			void									Add(TESForm* Form);
			bool									Copy(void);
		};

		class CSEGlobalClipboardOperator : public bgsee::BGSEEGlobalClipboardOperator
		{
		protected:
			CSEDefaultFormCollectionSerializer*			DefaultFormSerializer;
			CSEObjectRefCollectionSerializer*			ObjectRefSerializer;
		public:
			CSEGlobalClipboardOperator();
			virtual ~CSEGlobalClipboardOperator();

			virtual bool								GetIsFormTypeReplicable(UInt8 Type);
			virtual void								DisplayClipboardContents(bgsee::BGSEEPluginFileWrapper* File);

			virtual bgsee::BGSEEFormCollectionSerializer*		GetSerializer(bgsee::BGSEEFormListT& Forms);
			virtual bgsee::BGSEEFormCollectionSerializer*		GetDeserializer(bgsee::BGSEEPluginFileWrapper* File);

			virtual void								PreCopyCallback(bgsee::BGSEEFormListT& CopyForms,
																		bgsee::BGSEEPluginFileWrapper* File);
			virtual void								PostCopyCallback(bool Successful);
			virtual void								PrePasteCallback(bgsee::BGSEEPluginFileWrapper* File);
			virtual void								PostPasteCallback(bool Successful, bgsee::BGSEEFormCollectionSerializer* Deserializer);
		};

		void Initialize(void);
	}
}