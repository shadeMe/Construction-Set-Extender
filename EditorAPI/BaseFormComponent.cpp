#include "BaseFormComponent.h"

bool TESLeveledList::WalkForCircularPath( std::string& Output, TESLeveledList* Check, TESLeveledList* Against )
{
	bool Result = true;
	
	TESForm* CheckBase = CS_CAST(Check, TESLeveledList, TESForm);
	TESForm* CheckAgainstBase = CS_CAST(Against, TESLeveledList, TESForm);
	SME_ASSERT(CheckBase && CheckAgainstBase);	

	for (TESLeveledList::LevListT::Iterator Itr = Check->levList.Begin(); Itr.Get() && Itr.End() == false; ++Itr)
	{
		TESForm* Item = Itr->form;
		TESLeveledList* Inner = CS_CAST(Item, TESForm, TESLeveledList);

		if (Item->formID == CheckAgainstBase->formID)
		{
			// the source lev list is an item
			Output += CheckBase->GetEditorID();
			Output += " >";
			Output += CheckAgainstBase->GetEditorID();

			Result = false;
			break;
		}
		else if (Inner)
		{
			std::string TempStr = Output + CheckBase->GetEditorID();
			TempStr += " >";
			
			// recurse
			if (WalkForCircularPath(TempStr, Inner, Against) == false)
			{
				Output = TempStr;
				Result = false;
				break;
			}
		}
	}

	return Result;
}

bool TESLeveledList::CheckForCircularPaths( std::string& Output )
{
	return WalkForCircularPath(Output, this, this);
}