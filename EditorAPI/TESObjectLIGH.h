#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESObjectLIGH class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// DC
	class TESObjectLIGH : public TESBoundAnimObject, public TESFullName, public TESModel, public TESIcon, public TESScriptableForm, public TESWeightForm, public TESValueForm
	{
	public:
		enum LightFlags
		{
			kLightFlags_Dynamic			= /*00*/ 0x001,
			kLightFlags_CanCarry		= /*01*/ 0x002,
			kLightFlags_Negative		= /*02*/ 0x004,
			kLightFlags_Flicker			= /*03*/ 0x008,
			kLightFlags_Unk04			= /*04*/ 0x010,
			kLightFlags_OffByDefault	= /*05*/ 0x020,
			kLightFlags_FlickerSlow		= /*06*/ 0x040,
			kLightFlags_Pulse			= /*07*/ 0x080,
			kLightFlags_PulseSlow		= /*08*/ 0x100,
			kLightFlags_SpotLight		= /*09*/ 0x200,
			kLightFlags_SpotShadow		= /*10*/ 0x400
		};

		// members
		//     /*00*/ TESBoundAnimObject
		//     /*58*/ TESFullName
		//     /*64*/ TESModel
		//	   /*88*/ TESIcon
		//	   /*A0*/ TESScriptableForm
		//	   /*AC*/ TESWeightForm
		//	   /*B4*/ TESValueForm
		/*BC*/ UInt32					time;
		/*C0*/ UInt32					radius;
		/*C4*/ RGBA						color;
		/*C8*/ UInt32					lightFlags;
		/*CC*/ float					fallOffExponent;
		/*D0*/ float					FOV;
		/*D4*/ float					fade;
		/*D8*/ TESSound*				loopSound;	
	};
}