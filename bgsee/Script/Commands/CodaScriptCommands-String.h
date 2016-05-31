#pragma once
#include "CodaScriptCommand.h"

namespace bgsee
{
	namespace script
	{
		namespace commands
		{
			namespace string
			{
				CodaScriptCommandRegistrarDecl;

				CodaScriptParametricCommandPrototype(StringLength,
					"StrLen",
					"Returns the number of characters in a string.",
					0,
					1,
					OneString,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptCommandPrototype(StringCompare,
					"StrCmp",
					"Returns 0 if the strings are equal, 1 if the first string occurs alphabetically before the second, -1 if the first string occurs alphabetically after the second.",
					0,
					3,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptCommandPrototype(StringErase,
					0,
					"Erases the specified number of characters from the string starting at the specified position.",
					0,
					3,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptCommandPrototype(StringFind,
					0,
					"Returns the index of the first occurrence of the substring within the source string, starting at the specified position.",
					0,
					4,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptCommandPrototype(StringInsert,
					0,
					"Inserts a literal into a string at the specified position.",
					0,
					3,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptCommandPrototype(StringSubStr,
					0,
					"Returns a substring of the source source string, starting at the specified position.",
					0,
					3,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(StringIsNumber,
					"StrIsNum",
					"Returns true if the passed string is a number.",
					0,
					1,
					OneString,
					ICodaScriptDataStore::kDataType_Numeric);
			}
		}
	}
}