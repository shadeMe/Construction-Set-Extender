#pragma once

#include "TESForm.h"

//	EditorAPI: TESCondition class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    ..
*/

// 18
class TESCondition
{
public:
	union Param
	{
		float			number;
		TESForm*		form;
	};

	// members
	/*00*/ UInt32				operatorAndFlags;
	/*04*/ float				comparisonValue;
	/*08*/ UInt16				functionIndex;		// opcode & 0x0FFF
	/*0A*/ UInt16				unk0A;
	/*0C*/ Param				param1;
	/*10*/ Param				param2;
	/*14*/ UInt32				unk14;
};
STATIC_ASSERT(sizeof(TESCondition) == 0x18);

typedef tList<TESCondition>		ConditionListT;