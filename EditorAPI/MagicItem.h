#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "BaseFormComponent.h"

//	EditorAPI: MagicItem class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	MagicItem is an abstract form component for objects with 'castable' effects - Spells, Enchantments, Ingredients, & Potions
    It is *not* the base for sigil stones, which cannot be cast directly.
*/

namespace EditorAPI
{
	class   TESFile;

	// E8
	class MagicItem : public TESFullName, public EffectItemList
	{
	public:

		// members
		//     /*00/00*/ TESFullName 
		//     /*0C/0C*/ EffectItemList 
	};
}