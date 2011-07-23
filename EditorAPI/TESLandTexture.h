#pragma once

#include "TESForm.h"

//	EditorAPI: TESLandTexture class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

class	TESGrass;

// 58
class TESLandTexture : public TESForm, public TESTexture
{
public:
	enum HavokMaterialType
	{
		kHavokMaterialType_Stone					= 0x0,
		kHavokMaterialType_Cloth,
		kHavokMaterialType_Dirt,
		kHavokMaterialType_Glass,
		kHavokMaterialType_Grass,
		kHavokMaterialType_Metal,
		kHavokMaterialType_Organic,
		kHavokMaterialType_Skin,
		kHavokMaterialType_Water,
		kHavokMaterialType_Wood,
		kHavokMaterialType_HeavyStone,
		kHavokMaterialType_HeavyMetal,
		kHavokMaterialType_HeavyWood,
		kHavokMaterialType_Chain,
		kHavokMaterialType_Snow,
		kHavokMaterialType_StoneStairs,
		kHavokMaterialType_ClothStairs,
		kHavokMaterialType_DirtStairs,
		kHavokMaterialType_GlassStairs,
		kHavokMaterialType_GrassStairs,
		kHavokMaterialType_MetalStairs,
		kHavokMaterialType_OrganicStairs,
		kHavokMaterialType_SkinStairs,
		kHavokMaterialType_WaterStairs,
		kHavokMaterialType_WoodStairs,
		kHavokMaterialType_HeavyStoneStairs,
		kHavokMaterialType_HeavyMetalStairs,
		kHavokMaterialType_HeavyWoodStaris,
		kHavokMaterialType_ChainStairs,
		kHavokMaterialType_SnowStairs,
		kHavokMaterialType_Elevator					= 0x1E,
		kHavokMaterialType__MAX
	};

	typedef tList<TESGrass> PotentialGrassListT;

	// members
	//     /*00*/ TESForm
	//	   /*24*/ TESTexture
	/*3C*/ UInt32						unk3C;
	/*40*/ UInt8						havokMaterial;
	/*41*/ UInt8						friction;
	/*42*/ UInt8						restitution;
	/*43*/ UInt8						specularExponent;
	/*44*/ PotentialGrassListT			potentialGrassList;
	/*4C*/ UInt32						unk4C;					// could be a linked list, ctor initializes it as if it were
	/*50*/ UInt32						unk50;
	/*54*/ UInt32						unk54;					// init to 0
};