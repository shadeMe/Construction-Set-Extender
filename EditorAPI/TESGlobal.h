#pragma once

#include "TESForm.h"

//	EditorAPI: TESGlobal class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    TESGlobal is the class that is used to store numeric data (integer/floating point) in the global context
	Globals are referenced by scripts and conditions
*/

// 2C
class TESGlobal : public TESFormIDListView
{
public:
	enum GlobalType
	{
		kGlobalType_Short	= 's',
		kGlobalType_Long	= 'l',
		kGlobalType_Float	= 'f'
	};

	// members
	//     /*00*/ TESForm
	/*24*/ UInt8					globalType;
	/*25*/ UInt8					globalTypePad05[3];
	/*28*/ float					value;
};