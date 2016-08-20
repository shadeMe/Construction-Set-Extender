#include "CodaScriptCommands-String.h"

namespace bgsee
{
	namespace script
	{
		namespace commands
		{
			namespace string
			{
				CodaScriptCommandRegistrarDef("String")

				CodaScriptCommandPrototypeDef(StringLength);
				CodaScriptCommandPrototypeDef(StringCompare);
				CodaScriptCommandPrototypeDef(StringErase);
				CodaScriptCommandPrototypeDef(StringFind);
				CodaScriptCommandPrototypeDef(StringInsert);
				CodaScriptCommandPrototypeDef(StringSubStr);
				CodaScriptCommandPrototypeDef(StringIsNumber);

				CodaScriptCommandParamData(StringCompare, 3)
				{
					{ "First String",						ICodaScriptDataStore::kDataType_String	},
					{ "Second String",						ICodaScriptDataStore::kDataType_String	},
					{ "Ignore Case",						ICodaScriptDataStore::kDataType_Numeric	}
				};

				CodaScriptCommandParamData(StringErase, 3)
				{
					{ "String",								ICodaScriptDataStore::kDataType_String	},
					{ "Start Index",						ICodaScriptDataStore::kDataType_Numeric },
					{ "Number Of Characters",				ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(StringFind, 4)
				{
					{ "Source String",						ICodaScriptDataStore::kDataType_String	},
					{ "Query String",						ICodaScriptDataStore::kDataType_String	},
					{ "Start Index",						ICodaScriptDataStore::kDataType_Numeric },
					{ "Ignore Case",						ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(StringInsert, 3)
				{
					{ "Source String",						ICodaScriptDataStore::kDataType_String	},
					{ "Insert String",						ICodaScriptDataStore::kDataType_String	},
					{ "Insert Index",						ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(StringSubStr, 3)
				{
					{ "Source String",						ICodaScriptDataStore::kDataType_String	},
					{ "Start Index",						ICodaScriptDataStore::kDataType_Numeric },
					{ "Length",								ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandHandler(StringLength)
				{
					CodaScriptStringParameterTypeT Buffer = nullptr;

					CodaScriptCommandExtractArgs(&Buffer);

					if (Buffer)
						Result->SetNumber(strlen(Buffer));

					return true;
				}

				CodaScriptCommandHandler(StringCompare)
				{
					CodaScriptStringParameterTypeT BufferA = nullptr, BufferB = nullptr;
					double IgnoreCase = 0;

					CodaScriptCommandExtractArgs(&BufferA, &BufferB, &IgnoreCase);

					if (BufferA && BufferB)
					{
						if (IgnoreCase)
							Result->SetNumber(_stricmp(BufferA, BufferB));
						else
							Result->SetNumber(strcmp(BufferA, BufferB));
					}

					return true;
				}

				CodaScriptCommandHandler(StringErase)
				{
					CodaScriptStringParameterTypeT Buffer = nullptr;
					double StartIndex = 0, Length = 0;

					CodaScriptCommandExtractArgs(&Buffer, &StartIndex, &Length);

					if (Buffer)
					{
						std::string STLBuffer(Buffer);
						STLBuffer.erase(StartIndex, Length);
						Result->SetString(STLBuffer.c_str());
					}

					return true;
				}

				CodaScriptCommandHandler(StringFind)
				{
					CodaScriptStringParameterTypeT BufferA = nullptr, BufferB = nullptr;
					double StartIndex = 0, IgnoreCase = 0;

					CodaScriptCommandExtractArgs(&BufferA, &BufferB, &StartIndex, &IgnoreCase);

					if (BufferA && BufferB)
					{
						std::string STLBufferA(BufferA), STLBufferB(BufferB);

						if (IgnoreCase)
						{
							SME::StringHelpers::MakeLower(STLBufferA);
							SME::StringHelpers::MakeLower(STLBufferB);
						}

						Result->SetNumber(STLBufferA.find(STLBufferB, StartIndex));
					}

					return true;
				}

				CodaScriptCommandHandler(StringInsert)
				{
					CodaScriptStringParameterTypeT BufferA = nullptr, BufferB = nullptr;
					double InsertIndex = 0;

					CodaScriptCommandExtractArgs(&BufferA, &BufferB, &InsertIndex);

					if (BufferA && BufferB)
					{
						std::string STLBuffer(BufferA);
						STLBuffer.insert(InsertIndex, BufferB);
						Result->SetString(STLBuffer.c_str());
					}

					return true;
				}

				CodaScriptCommandHandler(StringSubStr)
				{
					CodaScriptStringParameterTypeT Buffer = nullptr;
					double StartIndex = 0, Length = 0;

					CodaScriptCommandExtractArgs(&Buffer, &StartIndex, &Length);

					if (Buffer)
					{
						std::string STLBuffer(Buffer);
						STLBuffer = STLBuffer.substr(StartIndex, Length);
						Result->SetString(STLBuffer.c_str());
					}

					return true;
				}

				CodaScriptCommandHandler(StringIsNumber)
				{
					CodaScriptStringParameterTypeT Buffer = nullptr;

					CodaScriptCommandExtractArgs(&Buffer);

					if (Buffer)
					{
						std::string STLBuffer(Buffer);
						Result->SetNumber(1);

						for (std::string::iterator Itr = STLBuffer.begin(); Itr != STLBuffer.end(); Itr++)
						{
							if (isdigit(*Itr) == 0 || isxdigit(*Itr) == 0)
							{
								Result->SetNumber(0);
								break;
							}
						}
					}

					return true;
				}
			}
		}
	}
}