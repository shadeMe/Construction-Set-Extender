#pragma once

#include "TESObject.h"

//	EditorAPI: TESObjectCONT class.

/*
	...
*/

// BC
class TESObjectCONT : public TESBoundAnimObject, public TESContainer, public TESFullName, public TESModel, public TESScriptableForm, public TESWeightForm
{
public:
	enum ContainerFlags
	{
		kContainerFlags_Respawns		= /*01*/ 0x2
	};

	// members
	//     /*00*/ TESBoundAnimObject
	//     /*58*/ TESContainer
	//     /*68*/ TESFullName
	//	   /*74*/ TESModel
	//	   /*98*/ TESScriptableForm
	//	   /*A4*/ TESWeightForm
	/*AC*/ UInt32					unkAC;
	/*B0*/ TESSound*				openSound;
	/*B4*/ TESSound*				closeSound;
	/*B8*/ UInt8					containerFlags;
	/*B9*/ UInt8					padB9[3];
};