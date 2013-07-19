#pragma once

#include "TESForm.h"
#include "Magic.h"
#include "EffectSetting.h"

//	EditorAPI: EffectItem class and co.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	EffectItem - data for individual effects on magic items.  Stores effect setting & override values + additional data
*/

class   TESForm;
class   TESFile;
class   Actor;
class   Script;

// 34
class EffectItem
{
public:
	// 18
	class ScriptEffectInfo
	{
	public:
		// members
		/*00*/ UInt32         scriptFormID;
		/*04*/ UInt32         school;
		/*08*/ BSString		  name;
		/*10*/ UInt32         fxMgefCode; // effect setting from which FX are taken
		/*14*/ bool           hostile;
		/*15*/ UInt8          pad15[3];
	};
	STATIC_ASSERT(sizeof(ScriptEffectInfo) == 0x18);

	// members
	// NOTE: setting these directly can break invariants assumed by game code (that 'effect' is a valid pointer, etc.)
	// wherever possible, use the Get/Set methods instead
	/*00*/ UInt32             mgefCode;
	/*04*/ SInt32             magnitude;
	/*08*/ SInt32             area;
	/*0C*/ SInt32             duration;
	/*10*/ UInt32             range;
	/*14*/ UInt32             actorValue;		// this can be set directly, as no methods exist to do so
	/*18*/ ScriptEffectInfo*  scriptInfo;		// set to a new ScriptEffectInfo if SEFF, null otherwise
	/*1C*/ EffectSetting*     effect;			// quite a bit of game code will CTD if this is null (editor as well?)
	/*20*/ float              cost;
	// These fields are initialized before or just after an EffectItem is edited in the Efit dialog
	/*24*/ EffectSetting*     filterMgef; // filter effect from source magic item
	/*28*/ SInt32             origBaseMagicka; // pre-editing base magicka cost
	/*2C*/ SInt32             origItemMagicka; // pre-editing base magicka cost of magic item
	/*30*/ UInt32             origItemMastery; // pre-editing mastery of magic item (not used by vanilla dialog)
};
STATIC_ASSERT(sizeof(EffectItem) == 0x34);

// Actually derived from BSSimpleList<EffectItem*>, as defined in COEF
// DC
class EffectItemList
{
public:
	typedef tList<EffectItem> EffectItemListT;

	// members
	//     /*00*/ void**				vtbl;
	/*04*/ EffectItemListT				effects;
	/*0C*/ UInt32						hostileCount; // includes 'ignored' effects, so technically broken, but no vanilla effects are ignored
	/*10*/ EffectSetting				filterEffect; // used as a filter for allowable magic effects

	virtual void						VFn00();

	// methods
	void								PopulateListView(HWND ListView);
};
STATIC_ASSERT(sizeof(EffectItemList) == 0xDC);