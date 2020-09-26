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
	/*078*/ UInt32				unk078[(0x138 - 0x78) >> 2];
	/*138*/ SoundListT			soundList;
	/*140*/ UInt32				unk140[(0x180 - 0x140) >> 2];
};
STATIC_ASSERT(sizeof(TESWeather) == 0x180);
