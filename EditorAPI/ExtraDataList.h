#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

//	EditorAPI: ExtraDataList class and its derivatives.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    ExtraData is Bethesda's tool for attaching arbitrary information to other classes.
    BaseExtraList seems to be the generic extra data manager, and is used for DialogExtraData
    ExtraDataList seems to be targeted specifically to forms (?)
*/

namespace EditorAPI
{
	class	BSExtraData;

	// 14
	class BaseExtraList
	{
	public:
		// members
		//     /*00*/ void**			vtbl          
		/*04*/ BSExtraData*				extraList;				// LL of extra data nodes
		/*08*/ UInt8					extraTypes[0x0C];		// if a bit is set, then the extralist should contain that extradata
																// bits are numbered starting from the lsb
	};

	// 14
	class ExtraDataList : public BaseExtraList
	{
	public:
		// no additional members
	};
}