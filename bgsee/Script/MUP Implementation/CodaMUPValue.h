#pragma once

#include "mpIValue.h"
#include "mpTypes.h"
#include "CodaDataTypes.h"

namespace bgsee
{
	namespace script
	{
		namespace mup
		{
			// a wrapper for CodaScriptBackingStore
			class CodaScriptMUPValue : public ICodaScriptDataStoreOwner, public IValue
			{
				static int									GIC;
			protected:
				char_type									m_cType;				///< A byte indicating the type of the represented value
				EFlags										m_iFlags;				///< Additional flags
				ValueCache*									m_pCache;				///< Pointer to the Value Cache
				mutable CodaScriptBackingStore				m_DataStore;			///< Actual container for the data
				string_type									m_StringBuffer;			///< Storage buffer for GetString() calls

				void										CheckType(char_type a_cType) const;
				void										CheckType(ICodaScriptDataStore::DataType a_cType) const;
				void										Assign(const CodaScriptMUPValue &a_Val);
				void										Assign(const IValue& a_Val);
				void										Assign(const CodaScriptBackingStore& a_Val);
				char_type									GetMUPType(ICodaScriptDataStore::DataType Type) const;

				virtual void								Release();
			public:
				explicit CodaScriptMUPValue(char_type cType = 'v');
				explicit CodaScriptMUPValue(float_type val);
				explicit CodaScriptMUPValue(string_type val);
				explicit CodaScriptMUPValue(const char_type *val);
				explicit CodaScriptMUPValue(CodaScriptBackingStore* val);
				explicit CodaScriptMUPValue(CodaScriptReferenceDataTypeT val);
				explicit CodaScriptMUPValue(ICodaScriptArrayDataType::SharedPtrT val);
				explicit CodaScriptMUPValue(const CodaScriptBackingStore& val);
				explicit CodaScriptMUPValue(const CodaScriptMUPValue& a_Val );
				explicit CodaScriptMUPValue(const IValue &a_Val);
				virtual ~CodaScriptMUPValue();

				virtual IValue&								At(int nRow, int nCol = 0);
				virtual IValue&								At(const IValue &row, const IValue &col);

				CodaScriptMUPValue&							operator=(const CodaScriptMUPValue &a_Val);
				virtual IValue&								operator=(const IValue &ref);
				virtual IValue&								operator=(int_type a_iVal);
				virtual IValue&								operator=(float_type a_fVal);
				virtual IValue&								operator=(string_type a_sVal);
				virtual IValue&								operator=(bool val);
				virtual IValue&								operator=(const matrix_type &a_vVal);
				virtual IValue&								operator=(const cmplx_type &val);
				virtual IValue&								operator=(const char_type *a_szVal);
				virtual IValue&								operator+=(const IValue &val);
				virtual IValue&								operator-=(const IValue &val);
				virtual IValue&								operator*=(const IValue &val);

				virtual char_type							GetType() const;
				virtual int_type							GetInteger() const;
				virtual float_type							GetFloat() const;
				virtual float_type							GetImag() const;
				virtual bool								GetBool() const;
				virtual const cmplx_type&					GetComplex() const;
				virtual const string_type&					GetString() const;
				virtual const matrix_type&					GetArray() const;
				virtual int									GetRows() const;
				virtual int									GetCols() const;

				virtual bool								IsVariable() const;
				virtual IToken*								Clone() const;
				virtual CodaScriptMUPValue*					AsValue();
				virtual string_type							AsciiDump() const;
				virtual CodaScriptBackingStore*				GetStore(void) const;
				void										BindToCache(ValueCache *pCache);

				virtual ICodaScriptDataStore*				GetDataStore();
				virtual ICodaScriptDataStoreOwner&			operator=(const ICodaScriptDataStore& rhs);
				virtual void								SetIdentifier(const char* Identifier);

				static const int&							GetGIC() { return GIC; }

				// Conversion operators
				operator int ();
				operator string_type();
				operator float_type();
				operator bool();
			};
		}
	}
}