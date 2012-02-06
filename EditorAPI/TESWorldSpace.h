#pragma once

#include "obse\NiNodes.h"

#include "TESObjectCELL.h"

//	EditorAPI: TESWorldSpace class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

class	TESClimate;
class	TESWaterForm;
class	TESTerrainLODQuadRoot;

// CC
class TESWorldSpace : public TESFormIDListView, public TESFullName, public TESTexture
{
public:
	enum
	{
		kWorldSpaceMusicType_Default		= 0x0,
		kWorldSpaceMusicType_Public,
		kWorldSpaceMusicType_Dungeon,
		kWorldSpaceMusicType__MAX
	};

	enum
	{
		kWorldSpaceFlags_SmallWorld			=	/*00*/ 0x01,
		kWorldSpaceFlags_NoFastTravel		=	/*01*/ 0x02,
		kWorldSpaceFlags_OblivionWorld		=	/*02*/ 0x04,
		kWorldSpaceFlags_Unk03				=	/*03*/ 0x08,
		kWorldSpaceFlags_NoLODWater			=	/*04*/ 0x10
	};

	// members
	//     /*00*/ TESFormIDListView
	//     /*24*/ TESFullName
	//     /*30*/ TESTexture
	/*48*/ NiTMapBase<int, TESObjectCELL*>*				cellMap;	// key is coordinates of cell: (x << 16 + y)
	/*4C*/ TESObjectCELL*								unk4C;
	/*50*/ UInt32										unk50;		// never initialized? NULL checked by the heightmap editor when generating LOD geometry
	/*54*/ UInt8										unk54;
	/*55*/ UInt8										unk55[3];
	/*58*/ NiTMapBase<int, TESTerrainLODQuadRoot*>		terrainLODQuadMap;
	/*68*/ TESWorldSpace*								unk68;		// initialized to 'this'
	/*6C*/ UInt32										unk6C;
	/*70*/ UInt32										unk70;
	/*74*/ UInt32										unk74;
	/*78*/ TESClimate*									climate;
	/*7C*/ UInt8										worldSpaceFlags;
	/*7D*/ UInt8										pad7D[3];
	/*80*/ UInt32										unk80;
	/*84*/ UInt32										unk84;
	/*88*/ TESWorldSpace*								parentWorldspace;
	/*8C*/ TESWaterForm*								waterType;
	/*90*/ UInt32										usableDimensionsX;
	/*94*/ UInt32										usableDimensionsY;
	/*98*/ UInt16										northWestCoordX;
	/*9A*/ UInt16										northWestCoordY;
	/*9C*/ UInt16										southEastCoordX;
	/*9E*/ UInt16										southEastCoordY;
	/*A0*/ UInt32										musicType;
	/*A4*/ float										unkA4;
	/*A8*/ float										unkA8;
	/*AC*/ float										unkAC;
	/*B0*/ float										unkB0;
	/*B4*/ NiTMapBase<UInt32, bool>						unkB4;
	/*C4*/ UInt32										unkC4;
	/*C8*/ UInt32										unkC8;		// init to 7, something to do with the player base form?
};