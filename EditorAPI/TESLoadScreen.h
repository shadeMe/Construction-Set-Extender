#pragma once

#include "TESForm.h"

//	EditorAPI: TESLoadScreen class.

/*
	...
*/

// 54
class TESLoadScreen : public TESForm, public TESTexture, public TESDescription
{
public:
	typedef tList<TESForm>		LoadFormListT;

	// members
	//     /*00*/ TESForm
	//     /*24*/ TESTexture
	//     /*3C*/ TESDescription
	/*4C*/ LoadFormListT		loadFormList;		// either TESObjectCELL/TESWorldSpace
};