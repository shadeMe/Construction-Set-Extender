#include "mpVariable.h"
#include "mpError.h"
#include "CodaMUPValue.h"
#include "CodaMUPVariable.h"
#include "CodaUtilities.h"

namespace bgsee
{
	namespace script
	{
		namespace mup
		{

			int CodaScriptMUPVariable::GIC = 0;

			CodaScriptMUPValue* CodaScriptMUPVariable::GetCurrentValue() const
			{
				if (BoundValues.size() == 0)
					return nullptr;
				else
					return BoundValues.top();
			}

			//-----------------------------------------------------------------------------------------------
			/** \brief Create a variable and bind a value to it.
				\param pVal Pointer of the value to bind to this variable.

			  It is possible to create an empty variable object by setting pVal to null.
			  Such variable objects must be bound later in order to be of any use. The parser
			  does NOT assume ownership over the pointer!
			*/
			CodaScriptMUPVariable::CodaScriptMUPVariable(const CodaScriptSourceCodeT& Name, bool RestrictedAssignment)
				:IValue(cmVAL)
				, Name(Name), BoundValues(), CurrentValue(nullptr), RestrictedAssignment(RestrictedAssignment)
			{
				AddFlags(IToken::flVOLATILE);
				GIC++;
			}

			CodaScriptMUPVariable::CodaScriptMUPVariable(const CodaScriptMUPVariable &obj)
				:IValue(cmVAL),
				Name(obj.Name),
				BoundValues(obj.BoundValues),
				CurrentValue(obj.CurrentValue),
				RestrictedAssignment(obj.RestrictedAssignment)
			{
				AddFlags(IToken::flVOLATILE);
				GIC++;
			}

			//-----------------------------------------------------------------------------------------------
			/** \brief Assign a value to the variable.
				\param ref Reference to the value to be assigned
			*/
			IValue& CodaScriptMUPVariable::operator=(const CodaScriptMUPValue &ref)
			{
				return operator=(static_cast<const IValue&>(ref));
			}

			IValue& CodaScriptMUPVariable::operator=(const IValue &ref)
			{
				SME_ASSERT(CurrentValue);
				ICodaScriptDataStore* Store = const_cast<IValue&>(ref).GetStore();
				if (RestrictedAssignment && (Store->GetIsArray() || Store->GetIsReference()))
				{
					char Buffer[0x100] = { 0 };
					FORMAT_STR(Buffer, "Cannot assign value of type '%c' to restricted variable '%s'", Store->GetType(), Name.c_str());
					throw ParserError(Buffer);
				}
				*CurrentValue = ref;
				return *this;
			}


			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::operator=(int_type val)
			{
				SME_ASSERT(CurrentValue);
				if (RestrictedAssignment)
				{
					char Buffer[0x100] = { 0 };
					FORMAT_STR(Buffer, "Cannot assign value of type '%c' to restricted variable '%s'",
							   ICodaScriptDataStore::DataType::kDataType_Reference, Name.c_str());
					throw ParserError(Buffer);
				}

				return CurrentValue->operator=(val);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::operator=(float_type val)
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->operator=(val);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::operator=(string_type val)
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->operator=(val);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::operator=(bool_type val)
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->operator=(val);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::operator=(const matrix_type &val)
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->operator=(val);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::operator=(const cmplx_type &val)
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->operator=(val);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::operator+=(const IValue &val)
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->operator+=(val);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::operator-=(const IValue &val)
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->operator-=(val);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::operator*=(const IValue &val)
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->operator*=(val);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::At(int nRow, int nCol)
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->At(nRow, nCol);
			}

			//-----------------------------------------------------------------------------------------------
			IValue& CodaScriptMUPVariable::At(const IValue &row, const IValue &col)
			{
				SME_ASSERT(CurrentValue);
				try
				{
					return CurrentValue->At(row, col);
				}
				catch (ParserError &exc)
				{
					// add the identifier to the error context
					exc.GetContext().Ident = GetIdent();
					throw exc;
				}
			}

			//-----------------------------------------------------------------------------------------------
			CodaScriptMUPVariable::~CodaScriptMUPVariable()
			{
				SME_ASSERT(GIC-- > 0);
			}

			//-----------------------------------------------------------------------------------------------
			/** \brief Returns a character representing the type of the variable.
				\throw nothrow
			*/
			char_type CodaScriptMUPVariable::GetType() const
			{
				return (CurrentValue) ? CurrentValue->GetType() : 'v';
			}

			//-----------------------------------------------------------------------------------------------
			int_type CodaScriptMUPVariable::GetInteger() const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetInteger();
			}

			//-----------------------------------------------------------------------------------------------
			float_type CodaScriptMUPVariable::GetFloat() const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetFloat();
			}

			//-----------------------------------------------------------------------------------------------
			float_type CodaScriptMUPVariable::GetImag() const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetImag();
			}

			//-----------------------------------------------------------------------------------------------
			const cmplx_type& CodaScriptMUPVariable::GetComplex() const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetComplex();
			}

			//-----------------------------------------------------------------------------------------------
			const string_type& CodaScriptMUPVariable::GetString() const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetString();
			}

			//-----------------------------------------------------------------------------------------------
			bool CodaScriptMUPVariable::GetBool() const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetBool();
			}

			//-----------------------------------------------------------------------------------------------
			const matrix_type& CodaScriptMUPVariable::GetArray() const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetArray();
			}

			//-----------------------------------------------------------------------------------------------
			int CodaScriptMUPVariable::GetRows() const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetRows();
			}

			//-----------------------------------------------------------------------------------------------
			int CodaScriptMUPVariable::GetCols() const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetCols();
			}

			//-----------------------------------------------------------------------------------------------
			bool CodaScriptMUPVariable::IsVariable() const
			{
				return false;
			}

			//-----------------------------------------------------------------------------------------------
			IToken* CodaScriptMUPVariable::Clone() const
			{
				return new CodaScriptMUPVariable(*this);
			}

			//-----------------------------------------------------------------------------------------------
			CodaScriptMUPValue* CodaScriptMUPVariable::AsValue()
			{
				SME_ASSERT(IsBound());
				return CurrentValue;
			}

			CodaScriptBackingStore* CodaScriptMUPVariable::GetStore(void) const
			{
				SME_ASSERT(CurrentValue);
				return CurrentValue->GetStore();
			}

			const CodaScriptSourceCodeT& CodaScriptMUPVariable::GetName() const
			{
				return Name;
			}

			void CodaScriptMUPVariable::Bind(const CodaScriptVariable* Var)
			{
#ifdef _DEBUG
				bool NameMismatch = _stricmp(Var->GetName(), Name.c_str());
				SME_ASSERT(NameMismatch == false);
#endif // _DEBUG

				CodaScriptMUPValue* NewVal = dynamic_cast<CodaScriptMUPValue*>(Var->GetStoreOwner());
				SME_ASSERT(NewVal);

				BoundValues.push(NewVal);
				CurrentValue = GetCurrentValue();
			}

			void CodaScriptMUPVariable::Unbind()
			{
				SME_ASSERT(BoundValues.empty() == false);

				BoundValues.pop();
				CurrentValue = GetCurrentValue();
			}

			bool CodaScriptMUPVariable::IsBound() const
			{
				return CurrentValue != nullptr;
			}

		}
	}
}