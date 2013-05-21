#pragma once

#include "TESForm.h"
#include "TESConditionItem.h"

//	EditorAPI: TESIdleForm class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// 68
class TESIdleForm : public TESForm, public TESModelAnim
{
public:
	// saved in the flags member
	enum
	{
		kAnimGroupSection_LowerBody		= 0,
		kAnimGroupSection_LeftArm,
		kAnimGroupSection_LeftHand,
		kAnimGroupSection_RightArm,
		kAnimGroupSection_SpecialIdle,
		kAnimGroupSection_WholeBody,
		kAnimGroupSection_UpperBody,

		kAnimGroupSection__MAX
	};

	enum
	{
		kIdleFormFlags_DoesntReturnFile		= 1 << 7
	};

	// members
	//     /*00*/ TESForm
	//     /*24*/ TESModelAnim
	/*48*/ BSString				rootPath;			// set to the root idle tree's path, i.e., Characters\_Male\IdleAnims
	/*50*/ ConditionListT		conditions;
	/*58*/ UInt8				flags;
	/*59*/ UInt8				pad59[3];
	/*5C*/ UInt32				unk5C;
	/*60*/ TESIdleForm*			previousLink;
	/*64*/ TESIdleForm*			nextLink;

	// methods
	static void					InitializeIdleFormTreeRootNodes();		// actually belongs to class BSTCaseInsensitiveMap<IDLE_ANIM_ROOT>
	static void					ResetIdleFormTree();

	static void**				IdleFormTree;			// BSTCaseInsensitiveMap<IDLE_ANIM_ROOT>*
};