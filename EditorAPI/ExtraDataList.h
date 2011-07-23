#pragma once

#include "ExtraData.h"

//	EditorAPI: ExtraDataList class and its derivatives.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    ExtraData is Bethesda's tool for attaching arbitrary information to other classes.
    BaseExtraList seems to be the generic extra data manager, and is used for DialogExtraData
    ExtraDataList seems to be targeted specifically to forms (?)
*/

class	TESForm;
class	TESObjectREFR;
class	TESGlobal;

// 14
class BaseExtraList
{
public:
	// members
	//     /*00*/ void**			vtbl
	/*04*/ BSExtraData*				extraList;				// LL of extra data nodes
	/*08*/ UInt8					extraTypes[0x0C];		// if a bit is set, then the extralist should contain that extradata
															// bits are numbered starting from the lsb

	virtual void					VFn00();
};
STATIC_ASSERT(sizeof(BaseExtraList) == 0x14);

// 14
class ExtraDataList : public BaseExtraList
{
public:
	// no additional members

	// methods
	void							Link(TESForm* LinkedForm);
	void							CopyList(ExtraDataList* Source);
	BSExtraData*					GetExtraDataByType(UInt8 Type);

	void							ModExtraEnableStateParent(TESObjectREFR* Parent);
	void							ModExtraOwnership(TESForm* Owner);
	void							ModExtraGlobal(TESGlobal* Global);
	void							ModExtraRank(int Rank);
	void							ModExtraCount(UInt32 Count);
};
STATIC_ASSERT(sizeof(ExtraDataList) == 0x14);

extern CRITICAL_SECTION*		g_ExtraListCS;