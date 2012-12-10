#pragma once

#include "TESObject.h"
#include "BaseFormComponent.h"

//	EditorAPI: TESActorBase class and its derivatives.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	TESActorBase is the fundamental base form class for actors.
*/

class   TESCombatStyle;

// 118
class TESActorBase : public TESBoundAnimObject, public TESActorBaseData, public TESContainer, public TESSpellList, public TESAIForm,
				public TESHealthForm, public TESAttributes, public TESAnimation, public TESFullName, public TESModel, public TESScriptableForm
{
public:
	// members
	//     /*000*/ TESBoundAnimObject
	//     /*058*/ TESActorBaseData
	//     /*078*/ TESContainer
	//     /*088*/ TESSpellList
	//     /*09C*/ TESAIForm
	//     /*0B4*/ TESHealthForm
	//     /*0BC*/ TESAttributes
	//     /*0C8*/ TESAnimation
	//     /*0DC*/ TESFullName
	//     /*0E8*/ TESModel
	//     /*10C*/ TESScriptableForm
};
STATIC_ASSERT(sizeof(TESActorBase) == 0x118);