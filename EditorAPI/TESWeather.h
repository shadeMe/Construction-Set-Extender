#pragma once

#include "TESForm.h"

//	EditorAPI: TESWeather class.

/*
	...
*/

// 180
class TESWeather : public TESFormIDListView
{
public:
	// 08
	struct SoundData
	{
		enum SoundType
		{
			kSoundType_Default			=	0x0,
			kSoundType_Precipitation,
			kSoundType_Wind,
			kSoundType_Thunder,
			kSoundType__MAX
		};

		/*00*/ UInt32			soundFormID;
		/*04*/ UInt32			soundType;
	};
	typedef tList<SoundData>	SoundListT;

	// members
	//     /*000*/ TESFormIDListView
	/*024*/ TESTexture			upperLayer;
	/*03C*/ TESTexture			lowerLayer;
	/*054*/ TESModel			precipitationMesh;
	/*078*/
	/*138*/ SoundListT			soundList;
	/*140*/
};