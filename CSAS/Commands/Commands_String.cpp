#include "Commands_String.h"
#include "..\ScriptCommands.h"
#include "..\ScriptRunner.h"

namespace CSAutomationScript
{
	void RegisterStringCommands()
	{
		REGISTER_CSASCOMMAND(StringLength, "String Functions");
		REGISTER_CSASCOMMAND(StringCompare, "String Functions");
		REGISTER_CSASCOMMAND(StringErase, "String Functions");
		REGISTER_CSASCOMMAND(StringFind, "String Functions");
		REGISTER_CSASCOMMAND(StringInsert, "String Functions");
		REGISTER_CSASCOMMAND(StringSubStr, "String Functions");
	}

	BEGIN_CSASCOMMAND_HANDLER(StringLength)
	{
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Buffer))
			return false;

		if (Buffer)
			Result->SetNumber(strlen(Buffer));

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(StringLength, "Rreturns the number of characters in a string", CSASDataElement::kParamType_Numeric, kParams_OneString, 1);

	BEGIN_CSASCOMMAND_PARAMINFO(StringCompare, 3)
	{
		{ "First String", CSASDataElement::kParamType_String },
		{ "Second String", CSASDataElement::kParamType_String },
		{ "Ignore Case", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(StringCompare)
	{
		char BufferA[0x4000] = {0}, BufferB[0x4000] = {0};
		double IgnoreCase = 0;

		if (!EXTRACT_CSASARGS(&BufferA, &BufferB, &IgnoreCase))
			return false;

		if (BufferA && BufferB)
		{
			if (IgnoreCase)
				Result->SetNumber(_stricmp(BufferA, BufferB));
			else
				Result->SetNumber(strcmp(BufferA, BufferB));
		}

		return true;
	}
	DEFINE_CSASCOMMAND(StringCompare, "Returns 0 if the strings are equal, 1 if the first string occurs alphabetically before the second, -1 if the first string occurs alphabetically after the second", CSASDataElement::kParamType_Numeric, 3);

	BEGIN_CSASCOMMAND_PARAMINFO(StringErase, 3)
	{
		{ "String", CSASDataElement::kParamType_String },
		{ "Start Index", CSASDataElement::kParamType_Numeric },
		{ "Number Of Characters", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(StringErase)
	{
		char Buffer[0x4000] = {0};
		double StartIndex = 0, Length = 0;

		if (!EXTRACT_CSASARGS(&Buffer, &StartIndex, &Length))
			return false;

		if (Buffer)
		{
			std::string STLBuffer(Buffer);
			STLBuffer.erase(StartIndex, Length);
			Result->SetString(STLBuffer.c_str());
		}

		return true;
	}
	DEFINE_CSASCOMMAND(StringErase, "Erases the specified number of characters from the string starting at the specified position", CSASDataElement::kParamType_String, 3);

	BEGIN_CSASCOMMAND_PARAMINFO(StringFind, 4)
	{
		{ "Source String", CSASDataElement::kParamType_String },
		{ "Query String", CSASDataElement::kParamType_String },
		{ "Start Index", CSASDataElement::kParamType_Numeric },
		{ "Ignore Case", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(StringFind)
	{
		char BufferA[0x4000] = {0}, BufferB[0x4000] = {0};
		double StartIndex = 0, IgnoreCase = 0;

		if (!EXTRACT_CSASARGS(&BufferA, &BufferB, &StartIndex, &IgnoreCase))
			return false;

		if (BufferA && BufferB)
		{
			std::string STLBufferA(BufferA), STLBufferB(BufferB);

			if (IgnoreCase)
			{
				MakeLower(STLBufferA);
				MakeLower(STLBufferB);
			}

			Result->SetNumber(STLBufferA.find(STLBufferB, StartIndex));
		}

		return true;
	}
	DEFINE_CSASCOMMAND(StringFind, "Returns the index of the first occurrence of the substring within the source string, starting at the specified position", CSASDataElement::kParamType_Numeric, 4);

	BEGIN_CSASCOMMAND_PARAMINFO(StringInsert, 3)
	{
		{ "Source String", CSASDataElement::kParamType_String },
		{ "Insert String", CSASDataElement::kParamType_String },
		{ "Insert Index", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(StringInsert)
	{
		char BufferA[0x4000] = {0}, BufferB[0x4000] = {0};
		double InsertIndex = 0;

		if (!EXTRACT_CSASARGS(&BufferA, &BufferB, &InsertIndex))
			return false;

		if (BufferA && BufferB)
		{
			std::string STLBuffer(BufferA);
			STLBuffer.insert(InsertIndex, BufferB);
			Result->SetString(STLBuffer.c_str());
		}

		return true;
	}
	DEFINE_CSASCOMMAND(StringInsert, "Inserts a substring into a string at the specified position", CSASDataElement::kParamType_String, 3);

	BEGIN_CSASCOMMAND_PARAMINFO(StringSubStr, 4)
	{
		{ "Source String", CSASDataElement::kParamType_String },
		{ "Start Index", CSASDataElement::kParamType_Numeric },
		{ "Length", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(StringSubStr)
	{
		char Buffer[0x4000] = {0};
		double StartIndex = 0, Length = 0;

		if (!EXTRACT_CSASARGS(&Buffer, &StartIndex, &Length))
			return false;

		if (Buffer)
		{
			std::string STLBuffer(Buffer);
			STLBuffer = STLBuffer.substr(StartIndex, Length);
			Result->SetString(STLBuffer.c_str());
		}

		return true;
	}
	DEFINE_CSASCOMMAND(StringSubStr, "Returns a substring of the source source string, starting at the specified position", CSASDataElement::kParamType_String, 3);
}