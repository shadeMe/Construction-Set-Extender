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
class	TESWaterForm;

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

	enum CellProcessLevels
	{
		kCellProcessLevel_None      = 0x0,  // default value, no associated NiNode or Havok objects
		kCellProcessLevel_Removing  = 0x1,  // set while nodes and havok objects are being destroyed
		kCellProcessLevel_Building  = 0x2,  // set while nodes and havok objects are being created
		kCellProcessLevel_Loaded    = 0x3,  // loaded cells that are not processed
		kCellProcessLevel_Active    = 0x6,  // current interior cell, or exterior cells within fixed radius of current exterior cell
		kCellProcessLevel__MAX      = 0x7
	};

	// 08
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

	// 04
	union CellData
	{
		/*00*/ CellCoordinates*		coords;		// if exterior
		/*00*/ LightingData*		lighting;	// if interior
	};

	// members
	//     /*00*/ TESForm
	//     /*24*/ TESFullName
	//     /*30*/ TESMemContextForm - empty, no members
	/*30*/ UInt8				cellFlags;
	/*31*/ UInt8				cellProcessFlags;
	/*32*/ UInt8				unk32;
	/*33*/ UInt8				pad27;
	/*34*/ ExtraDataList		extraData;
	/*48*/ CellData				cellData;
	/*4C*/ TESObjectLAND*		land;
	/*50*/ TESPathGrid*			pathGrid;
	/*54*/ ObjectREFRList		objectList;

	// methods
	const char*					GetEditorID() const;

	TESWorldSpace*				GetParentWorldSpace();
	bool						GetIsInterior();
	TESObjectREFR*				FindFirstRef(TESForm* BaseForm, bool MustHaveEditorID = false);
	void						SetPathGrid(TESPathGrid* PathGrid);

	void						AddObjectReference(TESObjectREFR* Ref);
	void						RemoveObjectReference(TESObjectREFR* Ref);

	void						ModExtraCellWaterHeight(float Height);
	void						ModExtraCellWaterType(TESWaterForm* Water);
};

// 04
class TESChildCell
{
public:
	// no members, other than vtbl

	virtual TESObjectCELL*		GetChildCell() = 0;
};