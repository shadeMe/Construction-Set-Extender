#pragma once

#include "TESForm.h"

//	EditorAPI: TESObjectANIO class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

class	TESIdleForm;

// 4C
class TESObjectANIO : public TESForm, public TESModel
{
public:
	// members
	//     /*00*/ TESForm
	//     /*24*/ TESModel
	/*48*/ TESIdleForm*			idleForm;
};