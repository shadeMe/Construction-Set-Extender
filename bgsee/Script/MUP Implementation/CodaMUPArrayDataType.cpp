#include "CodaMUPArrayDataType.h"

namespace bgsee
{
	namespace script
	{
		namespace mup
		{
			int		CodaScriptMUPArrayDataType::GIC = 0;

			void CodaScriptMUPArrayDataType::Copy( const CodaScriptMUPArrayDataType& Source )
			{
				this->DataStore = Source.DataStore;
			}

			template<typename ElementT>
			bool CodaScriptMUPArrayDataType::AddElement(ElementT Element, int Index, bool Replace)
			{
				if (Index == -1)
				{
					DataStore.push_back(CodaScriptMUPValue(Element));
					return true;
				}
				else if (Index < Size())
				{
					if (Replace)
						DataStore[Index] = CodaScriptMUPValue(Element);
					else
						DataStore.emplace(DataStore.begin() + Index, CodaScriptMUPValue(Element));

					return true;
				}
				else
					return false;
			}

			CodaScriptMUPArrayDataType::CodaScriptMUPArrayDataType() :
				ICodaScriptArrayDataType(),
				DataStore()
			{
				GIC++;

				DataStore.reserve(5);
			}

			CodaScriptMUPArrayDataType::CodaScriptMUPArrayDataType( CodaScriptBackingStore* Elements, UInt32 Size ) :
				ICodaScriptArrayDataType(),
				DataStore(Size)
			{
				GIC++;

				for (int i = 0; i < Size; i++)
					AddElement<const CodaScriptBackingStore&>(Elements[i], -1, true);
			}

			CodaScriptMUPArrayDataType::CodaScriptMUPArrayDataType( CodaScriptMUPArrayDataType* Source ) :
				ICodaScriptArrayDataType(),
				DataStore()
			{
				GIC++;

				Copy(*Source);
			}

			CodaScriptMUPArrayDataType::~CodaScriptMUPArrayDataType()
			{
				Clear();

				GIC--;
				SME_ASSERT(GIC >= 0);
			}

			CodaScriptMUPArrayDataType::CodaScriptMUPArrayDataType( const CodaScriptMUPArrayDataType& rhs ) :
				ICodaScriptArrayDataType(),
				DataStore()
			{
				GIC++;

				Copy(rhs);
			}

			CodaScriptMUPArrayDataType::CodaScriptMUPArrayDataType( UInt32 Size ) :
				ICodaScriptArrayDataType(),
				DataStore()
			{
				GIC++;

				DataStore.reserve(Size);
			}

			CodaScriptMUPArrayDataType& CodaScriptMUPArrayDataType::operator=( const CodaScriptMUPArrayDataType& rhs )
			{
				Copy(rhs);

				return *this;
			}

			bool CodaScriptMUPArrayDataType::Insert(CodaScriptBackingStore* Data, int Index /* = -1 */, bool Replace /* = true */)
			{
				return AddElement<const CodaScriptBackingStore&>(*Data, Index, Replace);
			}

			bool CodaScriptMUPArrayDataType::Insert(ICodaScriptArrayDataType::SharedPtrT Data, int Index /* = -1 */, bool Replace /* = true */)
			{
				return AddElement<ICodaScriptArrayDataType::SharedPtrT>(Data, Index, Replace);
			}

			bool CodaScriptMUPArrayDataType::Insert(CodaScriptReferenceDataTypeT Data, int Index /* = -1 */, bool Replace /* = true */)
			{
				return AddElement<CodaScriptReferenceDataTypeT>(Data, Index, Replace);
			}

			bool CodaScriptMUPArrayDataType::Insert(CodaScriptStringParameterTypeT Data, int Index /* = -1 */, bool Replace /* = true */)
			{
				return AddElement<CodaScriptStringParameterTypeT>(Data, Index, Replace);
			}

			bool CodaScriptMUPArrayDataType::Insert(CodaScriptNumericDataTypeT Data, int Index /* = -1 */, bool Replace /* = true */)
			{
				return AddElement<CodaScriptNumericDataTypeT>(Data, Index, Replace);
			}

			bool CodaScriptMUPArrayDataType::Erase( UInt32 Index )
			{
				if (Index >= Size())
					return false;

				MutableElementArrayT::iterator Itr = DataStore.begin() + Index;
				DataStore.erase(Itr);

				return true;
			}

			void CodaScriptMUPArrayDataType::Clear( void )
			{
				DataStore.clear();
			}

			bool CodaScriptMUPArrayDataType::At( UInt32 Index, CodaScriptBackingStore& OutBuffer ) const
			{
				if (Index >= Size())
					return false;

				OutBuffer = *(DataStore[Index].GetStore());
				return true;
			}

			bool CodaScriptMUPArrayDataType::At( UInt32 Index, CodaScriptMUPValue** OutBuffer ) const
			{
				if (Index >= Size())
					return false;

				SME_ASSERT(OutBuffer);

				*OutBuffer = const_cast<CodaScriptMUPValue*>(&(DataStore[Index]));
				return true;
			}

			UInt32 CodaScriptMUPArrayDataType::Size( void ) const
			{
				return DataStore.size();
			}
		}
	}
}