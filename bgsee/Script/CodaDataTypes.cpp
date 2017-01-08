#include "CodaDataTypes.h"

namespace bgsee
{
	namespace script
	{
		int		CodaScriptBackingStore::GIC = 0;

		void CodaScriptBackingStore::Reset( void )
		{
			switch (Type)
			{
			case kDataType_String:
				SAFEDELETE_ARRAY(StringData);
				break;
			}

			Type = kDataType_Invalid;
		}

		void CodaScriptBackingStore::Copy( const CodaScriptBackingStore& Source )
		{
			switch (Source.Type)
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

		bool CodaScriptBackingStore::GetHasImplicitCast( DataType NewType ) const
		{
			bool Result = false;

			switch (Type)
			{
			case kDataType_Reference:
				if (NewType == kDataType_Numeric)
					Result = true;

				break;
			}

			return Result;
		}

		CodaScriptReferenceDataTypeT CodaScriptBackingStore::GetFormID() const
		{
			SME_ASSERT(GetIsReference());
			return RefData;
		}

		CodaScriptNumericDataTypeT CodaScriptBackingStore::GetNumber() const
		{
			SME_ASSERT(GetIsNumber());

			switch (Type)
			{
			case kDataType_Numeric:
				return NumericData;
			case kDataType_Reference:
				return RefData;
			}

			// this should never happen, duh!
			return 0;
		}

		CodaScriptStringParameterTypeT CodaScriptBackingStore::GetString() const
		{
			SME_ASSERT(GetIsString());
			return StringData;
		}

		ICodaScriptArrayDataType::SharedPtrT CodaScriptBackingStore::GetArray() const
		{
			SME_ASSERT(GetIsArray());
			return ArrayData;
		}

		void CodaScriptBackingStore::SetFormID( CodaScriptReferenceDataTypeT Data )
		{
			Reset();
			Type = kDataType_Reference;
			RefData = Data;
		}

		void CodaScriptBackingStore::SetNumber( CodaScriptNumericDataTypeT Data )
		{
			Reset();
			Type = kDataType_Numeric;
			NumericData = Data;
		}

		void CodaScriptBackingStore::SetString( CodaScriptStringParameterTypeT Data )
		{
			Reset();
			Type = kDataType_String;

			if (Data == nullptr)
				Data = "";

			UInt32 Size = strlen(Data);
			StringData = new CodaScriptCharDataTypeT[Size + 1];

			StringData[Size] = '\0';
			if (Size)
				strcpy_s(StringData, Size + 1, Data);
		}

		void CodaScriptBackingStore::SetArray( ICodaScriptDataStore* Data )
		{
			CodaScriptBackingStore* RHS = dynamic_cast<CodaScriptBackingStore*>(Data);
			SME_ASSERT(RHS && RHS->Type == kDataType_Array);

			Reset();
			Type = kDataType_Array;
			ArrayData = RHS->ArrayData;
		}

		void CodaScriptBackingStore::SetArray(ICodaScriptArrayDataType::SharedPtrT Data )
		{
			Reset();
			Type = kDataType_Array;
			ArrayData = Data;
		}

		CodaScriptBackingStore::CodaScriptBackingStore( CodaScriptBackingStore* Data )
			: ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			Copy(*Data);
		}

		CodaScriptBackingStore::CodaScriptBackingStore( CodaScriptNumericDataTypeT Num )
			: ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			SetNumber(Num);
		}

		CodaScriptBackingStore::CodaScriptBackingStore( CodaScriptStringParameterTypeT Str )
			: ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			SetString(Str);
		}

		CodaScriptBackingStore::CodaScriptBackingStore( CodaScriptReferenceDataTypeT Form )
			: ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			SetFormID(Form);
		}

		CodaScriptBackingStore::CodaScriptBackingStore(ICodaScriptArrayDataType::SharedPtrT Array )
			: ICodaScriptDataStore(), NumericData(0), ArrayData()
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
			: ICodaScriptDataStore(), NumericData(0), ArrayData()
		{
			GIC++;

			Copy(rhs);
		}

		CodaScriptBackingStore::CodaScriptBackingStore()
			: ICodaScriptDataStore(), NumericData(0), ArrayData()
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