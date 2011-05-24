#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESObjectACTI class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// 98
	class TESObjectACTI : public TESBoundAnimObject, public TESFullName, public TESModel, public TESScriptableForm
	{
	public:
		// members
		//     /*00*/ TESBoundAnimObject
		//     /*58*/ TESFullName
		//     /*64*/ TESModel
		//	   /*88*/ TESScriptableForm
		/*94*/ TESSound*				loopSound;
	};
}