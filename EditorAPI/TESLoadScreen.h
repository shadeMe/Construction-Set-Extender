#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"

//	EditorAPI: TESLoadScreen class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

// 54
class TESLoadScreen : public TESForm, public TESTexture, public TESDescription
{
public:
	// members
	//     /*00*/ TESForm
	//     /*24*/ TESTexture
	//     /*3C*/ TESDescription
	/*4C*/ 
};