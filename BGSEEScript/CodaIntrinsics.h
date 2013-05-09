#pragma once

#include "CodaForwardDecls.inl"
#include "CodaBaseInterfaces.inl"

namespace BGSEditorExtender
{
	namespace BGSEEScript
	{
		template<typename T>
		class CodaScriptSimpleInstanceCounter
		{
			int&										BaseGIC;
			int											InitGIC;
		public:
			CodaScriptSimpleInstanceCounter() :
			  BaseGIC(T::GIC), InitGIC(BaseGIC)
			  {
				  ;//
			  }

			  ~CodaScriptSimpleInstanceCounter()
			  {
				  ;//
			  }

			  int	GetCount(void) const
			  {
				  return BaseGIC - InitGIC;
			  }
		};
	}
}