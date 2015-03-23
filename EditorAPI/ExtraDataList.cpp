#include "ExtraDataList.h"

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

void ExtraDataList::ModExtraDistantData(TESObjectLAND* Land, Vector3* Position)
{
	thisCall<void>(0x0045F1D0, this, Land, Position);
}

void BaseExtraList::AddExtra( BSExtraData* xData )
{
	thisCall<void>(0x0045B0D0, this, xData);
}

void BaseExtraList::RemoveExtra( UInt8 Type )
{
	thisCall<void>(0x0045B140, this, Type);
}

void BaseExtraList::Dtor(void)
{
	thisVirtualCall<void>(0x0, this, true);
}