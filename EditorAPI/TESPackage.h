#pragma once

#include "TESForm.h"
#include "TESConditionItem.h"

//	EditorAPI: TESPackage class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding
//	This one's all kyoma though - Muches gracias!
/*
	...
*/

// 44
class TESPackage : public TESForm
{
public:
	enum PackageFlags
	{
		kPackFlag_OffersServices 		/*0x00000001*/ = 1 <<  0,
		kPackFlag_MustReachLocation 	/*0x00000002*/ = 1 <<  1,
		kPackFlag_MustComplete 			/*0x00000004*/ = 1 <<  2,
		kPackFlag_LockDoorsAtStart 		/*0x00000008*/ = 1 <<  3,
		kPackFlag_LockDoorsAtEnd 		/*0x00000010*/ = 1 <<  4,
		kPackFlag_LockDoorsAtLocation 	/*0x00000020*/ = 1 <<  5,
		kPackFlag_UnlockDoorsAtStart 	/*0x00000040*/ = 1 <<  6,
		kPackFlag_UnlockDoorsAtEnd 		/*0x00000080*/ = 1 <<  7,
		kPackFlag_UnlockDoorsAtLocation /*0x00000100*/ = 1 <<  8,
		kPackFlag_ContinueIfPCNear 		/*0x00000200*/ = 1 <<  9,
		kPackFlag_OncePerDay 			/*0x00000400*/ = 1 << 10,
		kPackFlag_Unk11 				/*0x00000800*/ = 1 << 11,
		kPackFlag_SkipFalloutBehavior 	/*0x00001000*/ = 1 << 12,
		kPackFlag_AlwaysRun 			/*0x00002000*/ = 1 << 13,
		kPackFlag_Scripted 				/*0x00004000*/ = 1 << 14,	//set by AddScriptPackage
		kPackFlag_Unk15 				/*0x00008000*/ = 1 << 15,
		kPackFlag_Waiting 				/*0x00010000*/ = 1 << 16,	//set by Wait/StopWaiting/etc.
		kPackFlag_AlwaysSneak 			/*0x00020000*/ = 1 << 17,
		kPackFlag_AllowSwimming 		/*0x00040000*/ = 1 << 18,
		kPackFlag_AllowFalls 			/*0x00080000*/ = 1 << 19,
		kPackFlag_ArmorUnequipped 		/*0x00100000*/ = 1 << 20,
		kPackFlag_WeaponsUnequipped 	/*0x00200000*/ = 1 << 21,
		kPackFlag_DefensiveCombat 		/*0x00400000*/ = 1 << 22,
		kPackFlag_UseHorse 				/*0x00800000*/ = 1 << 23,
		kPackFlag_NoIdleAnims 			/*0x01000000*/ = 1 << 24,
		kPackFlag_Unk25 				/*0x02000000*/ = 1 << 25,
		kPackFlag_Unk26 				/*0x04000000*/ = 1 << 26,
		kPackFlag_Unk27 				/*0x08000000*/ = 1 << 27,
		kPackFlag_Unk28 				/*0x10000000*/ = 1 << 28,
		kPackFlag_Unk29 				/*0x20000000*/ = 1 << 29,
		kPackFlag_Unk30 				/*0x40000000*/ = 1 << 30,
		kPackFlag_Unk31 				/*0x80000000*/ = 1 << 31,
	};

	enum PackageTypes
	{
		kPackageType_Find				= 0,
		kPackageType_Follow				= 1,
		kPackageType_Escort				= 2,
		kPackageType_Eat				= 3,
		kPackageType_Sleep				= 4,
		kPackageType_Wander				= 5,
		kPackageType_Travel				= 6,
		kPackageType_Accompany			= 7,
		kPackageType_UseItemAt			= 8,
		kPackageType_Ambush				= 9,
		kPackageType_FleeNotCombat		= 10,
		kPackageType_CastMagic			= 11,
		kPackageType_Combat				= 12,
		kPackageType_CombatLow			= 13,
		kPackageType_Activate			= 14,
		kPackageType_Alarm				= 15,
		kPackageType_Flee				= 16,
		kPackageType_Trespass			= 17,
		kPackageType_Dialogue			= 18,
		kPackageType_Spectator			= 19,
		kPackageType_ReactToDead		= 20,
		kPackageType_GetUpFurniture		= 21,
		kPackageType_Mount				= 22,
		kPackageType_Unmount			= 23,
		kPackageType_DoNothing			= 24,
		kPackageType_CastTargetSpell	= 25,
		kPackageType_CastTouchSpell		= 26,
		kPackageType_VampireFeed		= 27,
		kPackageType_Surface			= 28,
		kPackageType_SearchForAttacker	= 29,
		kPackageType_ClearMountPosition = 30,
		kPackageType_SummonDefend		= 31,	//seen for actors with CommandEffect, ReanimateEffect & SummonEffect
		kPackageType_MovementBlocked	= 32,
	};

	enum Procedures // UInt32
	{
		kProcedure_TRAVEL				  =  0,
		kProcedure_WANDER				  =  1,
		kProcedure_ACTIVATE				  =  2,
		kProcedure_AQUIRE	  /*sic*/	  =  3,
		kProcedure_SLEEP				  =  4,
		kProcedure_EAT					  =  5,
		kProcedure_FOLLOW				  =  6,
		kProcedure_ESCORT				  =  7,
		kProcedure_ALARM				  =  8,
		kProcedure_COMBAT				  =  9,
		kProcedure_FLEE					  = 10,
		kProcedure_YIELD				  = 11,
		kProcedure_DIALOGUE				  = 12,
		kProcedure_WAIT					  = 13,
		kProcedure_TRAVEL_TARGET		  = 14,
		kProcedure_PURSUE				  = 15,
		kProcedure_GREET				  = 16,
		kProcedure_CREATE_FOLLOW		  = 17,
		kProcedure_OBSERVE_COMBAT		  = 18,
		kProcedure_OBSERVE_DIALOGUE		  = 19,
		kProcedure_GREET_DEAD			  = 20,
		kProcedure_WARN					  = 21,
		kProcedure_GET_UP				  = 22,
		kProcedure_MOUNT_HORSE			  = 23,
		kProcedure_DISMOUNT_HORSE		  = 24,
		kProcedure_DO_NOTHING			  = 25,
		kProcedure_CAST_SPELL			  = 26,
		kProcedure_AIM					  = 27,
		kProcedure_NOTIFY				  = 28,
		kProcedure_ACCOMPANY			  = 29,
		kProcedure_USE_ITEM_AT			  = 30,
		kProcedure_FEED					  = 31,
		kProcedure_AMBUSH_WAIT			  = 32,
		kProcedure_SURFACE				  = 33,
		kProcedure_WAIT_FOR_SPELL		  = 34,
		kProcedure_CHOOSE_CAST			  = 35,
		kProcedure_FLEE_NON_COMBAT		  = 36,
		kProcedure_REMOVE_WORN_ITEMS	  = 37,
		kProcedure_SEARCH				  = 38,
		kProcedure_CLEAR_MOUNT_POSITION	  = 39,
		kProcedure_SUMMON_CREATURE_DEFEND = 40,
		kProcedure_MOVEMENT_BLOCKED		  = 41,
		kProcedure_UNEQUIP_ARMOR		  = 42,
		kProcedure_DONE					  = 43,
		kProcedure__MAX
	};

	enum ObjectTypes
	{// order only somewhat related to kFormType_XXX (values off by 17, 20, or 21)
		kObjectType_Activator		= 0x01,
		kObjectType_Apparatus,
		kObjectType_Armor,
		kObjectType_Book,
		kObjectType_Clothing,
		kObjectType_Container,
		kObjectType_Door,
		kObjectType_Ingredient,
		kObjectType_Light,
		kObjectType_Misc,
		kObjectType_Flora,
		kObjectType_Furniture,
		kObjectType_WeaponAny,
		kObjectType_Ammo,
		kObjectType_NPC,
		kObjectType_Creature,		// 10
		kObjectType_Soulgem,
		kObjectType_Key,
		kObjectType_Alchemy,
		kObjectType_Food,
		kObjectType_CombatWearable,
		kObjectType_Wearable,
		kObjectType_WeaponNone,
		kObjectType_WeaponMelee,
		kObjectType_WeaponRanged,
		kObjectType_SpellsAny,
		kObjectType_SpellsTarget,
		kObjectType_SpellsTouch,
		kObjectType_SpellsSelf,
		kObjectType_SpellsAlteration,
		kObjectType_SpellsConjuration,
		kObjectType_SpellsDestruction,	// 20
		kObjectType_SpellsIllusion,
		kObjectType_SpellsMysticism,
		kObjectType_SpellsRestoration,
		//...
		kObjectType_Max				= 0x24,
	};
	union Object
	{
		TESForm*				form;
		TESObjectREFR*			refr;
		UInt32					code;
	};

	enum LocationTypes
	{
		kLocationType_NearReference		= 0,
		kLocationType_InCell			= 1,
		kLocationType_CurrentLocation	= 2,
		kLocationType_EditorLocation	= 3,
		kLocationType_ObjectID			= 4,
		kLocationType_ObjectType		= 5,
		kLocationType__MAX,
	};

	// 0C
	struct LocationData
	{
		// members
		/*00*/ UInt8			locationType;
		/*01*/ UInt8			pad01[3];
		/*04*/ UInt32			radius;
		/*08*/ Object			object;
	};

	enum TargetTypes
	{
		kTargetType_Reference	= 0,
		kTargetType_BaseObject	= 1,
		kTargetType_ObjectCode	= 2,
	};

	// 0C
	struct TargetData
	{
		// members
		/*00*/ UInt8			targetType;
		/*01*/ UInt8			pad01[3];
		/*04*/ Object			target;
		/*08*/ UInt32			count;
	};

	// 08
	struct ScheduleData
	{
		enum
		{
			kDay_Any = 0,
			kTime_Any = 0xFF,
		};

		enum
		{
			kMonth_January = 0,
			kMonth_February,
			kMonth_March,
			kMonth_April,
			kMonth_May,
			kMonth_June,
			kMonth_July,
			kMonth_August,
			kMonth_September,
			kMonth_October,
			kMonth_November,
			kMonth_December,
			kMonth_Spring,	// march, april, may
			kMonth_Summer,	// june, july, august
			kMonth_Fall,	// september, august, november
			kMonth_Winter,	// december, january, february

			kMonth_Any = 0xFF,
		};

		enum
		{
			kWeekday_Sundas = 0,
			kWeekday_Morndas,
			kWeekday_Tirdas,
			kWeekday_Middas,
			kWeekday_Turdas,
			kWeekday_Fredas,
			kWeekday_Loredas,
			kWeekday_Weekdays,
			kWeekday_Weekends,
			kWeekday_MWF,
			kWeekday_TT,

			kWeekday_Any = 0xFF
		};

		// members
		/*00*/ UInt8			month;
		/*01*/ UInt8			weekDay;
		/*02*/ UInt8			date;
		/*03*/ UInt8			time;
		/*04*/ UInt32			duration;
	};

	// members
	//     /*00*/ TESForm
	/*24*/ UInt32				packageFlags;
	/*28*/ UInt8				type;
	/*29*/ UInt8				pad021[3];
	/*2C*/ LocationData*		location;
	/*30*/ TargetData*			target;
	/*34*/ ScheduleData			schedule;
	/*3C*/ ConditionListT		conditions;

	// methods
	static void					InitializeListViewColumns(HWND ListView);

	static HWND*				WindowHandle;
};