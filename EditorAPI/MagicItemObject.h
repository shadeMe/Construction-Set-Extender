#pragma once

#include "BaseFormComponent.h"
#include "MagicItem.h"
#include "TESObject.h"

//	EditorAPI: MagicItemObject class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	MagicItemObject is an abstract form component for objects with 'castable' effects that can be
    placed as references, i.e. Potions & Ingredients.
*/

// 140
class MagicItemObject : public TESBoundObject, public MagicItem
{
public:
	// members
	//     /*000/000*/ TESBoundObject
	//     /*024/058*/ MagicItem.TESFullName
	//     /*030/064*/ MagicItem.EffectItemList
};
STATIC_ASSERT(sizeof(MagicItemObject) == 0x140);

// 198
class IngredientItem : public MagicItemObject, public TESModel, public TESIcon, public TESScriptableForm, public TESWeightForm
{
public:
	enum IngredientFlags
	{
		kIngrFlag_NoAutocalc    = /*00*/ 0x01,
		kIngrFlag_Edible        = /*01*/ 0x02,
	};

	// members
	//     /*000/000*/ TESBoundObject
	//     /*024/058*/ MagicItem.TESFullName
	//     /*030/064*/ MagicItem.EffectItemList
	//     /*040/140*/ TESModel
	//     /*058/164*/ TESIcon
	//     /*064/17C*/ TESScriptableForm
	//     /*070/188*/ TESWeightForm
	/*190*/ SInt32           baseCost;			// base magicka cost / gold cost for non-autocalc
	/*194*/ UInt8            ingrFlags;
	/*195*/ UInt8            ingrPad7D[3];		// saved & loaded, but not initialized
};
STATIC_ASSERT(sizeof(IngredientItem) == 0x198);

// 198
class AlchemyItem : public MagicItemObject, public TESModel, public TESIcon, public TESScriptableForm, public TESWeightForm
{
public:
	enum AlchemyItemFlags
	{
		kAlchFlag_NoAutocalc    = /*00*/ 0x01,
		kAlchFlag_Edible        = /*01*/ 0x02,
	};

	// members
	//     /*000/000*/ TESBoundObject
	//     /*024/058*/ MagicItem.TESFullName
	//     /*030/064*/ MagicItem.EffectItemList
	//     /*040/140*/ TESModel
	//     /*058/164*/ TESIcon
	//     /*064/17C*/ TESScriptableForm
	//     /*070/188*/ TESWeightForm
	/*190*/ SInt32           baseCost;			// base magicka cost / gold cost for non-autocalc
	/*194*/ UInt8            alchFlags;
	/*195*/ UInt8            alchPad7D[3];		// saved & loaded, but not initialized
};
STATIC_ASSERT(sizeof(AlchemyItem) == 0x198);