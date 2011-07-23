#pragma once

#include "TESObject.h"

//	EditorAPI: TESGrass class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// 9C
class TESGrass : public TESBoundObject, public TESModel
{
public:
	enum GrassPositioning
	{
		kGrassPositioning_AboveAtLeast			= 0x0,
		kGrassPositioning_AboveAtMost			= 0x1,
		kGrassPositioning_BelowAtLeast			= 0x2,
		kGrassPositioning_BelowAtMost			= 0x3,
		kGrassPositioning_EitherAtLeast			= 0x4,
		kGrassPositioning_EitherAtMost			= 0x5,
		kGrassPositioning_EitherAtMostAbove		= 0x6,
		kGrassPositioning_EitherAtMostBelow		= 0x7,
		kGrassPositioning__MAX
	};

	enum GrassFlags
	{
		kGrassFlags_VertexLightning				= /*00*/ 0x1,
		kGrassFlags_UniformScaling				= /*01*/ 0x2,
		kGrassFlags_FitToSlope					= /*02*/ 0x4
	};

	// members
	//     /*00*/ TESBoundObject
	//     /*58*/ TESModel
	/*7C*/ UInt8						density;			// init to 30
	/*7D*/ UInt8						minSlope;
	/*7E*/ UInt8						maxSlope;			// init to 90
	/*7F*/ UInt8						pad7F;
	/*80*/ UInt16						distanceFromWater;
	/*82*/ UInt16						pad82;
	/*84*/ UInt32						positioning;
	/*88*/ float						positionRange;		// init to 32
	/*8C*/ float						heightRange;		// init to 0.2
	/*90*/ float						colorRange;			// init to 0.5
	/*94*/ float						wavePeriod;			// init to 10
	/*98*/ UInt8						grassFlags;
	/*99*/ UInt8						pad99[3];
};