#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESObjectCLOT class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// 158
	class TESObjectCLOT : public TESBoundObject, public TESFullName, public TESScriptableForm, public TESEnchantableForm, public TESValueForm, public TESWeightForm, public TESBipedModelForm
	{
	public:
		// members
		//     /*00*/ TESBoundObject
		//     /*58*/ TESFullName
		//     /*64*/ TESScriptableForm
		//	   /*70*/ TESEnchantableForm
		//	   /*80*/ TESValueForm
		//	   /*88*/ TESWeightForm
		//	   /*90*/ TESBipedModelForm
		
		// no additional members
	};
}