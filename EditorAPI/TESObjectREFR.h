#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"
#include "obse\NiNodes.h"

#include "TESForm.h"
#include "TESObjectCELL.h"
#include "ExtraDataList.h"


//	EditorAPI: TESObjectREFR class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    TESObjectREFR is the parent for all 'instances' of base forms in the game world
    While each ref is a distinct form, it also points back to the 'base' form that it instantiates
    Refs store local data like position, size, flags, etc.
*/

namespace EditorAPI
{
	class   ActorAnimData;
	class   MagicCaster;
	class   MagicTarget;

	// 60
	class IMPORTCLASS TESObjectREFR : public TESForm, public TESMemContextForm, public TESChildCell
	{
	public:
		// members
		//     /*00*/ TESForm          
		//     /*24*/ TESChildCell
		//     /*28*/ TESMemContextForm - empty, no members
		/*28*/ TESForm*				baseForm;
		/*2C*/ Vector3				rotation;
		/*38*/ Vector3				position;
		/*44*/ float				scale;
		/*48*/ TESObjectCELL*		parentCell;
		/*4C*/ ExtraDataList		extraData;
	};
}