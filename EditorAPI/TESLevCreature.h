#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESLevCreature class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// 78
	class TESLevCreature : public TESBoundObject, public TESLeveledList, public TESScriptableForm
	{
	public:
		// members
		//     /*00*/ TESBoundObject
		//     /*58*/ TESLeveledList
		//	   /*68*/ TESScriptableForm
		/*74*/ TESActorBase*				templateForm;
	};
}