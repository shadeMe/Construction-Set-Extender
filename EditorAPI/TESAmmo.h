#pragma once

#include "TESObject.h"

//	EditorAPI: TESAmmo class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// D0
class TESAmmo : public TESBoundObject, public TESFullName, public TESModel, public TESIcon, public TESEnchantableForm, public TESValueForm, public TESWeightForm, public TESAttackDamageForm
{
public:
	enum AmmoFlags
	{
		kAmmoFlags_IgnoreNormalWeaponResistance		= /*00*/ 0x01
	};

	// members
	//     /*00*/ TESBoundObject
	//     /*58*/ TESFullName
	//     /*64*/ TESModel
	//	   /*88*/ TESIcon
	//	   /*A0*/ TESEnchantableForm
	//	   /*B0*/ TESValueForm
	//	   /*B8*/ TESWeightForm
	//	   /*C0*/ TESAttackDamageForm
	/*C8*/ float							speed;
	/*CC*/ UInt8							ammoFlags;
	/*CD*/ UInt8							padCD[3];
};