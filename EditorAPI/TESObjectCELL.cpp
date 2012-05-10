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

void TESObjectCELL::SetPathGrid( TESPathGrid* PathGrid )
{
	thisCall<void>(0x00532C20, this, PathGrid);
}

void TESObjectCELL::ModExtraCellWaterHeight( float Height )
{
	thisCall<void>(0x00532D30, this, Height);
}

void TESObjectCELL::ModExtraCellWaterType( TESWaterForm* Water )
{
	thisCall<void>(0x00534420, this, Water);
}
