#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"
#include "MagicItem.h"

//	EditorAPI: MagicItemForm class and derivatives.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	MagicItemForm is an abstract form component for objects with 'castable' effects that cannot be 
    placed as references, i.e. Spells & Enchantments.
*/

namespace EditorAPI
{
	// 10C
	class MagicItemForm : public TESForm, public MagicItem
	{
	public:
		// members
		//     /*000/000*/ TESForm
		//     /*018/024*/ MagicItem.TESFullName
		//     /*024/030*/ MagicItem.EffectItemList
	};

	// 11C
	class EnchantmentItem : public MagicItemForm
	{
	public:
		enum EnchantmentFlags
		{
			kEnchFlag_NoAutoCalc = 0x00000001,
		};

		// members
		//     /*000*/ TESForm
		//     /*024*/ MagicItem.TESFullName
		//     /*030*/ MagicItem.EffectItemList
		/*10C*/ UInt32           enchType;			// from Magic::CastTypes
		/*110*/ SInt32           charge;			// base available charge for non-autocalc
		/*114*/ SInt32           baseCost;			// base magicka / charge cost for non-autocalc
		/*118*/ UInt8            enchFlags;
		/*119*/ UInt8            enchPad041[3];		// saved & loaded, but not initialized
	};

	// 11C
	class SpellItem : public MagicItemForm
	{
	public:
		enum SpellFlags
		{
			kSpellFlag_NoAutoCalc                   = /*00*/ 0x01,
			kSpellFlag_ImmuneToSilence1             = /*01*/ 0x02,
			kSpellFlag_PlayerStartSpell             = /*02*/ 0x04,
			kSpellFlag_ImmuneToSilence2             = /*03*/ 0x08,
			kSpellFlag_AOEIgnoresLOS                = /*04*/ 0x10,
			kSpellFlag_SEFFAlwaysApplies            = /*05*/ 0x20,
			kSpellFlag_NoAbsorbOrReflect            = /*06*/ 0x40,
			kSpellFlag_TouchExplodesWithNoTarget    = /*07*/ 0x80,
		};

		// members
		//     /*000*/ TESForm
		//     /*024*/ MagicItem.TESFullName
		//     /*030*/ MagicItem.EffectItemList
		/*10C*/ UInt32           spellType;				// from Magic::MagicTypes, from Spell - Ability
		/*110*/ SInt32           baseCost;				// base magicka cost for non-autocalc
		/*114*/ UInt32           masteryLevel;			// for non-autocalc
		/*118*/ UInt8            spellFlags;
		/*119*/ UInt8            spellPad041[3];		// saved & loaded, but not initialized
	};
}