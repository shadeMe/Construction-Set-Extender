#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESActorBase.h"

//	EditorAPI: TESCreature class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
	...
*/

class	TESClass;
class	TESCombatStyle;
class	TESSound;

// 2C
class CreatureSounds
{
public:
	enum SoundType
	{
		kSoundType_LeftFoot				= 0x0,
		kSoundType_RightFoot,
		kSoundType_LeftBack,
		kSoundType_RightBack,
		kSoundType_Idle,
		kSoundType_Aware,
		kSoundType_Attack,
		kSoundType_Hit,
		kSoundType_Death,
		kSoundType_Weapon,
		kSoundType__MAX,
	};

	// 0C
	struct SoundEntry
	{
		// members
		/*00*/ TESSound*			sound;
		/*04*/ UInt8				chance;
		/*05*/ UInt8				pad05[3];
		/*04*/ UInt32				type;
	};
	typedef tList<SoundEntry> SoundListT;

	// members
	//     /*00*/ void**			vtbl;
	/*04*/ SoundListT*				leftFootSounds;
	/*08*/ SoundListT*				rightFootSounds;
	/*0C*/ SoundListT*				leftBackSounds;
	/*10*/ SoundListT*				rightBackSounds;
	/*14*/ SoundListT*				idleSounds;
	/*18*/ SoundListT*				awareSounds;
	/*1C*/ SoundListT*				hitSounds;
	/*20*/ SoundListT*				deathSounds;
	/*24*/ SoundListT*				weaponSounds;

	virtual void					UnkVFn00();
};

// 18C
class TESCreature : public TESActorBase, public TESAttackDamageForm, public TESModelList
{
public:
	// 04
	union CreatureSoundData
	{
		// members
		/*00*/ CreatureSounds*				sounds;
		/*00*/ TESCreature*					soundBase;			// the creature the sounds are inherited from
	};

	enum CreatureType
	{
		kCreatureType_Creature				= 0x0,
		kCreatureType_Daedra,
		kCreatureType_Undead,
		kCreatureType_Humanoid,
		kCreatureType_Horse,
		kCreatureType_Giant,
		kCreatureType__MAX
	};

	// members
	//     /*000*/ TESActorBase
	//	   /*118*/ TESAttackDamageForm
	//	   /*120*/ TESModelList
	/*134*/ CreatureSoundData				soundData;
	/*138*/ UInt8							creatureType;
	/*139*/ UInt8							combatSkill;
	/*13A*/ UInt8							magicSkill;
	/*13B*/ UInt8							stealthSkill;
	/*13C*/ UInt16							soulLevel;
	/*13E*/ UInt8							attackReach;							
	/*13F*/ UInt8							pad13F;
	/*140*/	float							turningSpeed;
	/*144*/ float							footWeight;
	/*148*/ float							baseScale;
	/*14C*/ TESCombatStyle*					combatStyle;
	/*150*/ TESModel						bloodSpray;
	/*174*/ TESTexture						bloodTexture;
};