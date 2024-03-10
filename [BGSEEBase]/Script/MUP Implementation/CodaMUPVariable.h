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
			// a second level of indirection b'ween a regular MUP Variable and a CodaScriptVariable
			// allows the rebinding of variables after the bytecode has been generated
			class CodaScriptMUPVariable : public IValue
			{
				static int					GIC;
			protected:
				typedef std::stack<CodaScriptMUPValue*>			ValueStackT;

				CodaScriptSourceCodeT		Name;
				ValueStackT					BoundValues;
				CodaScriptMUPValue*			CurrentValue;					// the value on the top of the stack, if any
				bool						RestrictedAssignment;			// when true, references and arrays can't be assigned to this variable

				CodaScriptMUPValue*			GetCurrentValue() const;
			public:
				CodaScriptMUPVariable(const CodaScriptSourceCodeT& Name, bool RestrictedAssignment);
				CodaScriptMUPVariable(const CodaScriptMUPVariable &a_Var);
				virtual ~CodaScriptMUPVariable();

				virtual IValue&				At(int nRow, int nCol);
				virtual IValue&				At(const IValue &nRows, const IValue &nCols);

				virtual IValue&				operator=(const CodaScriptMUPValue &val);
				virtual IValue&				operator=(const matrix_type &val);
				virtual IValue&				operator=(const cmplx_type &val);
				virtual IValue&				operator=(int_type val);
				virtual IValue&				operator=(float_type val);
				virtual IValue&				operator=(string_type val);
				virtual IValue&				operator=(bool_type val);
				virtual IValue&				operator=(const IValue &ref);

				virtual IValue&				operator+=(const IValue &ref);
				virtual IValue&				operator-=(const IValue &ref);
				virtual IValue&				operator*=(const IValue &val);


				virtual char_type			GetType() const;

				virtual int_type			GetInteger() const;
				virtual float_type			GetFloat() const;
				virtual float_type			GetImag() const;
				virtual bool				GetBool() const;
				virtual const cmplx_type&	GetComplex() const;
				virtual const string_type&	GetString() const;
				virtual const matrix_type&	GetArray() const;
				virtual int					GetRows() const;
				virtual int					GetCols() const;

				virtual bool						IsVariable() const;
				virtual IToken*						Clone() const;
				virtual CodaScriptMUPValue*			AsValue();
				virtual CodaScriptBackingStore*		GetStore(void) const;

				const CodaScriptSourceCodeT&		GetName() const;
				void								Bind(const CodaScriptVariable* Var);
				void								Unbind();
				bool								IsBound() const;

				static const int&					GetGIC() { return GIC; }

				typedef std::unique_ptr<CodaScriptMUPVariable>				PtrT;
			};

		}
	}
}