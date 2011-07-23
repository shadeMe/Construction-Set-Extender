#pragma once

#include "TESForm.h"

//	EditorAPI: TESWeather class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// 180
class TESWeather : public TESFormIDListView
{
public:
	// members
	//     /*00*/ TESFormIDListView
	/*24*/ TESTexture			upperLayer;
	/*3C*/ TESTexture			lowerLayer;
	/*54*/ TESModel				precipitationMesh;
	/*78*/
};