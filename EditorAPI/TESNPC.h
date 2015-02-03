#pragma once

#include "TESActorBase.h"
#include "FaceGen.h"
#include "obse\NiObjects.h"

//	EditorAPI: TESNPC class.

/*
	...
*/

class	TESClass;
class	TESCombatStyle;

// 230
class TESNPC : public TESActorBase, public TESRaceForm
{
public:
	enum
	{
		kHeadPeviewCheckBox = 1014,
	};

	// members
	//     /*000*/ TESActorBase
	//     /*118*/ TESRaceForm
	/*120*/ UInt8							skillLevels[0x15];
	/*135*/ UInt8							pad135[3];
	/*138*/ TESClass*						npcClass;
	/*13C*/ FaceGenFaceParameters			vampireFaceData;
	/*19C*/ FaceGenFaceParameters			regularFaceData;
	/*1FC*/ TESHair*						hair;
	/*200*/ float							hairLength;
	/*204*/ TESEyes*						eyes;
	/*208*/ void*							unk208;					// smart pointer
	/*20C*/ void*							unk20C;					// smart pointer
	/*210*/ void*							unk210;					// smart pointer, BSFaceGenNiNode* ?
	/*214*/ UInt16							faceDataID;				// either 0xFF or race->faceDataID
																	// compared with latter, presumably used as a FaceGen morph caching optimization
	/*216*/ UInt16							pad216;
	/*218*/ TESCombatStyle*					combatStyle;			// NULL when set as DEFAULT in CS
	/*21C*/ RGBA							hairColor;
	/*220*/ FaceGenUndoListT				undoList;

	// methods
	void									ExportFaceGenTextures(void);
};
STATIC_ASSERT(sizeof(TESNPC) == 0x230);