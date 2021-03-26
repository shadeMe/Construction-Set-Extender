#pragma once

#include "TESForm.h"
#include "TESObjectCELL.h"

//	EditorAPI: TESObjectLAND class.

/*
	...
*/

class TESGrassAreaParam;

// 34
class TESObjectLAND : public TESForm, public TESChildCell
{
public:
	// 08
	struct LandHeightLimit
	{
		/*00*/ float			maxHeight;
		/*04*/ float			minHeight;
	};

	// 10
	struct LandHeightMap
	{
		// D8C
		struct Quadrant
		{
			/*00*/ Vector3		verts[17][17];		// x, y range from -2048 to 2048
		};

		/*00*/ Quadrant*		quadrants[4];		// order: LL, LR, UL, UR; 128 game units between each vert
	};

	// 10
	struct Land3D
	{
		/*00*/ NiNode*			landNode[4];		// land node in the parent cell ( parent -> quad 0-3 -> land)
	};

	// 10
	struct LandUnk0C
	{
		/*00*/ UInt8	data[0x1210][4];
	};

	// 10
	struct LandUnk10
	{
		/*00*/ UInt8	data[0x124][4];				// alloc request size 121 padded to 124
	};

	// 20
	struct LandTexture
	{
		/*00*/ TESLandTexture*		textures[8];	// max 8 per quad

		// used when loading the textures
		enum
		{
			kPresenceMask_Tex8_Present		= 1 << 0,
			kPresenceMask_Tex8_Absent		= 1 << 1,

			kPresenceMask_Tex7_Present		= 1 << 2,
			kPresenceMask_Tex7_Absent		= 1 << 3,

			kPresenceMask_Tex6_Present		= 1 << 4,
			kPresenceMask_Tex6_Absent		= 1 << 5,

			kPresenceMask_Tex5_Present		= 1 << 6,
			kPresenceMask_Tex5_Absent		= 1 << 7,

			kPresenceMask_Tex4_Present		= 1 << 8,
			kPresenceMask_Tex4_Absent		= 1 << 9,

			kPresenceMask_Tex3_Present		= 1 << 10,
			kPresenceMask_Tex3_Absent		= 1 << 11,

			kPresenceMask_Tex2_Present		= 1 << 12,
			kPresenceMask_Tex2_Absent		= 1 << 13,

			kPresenceMask_Tex1_Present		= 1 << 14,
			kPresenceMask_Tex1_Absent		= 1 << 15,
		};
	};
	STATIC_ASSERT(sizeof(LandTexture) == 0x20);

	// 484
	struct LandUnk40
	{
		/*00*/ UInt8	data[0x484];
	};

	typedef cseOverride::NiTPointerMap<UInt32, TESGrassAreaParam*>		GrassAreaParamsMapT;

	// A0
	struct Data
	{
		/*00*/ Land3D*				nodeData;
		/*04*/ LandHeightMap*		heightMap;
		/*08*/ void*				unk08;					// normal data, 4 ptrs to 0xD8C bytes
		/*0C*/ LandUnk0C*			unk0C;					// vertex color data
		/*10*/ LandUnk10*			unk10;					// vertex texture data
		/*14*/ UInt32				unk14;
		/*18*/ float				unk18;					// init'd to FLT_MAX
		/*1C*/ float				unk1C;					// init'd to -FLT_MAX
		/*20*/ TESLandTexture*		defaultTextures[4];
		/*30*/ LandTexture*			quadTextures[4];
		/*40*/ LandUnk40*			unk40[4];
		/*50*/ UInt32				unk50;
		/*54*/ GrassAreaParamsMapT	grassAreaParams[4];
		/*94*/ UInt32				unk94;
		/*98*/ SInt32				cellCoordX;
		/*9C*/ SInt32				cellCoordY;
	};


	enum
	{
		kLandFlags_Unk03	= 1 << 3,		// set when 3D is loaded?
		kLandFlags_Unk0A	= 1 << 10,		// set when the parent cell's worldspace has a parent (worldspace)
	};

	// members
	//     /*00*/ TESForm
	//     /*24*/ TESChildCell
	/*28*/ UInt8				landFlags;
	/*29*/ UInt8				pad29[3];
	/*2C*/ TESObjectCELL*		parentCell;
	/*30*/ Data*				landData;

	// methods
	const LandHeightLimit*		GetHeightLimits(LandHeightLimit* Out);
	bool						ReleaseLandData();
	void						Refresh3D(bool ReloadTextures);
	NiNode*						GetQuadLandNode(int Quad);
	TESLandTexture*				GetLandTextureAt(Vector3* Coords);

	static HWND*				WindowHandle;
	static TESLandTexture**		DefaultLandTexture;
};