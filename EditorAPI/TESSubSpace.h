#pragma once

#include "TESObject.h"

//	EditorAPI: TESSubSpace class.

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
};