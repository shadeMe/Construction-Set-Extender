#pragma once

#include "TESForm.h"

//	EditorAPI: TESIdleForm class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// 68
class TESIdleForm : public TESForm, public TESModelAnim
{
public:
	// members
	//     /*00*/ TESForm
	//     /*24*/ TESModelAnim
	/*3C*/ UInt32				unk3C;

	// methods
	static void					InitializeIdleFormTreeRootNodes();		// actually belongs to class BSTCaseInsensitiveMap<IDLE_ANIM_ROOT>
	static void					ResetIdleFormTree();

	static void**				IdleFormTree;			// BSTCaseInsensitiveMap<IDLE_ANIM_ROOT>*
};