#pragma once

#include "TESForm.h"
#include "ActorValues.h"

//	EditorAPI: TESClass class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    ...
*/

// 8C
class TESClass : public TESFormIDListView, public TESFullName, public TESDescription, public TESTexture
{
public:
	enum ClassFlags
	{
		kClassFlags_Playable	= /*00*/ 0x01,
		kClassFlags_Guard		= /*01*/ 0x02
	};

	enum BuySellOtherFlags
	{
		kBuySellOtherFlags_Weapons		 = /*00*/ 0x00000001,
		kBuySellOtherFlags_Armor		 = /*01*/ 0x00000002,
		kBuySellOtherFlags_Clothing		 = /*02*/ 0x00000004,
		kBuySellOtherFlags_Books         = /*03*/ 0x00000008,
		kBuySellOtherFlags_Ingredients   = /*04*/ 0x00000010,
		kBuySellOtherFlags_Unk05         = /*05*/ 0x00000020,
		kBuySellOtherFlags_Unk06         = /*06*/ 0x00000040,
		kBuySellOtherFlags_Lights        = /*07*/ 0x00000080,
		kBuySellOtherFlags_Apparatus     = /*08*/ 0x00000100,
		kBuySellOtherFlags_Unk09         = /*09*/ 0x00000200,
		kBuySellOtherFlags_Misc          = /*0A*/ 0x00000400,
		kBuySellOtherFlags_Spells        = /*0B*/ 0x00000800,
		kBuySellOtherFlags_MagicItems    = /*0C*/ 0x00001000,
		kBuySellOtherFlags_Potions       = /*0D*/ 0x00002000,
		kBuySellOtherFlags_Training      = /*0E*/ 0x00004000,
		kBuySellOtherFlags_Unk0F		 = /*0F*/ 0x00008000,
		kBuySellOtherFlags_Recharge      = /*10*/ 0x00010000,
		kBuySellOtherFlags_Repair		 = /*11*/ 0x00020000
	};

	// members
	//     /*00*/ TESForm
	//	   /*24*/ TESFullName
	//	   /*30*/ TESDescription
	//	   /*40*/ TESTexture
	/*58*/ UInt32						primaryAttributes[2];	// value of type ActorValues::ActorValueCodes
	/*60*/ UInt32						specialization;
	/*64*/ UInt32						majorSkills[7];
	/*80*/ UInt8						classFlags;
	/*81*/ UInt8						classFlagsPad81[3];
	/*84*/ UInt32						buySellOtherFlags;
	/*88*/ UInt8						trainingSkill;
	/*89*/ UInt8						trainingLevel;
	/*8A*/ UInt8						trainingLevelPad8A[3];
};