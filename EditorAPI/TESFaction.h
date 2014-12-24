#pragma once

#include "TESForm.h"

//	EditorAPI: TESFaction class.

/*
    ...
*/

// 50
class TESFaction : public TESFormIDListView, public TESFullName, public TESReactionForm
{
public:
	enum FactionFlags
	{
		kFactionFlags_HiddenFromPC	= /*00*/ 0x01,
		kFactionFlags_Evil			= /*01*/ 0x02,
		kFactionFlags_SpecialCombat = /*02*/ 0x04
	};

	// 28
	struct RankData
	{
		BSString			maleRank;
		BSString			femaleRank;
		TESTexture			insignia;
	};
	typedef tList<RankData> RankDataListT;

	// members
	//     /*00*/ TESForm
	//	   /*24*/ TESFullName
	//	   /*30*/ TESReactionForm
	/*40*/ UInt8						factionFlags;
	/*41*/ UInt8						factionFlagsPad41[3];
	/*44*/ float						crimeGoldMultiplier;
	/*48*/ RankDataListT				rankDataList;
};