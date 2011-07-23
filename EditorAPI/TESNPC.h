#pragma once

#include "TESActorBase.h"
#include "FaceGen.h"
#include "obse\NiObjects.h"

//	EditorAPI: TESNPC class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

class	TESClass;
class	TESCombatStyle;

// 230
class TESNPC : public TESActorBase, public TESRaceForm
{
public:
	// members
	//     /*000*/ TESActorBase
	//     /*118*/ TESRaceForm
	/*120*/ UInt8							skillLevels[0x15];
	/*135*/ UInt8							pad135[3];
	/*138*/ TESClass*						npcClass;
	/*13C*/ FaceGenFaceData18				unk13C[4];
	/*19C*/ FaceGenFaceData18				unk19C[4];
	/*1FC*/ TESHair*						hair;
	/*200*/ float							hairLength;
	/*204*/ TESEyes*						eyes;
	/*208*/ UInt32							unk208;
	/*20C*/ UInt32							unk20C;
	/*210*/ void*							unk210;					// smart pointer, BSFaceGenNiNode* ?
	/*214*/ UInt16							unk214;					// set to race->unk478
	/*216*/ UInt16							pad216[2];
	/*218*/ TESCombatStyle*					combatStyle;			// NULL when set as DEFAULT in CS
	/*21C*/ RGBA							hairColor;
	/*220*/ FaceGenUndoListT				undoList;
};