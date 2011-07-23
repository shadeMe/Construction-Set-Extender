#pragma once

#include "TESObject.h"

//	EditorAPI: TESLevCreature class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// 78
class TESLevCreature : public TESBoundObject, public TESLeveledList, public TESScriptableForm
{
public:
	// members
	//     /*00*/ TESBoundObject
	//     /*58*/ TESLeveledList
	//	   /*68*/ TESScriptableForm
	/*74*/ TESActorBase*				templateForm;
};