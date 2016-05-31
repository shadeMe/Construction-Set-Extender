#pragma once
#include "CodaScriptCommand.h"

namespace bgsee
{
	namespace script
	{
		namespace commands
		{
			namespace array
			{
				CodaScriptCommandRegistrarDecl;

				CodaScriptParametricCommandPrototype(ArrayCreate,
					"ArCreate",
					"Creates an empty array and reserves as many elements as the passed size. Note that new elements are to be appended, not inserted by index, as the command doesn't initialize the array.",
					0,
					1,
					OneNumber,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptCommandPrototype(ArrayInsert,
					"ArInsert",
					"Inserts an element into an array at a specific index. Pass -1 as index to append. Returns true if successful.",
					0,
					3,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptCommandPrototype(ArrayErase,
					"ArErase",
					"Removes the element at a specific index. Returns true if successful.",
					0,
					2,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(ArrayClear,
					"ArClear",
					"Removes all elements in an array.",
					0,
					1,
					OneArray,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(ArraySize,
					"ArSize",
					"Returns the size of an array.",
					0,
					1,
					OneArray,
					ICodaScriptDataStore::kDataType_Numeric);
			}
		}
	}
}