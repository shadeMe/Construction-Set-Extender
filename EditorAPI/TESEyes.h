#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"

//	EditorAPI: TESEyes class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    ...
*/

namespace EditorAPI
{
	// 48
	class TESEyes : public TESFormIDListView, public TESFullName, public TESTexture
	{
	public:
		enum EyeFlags
		{
			kEyeFlags_Playable		= /*00*/ 0x01
		};

		// members
		//     /*00*/ TESForm
		//	   /*24*/ TESFullName
		//	   /*30*/ TESTexture
		/*48*/ UInt8						eyeFlags;
		/*49*/ UInt8						eyeFlagsPad49[3];
	};
}