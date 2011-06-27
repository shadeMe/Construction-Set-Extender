#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"

//	EditorAPI: BirthSign class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

// 6C
class BirthSign : public TESFormIDListView, public TESFullName, public TESTexture, public TESDescription, public TESSpellList
{
public:
	// members
	//     /*00*/ TESForm
	//     /*24*/ TESFullName
	//     /*30*/ TESTexture
	//	   /*48*/ TESDescription
	//	   /*58*/ TESSpellList

	// no additional members
};