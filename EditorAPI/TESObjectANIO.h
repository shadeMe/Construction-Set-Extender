#pragma once

#include "TESForm.h"

//	EditorAPI: TESObjectANIO class.

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