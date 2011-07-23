#pragma once

#include "TESForm.h"

//	EditorAPI: TESWaterForm class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// CC
class TESWaterForm : public TESForm, public TESAttackDamageForm, public TESTexture
{
public:
	// members
	//     /*00*/ TESForm
	//     /*24*/ TESAttackDamageForm
	//     /*2C*/ TESTexture
	/*44*/
};