#pragma once

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		// these typedefs could, of course, be wrapped but what would be the point? I say "meh"
		typedef double													CodaScriptNumericDataTypeT;
		typedef char													CodaScriptCharDataTypeT;
		typedef CodaScriptCharDataTypeT*								CodaScriptStringDataTypeT;
		typedef const CodaScriptCharDataTypeT*							CodaScriptStringParameterTypeT;
		typedef UInt32													CodaScriptReferenceDataTypeT;

		class ICodaScriptCommandHandlerHelper;
		class ICodaScriptSyntaxTreeEvaluator;
		class ICodaScriptExpressionByteCode;

		class ICodaScriptDataStore
		{
		public:
			enum DataType
			{
				kDataType_Invalid	= '?',								// default implementation details:
				kDataType_Numeric	= 'n',								//		-
				kDataType_Reference	= 'r',								//		formIDs
				kDataType_String	= 's',								//		-
				kDataType_Array		= 'a',								//		ref counted
			};
		protected:
			DataType														Type;
		public:
			ICodaScriptDataStore() :
				Type(kDataType_Invalid)
			{
				;//
			}

			virtual ~ICodaScriptDataStore() = 0
			{
				;//
			}

			DataType														GetType() const
			{
				return Type;
			}

			bool															GetIsNumber() const
			{
				return (Type == kDataType_Numeric || GetHasImplicitCast(kDataType_Numeric));
			}

			bool															GetIsReference() const
			{
				return (Type == kDataType_Reference);
			}

			bool															GetIsString() const
			{
				return (Type == kDataType_String);
			}

			bool															GetIsArray() const
			{
				return (Type == kDataType_Array);
			}

			virtual bool													GetHasImplicitCast(DataType NewType) const = 0;
																			// the GetXXX accessory functions should perform the necessary casting internally
			virtual CodaScriptReferenceDataTypeT							GetFormID() const = 0;
			virtual CodaScriptNumericDataTypeT								GetNumber() const = 0;
			virtual CodaScriptStringParameterTypeT							GetString() const = 0;

			virtual void													SetFormID(CodaScriptReferenceDataTypeT Data) = 0;
			virtual void													SetNumber(CodaScriptNumericDataTypeT Data) = 0;
			virtual void													SetString(CodaScriptStringParameterTypeT Data) = 0;
			virtual void													SetArray(ICodaScriptDataStore* Data) = 0;

			virtual ICodaScriptDataStore&									operator=(const ICodaScriptDataStore& rhs) = 0;
			virtual ICodaScriptDataStore&									operator=(CodaScriptNumericDataTypeT Num) = 0;
			virtual ICodaScriptDataStore&									operator=(CodaScriptStringParameterTypeT Str) = 0;
			virtual ICodaScriptDataStore&									operator=(CodaScriptReferenceDataTypeT Form) = 0;
		};

		class ICodaScriptDataStoreOwner
		{
		public:
			virtual ~ICodaScriptDataStoreOwner() = 0
			{
				;//
			}

			virtual ICodaScriptDataStore*									GetDataStore() = 0;
			virtual ICodaScriptDataStoreOwner&								operator=(const ICodaScriptDataStore& rhs) = 0;
		};

		class ICodaScriptCommand
		{
		public:
			virtual ~ICodaScriptCommand() = 0
			{
				;//
			}

			struct ParameterInfo
			{
				const char*							Name;
				UInt8								Type;

				enum
				{
					kType_Multi = '!'
				};
			};

			virtual const char*					GetName(void) = 0;
			virtual const char*					GetAlias(void) = 0;

			virtual const char*					GetDescription(void) = 0;
			virtual const char*					GetDocumentation(void) = 0;		// can contain HTML markup
			virtual int							GetParameterData(int* OutParameterCount = NULL,
																ParameterInfo** OutParameterInfoArray = NULL,
																UInt8* OutResultType = NULL) = 0;
																				// returns parameter count
																				// parameter count must be -1 for variadic functions

			virtual bool						Execute(ICodaScriptDataStore* Arguments,
														ICodaScriptDataStore* Result,
														ParameterInfo* ParameterData,
														int ArgumentCount,
														ICodaScriptCommandHandlerHelper* Utilities,
														ICodaScriptSyntaxTreeEvaluator* ExecutionAgent,
														ICodaScriptExpressionByteCode* ByteCode) = 0;
																				// return false to skip result validation
		};

		class ICodaScriptCommandHandlerHelper
		{
		public:
			virtual ~ICodaScriptCommandHandlerHelper() = 0
			{
				;//
			}

			virtual ICodaScriptDataStore*		ArrayAllocate(UInt32 InitialSize = 0) = 0;
			virtual bool						ArrayPushback(ICodaScriptDataStore* AllocatedArray, CodaScriptNumericDataTypeT Data) = 0;
			virtual bool						ArrayPushback(ICodaScriptDataStore* AllocatedArray, CodaScriptStringParameterTypeT Data) = 0;
			virtual bool						ArrayPushback(ICodaScriptDataStore* AllocatedArray, CodaScriptReferenceDataTypeT Data) = 0;
			virtual bool						ArrayPushback(ICodaScriptDataStore* AllocatedArray, ICodaScriptDataStore* ArrayData) = 0;
			virtual bool						ArrayAt(ICodaScriptDataStore* AllocatedArray, UInt32 Index, ICodaScriptDataStore** OutBuffer) = 0;
			virtual bool						ArrayErase(ICodaScriptDataStore* AllocatedArray, UInt32 Index) = 0;
			virtual void						ArrayClear(ICodaScriptDataStore* AllocatedArray) = 0;
			virtual UInt32						ArraySize(ICodaScriptDataStore* AllocatedArray) = 0;

			virtual bool						ExtractArguments(ICodaScriptDataStore* Arguments,
																ICodaScriptCommand::ParameterInfo* ParameterData,
																UInt32 ArgumentCount,
																...) = 0;
																// pass CodaScriptStringParameterTypeT pointers for string arguments
																// ICodaScriptDataStore pointers for arrays and multitype args
																// variadic functions must manually extract their arguments
		};
	}
}