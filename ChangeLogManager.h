#pragma once

#include "[BGSEEBase]\ChangeLogManager.h"

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

		DECLARE_BASIC_EVENT_SINK(ChangeLogTESForm);
		DECLARE_BASIC_EVENT_SINK(ChangeLogPreSave);
		DECLARE_BASIC_EVENT_SINK(ChangeLogPreLoad);
		DECLARE_BASIC_EVENT_SINK(ChangeLogPostLoad);

		void Initialize(void);
		void Deinitialize();
	}
}