#include "EditorBSExtraData.h"
#include "ExtenderInternals.h"

bool BaseExtraList::HasType(UInt32 type) const
{
	UInt32 index = (type >> 3);
	UInt8 bitMask = 1 << (type % 8);
	return (m_presenceBitfield[index] & bitMask) != 0;
}

BSExtraData * BaseExtraList::GetByType(UInt32 type) const
{
	if (!HasType(type)) return NULL;

	for(BSExtraData * traverse = m_data; traverse; traverse = traverse->next)
		if(traverse->type == type)
			return traverse;

	DebugPrint("ExtraData HasType(%d) is true but it wasn't found!", type);
	return NULL;
}

void BaseExtraList::MarkType(UInt32 type, bool bCleared)
{
	UInt32 index = (type >> 3);
	UInt8 bitMask = 1 << (type % 8);
	UInt8& flag = m_presenceBitfield[index];
	if (bCleared) {
		flag &= ~bitMask;
	} else {
		flag |= bitMask;
	}
}

bool BaseExtraList::Remove(BSExtraData* toRemove)
{
	if (!toRemove) return false;

	if (HasType(toRemove->type)) {
		bool bRemoved = false;
		if (m_data == toRemove) {
			m_data = m_data->next;
			bRemoved = true;
		}

		for (BSExtraData* traverse = m_data; traverse; traverse = traverse->next) {
			if (traverse->next == toRemove) {
				traverse->next = toRemove->next;
				bRemoved = true;
				break;
			}
		}
		if (bRemoved) {
			MarkType(toRemove->type, true);
		}
		return true;
	}

	return false;
}

bool BaseExtraList::RemoveByType(UInt32 type)
{
	if (HasType(type)) {
		return Remove(GetByType(type));
	}
	return false;
}

void BaseExtraList::RemoveAll()
{
	while (m_data) {
		BSExtraData* data = m_data;
		m_data = data->next;
		MarkType(data->type, true);
		FormHeap_Free(data);
	}
}

bool BaseExtraList::Add(BSExtraData* toAdd)
{
	if (!toAdd || HasType(toAdd->type)) return false;
	
	BSExtraData* next = m_data;
	m_data = toAdd;
	toAdd->next = next;
	MarkType(toAdd->type, false);
	return true;
}

bool BaseExtraList::IsWorn()
{
	return (HasType(kExtraData_Worn) || HasType(kExtraData_WornLeft));
}

void BaseExtraList::DebugDump()
{
	DebugPrint("BaseExtraList Dump:");
	gLog.Indent();

	if (m_data)
	{
		for(BSExtraData * traverse = m_data; traverse; traverse = traverse->next)
			DebugPrint("%s", GetObjectClassName(traverse));
	}
	else
		DebugPrint("No data in list");

	gLog.Outdent();
}


// new methods
BSExtraData*	BaseExtraList::ModExtraCharge(float charge)
{
	return (BSExtraData*)thisCall(0x0045BE20, this, charge);	
}

BSExtraData*	BaseExtraList::ModExtraCount(UInt32 count)
{
	return (BSExtraData*)thisCall(0x0045E2A0, this, count);	
}

BSExtraData*	BaseExtraList::ModExtraEnableStateParent(TESObjectREFR* parent)
{
	return (BSExtraData*)thisCall(0x0045CAA0, this, parent);
}

BSExtraData*	BaseExtraList::ModExtraEnableStateParentOppositeState(bool oppositeState)
{
	BSExtraData* xData = GetByType(kExtraData_EnableStateParent);
	if (xData) {
		ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
		if (xParent) {
			xParent->oppositeState = oppositeState;
		}
	}
	return xData;
}

BSExtraData*	BaseExtraList::ModExtraGlobal(TESGlobal* global)
{
	return (BSExtraData*)thisCall(0x0045E120, this, global);
}

BSExtraData*	BaseExtraList::ModExtraHealth(float health)
{
	return (BSExtraData*)thisCall(0x0045BC40, this, health);
}

BSExtraData*	BaseExtraList::ModExtraOwnership(TESForm* owner)
{
	return (BSExtraData*)thisCall(0x0045E060, this, owner);
}

BSExtraData*	BaseExtraList::ModExtraRank(int rank)
{
	return (BSExtraData*)thisCall(0x0045E1E0, this, rank);
}

BSExtraData*	BaseExtraList::ModExtraSoul(UInt32 soul)
{
	return (BSExtraData*)thisCall(0x0045BEC0, this, soul);
}

BSExtraData*	BaseExtraList::ModExtraTimeLeft(float timeLeft)
{
	return (BSExtraData*)thisCall(0x0045BD80, this, timeLeft);
}