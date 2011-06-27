#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"


//	EditorAPI: ActorValues.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    'ActorValues' is a generic container class used to group the various global data and functions related 
    to actor values.
*/

class ActorValues
{
public:    
	// ActorValues - enumeration
	enum ActorValueCodes
	{
		kActorVal_Strength              = 0x00,
		kActorVal_Intelligence,
		kActorVal_Willpower,
		kActorVal_Agility,
		kActorVal_Speed,
		kActorVal_Endurance             = 0x05,
		kActorVal_Personality,
		kActorVal_Luck                  = 0x07,
		kActorVal__MAX_Attribute,
		//
		kActorVal_Health                = 0x08,
		kActorVal_Magicka,
		kActorVal_Fatigue               = 0x0A,
		kActorVal_Encumbrance           = 0x0B,
		kActorVal__MAX_DerivedAttribute,
		//
		kActorVal_Armorer               = 0x0C,
		kActorVal_Athletics,
		kActorVal_Blade,
		kActorVal_Block                 = 0x0F,
		kActorVal_Blunt                 = 0x10,
		kActorVal_HandToHand,
		kActorVal_HeavyArmor,
		kActorVal_Alchemy,            
		kActorVal_Alteration,
		kActorVal_Conjuration           = 0x15,
		kActorVal_Destruction,
		kActorVal_Illusion,
		kActorVal_Mysticism             = 0x18,
		kActorVal_Restoration,
		kActorVal_Acrobatics            = 0x1A,
		kActorVal_LightArmor,
		kActorVal_Marksman,
		kActorVal_Mercantile,
		kActorVal_Security,
		kActorVal_Sneak                 = 0x1F,
		kActorVal_Speechcraft           = 0x20,
		kActorVal__MAX_Skill,
		//
		kActorVal_Aggression            = 0x21,
		kActorVal_Confidence,
		kActorVal_Energy,
		kActorVal_Responsibility        = 0x24,
		kActorVal__MAX_Trait,
		//
		kActorVal_Bounty                = 0x25,
		kActorVal_Fame,
		kActorVal_Infamy                = 0x27,
		kActorVal__MAX_Virtue,
		//
		kActorVal_MagickaMultiplier     = 0x28,
		kActorVal_NightEyeBonus         = 0x29,
		kActorVal_AttackBonus           = 0x2A,
		kActorVal_DefendBonus           = 0x2B,
		kActorVal_CastingPenalty        = 0x2C,
		kActorVal_Blindness,
		kActorVal_Chameleon,
		kActorVal_Invisibility          = 0x2F,
		kActorVal_Paralysis             = 0x30,
		kActorVal_Silence,
		kActorVal_Confusion,
		kActorVal_DetectItemRange,
		kActorVal_SpellAbsorbChance,
		kActorVal_SpellReflectChance    = 0x35,
		kActorVal_SwimSpeedMultiplier   = 0x36,
		kActorVal_WaterBreathing,
		kActorVal_WaterWalking          = 0x38,
		kActorVal_StuntedMagicka,
		kActorVal_DetectLifeRange       = 0x3A,
		kActorVal_ReflectDamage,
		kActorVal_Telekinesis,
		kActorVal_ResistFire,
		kActorVal_ResistFrost,
		kActorVal_ResistDisease         = 0x3F,
		kActorVal_ResistMagic           = 0x40,
		kActorVal_ResistNormalWeapons,
		kActorVal_ResistParalysis,
		kActorVal_ResistPoison,
		kActorVal_ResistShock,
		kActorVal_Vampirism             = 0x45,
		kActorVal_Darkness,
		kActorVal_ResistWaterDamage     = 0x47,
		//
		kActorVal__MAX                  = 0x48, // 0x48, 0xFF, and 0xFFFFFFFF are all used to indicate
		kActorVal__NONE                 = 0xFF, // 'invalid' or 'no av'.  This can cause confusion (and bugs)
		kActorVal__UBOUND               = 0xFFFFFFFF, // used with EffectSetting::ResistAV 
	};

	// ActorValues - groups
	// inferred from CS code
	enum ActorValueGroups
	{
		kAVGroup_Attribute          = 0x0,  // 0x00 - 0x07
		kAVGroup_DerivedAttribute   = 0x1,  // 0x08 - 0x0B
		kAVGroup_Skill              = 0x2,  // 0x0C - 0x20
		kAVGroup_Trait              = 0x3,  // 0x21 - 0x24 (for AI forms)
		kAVGroup_Virtue             = 0x4,  // 0x25 - 0x27 (for player only?)
		kAVGroup_Misc               = 0x5,  // 0x28 - 0x29  not sure of name or purpose
		kAVGroup_Combat             = 0x6,  // 0x2A - ????  not sure of name or purpose
		kAVGroup__MAX               = 0x7,  
	};

	// ActorValues - modifiers
	enum ActorValueModifiers
	{
		kAVModifier_Max         = 0x0, // stored in MiddleLow Process
		kAVModifier_Offset      = 0x1, // stored in Actor Ref
		kAVModifier_Damage      = 0x2, // stored in Low Process
	};

	// Skill mastery levels
	enum SkillMasteryLevels 
	{   
		kMastery_Novice         = 0x0,
		kMastery_Apprentice     = 0x1,
		kMastery_Journeyman     = 0x2,
		kMastery_Expert         = 0x3,
		kMastery_Master         = 0x4,
		kMastery__MAX           = 0x5,
	};
};