#pragma once

#include "BaseFormComponent.h"
#include "EffectItem.h"

//	EditorAPI: MagicItem class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	MagicItem is an abstract form component for objects with 'castable' effects - Spells, Enchantments, Ingredients, & Potions
	It is *not* the base for sigil stones, which cannot be cast directly.
*/

class   TESFile;

typedef std::vector<TESForm*>	MagicItemScriptCrossRefListT;

// E8
class MagicItem : public TESFullName, public EffectItemList
{
public:

	// members
	//     /*00/00*/ TESFullName
	//     /*0C/0C*/ EffectItemList

	// methods
	UInt32								GetScriptReferences(MagicItemScriptCrossRefListT& OutList);
};
STATIC_ASSERT(sizeof(MagicItem) == 0xE8);