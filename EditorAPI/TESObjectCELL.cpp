#include "TESObjectCELL.h"

TESWorldSpace* TESObjectCELL::GetParentWorldSpace()
{
	return thisCall<TESWorldSpace*>(0x00532E50, this);
}

bool TESObjectCELL::GetIsInterior()
{
	return thisCall<bool>(0x00532240, this);
}

TESObjectREFR* TESObjectCELL::LookupRefByBaseForm( TESForm* BaseForm, bool HasEditorID /*= false*/ )
{
	for (ObjectREFRList::Iterator Itr = objectList.Begin(); !Itr.End() && Itr.Get() && BaseForm; ++Itr)
	{
		TESObjectREFR* Ref = Itr.Get();

		if (Ref->baseForm == BaseForm)
		{
			if (!HasEditorID || Ref->editorID.c_str())
				return Ref;
		}
	}

	return NULL;
}