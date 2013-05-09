#pragma once

#include "mpIValue.h"
#include "mpTypes.h"
#include "CodaDataTypes.h"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		namespace mup
		{
			// a wrapper for CodaScriptBackingStore
			class CodaScriptMUPValue : public ICodaScriptDataStoreOwner, public IValue
			{
				friend class CodaScriptVM;

				static int									GIC;
			protected:
				char_type									m_cType;				///< A byte indicating the type of the represented value
				EFlags										m_iFlags;				///< Additional flags
				ValueCache*									m_pCache;				///< Pointer to the Value Cache
				mutable CodaScriptBackingStore				m_DataStore;			///< Actual container for the data
				std::auto_ptr<string_type>					m_StringBuffer;			///< Storage buffer for GetString() calls

				void										CheckType(char_type a_cType) const;
				void										CheckType(ICodaScriptDataStore::DataType a_cType) const;
				void										Assign(const CodaScriptMUPValue &a_Val);
				char_type									GetMUPType(ICodaScriptDataStore::DataType Type) const;

				virtual void								Release();
			public:
				explicit CodaScriptMUPValue(char_type cType = 'v');
				CodaScriptMUPValue(float_type val);
				CodaScriptMUPValue(string_type val);
				CodaScriptMUPValue(const char_type *val);
				CodaScriptMUPValue(CodaScriptBackingStore* val);
				CodaScriptMUPValue(CodaScriptReferenceDataTypeT val);
				CodaScriptMUPValue(CodaScriptSharedHandleArrayT val);
				CodaScriptMUPValue(const CodaScriptBackingStore& val);
				CodaScriptMUPValue(const CodaScriptMUPValue& a_Val );
				CodaScriptMUPValue(const IValue &a_Val);
				CodaScriptMUPValue& operator=(const CodaScriptMUPValue &a_Val);
				virtual ~CodaScriptMUPValue();

				virtual IValue&								At(int nRow, int nCol = 0);
				virtual IValue&								At(const IValue &row, const IValue &col);

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

				virtual bool								IsVolatile() const;
				virtual IToken*								Clone() const;
				virtual CodaScriptMUPValue*					AsValue();
				virtual string_type							AsciiDump() const;
				virtual CodaScriptBackingStore*				GetStore(void) const;
				void										BindToCache(ValueCache *pCache);

				virtual ICodaScriptDataStore*				GetDataStore();
				virtual ICodaScriptDataStoreOwner&			operator=(const ICodaScriptDataStore& rhs);
			};
		}
	}
}