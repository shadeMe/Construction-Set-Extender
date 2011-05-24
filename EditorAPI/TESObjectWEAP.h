#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESObjectWEAP class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// EC
	class TESObjectWEAP : public TESBoundObject, public TESFullName, public TESModel, public TESIcon, public TESScriptableForm, public TESEnchantableForm, public TESValueForm, public TESWeightForm, public TESHealthForm, public TESAttackDamageForm
	{
	public:
		enum WeaponType
		{
			kWeaponType_BladeOneHand		= 0x0,
			kWeaponType_BladeTwoHand,
			kWeaponType_BluntOneHand,
			kWeaponType_BluntTwoHand,
			kWeaponType_Staff,
			kWeaponType_Bow,
			kWeaponType__MAX
		};

		enum WeaponFlags
		{
			kWeaponFlags_IgnoreNormalWeaponResistance		= /*00*/ 0x01
		};

		// members
		//     /*00*/ TESBoundObject
		//     /*58*/ TESFullName
		//     /*64*/ TESModel
		//	   /*88*/ TESIcon
		//	   /*A0*/ TESScriptableForm
		//	   /*AC*/ TESEnchantableForm
		//	   /*BC*/ TESValueForm
		//	   /*C4*/ TESWeightForm
		//	   /*CC*/ TESHealthForm
		//	   /*D4*/ TESAttackDamageForm
		/*DC*/ UInt8						weaponType;
		/*DD*/ UInt8						padDD[3];
		/*E0*/ float						speed;
		/*E4*/ float						reach;
		/*E8*/ UInt8						weaponFlags;
		/*E9*/ UInt8						padE9[3];
	};
}