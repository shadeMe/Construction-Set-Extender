#include "TESObjectCELL.h"

TESWorldSpace* TESObjectCELL::GetParentWorldSpace()
{
	return thisCall<TESWorldSpace*>(0x00532E50, this);
}

bool TESObjectCELL::GetIsInterior()
{
	return thisCall<bool>(0x00532240, this);
}

TESObjectREFR* TESObjectCELL::FindFirstRef( TESForm* BaseForm, bool MustHaveEditorID /*= false*/ )
{
	for (ObjectREFRList::Iterator Itr = objectList.Begin(); !Itr.End() && Itr.Get() && BaseForm; ++Itr)
	{
		TESObjectREFR* Ref = Itr.Get();

		if (Ref->baseForm == BaseForm)
		{
			if (MustHaveEditorID == false || Ref->editorID.c_str())
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

void TESObjectCELL::RemoveObjectReference( TESObjectREFR* Ref )
{
	thisCall<void>(0x00534910, this, Ref);
}

void TESObjectCELL::AddObjectReference( TESObjectREFR* Ref )
{
	thisCall<void>(0x00539170, this, Ref);
}

const char* TESObjectCELL::GetEditorID() const
{
	return thisVirtualCall<const char*>(0xC8, this);
}

TESObjectLAND* TESObjectCELL::GetLand()
{
	return thisCall<TESObjectLAND*>(0x005340A0, this);
}