#pragma once

#include "TESForm.h"
#include "Script.h"
#include "TESConditionItem.h"

//	EditorAPI: TESTopicInfo class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

class	TESTopic;

// A0
class TESTopicInfo : public TESForm
{
public:
	typedef tList<TESTopic> TopicListT;

	// 08
	struct LinkedTopics
	{
		/*00*/ TopicListT		topicsLinkedFrom;
		/*04*/ TopicListT		topicsLinkedTo;		// doubles as choices for appropriate infotype
	};

	// 24
	struct ResponseData
	{
		enum
		{
			kEmotionType_Neutral	= 0,
			kEmotionType_Anger,
			kEmotionType_Disgust,
			kEmotionType_Fear,
			kEmotionType_Sad,
			kEmotionType_Happy,
			kEmotionType_Surprise,
			kEmotionType__MAX
		};

		/*00*/ UInt32			emotionType;
		/*04*/ UInt32			emotionValue;
		/*08*/ UInt32			unk08;
		/*0C*/ UInt32			unk0C;
		/*10*/ BSString		responseText;
		/*18*/ BSString		actorNotes;
		/*20*/ UInt32			unk20;
	};
	typedef tList<ResponseData> ResponseListT;

	enum
	{
		kInfoType_Topic					= 0,
		kInfoType_Conversation,
		kInfoType_Combat,
		kInfoType_Persuasion,
		kInfoType_Detection,
		kInfoType_Service,
		kInfoType_Miscellaneous,
		kInfoType__MAX
	};

	enum
	{
		kTopicInfoFlags_Goodbye			=	/*00*/ 0x001,
		kTopicInfoFlags_Random			=	/*01*/ 0x002,
		kTopicInfoFlags_SayOnce			=	/*02*/ 0x004,
		kTopicInfoFlags_Unk03			=	/*03*/ 0x008,
		kTopicInfoFlags_InfoRefusal		=	/*04*/ 0x010,
		kTopicInfoFlags_RandomEnd		=	/*05*/ 0x020,
		kTopicInfoFlags_RunforRumors	=	/*06*/ 0x040
	};

	// members
	//     /*00*/ TESForm
	/*24*/ TESTopic*			unk024;			// always NULL ?
	/*28*/ ConditionListT		conditions;
	/*30*/ UInt16				unk30;			// init to -1. used to determine previous info	?
	/*32*/ UInt16				infotype;
	/*34*/ UInt8				topicInfoFlags;
	/*35*/ UInt8				pad35[3];
	/*38*/ TopicListT			addedTopics;
	/*40*/ LinkedTopics*		linkedTopics;
	/*44*/ ResponseListT		responseList;
	/*4C*/ Script				resultScript;
};
typedef TESTopicInfo::ResponseData	DialogResponse;