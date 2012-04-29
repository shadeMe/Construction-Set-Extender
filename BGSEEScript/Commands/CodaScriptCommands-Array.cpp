#include "CodaScriptCommands-Array.h"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		namespace Commands
		{
			namespace Array
			{
				CodaScriptCommandRegistrarDef("Array")

				CodaScriptCommandPrototypeDef(ArrayCreate);
				CodaScriptCommandPrototypeDef(ArrayInsert);
				CodaScriptCommandPrototypeDef(ArrayErase);
				CodaScriptCommandPrototypeDef(ArrayClear);
				CodaScriptCommandPrototypeDef(ArraySize);

				CodaScriptCommandParamData(ArrayInsert, 3)
				{
					{ "Array",								ICodaScriptDataStore::kDataType_Array	},
					{ "Element",							ParameterInfo::kType_Multi				},
					{ "Index",								ICodaScriptDataStore::kDataType_Numeric	}
				};

				CodaScriptCommandParamData(ArrayErase, 2)
				{
					{ "Array",								ICodaScriptDataStore::kDataType_Array	},
					{ "Index",								ICodaScriptDataStore::kDataType_Numeric	}
				};

				CodaScriptCommandHandler(ArrayCreate)
				{
					*Result = *Utilities->ArrayAllocate();
					return true;
				}

				CodaScriptCommandHandler(ArrayInsert)
				{
					ICodaScriptDataStore* Array = NULL;
					ICodaScriptDataStore* Element = NULL;
					CodaScriptNumericDataTypeT Index = -1;

					CodaScriptCommandExtractArgs(&Array, &Element, &Index);

					CodaScriptBackingStore* ArrayStore = dynamic_cast<CodaScriptBackingStore*>(Array);
					CodaScriptBackingStore* ElementStore = dynamic_cast<CodaScriptBackingStore*>(Element);
					SME_ASSERT(ArrayStore && ElementStore);

					Result->SetNumber(ArrayStore->GetArray()->Insert(ElementStore, Index));
					return true;
				}

				CodaScriptCommandHandler(ArrayErase)
				{
					ICodaScriptDataStore* Array = NULL;
					CodaScriptNumericDataTypeT Index = -1;

					CodaScriptCommandExtractArgs(&Array, &Index);

					CodaScriptBackingStore* ArrayStore = dynamic_cast<CodaScriptBackingStore*>(Array);
					SME_ASSERT(ArrayStore);

					Result->SetNumber(ArrayStore->GetArray()->Erase(Index));
					return true;
				}

				CodaScriptCommandHandler(ArrayClear)
				{
					ICodaScriptDataStore* Array = NULL;

					CodaScriptCommandExtractArgs(&Array);

					CodaScriptBackingStore* ArrayStore = dynamic_cast<CodaScriptBackingStore*>(Array);
					SME_ASSERT(ArrayStore);

					ArrayStore->GetArray()->Clear();
					return true;
				}

				CodaScriptCommandHandler(ArraySize)
				{
					ICodaScriptDataStore* Array = NULL;

					CodaScriptCommandExtractArgs(&Array);

					CodaScriptBackingStore* ArrayStore = dynamic_cast<CodaScriptBackingStore*>(Array);
					SME_ASSERT(ArrayStore);

					Result->SetNumber(ArrayStore->GetArray()->Size());
					return true;
				}
			}
		}
	}
}