#pragma once

#include "TESObject.h"

//	EditorAPI: TESLevCreature class.

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