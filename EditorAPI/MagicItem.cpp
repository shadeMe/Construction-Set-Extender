#include "MagicItem.h"

UInt32 MagicItem::GetScriptReferences(MagicItemScriptCrossRefListT& OutList)
{
	OutList.clear();

	for (EffectItemList::EffectItemListT::Iterator Itr = effects.Begin(); Itr.Get() && !Itr.End(); ++Itr)
	{
		EffectItem* Current = Itr.Get();
		if (Current->scriptInfo && Current->scriptInfo->scriptFormID)
		{
			TESForm* Script = TESForm::LookupByFormID(Current->scriptInfo->scriptFormID);
			if (Script)
				OutList.push_back(Script);
		}
	}

	return OutList.size();
}