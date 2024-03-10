#pragma once

#include "CodaMUPValue.h"

namespace bgsee
{
	namespace script
	{
		namespace mup
		{
			class CodaScriptMUPArrayDataType : public ICodaScriptArrayDataType
			{
				static int												GIC;
			protected:
				typedef std::vector<CodaScriptMUPValue>					MutableElementArrayT;

				MutableElementArrayT									DataStore;

				void													Copy(const CodaScriptMUPArrayDataType& Source);
				template<typename ElementT>
				bool													AddElement(ElementT Element, int Index, bool Replace);
			public:
				CodaScriptMUPArrayDataType();
				CodaScriptMUPArrayDataType(UInt32 Size);
				CodaScriptMUPArrayDataType(CodaScriptMUPArrayDataType* Source);
				CodaScriptMUPArrayDataType(CodaScriptBackingStore* Elements, UInt32 Size);
				~CodaScriptMUPArrayDataType();

				CodaScriptMUPArrayDataType(const CodaScriptMUPArrayDataType& rhs);
				CodaScriptMUPArrayDataType& operator=(const CodaScriptMUPArrayDataType& rhs);

				virtual bool											Insert(CodaScriptBackingStore* Data, int Index = -1, bool Replace = true);
				virtual bool											Insert(CodaScriptNumericDataTypeT Data, int Index = -1, bool Replace = true);
				virtual bool											Insert(CodaScriptStringParameterTypeT Data, int Index = -1, bool Replace = true);
				virtual bool											Insert(CodaScriptReferenceDataTypeT Data, int Index = -1, bool Replace = true);
				virtual bool											Insert(ICodaScriptArrayDataType::SharedPtrT Data, int Index = -1, bool Replace = true);

				virtual bool											Erase(UInt32 Index);
				virtual void											Clear(void);

				virtual bool											At(UInt32 Index, CodaScriptBackingStore& OutBuffer) const;
				virtual bool											At(UInt32 Index, CodaScriptMUPValue** OutBuffer) const;
				virtual UInt32											Size(void) const;

				static const int&										GetGIC() { return GIC; }
			};
		}
	}
}