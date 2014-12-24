#pragma once

#include "TESObject.h"

//	EditorAPI: TESObjectBOOK class.

/*
	...
*/

// E0
class TESObjectBOOK : public TESBoundObject, public TESFullName, public TESModel, public TESIcon, public TESScriptableForm, public TESEnchantableForm, public TESValueForm, public TESWeightForm, public TESDescription
{
public:
	enum BookFlags
	{
		kBookFlags_Scroll			/*00*/ = 0x1,
		kBookFlags_CantBeTaken		/*01*/ = 0x2
	};

	// members
	//     /*00*/ TESBoundObject
	//     /*58*/ TESFullName
	//     /*64*/ TESModel
	//	   /*88*/ TESIcon
	//	   /*A0*/ TESScriptableForm
	//	   /*AC*/ TESEnchantableForm
	//	   /*BC*/ TESValueForm
	//	   /*C4*/ TESWeightForm
	//	   /*CC*/ TESDescription
	/*DC*/ UInt8					bookFlags;
	/*DD*/ UInt8					teachesSkill;
	/*DE*/ UInt8					padDE[2];
};