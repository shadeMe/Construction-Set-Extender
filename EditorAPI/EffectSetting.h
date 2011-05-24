#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"
#include "Magic.h"

//	EditorAPI: EffectSetting class.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    EffectSetting, aka 'Magic Effect'.  This class stores relatvent global data for each of the 'named'
    magic effects (e.g. 'Fire Damage') in Oblivion, including 3D effects, icon, base cost, etc.

    EffectSettings are not tracked through the DataHandler like other forms - they have their own map,
    indexed by 'magic effect code'.  This code is static, and is used to uniquely identify an effect
    through the engine *instead* of formids.  EffectSettings are hard coded into the engine without
    preset formids.
*/

namespace EditorAPI
{
	class   TESObjectLIGH;
	class   TESEffectShader;
	class   TESSound;
	class   EffectSetting;  

	// CC
	class EffectSetting : public TESFormIDListView, public TESModel, public TESDescription, public TESFullName, public TESIcon
	{
	public:
		enum EffectSettingFlags
		{   //                                      # Can be overriden in a vanilla mod file (other overrides are discarded by EffectSetting.Load())
			//                                        + Displayed in CS with a checkbox  
			kMgefFlag__Overridable          = /*    #   */ 0x0FC03C00, // bitmask of overridable flags
			kMgefFlag_Hostile               = /* 00   + */ 0x00000001,
			kMgefFlag_Recovers              = /* 01   + */ 0x00000002, // used for some effect types - changes are reversed when removed
			kMgefFlag_Detrimental           = /* 02   + */ 0x00000004, // used for ValueModifier effects - AV is decreased rather than increased
			kMgefFlag_MagnitudeIsPercent    = /* 03   + */ 0x00000008, // for display purposes only
			kMgefFlag_OnSelf                = /* 04   + */ 0x00000010,
			kMgefFlag_OnTouch               = /* 05   + */ 0x00000020,
			kMgefFlag_OnTarget              = /* 06   + */ 0x00000040,
			kMgefFlag_NoDuration            = /* 07   + */ 0x00000080,
			kMgefFlag_NoMagnitude           = /* 08   + */ 0x00000100,
			kMgefFlag_NoArea                = /* 09   + */ 0x00000200,
			kMgefFlag_FXPersists            = /* 0A # + */ 0x00000400, // Effect (Hit) shader persists until effect expires (vs. playing only once)
			kMgefFlag_Spells                = /* 0B # + */ 0x00000800, // ignored in CS, filters for spellmaking menu in game
			kMgefFlag_Enchantments          = /* 0C # + */ 0x00001000, // ignored in CS, filters for enchanting menu in game
			kMgefFlag_NoAlchemy             = /* 0D # + */ 0x00002000, // ignored in CS, no effect in game
			kMgefFlag_UnknownF              = /* 0E     */ 0x00004000, // no effects have this flag set, checks the missing checkbox (nDlgItem = 0x666)
			kMgefFlag_NoRecast              = /* 0F   + */ 0x00008000, // no effects have this flag set, not sure quite what it does
			kMgefFlag_UseWeapon             = /* 10   + */ 0x00010000, // MgefParam is formID of summoned weapon
			kMgefFlag_UseArmor              = /* 11   + */ 0x00020000, // MgefParam is formID of summoned armor
			kMgefFlag_UseActor              = /* 12   + */ 0x00040000, // MgefParam is formID of summoned actor
			kMgefFlag_UseSkill              = /* 13   + */ 0x00080000, // uses skill avCode stored on effect items
			kMgefFlag_UseAttribute          = /* 14   + */ 0x00100000, // uses attribute avCode stored on effect items
			kMgefFlag_PCHasEffect           = /* 15     */ 0x00200000, // Whether no not the player 'knows' the effect, e.g. useable in spellmaking
																		// Can technically be overriden, but is immediately forced to zero during the
																		// EffectSetting.Load() routine. Set for any spell, or only one pc can cast?
																		// List of effects player knows is apparently stored as mgefCodes in savegame
			kMgefFlag_Disabled              = /* 16 #   */ 0x00400000, // No default effects have this.  many (all?) of the methods in EffectItemList 
																		// that loop over the effect items ignore effects with this flag.
																		// Spells with an effect with this flag are apparently uncastable.
			kMgefFlag_UnknownO              = /* 17 #   */ 0x00800000, // POSN,DISE - these effects have *only* this bit set, perhaps a flag for 'meta' effects?
																		// exempts effect from valid range check during creation
																		// makes effect unavailable in spellmaking alter
			kMgefFlag_UseActorValue         = /* 18 #   */ 0x01000000, // MgefParam field is avCode for target value
																		// Once set (by default or by a previously loaded mod file), it *cannot* be unset in
																		// EffectSetting.Load(), and the 'Data' field cannot be overriden either.
			kMgefFlag_ProjectileTypeLow     = /* 19 #   */ 0x02000000, // 2-bit field for projectile type: {00 = Ball, 01 = Spray, 10 = Bolt, 11 = Fog}
			kMgefFlag_ProjectileTypeHigh    = /* 1A #   */ 0x04000000, //  These are set with a pull-down list rather than a checkbox
			kMgefFlag_NoHitVisualFX         = /* 1B # + */ 0x08000000, // No Hit shader or Hit model VFX are played on target.  Doesn't affect sounds.
			kMgefFlag_PersistOnDeath        = /* 1C     */ 0x10000000, // Effect is not automatically removed when its target dies
			kMgefFlag_ExplodesWithForce     = /* 1D     */ 0x20000000, // causes explosion that can move loose objects (e.g. ragdolls)
			kMgefFlag_MagnitudeIsLevel      = /* 1E     */ 0x40000000, // for display purposes only
			kMgefFlag_MagnitudeIsFeet       = /* 1F     */ 0x80000000, // for display purposes only
		};

		// members
		//     /*00*/ TESForm           
		//     /*24*/ TESModel
		//     /*48*/ TESDescription
		//     /*58*/ TESFullName 
		//     /*64*/ TESIcon    
		/*7C*/ FilterFunc         filterFunc; // not saved. called if present on a template effect during Filter(), 
											// using target effect & filterParam as args.
		/*80*/ void*              filterParam; // not saved. passed to filterFunc if it's called during Filter()
		/*84*/ UInt32             mgefFlags;
		/*88*/ float              baseCost;
		/*8C*/ UInt32             mgefParam;
		/*90*/ UInt32             school;
		/*94*/ UInt32             resistAV; // an actor value i.e. kActorVal_ResistFire
		/*98*/ UInt16             numCounters; // count of counter effects, is size of counterArray
		/*9A*/ UInt16             padMgef06E;
		/*9C*/ TESObjectLIGH*     light;
		/*A0*/ float              projSpeed;
		/*A4*/ TESEffectShader*   effectShader;
		/*A8*/ TESEffectShader*   enchantShader;
		/*AC*/ TESSound*          castingSound;
		/*B0*/ TESSound*          boltSound;
		/*B4*/ TESSound*          hitSound;
		/*B8*/ TESSound*          areaSound;
		/*BC*/ float              enchantFactor;
		/*C0*/ float              barterFactor;
		/*C4*/ UInt32             mgefCode; // in CS, editor ID is initialized to this re-interpreted as a 4-character string
		/*C8*/ UInt32*            counterArray; // counters stored as dynamic array[numCounters] of effect codes
	};
}
