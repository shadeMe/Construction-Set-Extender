#pragma once

#include "TESForm.h"

//	EditorAPI: TESTopic class.

/*
	...
*/

class	TESQuest;
class	TESTopicInfo;

typedef NiTLargeArray<TESTopicInfo*> TopicInfoArray;		// actually derives from it

// 3C
class TESTopic : public TESForm, public TESFullName
{
public:
	// 20
	struct TopicData
	{
		// members
		/*00*/ TESQuest*		parentQuest;
		/*04*/ TopicInfoArray	questInfos;
		/*1C*/ TESQuest*		unk1C;		// serialized as the QSTR subrecord
	};

	typedef tList<TopicData>	TopicDataListT;

	// members
	//     /*00*/ TESForm
	//	   /*24*/ TESFullName
	/*30*/ UInt8			topicType;		// value of type TESTopicInfo::kInfoType_XXX
	/*31*/ UInt8			pad31[3];
	/*34*/ TopicDataListT	topicData;
};