#pragma once

#include "TESObjectCELL.h"

//	EditorAPI: TESPathGrid and related classes.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

class	TESPathGrid;
class	TESPathGridPoint;

typedef tList<TESPathGridPoint> PathGridPointListT;

// 28
class TESPathGridPoint
{
public:
	enum PointFlags
	{
		kPointFlags_Preferred	= /*00*/ 0x1
	};

	// members
	/*00*/ UInt32				unk00;
	/*04*/ Vector3				position;	// strangely, the preferred flag is set in the 3rd member (position.z)
	/*10*/ PathGridPointListT	linkedPoints;
	/*18*/ NiNode*				pointNiNode;
	/*1C*/ UInt8				selected;
	/*1D*/ UInt8				pad1D[3];
	/*20*/ TESObjectREFR*		linkedRef;
	/*24*/ TESPathGrid*			parentGrid;

	// methods
	void						LinkPoint(TESPathGridPoint* Point);
	void						UnlinkPoint(TESPathGridPoint* Point);
	bool						GetIsPointLinked(TESPathGridPoint* Point);
	void						UnlinkAllPoints(void);
	void						GenerateNiNode(UInt32 Unk01 = 1);
	void						ShowSelectionRing(void);
	void						HideSelectionRing(void);

	void						LinkToReference(TESObjectREFR* Ref);
	void						UnlinkFromReference(void);

	static TESPathGridPoint*	CreateInstance(void);
	void						DeleteInstance(void);
};

// 50
class TESPathGrid : public TESForm, public TESChildCell
{
public:
	// 10
	// used for edges that cross exterior cell boundaries - maps local node to coordinates of external node
	struct ExternalEdge
	{
		/*00*/ UInt16			localNodeID;		// actually the index of the node in the parent grid's node array
		/*02*/ UInt8			pad02[2];
		/*04*/ Vector3			position;
	};
	typedef tList<ExternalEdge>	ExternalEdgeListT;

	// members
	//     /*00*/ TESForm
	//     /*24*/ TESChildCell
	/*28*/ NiNode*												gridNiNode;
	/*2C*/ TESObjectCELL*										parentCell;
	/*30*/ NiTArray<TESPathGridPoint*>*							gridPoints;
	/*34*/ ExternalEdgeListT									externalEdgeList;
	/*3C*/ UInt16												gridPointCount;
	/*3E*/ UInt8												pad3E[2];
	/*40*/ NiTMapBase<TESObjectREFR*, PathGridPointListT*>		linkedGridPoints;

	// methods
	void														AddPoint(TESPathGridPoint* Point);
	void														RemovePoint(TESPathGridPoint* Point);
	SInt32														HasPoint(TESPathGridPoint* Point);		// returns the index of the passed point in the point array. -1 when not found

	void														SetParentCell(TESObjectCELL* Cell);

	void														LinkPointToReference(TESPathGridPoint* Point, TESObjectREFR* Ref);
	void														UnlinkPointFromReference(TESPathGridPoint* Point, TESObjectREFR* Ref);
	void														CleanupLinkedReferences(void);

	void														GenerateNiNode(void);
	void														UpdateNiNode(void);
};