#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESObjectDOOR class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// B0
	class TESObjectDOOR : public TESBoundAnimObject, public TESFullName, public TESModel, public TESScriptableForm
	{
	public:
		enum DoorFlags
		{
			kDoorFlags_OblivionGame			= /*00*/ 0x01,
			kDoorFlags_AutomaticDoor		= /*01*/ 0x02,
			kDoorFlags_Hidden				= /*02*/ 0x04,
			kDoorFlags_MinimalUse			= /*03*/ 0x08
		};

		typedef tList<TESForm> RandomTeleportListT;		// only cells (exteriors) and worldspaces

		// members
		//     /*00*/ TESBoundAnimObject
		//     /*58*/ TESFullName
		//     /*64*/ TESModel
		//	   /*88*/ TESScriptableForm
		/*94*/ UInt32					unk94;
		/*98*/ TESSound*				openSound;
		/*9C*/ TESSound*				closeSound;
		/*A0*/ TESSound*				loopSound;
		/*A4*/ UInt8					doorFlags;
		/*A5*/ UInt8					padA5[3];
		/*A8*/ RandomTeleportListT		randomTeleportList;
	};
}