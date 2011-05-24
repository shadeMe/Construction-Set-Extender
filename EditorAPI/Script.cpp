#include "Script.h"

namespace EditorAPI
{
	Script::VariableInfo* Script::LookupVariableInfoByName(const char* Name)
	{
		for (VariableListT::Iterator Itr = varList.Begin(); !Itr.End(); ++Itr)
		{
			VariableInfo* Variable = Itr.Get();

			if (!Variable->name.Compare(Name))
				return Variable;
		}

		return NULL;
	}

	Script::RefVariable* Script::LookupRefVariableByIndex(UInt32 Index)
	{
		UInt32 Idx = 1;	// yes, really starts at 1

		for (RefVariableListT::Iterator Itr = refList.Begin(); !Itr.End(); ++Itr)
		{
			RefVariable* Variable = Itr.Get();

			if (Idx == Index)
				return Variable;

			Idx++;
		}

		return NULL;
	}
}