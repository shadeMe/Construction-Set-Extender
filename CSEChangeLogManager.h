#pragma once

#include <BGSEEChangeLogManager.h>

namespace ConstructionSetExtender
{
	namespace ChangeLogManager
	{
		class CSEChangeEntry : public bgsee::BGSEEChangeEntry
		{
		protected:
			std::string						Buffer;
		public:
			CSEChangeEntry();
			virtual ~CSEChangeEntry();

			virtual const char*				Get() const;
		};

		class CSEBasicFormChangeEntry : public CSEChangeEntry
		{
		public:
			enum
			{
				kFormChange_SetActive = 0,
				kFormChange_SetDeleted,
				kFormChange_SetFormID,
				kFormChange_SetEditorID
			};

			CSEBasicFormChangeEntry(UInt8 ChangeType, TESForm* Form, UInt32 Value);
			virtual ~CSEBasicFormChangeEntry();
		};
		
		void							HandlePluginSave(TESFile* SaveFile);
		void							HandlePluginLoadProlog();
		void							HandlePluginLoadEpilog();

		void Initialize(void);
	}
}