#pragma once

#include "TESForm.h"

//	EditorAPI: TESCombatStyle class.

/*
	...
*/

// A4
class TESCombatStyle : public TESForm
{
public:
	enum
	{
		kCombatStyleFlags_DoNotAccquire					=	/*00*/ 0x01,
		kCombatStyleFlags_AdvancedSettings				=	/*00*/ 0x01,
		kCombatStyleFlags_ChanceBasedPowerAttack		=	/*01*/ 0x02,
		kCombatStyleFlags_IgnoreAlliesInArea			=	/*02*/ 0x04,
		kCombatStyleFlags_WillYield						=	/*03*/ 0x08,
		kCombatStyleFlags_RejectsYield					=	/*04*/ 0x10,
		kCombatStyleFlags_FleeingDisabled				=	/*05*/ 0x20,
		kCombatStyleFlags_PrefersRanged					=	/*06*/ 0x40,
		kCombatStyleFlags_MeleeAlertOK					=	/*07*/ 0x80
	};

	// 54
	struct AdvancedSettings
	{
		/*00*/ float				dodgeFatigueModMult;
		/*04*/ float				dodgeFatigueModBase;
		/*08*/ float				encumSpeedModBase;
		/*0C*/ float				encumSpeedModMult;
		/*10*/ float				dodgeUnderAttackMult;
		/*14*/ float				dodgeNotUnderAttackMult;
		/*18*/ float				dodgeBackUnderAttackMult;
		/*1C*/ float				dodgeBackNotUnderAttackMult;
		/*20*/ float				dodgeFWAttackingMult;
		/*24*/ float				dodgeFWNotAttackingMult;
		/*28*/ float				blockSkillModMult;
		/*2C*/ float				blockSkillModBase;
		/*30*/ float				blockUnderAttackMult;
		/*34*/ float				blockNotUnderAttackMult;
		/*38*/ float				attackSkillModMult;
		/*3C*/ float				attackSkillModBase;
		/*40*/ float				attackUnderAttackMult;
		/*44*/ float				attackNotUnderAttackMult;
		/*48*/ float				attackDuringBlockMult;
		/*4C*/ float				powerAttackFatigueModBase;
		/*50*/ float				powerAttackFatigueModMult;
	};

	// 7C
	struct CombatStyleData
	{
		/*00*/ UInt8		dodgeChance;
		/*01*/ UInt8		LRChance;
		/*02*/ UInt8		pad02[2];
		/*04*/ float		dodgeLRTimerMin;
		/*08*/ float		dodgeLRTimerMax;
		/*0C*/ float		dodgeFWTimerMin;
		/*10*/ float		dodgeFWTimerMax;
		/*14*/ float		dodgeBackTimerMin;
		/*18*/ float		dodgeBackTimerMax;
		/*1C*/ float		idleTimerMin;
		/*20*/ float		idleTimerMax;
		/*24*/ UInt8		blockChance;
		/*25*/ UInt8		attackChance;
		/*26*/ UInt8		pad26[2];
		/*28*/ float		staggerBonusToAttack;
		/*2C*/ float		KOBonusToAttack;
		/*30*/ float		H2HBonusToAttack;
		/*34*/ UInt8		powerAttackChance;
		/*35*/ UInt8		pad35[3];
		/*38*/ float		staggerBonusToPower;
		/*3C*/ float		KOBonusToPower;
		/*40*/ UInt8		attackChoiceChanceNormal;
		/*41*/ UInt8		attackChoiceChanceForward;
		/*42*/ UInt8		attackChoiceChanceBack;
		/*43*/ UInt8		attackChoiceChanceLeft;
		/*44*/ UInt8		attackChoiceChanceRight;
		/*45*/ UInt8		pad45[3];
		/*48*/ float		holdTimerMin;
		/*4C*/ float		holdTimerMax;
		/*50*/ UInt8		styleFlagsA;
		/*51*/ UInt8		acroDodgeChance;
		/*52*/ UInt8		pad52[2];
		/*54*/ float		rangeMultOptimal;
		/*58*/ float		rangeMultMax;
		/*5C*/ float		switchDistMelee;
		/*60*/ float		switchDistRanged;
		/*64*/ float		buffStandoffDist;
		/*68*/ float		rangedStandoffDist;
		/*6C*/ float		groupStandoffDist;
		/*70*/ UInt8		rushAttackChance;
		/*71*/ UInt8		pad71[3];
		/*74*/ float		rushAttackDistMult;
		/*78*/ UInt32		styleFlagsB;			// only stores kCombatStyleFlags_DoNotAccquire
	};

	// members
	//     /*00*/ TESForm
	/*24*/ CombatStyleData			styleSettings;
	/*A0*/ AdvancedSettings*		advancedSettings;
};