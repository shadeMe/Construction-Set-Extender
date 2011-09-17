#pragma once

#include "obse\NiNodes.h"

#include "TESForm.h"
#include "ExtraDataList.h"

//	EditorAPI: TESObjectCELL class and co.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    TESObjectCELL describes a physical location in the game world, tracking the scenegraph information(?)
    and all objects (TESObjectREFRs) inside it.
*/

class   TESObjectREFR;
class   TESObjectLAND;
class   TESPathGrid;
class   TESWorldSpace;

// 5C
class TESObjectCELL : public TESForm, public TESFullName, public TESMemContextForm
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

	// 8
	struct CellCoordinates // what is the actual type for this?
	{
		/*00*/ SInt32		x;
		/*04*/ SInt32		y;
	};

	// 24
	struct LightingData
	{
		/*00*/ RGBA			ambient;
		/*04*/ RGBA			directional;
		/*08*/ RGBA			fog;
		/*0C*/ float		fogNear;
		/*10*/ float		fogFar;
		/*14*/ UInt32		rotXY;
		/*18*/ UInt32		rotZ;
		/*1C*/ float		directionalFade;
		/*20*/ float		fogClipDistance;
	};

	union CellData
	{
		/*00*/ CellCoordinates*		coords;		// if exterior
		/*00*/ LightingData*		lighting;	// if interior
	};

	// members
	//     /*00*/ TESForm
	//     /*24*/ TESFullName
	//     /*30*/ TESMemContextForm - empty, no members
	/*30*/ UInt8				cellFlags24;
	/*31*/ UInt8				cellFlags25;
	/*32*/ UInt8				cellFlags26;
	/*33*/ UInt8				pad27;
	/*34*/ ExtraDataList		extraData;
	/*48*/ CellData				cellData;
	/*4C*/ TESObjectLAND*		land;
	/*50*/ TESPathGrid*			pathGrid;
	/*54*/ ObjectREFRList		objectList;

	// methods
	TESWorldSpace*				GetParentWorldSpace();
	bool						GetIsInterior();
	TESObjectREFR*				LookupRefByBaseForm(TESForm* BaseForm, bool HasEditorID = false);		// returns the first ref satisfying the conditions
	void						SetPathGrid(TESPathGrid* PathGrid);
};

// 04
class TESChildCell
{
public:
	// no members, other than vtbl

	virtual TESObjectCELL*		GetChildCell();
};