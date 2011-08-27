#include "TESPathGrid.h"

void TESPathGridPoint::LinkPoint( TESPathGridPoint* Point )
{
	thisCall<void>(0x00555B90, this, Point);
}

void TESPathGridPoint::UnlinkPoint( TESPathGridPoint* Point )
{
	thisCall<void>(0x00555C00, this, Point);
}

bool TESPathGridPoint::GetIsPointLinked( TESPathGridPoint* Point )
{
	return thisCall<bool>(0x00555AD0, this, Point);
}

void TESPathGridPoint::UnlinkFromReference( void )
{
	NiTMapBase<TESObjectREFR*, tList<TESPathGridPoint>*>* LinkedRefMap = &parentGrid->linkedGridPoints;

	if (linkedRef)
	{
		tList<TESPathGridPoint>* LinkedNodes = NULL;
		if (thisCall<UInt32>(0x004ADB90, LinkedRefMap, linkedRef, &LinkedNodes))	// NiTPointerMap_LookupByKey
		{
			LinkedNodes->Remove(this);
			if (LinkedNodes->Count() == 0)
			{
				thisCall<UInt32>(0x004BCBD0, LinkedRefMap, linkedRef);				// NiTPointerMap_Remove
				FormHeap_Free(LinkedNodes);
			}

			linkedRef = NULL;
		}
	}
}