#include "TESConditionItem.h"

UInt32 TESConditionItem::GetScriptableFormConditionCount( ConditionListT* ConditionList, TESForm* FormToCompare )
{
	UInt32 ScriptableConditions = 0;

	for (ConditionListT::Iterator Itr = ConditionList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		TESConditionItem* Condition = Itr.Get();

		if (Condition->functionIndex == (53 & 0x0FFF) || Condition->functionIndex == (79 & 0x0FFF))			// GetScriptVariable || GetQuestVariable
		{
			if (FormToCompare && Condition->param1.form == FormToCompare)
				ScriptableConditions++;
			else if (!FormToCompare)
				ScriptableConditions++;
		}
	}

	return ScriptableConditions;
}