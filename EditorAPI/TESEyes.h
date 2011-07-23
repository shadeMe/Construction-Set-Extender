#pragma once

#include "TESForm.h"

//	EditorAPI: TESEyes class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    ...
*/

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