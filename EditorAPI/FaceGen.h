#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"


//	EditorAPI: FaceGen class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	Container class for facegen related data.
*/

namespace EditorAPI
{
	// 18
	struct FaceGenFaceData18
	{
		/*00*/ UInt32			unk00;
		/*04*/ UInt32			unk04;	// init to 1
		/*08*/ UInt32			unk08;	// seen uninitialized
		/*0C*/ void*			unk0C;	
		/*10*/ void*			unk10;	
		/*14*/ void*			unk14;
	};

	// C4
	struct FaceGenUndo
	{
		/*00*/ FaceGenFaceData18		unk00[4];	// previous/base face data?
		/*60*/ FaceGenFaceData18		unk60[4];	// current face data?
		/*C0*/ UInt8					unkC0;		// initialized to 0
		/*C1*/ UInt8					unkC1[3];
	};

	typedef NiTArray<FaceGenUndo*> FaceGenUndoListT;
}