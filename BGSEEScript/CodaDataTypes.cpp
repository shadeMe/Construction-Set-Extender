#include "CodaDataTypes.h"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		int		CodaScriptBackingStore::GIC = 0;

		inline void CodaScriptBackingStore::Reset( void )
		{
			switch (Type)
			{
			case kDataType_String:
				SAFEDELETE_ARRAY(StringData);
				break;
			}

			Type = kDataType_Invalid;
		}

		inline void CodaScriptBackingStore::Copy( const CodaScriptBackingStore& Source )
		{
			switch (Type)
			{
			case kDataType_Invalid:
				break;
			case kDataType_Array:
				SetArray(Source.GetArray());
				break;
			case kDataType_String:
				SetString(Source.GetString());
				break;
			case kDataType_Reference:
				SetFormID(Source.GetFormID());
				break;
			default:
				SetNumber(Source.GetNumber());
				break;
			}
		}

		inline CodaScriptReferenceDataTypeT CodaScriptBackingStore::GetFormID() const
		{
			SME_ASSERT(Type == kDataType_Reference);
			return RefData;
		}

		inline CodaScriptNumericDataTypeT CodaScriptBackingStore::GetNumber() const
		{
			SME_ASSERT(Type == kDataType_Numeric);
			return NumericData;
		}

		inline CodaScriptStringParameterTypeT CodaScriptBackingStore::GetString() const
		{
			SME_ASSERT(Type == kDataType_String);
			return StringData;
		}

		inline CodaScriptSharedHandleArrayT CodaScriptBackingStore::GetArray() const
		{
			SME_ASSERT(Type == kDataType_Array);
			return ArrayData;
		}

		inline void CodaScriptBackingStore::SetFormID( CodaScriptReferenceDataTypeT Data )
		{
			Reset();
			Type = kDataType_Reference;
			RefData = Data;
		}

		inline void CodaScriptBackingStore::SetNumber( CodaScriptNumericDataTypeT Data )
		{
			Reset();
			Type = kDataType_Numeric;
			NumericData = Data;
		}

		inline void CodaScriptBackingStore::SetString( CodaScriptStringParameterTypeT Data )
		{
			Reset();
			Type = kDataType_String;

			if (Data == NULL)
				Data = "";

			UInt32 Size = strlen(Data);
			StringData = new char[Size + 1];

			StringData[Size] = '\0';
			if (Size)
				strcpy_s(StringData, Size + 1, Data);
		}

		inline void CodaScriptBackingStore::SetArray( ICodaScriptDataStore* Data )
		{
			CodaScriptBackingStore* RHS = dynamic_cast<CodaScriptBackingStore*>(Data);
			SME_ASSERT(RHS && RHS->Type == kDataType_Array);

			Reset();
			Type = kDataType_Array;
			ArrayData = RHS->ArrayData;
		}

		inline void CodaScriptBackingStore::SetArray( CodaScriptSharedHandleArrayT Data )
		{
			Reset();
			Type = kDataType_Array;
			ArrayData = Data;
		}

		CodaScriptBackingStore::CodaScriptBackingStore( CodaScriptBackingStore* Data )
			: ICodaScriptObject(), ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			Copy(*Data);
		}

		CodaScriptBackingStore::CodaScriptBackingStore( double Num )
			: ICodaScriptObject(), ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			SetNumber(Num);
		}

		CodaScriptBackingStore::CodaScriptBackingStore( const char* Str )
			: ICodaScriptObject(), ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			SetString(Str);
		}

		CodaScriptBackingStore::CodaScriptBackingStore( UInt32 Form )
			: ICodaScriptObject(), ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			SetFormID(Form);
		}

		CodaScriptBackingStore::CodaScriptBackingStore( CodaScriptSharedHandleArrayT Array )
			: ICodaScriptObject(), ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			SetArray(Array);
		}

		CodaScriptBackingStore::~CodaScriptBackingStore()
		{
			Reset();

			GIC--;
			SME_ASSERT(GIC >= 0);
		}

		CodaScriptBackingStore::CodaScriptBackingStore( const CodaScriptBackingStore& rhs )
			: ICodaScriptObject(), ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			Copy(rhs);
		}

		CodaScriptBackingStore::CodaScriptBackingStore()
			: ICodaScriptObject(), ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;
		}

		CodaScriptBackingStore& CodaScriptBackingStore::operator=( const CodaScriptBackingStore& rhs )
		{
			Copy(rhs);
			return *this;
		}

		ICodaScriptDataStore& CodaScriptBackingStore::operator=( const ICodaScriptDataStore& rhs )
		{
			SME_ASSERT(typeid(rhs) == typeid(CodaScriptBackingStore));

			Copy(dynamic_cast<const CodaScriptBackingStore&>(rhs));
			return *this;
		}

		ICodaScriptDataStore& CodaScriptBackingStore::operator=( CodaScriptNumericDataTypeT Num )
		{
			SetNumber(Num);
			return *this;
		}

		ICodaScriptDataStore& CodaScriptBackingStore::operator=( CodaScriptStringParameterTypeT Str )
		{
			SetString(Str);
			return *this;
		}

		ICodaScriptDataStore& CodaScriptBackingStore::operator=( CodaScriptReferenceDataTypeT Form )
		{
			SetFormID(Form);
			return *this;
		}

		CodaScriptBackingStore& CodaScriptBackingStore::operator+=( const CodaScriptBackingStore &rhs )
		{
			if (Type == rhs.Type)
			{
				switch (Type)
				{
				case kDataType_Numeric:
					NumericData += rhs.NumericData;
					break;
				case kDataType_String:
					SetString((std::string(std::string(StringData) + std::string(rhs.StringData))).c_str());
					break;
				}
			}

			return *this;
		}

		CodaScriptBackingStore& CodaScriptBackingStore::operator-=( const CodaScriptBackingStore &rhs )
		{
			if (Type == rhs.Type)
			{
				switch (Type)
				{
				case kDataType_Numeric:
					NumericData -= rhs.NumericData;
					break;
				}
			}

			return *this;
		}

		CodaScriptBackingStore& CodaScriptBackingStore::operator*=( const CodaScriptBackingStore &rhs )
		{
			if (Type == rhs.Type)
			{
				switch (Type)
				{
				case kDataType_Numeric:
					NumericData *= rhs.NumericData;
					break;
				}
			}

			return *this;
		}

		ICodaScriptArrayDataType::~ICodaScriptArrayDataType()
		{
			;//
		}

		int				CodaScriptVariable::GIC = 0;

		CodaScriptVariable::CodaScriptVariable( CodaScriptSourceCodeT& Name, ICodaScriptDataStoreOwner* Storage ) :
			ICodaScriptObject(),
			Name(Name),
			BoundOwner(Storage)
		{
			SME_ASSERT(Storage);
			GIC++;
		}

		CodaScriptVariable::CodaScriptVariable( const CodaScriptVariable& rhs )
		{
			;// blocked
		}

		CodaScriptVariable& CodaScriptVariable::operator=( const CodaScriptVariable& rhs )
		{
			;// blocked
			return *this;
		}

		CodaScriptVariable::~CodaScriptVariable()
		{
			GIC--;
			SME_ASSERT(GIC >= 0);
		}

		const char* CodaScriptVariable::GetName() const
		{
			return Name.c_str();
		}

		void CodaScriptVariable::SetName( CodaScriptSourceCodeT& Name )
		{
			SME_ASSERT(Name.length() > 2);
			this->Name = Name;
		}

		ICodaScriptDataStoreOwner* CodaScriptVariable::GetStoreOwner() const
		{
			SME_ASSERT(BoundOwner.get());
			return BoundOwner.get();
		}
	}
}