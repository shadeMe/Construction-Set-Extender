#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESLevItem class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// 68
	class TESLevItem : public TESBoundObject, public TESLeveledList
	{
	public:
		// members
		//     /*00*/ TESBoundObject
		//     /*58*/ TESLeveledList

		// no additional members
	};
}