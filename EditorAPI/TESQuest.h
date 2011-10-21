#pragma once

#include "TESForm.h"
#include "Script.h"
#include "TESConditionItem.h"

//	EditorAPI: TESQuest class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

class	TESObjectREFR;

// 74
class TESQuest : public TESFormIDListView, public TESScriptableForm, public TESIcon, public TESFullName
{
public:
	// 0C
	struct StageData
	{
		// 68
		struct QuestStageItem
		{
			enum
			{
				kQuestStageItemFlags_CompletesQuest	=	/*00*/ 0x1
			};

			/*00*/ UInt8			flags;
			/*01*/ UInt8			pad01[3];
			/*04*/ ConditionListT	conditions;
			/*0C*/ Script			resultScript;		// marked as temporary by the ctor (why?)
			/*60*/ BSStringT		logEntry;
		};
		typedef tList<QuestStageItem>	StageItemListT;

		/*00*/ UInt8			index;
		/*01*/ UInt8			unk01;
		/*02*/ UInt8			pad02[2];
		/*04*/ StageItemListT	stageItemList;

		// methods
		void					RemoveStageItem(QuestStageItem* Item);
	};
	typedef tList<StageData>	StageListT;

	// 10
	struct TargetData
	{
		enum
		{
			kTargetDataFlags_CompassMarkersIgnoreLocks	=	/*00*/ 0x1
		};

		/*00*/ UInt8			flags;
		/*01*/ UInt8			pad01[3];
		/*04*/ ConditionListT	conditionList;
		/*0C*/ TESObjectREFR*	target;
	};
	typedef tList<TargetData>	TargetListT;

	enum
	{
		kQuestFlags_StartGameEnabled				= /*00*/ 0x1,
		kQuestFlags_Completed						= /*01*/ 0x2,
		kQuestFlags_AllowRepeatedConversationTopics	= /*02*/ 0x4,
		kQuestFlags_AllowRepeatedStages				= /*03*/ 0x8
	};

	// members
	//     /*00*/ TESFormIDListView
	//     /*24*/ TESScriptableForm
	//     /*30*/ TESIcon
	//     /*48*/ TESFullName
	/*54*/ UInt8				questFlags;
	/*55*/ UInt8				priority;
	/*56*/ UInt8				pad56[2];
	/*58*/ StageListT			stageList;
	/*60*/ TargetListT			targetList;
	/*68*/ ConditionListT		conditions;
	/*70*/ UInt32				unk70;			// seen pointer to a struct {Script*, ...}

	// method
	void						SetStartGameEnabledFlag(bool State);
	void						SetAllowedRepeatedStagesFlag(bool State);
};