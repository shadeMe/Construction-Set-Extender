#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObjectMISC.h"

//	EditorAPI: TESSoulGem class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

// C0
class TESSoulGem : public TESObjectMISC
{
public:
	enum SoulLevel
	{
		kSoulLevel_None				= 0x0,
		kSoulLevel_Petty,
		kSoulLevel_Lesser,
		kSoulLevel_Common,
		kSoulLevel_Greater,
		kSoulLevel_Grand,
		kSoulLevel__MAX
	};
	// members
	//     /*00*/ TESObjectMISC
	/*BC*/ UInt8					soul;
	/*BD*/ UInt8					capacity;
	/*BE*/ UInt8					padBE[2];
};