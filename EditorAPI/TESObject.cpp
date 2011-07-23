#include "TESObject.h"

void TESObjectListHead::AddObject(TESObject* Object)
{
	thisCall<UInt32>(0x005135F0, this, Object);
}