#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESObjectARMO class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// 164
	class TESObjectARMO : public TESBoundObject, public TESFullName, public TESScriptableForm, public TESEnchantableForm, public TESValueForm, public TESWeightForm, public TESHealthForm, public TESBipedModelForm
	{
	public:
		// members
		//     /*00*/ TESBoundObject
		//     /*58*/ TESFullName
		//     /*64*/ TESScriptableForm
		//	   /*70*/ TESEnchantableForm
		//	   /*80*/ TESValueForm
		//	   /*88*/ TESWeightForm
		//	   /*90*/ TESHealthForm
		//	   /*98*/ TESBipedModelForm
		/*160*/ UInt16				armorRating;
		/*162*/ UInt16				pad162[2];
	};
}