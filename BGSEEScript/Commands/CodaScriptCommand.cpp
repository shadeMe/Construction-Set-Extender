#pragma once
#include "CodaScriptCommand.h"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		namespace Commands
		{
			CodaScriptCommandCommonParamDef(OneForm, 1)
			{
				{ "Form",				ICodaScriptDataStore::kDataType_Reference	}
			};

			CodaScriptCommandCommonParamDef(OneNumber, 1)
			{
				{ "Value",				ICodaScriptDataStore::kDataType_Numeric		}
			};

			CodaScriptCommandCommonParamDef(OneString, 1)
			{
				{ "Value",				ICodaScriptDataStore::kDataType_String		}
			};

			CodaScriptCommandCommonParamDef(OneArray, 1)
			{
				{ "Value",				ICodaScriptDataStore::kDataType_Array		}
			};

			CodaScriptCommandCommonParamDef(FormNumber, 2)
			{
				{ "Form",				ICodaScriptDataStore::kDataType_Reference	},
				{ "Value",				ICodaScriptDataStore::kDataType_Numeric		}
			};

			CodaScriptCommandCommonParamDef(FormString, 2)
			{
				{ "Form",				ICodaScriptDataStore::kDataType_Reference	},
				{ "Value",				ICodaScriptDataStore::kDataType_String		}
			};

			CodaScriptCommandCommonParamDef(TwoForms, 2)
			{
				{ "Form",				ICodaScriptDataStore::kDataType_Reference	},
				{ "Value",				ICodaScriptDataStore::kDataType_Reference	}
			};
		}
	}
}