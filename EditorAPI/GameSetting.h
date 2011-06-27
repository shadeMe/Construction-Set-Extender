#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"
#include "Setting.h"

//	EditorAPI: GameSetting class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    GameSetting is the class for all generic variables used in the game mechanics, e.g. "fActorLuckSkillMult"
    GameSetting derives from TESForm in the CS
*/

// 2C
class GameSetting : public TESFormIDListView, public Setting
{
public:
	// members
	//     /*00*/ TESForm
	//     /*24*/ Setting
};
