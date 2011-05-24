#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObjectACTI.h"

//	EditorAPI: TESFlora class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// A4
	class TESFlora : public TESProduceForm, public TESObjectACTI
	{
	public:
		// members
		//     /*00*/ TESProduceForm
		//     /*0C*/ TESObjectACTI
		
		// no additional members
	};
}