#pragma once

//	EditorAPI: FaceGen class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	Container class for facegen related data.
*/

// primary data block used to store face data
// 18
struct FaceGenFaceDataBlock
{
	/*00*/ UInt32			unk00;
	/*04*/ UInt32			unk04;	// init to 1
	/*08*/ UInt32			unk08;	// seen uninitialized
	/*0C*/ void*			unk0C;
	/*10*/ void*			unk10;
	/*14*/ void*			unk14;
};

// 60
struct FaceGenFaceParameters
{
	/*00*/ FaceGenFaceDataBlock		data[4];
};

// C4
struct FaceGenUndo
{
	/*00*/ FaceGenFaceParameters	unk00;		// previous/base face data?
	/*60*/ FaceGenFaceParameters	unk60;		// current face data?
	/*C0*/ UInt8					unkC0;		// initialized to 0
	/*C1*/ UInt8					unkC1[3];
};

typedef NiTArray<FaceGenUndo*> FaceGenUndoListT;