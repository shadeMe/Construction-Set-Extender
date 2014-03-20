#pragma once

#include "TESForm.h"
#include "TESObjectCELL.h"

//	EditorAPI: TESObjectLAND class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// 34
class TESObjectLAND : public TESForm, public TESChildCell
{
public:
	// 08
	struct HeightLimitData
	{
		/*00*/ float			maxHeight;
		/*04*/ float			minHeight;
	};

	// members
	//     /*00*/ TESForm
	//     /*24*/ TESChildCell
	/*28*/ UInt8				unk28;			// flags of some sort
	/*29*/ UInt8				pad29[3];
	/*2C*/ TESObjectCELL*		parentCell;
	/*30*/ void*				unk30;			// heightfield data

	// methods
	const HeightLimitData*		GetHeightLimits(HeightLimitData* Out);

	static HWND*				WindowHandle;
};