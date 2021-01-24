#pragma once
#include <bgsee\GlobalClipboard.h>
#include "Wrappers.h"

namespace cse
{
	namespace globalClipboard
	{
		class GlobalCopyBuffer
		{
			bgsee::FormListT		FormList;
		public:
			GlobalCopyBuffer();
			~GlobalCopyBuffer();

			void									Add(TESForm* Form);
			bool									Copy(void);
		};

		class GlobalClipboardOperator : public bgsee::GlobalClipboardOperator
		{
		protected:
			DefaultFormCollectionSerializer*			DefaultFormSerializer;
			ObjectRefCollectionSerializer*				ObjectRefSerializer;
		public:
			GlobalClipboardOperator();
			virtual ~GlobalClipboardOperator();

			virtual bool								GetIsFormTypeReplicable(UInt8 Type);
			virtual void								DisplayClipboardContents(bgsee::PluginFileWrapper* File);

			virtual bgsee::FormCollectionSerializer*	GetSerializer(bgsee::FormListT& Forms);
			virtual bgsee::FormCollectionSerializer*	GetDeserializer(bgsee::PluginFileWrapper* File);

			virtual void								PreCopyCallback(bgsee::FormListT& CopyForms,
																		bgsee::PluginFileWrapper* File);
			virtual bool								PostCopyCallback(bool SerializationSuccessful);
			virtual void								PrePasteCallback(bgsee::PluginFileWrapper* File);
			virtual bool								PostPasteCallback(bool DeserializationSuccessful, bgsee::FormCollectionSerializer* Deserializer);
		};

		void Initialize();
		void Deinitialize();
	}
}