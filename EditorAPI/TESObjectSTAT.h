#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESObjectSTAT class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// 7C
	class TESObjectSTAT : public TESBoundObject, public TESModel
	{
	public:
		// members
		//     /*00*/ TESBoundObject
		//     /*58*/ TESModel
		
		// no additional members
	};
}