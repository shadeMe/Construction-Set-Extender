#pragma once

#include "TESForm.h"
#include "TESObjectCELL.h"

//	EditorAPI: TESObjectLAND class.

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

	enum
	{
		kLandFlags_Unk03	= 1 << 3,
		kLandFlags_Unk0A	= 1 << 10,		// set when the parent cell's worldspace has a parent (worldspace)
	};

	// members
	//     /*00*/ TESForm
	//     /*24*/ TESChildCell
	/*28*/ UInt8				landFlags;
	/*29*/ UInt8				pad29[3];
	/*2C*/ TESObjectCELL*		parentCell;
	/*30*/ void*				unk30;			// height field data

	// methods
	const HeightLimitData*		GetHeightLimits(HeightLimitData* Out);

	static HWND*				WindowHandle;
};