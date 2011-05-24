#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESObjectTREE class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// CC
	class TESObjectTREE : public TESBoundTreeObject, public TESModelTree, public TESIconTree
	{
	public:
		typedef UInt32 SpeedTreeSeed;

		// members
		//     /*00*/ TESBoundTreeObject
		//     /*58*/ TESModelTree
		//	   /*7C*/ TESIconTree
		/*94*/ NiTArray<SpeedTreeSeed>				speedTreeSeeds;
		/*A4*/ float								leafCurvature;
		/*A8*/ float								minLeafAngle;
		/*AC*/ float								maxLeafAngle;
		/*B0*/ float								branchDimmingValue;
		/*B4*/ float								leafDimmingValue;
		/*B8*/ UInt32								shadowRadius;
		/*BC*/ float								rockSpeed;
		/*C0*/ float								rustleSpeed;
		/*C4*/ float								billBoardWidth;
		/*C8*/ float								billboardHeight;
	};
}