#pragma once

#include "TESObject.h"

//	EditorAPI: TESObjectACTI class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

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