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