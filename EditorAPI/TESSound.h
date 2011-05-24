#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESSound class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

namespace EditorAPI
{
	// 70
	class TESSound : public TESBoundAnimObject, public TESSoundFile
	{
	public:
		enum SoundFlags
		{
			kSoundFlags_RandomFreqShift		= /*00*/ 0x01,
			kSoundFlags_PlayAtRandom		= /*01*/ 0x02,
			kSoundFlags_EnvironmentIgnored	= /*02*/ 0x04,
			kSoundFlags_RandomLocation		= /*03*/ 0x08,
			kSoundFlags_Loop				= /*04*/ 0x10,
			kSoundFlags_MenuSound			= /*05*/ 0x20,
			kSoundFlags_2D					= /*06*/ 0x40,
			kSoundFlags_360LFE				= /*07*/ 0x80
		};

		// members
		//     /*00*/ TESBoundAnimObject
		//     /*58*/ TESSoundFile
		/*64*/ UInt8					minAttenuation;		// actual value = CS value * 5
		/*65*/ UInt8					maxAttenuation;		// actual value = CS value * 100
		/*66*/ UInt8					frequencyAdjust;
		/*67*/ UInt8					unk67;				// init to 0
		/*68*/ UInt32					soundFlags;
		/*6C*/ UInt16					staticAttenuation;	// actual value = CS value * 100
		/*6E*/ UInt16					startEndTimes;		// high byte - start time; low byte - end time
															// hour		= 3 * time / 32
															// minutes	= fractional(hour) * 60
	};
}
