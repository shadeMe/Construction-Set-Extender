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
	if (linkedRef)
	{
		parentGrid->UnlinkPointFromReference(this, linkedRef);
		linkedRef = NULL;
	}
}

void TESPathGridPoint::GenerateNiNode( UInt32 Unk01 /*= 1*/ )
{
	thisCall<void>(0x00556280, this, this->parentGrid, Unk01);
}

void TESPathGridPoint::LinkToReference( TESObjectREFR* Ref )
{
	parentGrid->LinkPointToReference(this, Ref);
}

void TESPathGridPoint::UnlinkAllPoints( void )
{
	thisCall<void>(0x00555C70, this);
}

void TESPathGridPoint::ShowSelectionRing( void )
{
	thisCall<void>(0x00555ED0, this);
}

void TESPathGridPoint::HideSelectionRing( void )
{
	thisCall<void>(0x00555F80, this);
}

void TESPathGridPoint::DeleteInstance( void )
{
	thisCall<void>(0x00556190, this);
	FormHeap_Free(this);
}

TESPathGridPoint* TESPathGridPoint::CreateInstance( void )
{
	TESPathGridPoint* NewInstance = (TESPathGridPoint*)FormHeap_Allocate(sizeof(TESPathGridPoint));
	thisCall<void>(0x00555860, NewInstance);
	return NewInstance;
}

void TESPathGrid::LinkPointToReference( TESPathGridPoint* Point, TESObjectREFR* Ref )
{
	thisCall<void>(0x0054D4A0, this, Ref, Point);
}

void TESPathGrid::UnlinkPointFromReference( TESPathGridPoint* Point, TESObjectREFR* Ref )
{
	if (Ref)
	{
		tList<TESPathGridPoint>* LinkedNodes = NULL;
		if (thisCall<UInt32>(0x004ADB90, &linkedGridPoints, Ref, &LinkedNodes))	// NiTPointerMap_LookupByKey
		{
			thisCall<void>(0x00452AE0, LinkedNodes, Point);
			if (LinkedNodes->Count() == 0)
			{
				thisCall<UInt32>(0x004BCBD0, &linkedGridPoints, Ref);			// NiTPointerMap_Remove
				Ref->RemoveCrossReference(this);

				FormHeap_Free(LinkedNodes);
			}
		}
	}
}

void TESPathGrid::UpdateNiNode( void )
{
	thisCall<void>(0x006F28A0, this->gridNiNode);
	_RENDERCMPT->UpdateAVObject(gridNiNode);
}

void TESPathGrid::GenerateNiNode( void )
{
	thisCall<void>(0x0054EC40, this);
}

void TESPathGrid::AddPoint( TESPathGridPoint* Point )
{
	thisCall<void>(0x0054CE10, this, Point);
}

void TESPathGrid::RemovePoint( TESPathGridPoint* Point )
{
	thisCall<void>(0x0054E510, this, Point);
}

SInt32 TESPathGrid::HasPoint( TESPathGridPoint* Point )
{
	return thisCall<SInt32>(0x0054CDB0, this, Point);
}

void TESPathGrid::SetParentCell( TESObjectCELL* Cell )
{
	thisCall<void>(0x006EF2C0, this, Cell);
}

void TESPathGrid::CleanupLinkedReferences( void )
{
	thisCall<void>(0x0054D3F0, this);
}