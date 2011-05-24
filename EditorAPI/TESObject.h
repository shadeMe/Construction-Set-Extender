#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"

//	EditorAPI: TESObject class and its derivatives.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	TESObject is the parent for all placeable form classes.  Most(all?) TESObjects are connected 
	in a doubly-linked list, maintained by the global data handler.
	TESObject maintains info on the number of TESObjectREFR that use a form, and a list of cells in 
	which those refs appear.  This complements the the normal Cross-Referencing of base forms in the CS. 

	TESBoundObject is a subclass, used for (I think) all objects that can be independently rendered.
*/

namespace EditorAPI
{
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
	};

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
    
		// members
		/*00*/ tList<CellUseInfo>		cellUses;
	};

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
	};

	// 58
	class TESBoundAnimObject : public TESBoundObject
	{
	public:
		// no additional members
	};

	// 58
	class TESBoundTreeObject : public TESBoundObject
	{
	public:
		// no additional members
	};
}
