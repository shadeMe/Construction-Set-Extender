#pragma once

#include "TESForm.h"

//	EditorAPI: TESConditionItem class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    ..
*/

class	TESConditionItem;
typedef tList<TESConditionItem>		ConditionListT;

// 18
class TESConditionItem
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

	// methods
	static UInt32				GetScriptableFormConditionCount(ConditionListT* ConditionList, TESForm* FormToCompare);
};
STATIC_ASSERT(sizeof(TESConditionItem) == 0x18);