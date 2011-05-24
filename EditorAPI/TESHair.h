#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"

//	EditorAPI: TESHair class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    ...
*/

namespace EditorAPI
{
	// 70
	class TESHair : public TESFormIDListView, public TESFullName, public TESModel, public TESTexture
	{
	public:
		enum HairFlags
		{
			kHairFlags_Playable		= /*00*/ 0x01,
			kHairFlags_NotMale		= /*01*/ 0x02,
			kHairFlags_NotFemale	= /*02*/ 0x04,
			kHairFlags_FixedColor	= /*03*/ 0x08,
		};

		// members
		//     /*00*/ TESForm
		//	   /*24*/ TESFullName
		//	   /*30*/ TESModel
		//	   /*54*/ TESTexture
		/*6C*/ UInt8						hairFlags;
		/*6D*/ UInt8						hairFlagsPad6D[3];
	};
}