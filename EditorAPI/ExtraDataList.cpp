#include "ExtraDataList.h"

CRITICAL_SECTION*					g_ExtraDataListMutex = (CRITICAL_SECTION*)0x00A0DA80;

void ExtraDataList::Link(TESForm* LinkedForm)
{
	thisCall<UInt32>(0x0045D740, this, LinkedForm);
}

void ExtraDataList::CopyList(ExtraDataList* Source)
{
	thisCall<UInt32>(0x00460380, this, Source);
}

BSExtraData* ExtraDataList::GetExtraDataByType( UInt8 Type )
{
	return thisCall<BSExtraData*>(0x0045B1B0, this, Type);
}

void ExtraDataList::ModExtraEnableStateParent( TESObjectREFR* Parent )
{
	thisCall<UInt32>(0x0045CAA0, this, Parent);
}

void ExtraDataList::ModExtraOwnership( TESForm* Owner )
{
	thisCall<UInt32>(0x0045E060, this, Owner);
}

void ExtraDataList::ModExtraGlobal( TESGlobal* Global )
{
	thisCall<UInt32>(0x0045E120, this, Global);
}

void ExtraDataList::ModExtraRank( int Rank )
{
	thisCall<UInt32>(0x0045E1E0, this, Rank);
}

void ExtraDataList::ModExtraCount( UInt32 Count )
{
	thisCall<UInt32>(0x0045E2A0, this, Count);
}