#include "TESObject.h"

void TESObjectListHead::AddObject(TESObject* Object)
{
	thisCall<UInt32>(0x005135F0, this, Object);
}

UInt32 TESBoundObject::IncrementObjectRefCount()
{
	return thisVirtualCall<UInt32>(0x150, this);
}

UInt32 TESBoundObject::DecrementObjectRefCount()
{
	return thisVirtualCall<UInt32>(0x154, this);
}

void TESBoundObject::CalculateBounds(NiNode* Object3D)
{
	thisCall<void>(0x00513A80, this, Object3D);
}

NiNode* TESObject::GenerateNiNode(TESObjectREFR* Ref, bool Unk01)
{
	return thisVirtualCall<NiNode*>(0x134, this, Ref, Unk01);
}
