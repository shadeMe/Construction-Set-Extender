#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"
#include "ActorValues.h"
#include "FaceGen.h"

//	EditorAPI: TESRace class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    ...
*/

class	TESHair;
class	TESEyes;

// 494
class TESRace : public TESFormIDListView, public TESFullName, public TESDescription, public TESSpellList, public TESReactionForm
{
public:
	// one for each of the sHairColorXX game settings
	enum HairColor
	{
		kHairColor_Color0		= 0x0,
		kHairColor_Color1,
		kHairColor_Color2,
		kHairColor_Color3,
		kHairColor_Color4,
		kHairColor_Color5,
		kHairColor_Color6,
		kHairColor_Color7,
		kHairColor_Color8,
		kHairColor_Color9,
		kHairColor_Color10,
		kHairColor_Color11,
		kHairColor_Color12,
		kHairColor_Color13,
		kHairColor_Color14,
		kHairColor_Color15		= 0xF,
		kHairColor__MAX
	};

	// 02
	struct SkillBonusData
	{
		/*00*/ UInt8		skill;
		/*01*/ UInt8		bonus;
	};

	typedef tList<TESHair> HairListT;
	typedef tList<TESEyes> EyeListT;

	// members
	//     /*00*/ TESForm
	//	   /*24*/ TESFullName
	//	   /*30*/ TESDescription
	//	   /*40*/ TESSpellList
	//	   /*54*/ TESReactionForm
	/*064*/ SkillBonusData				skillBonuses[7];
	/*072*/ UInt8						pad72[2];
	/*074*/ float						maleScale;
	/*078*/ float						femaleScale;
	/*07C*/ float						maleWeight;
	/*080*/ float						femaleWeight;
	/*084*/ UInt32						isPlayable;
	/*088*/ TESAttributes				maleAttributes;
	/*094*/ TESAttributes				femaleAttributes;
	/*0A0*/ HairListT					hairList;
	/*0A8*/ TESHair*					defaultMaleHair;
	/*0AC*/ TESHair*					defaultFemaleHair;
	/*0B0*/ UInt8						hairColor;
	/*0B1*/ UInt8						padB1[3];
	/*0B4*/ float						faceGenMainClamp;
	/*0B8*/ float						faceGenFaceClamp;
	/*0BC*/ EyeListT					eyeList;
	/*0C4*/ TESModel					maleTailModel;
	/*0E8*/ TESModel					femaleTailModel;
	/*10C*/ TESModel					headModel;
	/*130*/ TESModel					maleEarModel;
	/*154*/ TESModel					femaleEarModel;
	/*178*/ TESModel					mouthModel;
	/*19C*/ TESModel					lowerTeethModel;
	/*1C0*/ TESModel					upperTeethModel;
	/*1E4*/ TESModel					tongueModel;
	/*208*/ TESModel					leftEyeModel;
	/*22C*/ TESModel					rightEyeModel;
	/*250*/ TESTexture					headTexture;
	/*268*/ TESTexture					maleEarTexture;
	/*280*/ TESTexture					femaleEarTexture;
	/*298*/ TESTexture					mouthTexture;
	/*2B0*/ TESTexture					lowerTeethTexture;
	/*2C8*/ TESTexture					upperTeethTexture;
	/*2E0*/ TESTexture					toungeTexture;
	/*2F8*/ TESTexture					texture2F8;				// path is null, dlgIDs initialized to 1044 and 1058 resp. (neither is found in the class's subwindows)
	/*310*/ TESTexture					texture310;				// same as above
	/*328*/ TESTexture					maleUpperBodyTexture;
	/*340*/ TESTexture					maleLowerBodyTexture;
	/*358*/ TESTexture					maleHandTexture;
	/*370*/ TESTexture					maleFootTexture;
	/*388*/ TESTexture					maleTailTexture;
	/*3A0*/ TESTexture					femaleUpperBodyTexture;
	/*3B8*/ TESTexture					femaleLowerBodyTexture;
	/*3D0*/ TESTexture					femaleHandTexture;
	/*3E8*/ TESTexture					femaleFootTexture;
	/*400*/ TESTexture					femalTailTexture;
	/*418*/ FaceGenFaceData18			unk418[4];
	/*478*/ UInt16						unk478;					// init to 0
	/*47A*/ UInt8						pad47A[2];
	/*47C*/ TESRace*					maleVoiceRace;
	/*480*/ TESRace*					femaleVoiceRace;
	/*484*/ FaceGenUndoListT			faceGenUndoList;
};