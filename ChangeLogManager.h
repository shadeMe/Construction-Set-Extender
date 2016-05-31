#pragma once

#include <bgsee\ChangeLogManager.h>

namespace cse
{
	namespace changeLogManager
	{
		class ChangeEntry : public bgsee::ChangeEntry
		{
		protected:
			std::string						Buffer;
		public:
			ChangeEntry();
			virtual ~ChangeEntry();

			virtual const char*				Get() const;
		};

		class BasicFormChangeEntry : public ChangeEntry
		{
		public:
			enum
			{
				kFormChange_SetActive = 0,
				kFormChange_SetDeleted,
				kFormChange_SetFormID,
				kFormChange_SetEditorID
			};

			BasicFormChangeEntry(UInt8 ChangeType, TESForm* Form, UInt32 Value);
			virtual ~BasicFormChangeEntry();
		};
		
		void							HandlePluginSave(TESFile* SaveFile);
		void							HandlePluginLoadProlog();
		void							HandlePluginLoadEpilog();

		void Initialize(void);
	}
}