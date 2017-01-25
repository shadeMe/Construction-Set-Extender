#include "mpError.h"
#include "mpValueCache.h"
#include "CodaMUPValue.h"
#include "CodaMUPArrayDataType.h"

namespace bgsee
{
	namespace script
	{
		namespace mup
		{
			int			CodaScriptMUPValue::GIC = 0;

			CodaScriptMUPValue::CodaScriptMUPValue(char_type cType) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_cType(cType),
				m_iFlags(flNONE),
				m_pCache(nullptr),
				m_DataStore(0.0),
				m_StringBuffer()
			{
				GIC++;

				switch (cType)
				{
				case 'v':
				case 'i':
				case 'f':
					break;
				case 's':
					m_DataStore.SetString(nullptr);
					break;
				default:
					SME_ASSERT(cType == 'i' || cType == 'f' || cType == 's');		// type's always one of the three, array handling is opaque to the implementation
					break;
				}
			}

			CodaScriptMUPValue::CodaScriptMUPValue( float_type val ) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_cType('f'),
				m_iFlags(flNONE),
				m_pCache(nullptr),
				m_DataStore((CodaScriptNumericDataTypeT)val),
				m_StringBuffer()
			{
				GIC++;
			}

			CodaScriptMUPValue::CodaScriptMUPValue( string_type val ) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_cType('s'),
				m_iFlags(flNONE),
				m_pCache(nullptr),
				m_DataStore(val.c_str()),
				m_StringBuffer(val)
			{
				GIC++;
			}

			CodaScriptMUPValue::CodaScriptMUPValue( const char_type *val ) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_cType('s'),
				m_iFlags(flNONE),
				m_pCache(nullptr),
				m_DataStore(val),
				m_StringBuffer(val)
			{
				GIC++;
			}

			CodaScriptMUPValue::CodaScriptMUPValue( const CodaScriptMUPValue &a_Val ) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_iFlags(flNONE),
				m_pCache(nullptr),
				m_DataStore(),
				m_StringBuffer()
			{
				GIC++;

				Assign(a_Val);
			}

			CodaScriptMUPValue::CodaScriptMUPValue( const IValue &a_Val ) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_pCache(nullptr),
				m_DataStore(),
				m_StringBuffer()
			{
				GIC++;

				Assign(a_Val);
			}

			CodaScriptMUPValue::CodaScriptMUPValue( CodaScriptBackingStore *val ) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_cType('i'),
				m_iFlags(flNONE),
				m_pCache(nullptr),
				m_DataStore(val),
				m_StringBuffer()
			{
				GIC++;

				if (m_DataStore.GetType() == ICodaScriptDataStore::kDataType_String)
				{
					m_cType = 's';
					m_StringBuffer = val->GetString();
				}
				else if (m_DataStore.GetType() == ICodaScriptDataStore::kDataType_Numeric)
					m_cType = 'f';
			}

			CodaScriptMUPValue::CodaScriptMUPValue( const CodaScriptBackingStore& val ) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_cType('i'),
				m_iFlags(flNONE),
				m_pCache(nullptr),
				m_DataStore(val),
				m_StringBuffer()
			{
				GIC++;

				if (m_DataStore.GetType() == ICodaScriptDataStore::kDataType_String)
				{
					m_cType = 's';
					m_StringBuffer = val.GetString();
				}
				else if (m_DataStore.GetType() == ICodaScriptDataStore::kDataType_Numeric)
					m_cType = 'f';
			}

			CodaScriptMUPValue::CodaScriptMUPValue( CodaScriptReferenceDataTypeT val ) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_cType('i'),
				m_iFlags(flNONE),
				m_pCache(nullptr),
				m_DataStore(val),
				m_StringBuffer()
			{
				GIC++;
			}

			CodaScriptMUPValue::CodaScriptMUPValue( ICodaScriptArrayDataType::SharedPtrT val ) :
				ICodaScriptDataStoreOwner(),
				IValue(cmVAL),
				m_cType('i'),
				m_iFlags(flNONE),
				m_pCache(nullptr),
				m_DataStore(val),
				m_StringBuffer()
			{
				GIC++;
			}

			CodaScriptMUPValue& CodaScriptMUPValue::operator=( const CodaScriptMUPValue &a_Val )
			{
				Assign(a_Val);
				return *this;
			}

			CodaScriptMUPValue::~CodaScriptMUPValue()
			{
				GIC--;
				SME_ASSERT(GIC >= 0);
			}

			IValue& CodaScriptMUPValue::At( int nRow, int nCol /*= 0*/ )
			{
				if (m_DataStore.GetType() == ICodaScriptDataStore::kDataType_Array)
				{
					if (nRow >= m_DataStore.GetArray()->Size())
						throw ParserError( ErrorContext(ecINDEX_OUT_OF_BOUNDS, -1, GetIdent()) );

					CodaScriptMUPArrayDataType* Array = dynamic_cast<CodaScriptMUPArrayDataType*>(m_DataStore.GetArray().get());
					SME_ASSERT(Array);

					CodaScriptMUPValue* Out = nullptr;
					Array->At(nRow, &Out);
					SME_ASSERT(Out);

					return *Out;
				}
				else if (nRow==0 && nCol==0)
				{
					return *this;
				}
				else
					throw ParserError( ErrorContext(ecINDEX_OUT_OF_BOUNDS) );
			}

			IValue& CodaScriptMUPValue::At( const IValue &row, const IValue &col )
			{
				if (!row.IsInteger() || !col.IsInteger())
				{
					ErrorContext errc(ecTYPE_CONFLICT_IDX, GetExprPos());
					errc.Hint = _T("Array index must be an integer value.");
					errc.Type1 = (!row.IsInteger()) ? row.GetType() : col.GetType();
					errc.Type2 = 'i';
					throw ParserError(errc);
				}

				int nRow = row.GetInteger();
				return At(nRow, 0);
			}

			IValue& CodaScriptMUPValue::operator=( int_type a_iVal )
			{
				m_DataStore.SetFormID(a_iVal);

				m_cType = 'i';
				m_iFlags = flNONE;

				return *this;
			}

			IValue& CodaScriptMUPValue::operator=( float_type a_fVal )
			{
				m_DataStore.SetNumber(a_fVal);

				m_cType = 'f';
				m_iFlags = flNONE;

				return *this;
			}

			IValue& CodaScriptMUPValue::operator=( string_type a_sVal )
			{
				m_DataStore.SetString(a_sVal.c_str());
				m_StringBuffer = a_sVal;

				m_cType = 's';
				m_iFlags = flNONE;

				return *this;
			}

			IValue& CodaScriptMUPValue::operator=( bool val )
			{
				m_DataStore.SetNumber(val);

				m_cType = 'f';
				m_iFlags = flNONE;

				return *this;
			}

			IValue& CodaScriptMUPValue::operator=( const matrix_type &a_vVal )
			{
				SME_ASSERT(m_cType == 'm');
				return *this;
			}

			IValue& CodaScriptMUPValue::operator=( const cmplx_type &val )
			{
				m_DataStore.SetNumber(val.real());

				m_cType = 'f';
				m_iFlags = flNONE;
				return *this;
			}

			IValue& CodaScriptMUPValue::operator=( const char_type *a_szVal )
			{
				m_DataStore.SetString(a_szVal);
				m_StringBuffer = a_szVal;

				m_cType = 's';
				m_iFlags = flNONE;

				return *this;
			}

			ICodaScriptDataStoreOwner& CodaScriptMUPValue::operator=( const ICodaScriptDataStore& rhs )
			{
				m_DataStore = rhs;
				m_cType = GetMUPType(m_DataStore.GetType());

				if (m_cType == 's')
					m_StringBuffer = m_DataStore.GetString();

				return *this;
			}

			IValue& CodaScriptMUPValue::operator+=( const IValue &val )
			{
				CodaScriptBackingStore* Param = val.GetStore();
				SME_ASSERT(Param);

				if (m_DataStore.GetType() == Param->GetType() && m_DataStore.GetType() == ICodaScriptDataStore::kDataType_Numeric)
					m_DataStore += *Param;
				else
				{
					// Type conflict
					throw ParserError(ErrorContext(ecTYPE_CONFLICT_FUN, -1, _T("+"), m_DataStore.GetType(), Param->GetType(), 2));
				}

				return *this;
			}

			IValue& CodaScriptMUPValue::operator-=( const IValue &val )
			{
				CodaScriptBackingStore* Param = val.GetStore();
				SME_ASSERT(Param);

				if (m_DataStore.GetType() == Param->GetType() && m_DataStore.GetType() == ICodaScriptDataStore::kDataType_Numeric)
					m_DataStore -= *Param;
				else
				{
					// Type conflict
					throw ParserError(ErrorContext(ecTYPE_CONFLICT_FUN, -1, _T("-"), m_DataStore.GetType(), Param->GetType(), 2));
				}

				return *this;
			}

			IValue& CodaScriptMUPValue::operator*=( const IValue &val )
			{
				CodaScriptBackingStore* Param = val.GetStore();
				SME_ASSERT(Param);

				if (m_DataStore.GetType() == Param->GetType() && m_DataStore.GetType() == ICodaScriptDataStore::kDataType_Numeric)
					m_DataStore *= *Param;
				else
				{
					// Type conflict
					throw ParserError(ErrorContext(ecTYPE_CONFLICT_FUN, -1, _T("*"), m_DataStore.GetType(), Param->GetType(), 2));
				}

				return *this;
			}

			void CodaScriptMUPValue::BindToCache(ValueCache *pCache)
			{
				m_pCache = pCache;
			}

			CodaScriptBackingStore* CodaScriptMUPValue::GetStore( void ) const
			{
				return &m_DataStore;
			}

			void CodaScriptMUPValue::CheckType( char_type a_cType ) const
			{
				if (m_cType != a_cType)
				{
					ErrorContext err;
					err.Errc  = ecTYPE_CONFLICT;
					err.Type1 = m_cType;
					err.Type2 = a_cType;

					if (GetIdent().length())
						err.Ident = GetIdent();
					else
						err.Ident = "<intermediate/temporary>";

					throw ParserError(err);
				}
			}

			void CodaScriptMUPValue::CheckType( ICodaScriptDataStore::DataType a_cType ) const
			{
				if (m_DataStore.GetType() != a_cType && m_DataStore.HasImplicitCast(a_cType) == false)
				{
					ErrorContext err;
					err.Errc  = ecTYPE_CONFLICT;
					err.Type1 = m_DataStore.GetType();
					err.Type2 = a_cType;

					if (GetIdent().length())
						err.Ident = GetIdent();
					else
						err.Ident = "<intermediate/temporary>";

					throw ParserError(err);
				}
			}

			void CodaScriptMUPValue::Assign( const CodaScriptMUPValue &a_Val )
			{
				if (this==&a_Val)
					return;

				m_cType  = a_Val.m_cType;
				m_iFlags = a_Val.m_iFlags;
				m_DataStore = a_Val.m_DataStore;

				if (m_cType == 's')
					m_StringBuffer = a_Val.GetString();
			}

			void CodaScriptMUPValue::Assign(const IValue& a_Val)
			{
				if (this == &a_Val)
					return;

				CodaScriptBackingStore* Store = a_Val.GetStore();
				SME_ASSERT(Store);

				Assign(*Store);
			}

			void CodaScriptMUPValue::Assign(const CodaScriptBackingStore& a_Val)
			{
				m_DataStore = a_Val;

				if (m_DataStore.GetType() == ICodaScriptDataStore::kDataType_String)
				{
					m_cType = 's';
					m_StringBuffer = a_Val.GetString();
				}
				else if (m_DataStore.GetType() == ICodaScriptDataStore::kDataType_Numeric)
					m_cType = 'f';
			}

			void CodaScriptMUPValue::Release()
			{
				if (m_pCache)
					m_pCache->ReleaseToCache(this);
				else
					delete this;
			}

			char_type CodaScriptMUPValue::GetType() const
			{
				return m_cType;
			}

			int_type CodaScriptMUPValue::GetInteger() const
			{
				CheckType( ICodaScriptDataStore::kDataType_Numeric);
				return (int_type)m_DataStore.GetNumber();
			}

			float_type CodaScriptMUPValue::GetFloat() const
			{
				CheckType( ICodaScriptDataStore::kDataType_Numeric);
				return m_DataStore.GetNumber();
			}

			float_type CodaScriptMUPValue::GetImag() const
			{
				return 0.0;
			}

			bool CodaScriptMUPValue::GetBool() const
			{
				CheckType(ICodaScriptDataStore::kDataType_Numeric);
				return m_DataStore.GetNumber();
			}

			const cmplx_type& CodaScriptMUPValue::GetComplex() const
			{
				SME_ASSERT(m_cType == 'c');
				return *(new cmplx_type());
			}

			const string_type& CodaScriptMUPValue::GetString() const
			{
				CheckType(ICodaScriptDataStore::kDataType_String);
				return m_StringBuffer;
			}

			const matrix_type& CodaScriptMUPValue::GetArray() const
			{
				SME_ASSERT(m_cType == 'c');
				return *(new matrix_type());
			}

			int CodaScriptMUPValue::GetRows() const
			{
				CheckType(ICodaScriptDataStore::kDataType_Array);
				return m_DataStore.GetArray()->Size();
			}

			int CodaScriptMUPValue::GetCols() const
			{
				return 0;
			}

			bool CodaScriptMUPValue::IsVariable() const
			{
				return false;
			}

			IToken* CodaScriptMUPValue::Clone() const
			{
				return new CodaScriptMUPValue(*this);
			}

			CodaScriptMUPValue* CodaScriptMUPValue::AsValue()
			{
				return this;
			}

			string_type CodaScriptMUPValue::AsciiDump() const
			{
				return "";
			}

			ICodaScriptDataStore* CodaScriptMUPValue::GetDataStore()
			{
				return &m_DataStore;
			}

			void CodaScriptMUPValue::SetIdentifier(const char* Identifier)
			{
				SetIdent(Identifier);
			}

			IValue& CodaScriptMUPValue::operator=(const IValue &ref)
			{
				Assign(ref);
				return *this;
			}

			char_type CodaScriptMUPValue::GetMUPType(ICodaScriptDataStore::DataType Type) const
			{
				switch (Type)
				{
				case ICodaScriptDataStore::kDataType_Invalid:
					return 'v';
				case ICodaScriptDataStore::kDataType_String:
					return 's';
				default:
					return 'f';
				}
			}


			//-----------------------------------------------------------------------------------------------
			CodaScriptMUPValue::operator int ()
			{
				return GetInteger();
			}

			//-----------------------------------------------------------------------------------------------
			CodaScriptMUPValue::operator string_type()
			{
				return GetString();
			}

			//-----------------------------------------------------------------------------------------------
			CodaScriptMUPValue::operator float_type()
			{
				return GetFloat();
			}

			//-----------------------------------------------------------------------------------------------
			CodaScriptMUPValue::operator bool()
			{
				return GetBool();
			}
		}
	}
}