#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESObject.h"

//	EditorAPI: TESSubSpace class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

// 64
class TESSubSpace : public TESBoundObject
{
public:
	// members
	//     /*00*/ TESBoundObject
	/*58*/ UInt16					dimensionX;
	/*5A*/ UInt16					dimensionY;
	/*5C*/ UInt16					dimensionZ;
	/*5E*/ UInt16					pad5E;
	/*60*/ float					unk60;			// initialized to sqrt((x/2)² + (y/2)² + (z/2)²)
													// the name of the formula escapes me...
};