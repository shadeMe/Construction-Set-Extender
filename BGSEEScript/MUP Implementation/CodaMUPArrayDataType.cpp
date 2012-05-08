#include "CodaMUPArrayDataType.h"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		namespace mup
		{
			int		CodaScriptMUPArrayDataType::GIC = 0;

			void CodaScriptMUPArrayDataType::Copy( const CodaScriptMUPArrayDataType& Source )
			{
				this->DataStore = Source.DataStore;
			}

			template<typename ElementT>
			bool CodaScriptMUPArrayDataType::AddElement(ElementT Element, int Index)
			{
				if (Index == -1)
				{
					DataStore.push_back(CodaScriptMUPValue(Element));
					return true;
				}
				else if (Index < Size())
				{
					DataStore[Index] = CodaScriptMUPValue(Element);
					return true;
				}
				else
					return false;
			}

			CodaScriptMUPArrayDataType::CodaScriptMUPArrayDataType() :
				ICodaScriptObject(),
				ICodaScriptArrayDataType(),
				DataStore(5)
			{
				GIC++;
			}

			CodaScriptMUPArrayDataType::CodaScriptMUPArrayDataType( CodaScriptBackingStore* Elements, UInt32 Size ) :
				ICodaScriptObject(),
				ICodaScriptArrayDataType(),
				DataStore(Size)
			{
				GIC++;

				for (int i = 0; i < Size; i++)
					AddElement<const CodaScriptBackingStore&>(Elements[i], -1);
			}

			CodaScriptMUPArrayDataType::CodaScriptMUPArrayDataType( CodaScriptMUPArrayDataType* Source ) :
				ICodaScriptObject(),
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
				ICodaScriptObject(),
				ICodaScriptArrayDataType(),
				DataStore()
			{
				GIC++;

				Copy(rhs);
			}

			CodaScriptMUPArrayDataType::CodaScriptMUPArrayDataType( UInt32 Size ) :
				ICodaScriptObject(),
				ICodaScriptArrayDataType(),
				DataStore(Size, CodaScriptMUPValue(0.0))
			{
				GIC++;
			}

			CodaScriptMUPArrayDataType& CodaScriptMUPArrayDataType::operator=( const CodaScriptMUPArrayDataType& rhs )
			{
				Copy(rhs);

				return *this;
			}

			bool CodaScriptMUPArrayDataType::Insert(CodaScriptBackingStore* Data, int Index /* = -1 */)
			{
				return AddElement<const CodaScriptBackingStore&>(*Data, Index);
			}

			bool CodaScriptMUPArrayDataType::Insert(CodaScriptSharedHandleArrayT Data, int Index /* = -1 */)
			{
				return AddElement<CodaScriptSharedHandleArrayT>(Data, Index);
			}

			bool CodaScriptMUPArrayDataType::Insert(CodaScriptReferenceDataTypeT Data, int Index /* = -1 */)
			{
				return AddElement<CodaScriptReferenceDataTypeT>(Data, Index);
			}

			bool CodaScriptMUPArrayDataType::Insert(CodaScriptStringParameterTypeT Data, int Index /* = -1 */)
			{
				return AddElement<CodaScriptStringParameterTypeT>(Data, Index);
			}

			bool CodaScriptMUPArrayDataType::Insert(CodaScriptNumericDataTypeT Data, int Index /* = -1 */)
			{
				return AddElement<CodaScriptNumericDataTypeT>(Data, Index);
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