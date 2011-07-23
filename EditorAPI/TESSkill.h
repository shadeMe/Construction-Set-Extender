#pragma once

#include "TESForm.h"
#include "ActorValues.h"

//	EditorAPI: TESSkill class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// A0
class TESSkill : public TESFormIDListView, public TESDescription, public TESTexture
{
public:
	enum Specializations
	{
		kSpecializations_Combat		= 0x0,
		kSpecializations_Magic		= 0x1,
		kSpecializations_Stealth	= 0x2,
		kSpecializations__MAX
	};

	// members
	//     /*00*/ TESForm
	//	   /*24*/ TESDescription
	//	   /*34*/ TESTexture
	/*4C*/ UInt32							skill;
	/*50*/ UInt32							governingAttribute;
	/*54*/ UInt32							specialization;
	/*58*/ float							skillUseValue0;
	/*5C*/ float							skillUseValue1;
	/*60*/ TESDescription					apprenticePerkText;
	/*70*/ TESDescription					journeymanPerkText;
	/*80*/ TESDescription					expertPerkText;
	/*90*/ TESDescription					masterPerkText;
};