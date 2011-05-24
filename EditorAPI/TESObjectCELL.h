#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"
#include "obse\NiNodes.h"

#include "TESForm.h"
#include "ExtraDataList.h"


//	EditorAPI: TESObjectCELL class and co.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    TESObjectCELL describes a physical location in the game world, tracking the scenegraph information(?)
    and all objects (TESObjectREFRs) inside it.
*/

namespace EditorAPI
{
	class   TESObjectREFR;
	class   TESObjectLAND;
	class   TESPathGrid;
	class   TESWorldSpace;

	// 5C
	class TESObjectCELL : public TESForm, public TESMemContextForm, public TESFullName
	{
	public:
		typedef tList<TESObjectREFR>    ObjectREFRList;

		enum CellFlags
		{
			kCellFlags_Interior             = /*00*/ 0x01, //
			kCellFlags_HasWater             = /*01*/ 0x02, // also hand changed?
			kCellFlags_InvertFastTravel     = /*02*/ 0x04, // interiors: can travel, exteriors: cannot travel
			kCellFlags_ForceHideLand        = /*03*/ 0x08, // exteriors only
			kCellFlags_OblivionInterior     = /*03*/ 0x08, // interiors only 
			kCellFlags_Unk4                 = /*04*/ 0x10,
			kCellFlags_Public               = /*05*/ 0x20,
			kCellFlags_HandChanged          = /*06*/ 0x40, // also has water?
			kCellFlags_BehaveLikeExterior   = /*07*/ 0x80,
		};

		struct CellCoordinates // what is the actual type for this?
		{
			SInt32    x;
			SInt32    y;
		};

		// members
		//     /*00*/ TESForm           
		//     /*24*/ TESFullName
		//     /*30*/ TESMemContextForm - empty, no members
		/*30*/ UInt8              cellFlags24;
		/*31*/ UInt8              cellFlags25;
		/*32*/ UInt8              cellFlags26;
		/*33*/ UInt8              pad27;
		/*34*/ ExtraDataList      extraData; // includes ExtraEditorID in game
		/*48*/ CellCoordinates*   coords;
		/*4C*/ TESObjectLAND*     land;
		/*50*/ TESPathGrid*       pathGrid;
		/*54*/ ObjectREFRList     objectList;
	};

	// 04
	class TESChildCell
	{
	public:
		// no members, other than vtbl
	};
}