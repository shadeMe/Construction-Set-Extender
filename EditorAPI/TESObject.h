#pragma once

#include "TESForm.h"

//	EditorAPI: TESObject class and its derivatives.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	TESObject is the parent for all placeable form classes.  Most(all?) TESObjects are connected
	in a doubly-linked list, maintained by the global data handler.
	TESObject maintains info on the number of TESObjectREFR that use a form, and a list of cells in
	which those refs appear.  This complements the the normal Cross-Referencing of base forms in the CS.

	TESBoundObject is a subclass, used for (I think) all objects that can be independently rendered.
*/

class   TESObject;
class   TESObjectCELL;
class   TESObjectREFR;

// control structure for object list.  Seems to be BoundObjects in game, but includes static objects in CS.
// 10
class TESObjectListHead
{
public:
	// members
	/*00*/ UInt32        objectCount;
	/*04*/ TESObject*    first;
	/*08*/ TESObject*    last;
	/*0C*/ UInt32        unkC;

	// methods
	void				AddObject(TESObject* Object);
};
STATIC_ASSERT(sizeof(TESObjectListHead) == 0x10);

// 34
class TESObject : public TESForm
{
public:
	// members
	/*24*/ TESObjectListHead*	head;
	/*28*/ void*				unkObj28;			// for ref counting of loaded models (?)
													// struct {void* reflist; UInt32 modelUseCount?; BSSimpleList<UInt32> unk; ...?}
	/*2C*/ TESObject*			prev;
	/*30*/ TESObject*			next;
};
STATIC_ASSERT(sizeof(TESObject) == 0x34);

// size may be as large as 20, but probably is no larger than 08
// 08 ?
class TESCellUseList
{
public:
	struct CellUseInfo
	{
		TESObjectCELL*  cell;
		UInt32          count;
	};
	typedef tList<CellUseInfo>		CellUseInfoListT;

	// members
	/*00*/ CellUseInfoListT			cellUses;
};
STATIC_ASSERT(sizeof(TESCellUseList) == 0x08);

// 58
class TESBoundObject : public TESObject, public TESCellUseList
{
public:
	// members
	//     /*00*/ TESObject
	//     /*34*/ TESCellUseList
	/*3C*/ Vector3					center;				// filled in as requested
	/*48*/ Vector3					extents;			// filled in as requested
	/*54*/ UInt32					objectRefrCount;	// count of TESObjectREFR using this form

	// methods
	UInt32							IncrementObjectRefCount();
	UInt32							DecrementObjectRefCount();
	void							CalculateBounds(NiNode* Object3D);
};
STATIC_ASSERT(sizeof(TESBoundObject) == 0x58);

// 58
class TESBoundAnimObject : public TESBoundObject
{
public:
	// no additional members
};
STATIC_ASSERT(sizeof(TESBoundAnimObject) == 0x58);

// 58
class TESBoundTreeObject : public TESBoundObject
{
public:
	// no additional members
};
STATIC_ASSERT(sizeof(TESBoundTreeObject) == 0x58);