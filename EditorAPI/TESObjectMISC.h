#pragma once

#include "TESObject.h"

//	EditorAPI: TESObjectMISC class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// BC
class TESObjectMISC : public TESBoundObject, public TESFullName, public TESModel, public TESIcon, public TESScriptableForm, public TESValueForm, public TESWeightForm
{
public:
	// members
	//     /*00*/ TESBoundObject
	//     /*58*/ TESFullName
	//     /*64*/ TESModel
	//	   /*88*/ TESIcon
	//	   /*A0*/ TESScriptableForm
	//	   /*AC*/ TESValueForm
	//	   /*B4*/ TESWeightForm

	// no additional members
};