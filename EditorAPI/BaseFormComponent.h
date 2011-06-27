#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "BSStringT.h"

//	EditorAPI: BaseFormComponent class and its derivatives.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    BaseFormComponents define the various 'components' that true TESForm classes can inherit,
    e.g. Name, Texture, Model, Weight, etc.  The root of the hierarchy is the BaseFormComponent
    class, which exposes a common interface for Comparison, Copying, Serialization, etc.

    NOTE: "Form" or "Component" References refer to members that point to another form, which are
    tracked by the CS.
*/

class   TESFile;
class   TESForm;
class   NiNode;
struct  FULL_HASH;  // Useage unknown - struct {void* unk00; void* unk04; void* unk08;}
class   Script;
class   SpellItem;
class   TESLevSpell;
class	TESContainer;
class   TESObjectREFR;
class   TESActorBase;
class   TESObjectARMO;
class   TESObjectCLOT;
class   TESObjectWEAP;
class	KFModel;	 // (OBSE,GameTasks.h) may not be an explicitly named object
class	TESFaction;
class	TESPackage;
class	EnchantmentItem;
class	IngredientItem;
class	TESRace;
class	TESLevItem;


// 04
class BaseFormComponent
{
public:
	// members
	///*00*/ void**				vtbl;

	virtual void				Dtor(void);
};

// 0C
class TESFullName : public BaseFormComponent
{
public: 
	// members
	//     /00*/ void**         vtbl;
	/*04*/ BSStringT			name;
};

// 10
class TESDescription : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**			vtbl;
	/*04*/ BSStringT				description; 
	/*0C*/ UInt32					descDialogItem;		// Dialog Control ID for description control
};

// 18
class TESTexture : public BaseFormComponent
{
public: 
	// members
	//     /*00*/ void**         vtbl;
	/*04*/ BSStringT			texturePath;
	/*0C*/ UInt32				unkTexture0C;			// cached image? struct {UInt32 unkA; UInt32 unkB; void* privateObj;}
	/*10*/ UInt32				texturePathDlgItem;		// Dialog Control ID for texture path control
	/*14*/ UInt32				textureImageDlgItem;	// Dialog Control ID for texture image control
};

// 18
class TESIcon : public TESTexture
{
public:
	// no additional members
};

// 18
class TESIconTree : public TESIcon
{
public:
	// no additional members
};

// 24
class TESModel : public BaseFormComponent
{
public:
	typedef NiTListBase<FULL_HASH*> ModelHashList;  // probably a named class, but without it's own vtbl or RTTI info

	// members
	//     /*00*/ void**            vtbl;
	/*04*/ BSStringT				modelPath;  
	/*0C*/ float					modelBound;  
	/*10*/ ModelHashList			modelHashList;		// texture hash list ?
	/*20*/ UInt32					modelPathDlgItem;	// Dialog Control ID for model path
};

// 24
class TESModelTree : public TESModel
{
public:
	// no additional members
};

// 0C
class TESScriptableForm : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**		vtbl;
	/*04*/ Script*				script;				// script formid stored here during loading
	/*08*/ bool					scriptLinked;		// set once formid has been resolved into a Script*
	/*09*/ UInt8				scriptPad09[3];
};

// 08
class TESUsesForm : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**		vtbl;
	/*04*/ UInt8				uses;
	/*05*/ UInt8				usesPad05[3];
};

// 08
class TESValueForm : public BaseFormComponent
{
public:    
	// members
	//     /*00*/ void**		vtbl;
	/*04*/ SInt32				goldValue;
};

// 08
class TESHealthForm : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**		vtbl;
	/*04*/ UInt32				health;
};

// 08
class TESWeightForm : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**		vtbl;
	/*04*/ float				weight;
};

// 08
class TESQualityForm : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**		vtbl;
	/*04*/ float				quality;
};

// 08
class TESAttackDamageForm : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**		vtbl;
	/*04*/ UInt16				damage;
	/*06*/ UInt16				damagePad06;
};

// 0C
class TESAttributes : public BaseFormComponent
{
public:
	enum Attributes
	{
		kAttribute_Strength = 0,
		kAttribute_Intelligence,
		kAttribute_Willpower,
		kAttribute_Agility,
		kAttribute_Speed,
		kAttribute_Endurance,
		kAttribute_Personality,
		kAttribute_Luck,
		kAttribute__MAX,
	};

	// members
	//     /*00*/ void**		vtbl;
	/*04*/ UInt8				attributes[8];
};

// 14
class TESSpellList : public BaseFormComponent
{
public:
	typedef tList<SpellItem> SpellListT;
	typedef tList<TESLevSpell> LevSpellListT;

	// members
	//     /*00*/ void**		vtbl;
	/*04*/ SpellListT			spells;
	/*0C*/ LevSpellListT		leveledSpells;
};

#pragma once

// 10
class TESLeveledList : public BaseFormComponent
{
public: 
	enum LeveledListFlags
	{
		kLevListFlag_CalcAllLevels      = 0x01, // ignores max level difference, effective level is no greate than highest level in list
		kLevListFlag_CalcEachInCount    = 0x02, // for nested lists
	};

	// 0C
	struct ListEntry
	{
		/*00*/ UInt16			level;
		/*02*/ UInt16			pad02;
		/*04*/ TESForm*			form;
		/*08*/ SInt16			count;
		/*0A*/ UInt16			pad0A;
	};
	typedef tList<ListEntry> LevListT;

	// members
	//     /*00*/ void**		vtbl;
	/*04*/ LevListT				levList;    // list is sorted by level
	/*0C*/ UInt8				chanceNone;	
	/*0D*/ UInt8				levListFlags;
	/*0E*/ UInt16				padLevList0E;
};

// 10
class TESContainer : public BaseFormComponent
{
public:
	enum ContainerFlags
	{
		kContainer_Linked    = 0x01,	// cleared during loading, set by LinkComponent()
	};

	// 08
	struct ContentEntry
	{
		/*00*/ SInt32			count;  // negative counts have some meaning ...
		/*04*/ TESForm*			form;
	};

	typedef tList<ContentEntry> ContentListT;

	/// members
	//     /*00*/ void**        vtbl;
	/*04*/ UInt8				containerFlags;
	/*05*/ UInt8				pad05[3];
	/*08*/ ContentListT			contents;
};

// 14
class TESAnimation : public BaseFormComponent
{
public:
	typedef tList<char> AnimationListT;
	typedef tList<KFModel> KFModelListT;

	// members
	//     /*00*/ void**            vtbl;
	/*04*/ KFModelListT				kfModelList; // temporary list of animation models used during dialog editing
	/*0C*/ AnimationListT			animations; // animation names, dynamically allocated 
};

// 20
class TESActorBaseData : public BaseFormComponent
{
public:
	enum NPCFlags
	{
		kNPCFlag_Female                 = 0x00000001,
		kNPCFlag_Essential              = 0x00000002,
		kNPCFlag_Respawn                = 0x00000008,
		kNPCFlag_AutoCalcStats          = 0x00000010,
		kNPCFlag_PCLevelOffset          = 0x00000080,
		kNPCFlag_NoLowProc              = 0x00000200,
		kNPCFlag_NoRumors               = 0x00002000,
		kNPCFlag_Summonable             = 0x00004000,
		kNPCFlag_NoPersuasion           = 0x00008000,
		kNPCFlag_CanCorpseCheck         = 0x00100000,
	};

	enum CreatureFlags
	{
		kCreatureFlag_Biped             = 0x00000001,
		kCreatureFlag_Essential         = 0x00000002,
		kCreatureFlag_WeaponAndShield   = 0x00000004,
		kCreatureFlag_Respawn           = 0x00000008,
		kCreatureFlag_Swims             = 0x00000010,
		kCreatureFlag_Flies             = 0x00000020,
		kCreatureFlag_Walks             = 0x00000040,
		kCreatureFlag_PCLevelOffset     = 0x00000080,
		kCreatureFlag_HasSounds         = 0x00000100,   // has a CreatureSoundArray (instead of a TESCreature* for inherited sounds)
		kCreatureFlag_NoLowProc         = 0x00000200,
		kCreatureFlag_NoBloodParticle   = 0x00000800,
		kCreatureFlag_NoBloodTexture    = 0x00001000,
		kCreatureFlag_NoRumors          = 0x00002000,
		kCreatureFlag_Summonable        = 0x00004000,
		kCreatureFlag_NoHead            = 0x00008000,
		kCreatureFlag_NoRightArm        = 0x00010000,
		kCreatureFlag_NoLeftArm         = 0x00020000,
		kCreatureFlag_NoCombatInWater   = 0x00040000,
		kCreatureFlag_NoShadow          = 0x00080000,
		kCreatureFlag_NoCorpseCheck     = 0x00100000,   // inverse of corresponding flag for NPCs
	};

	// 08
	struct FactionInfo
	{
		TESFaction*     faction;    // 00
		SInt8           rank;       // 04
		UInt8           pad[3];     // 05
	};

	typedef tList<FactionInfo> FactionListT;

	// members
	//     /*00*/ void**        vtbl;
	/*04*/ UInt32				actorFlags; 
	/*08*/ UInt16				magicka;		// init to 50
	/*0A*/ UInt16				fatigue;		// init to 50
	/*0C*/ UInt16				barterGold;
	/*0E*/ UInt16				level;			// init to 1
	/*10*/ UInt16				minLevel;		// if PCLevelOffset
	/*12*/ UInt16				maxLevel;		// if PCLevelOffset
	/*14*/ TESLevItem*			deathItem;
	/*18*/ FactionListT			factionList; 
};

// 18
class TESAIForm : public BaseFormComponent
{
public:
	enum AIStats
	{
		kAIStat_Aggression      = 0,
		kAIStat_Confidence      = 1,
		kAIStat_Energy          = 2,
		kAIStat_Responsibility  = 3,    
		kAIStat__MAX,
	};

	enum ServiceFlags
	{
		kService_Weapons        = 1 << 0x00,
		kService_Armor          = 1 << 0x01,
		kService_Clothing       = 1 << 0x02,
		kService_Books          = 1 << 0x03,
		kService_Ingredients    = 1 << 0x04,
		kService_Lights         = 1 << 0x07,
		kService_Apparatus      = 1 << 0x08,
		kService_Misc           = 1 << 0x0A,
		kService_Spells         = 1 << 0x0B,
		kService_MagicItems     = 1 << 0x0C,
		kService_Potions        = 1 << 0x0D,
		kService_Training       = 1 << 0x0E,
		kService_Recharge       = 1 << 0x10,
		kService_Repair         = 1 << 0x11,
	};

	typedef tList<TESPackage> PackageListT;

	// members
	//     /*00*/ void**        vtbl;
	/*04*/ UInt8				aiStats[4];
	/*08*/ UInt32				serviceFlags;
	/*0C*/ UInt8				trainingSkill;  // skill offset, i.e. (avCode - 12)
	/*0D*/ UInt8				trainingLevel;
	/*0E*/ UInt8				pad0E[2];
	/*10*/ PackageListT			packages;
};

// 10
class TESReactionForm : public BaseFormComponent
{
public:
	// 08
	struct ReactionInfo
	{
		TESForm*			target;
		SInt32				reaction;
	};

	typedef tList<ReactionInfo> ReactionListT;

	// members
	//     /*00*/ void**			vtbl;
	/*04*/ ReactionListT			reactionList; 
	/*0C*/ UInt8					unk0C;			// intialized to 6
	/*0D*/ UInt8					unk0D[2];
};

// 0C
class TESSoundFile : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**			vtbl;
	/*04*/ BSStringT				soundFilePath; 
};

// C8
class TESBipedModelForm : public BaseFormComponent
{
public:
	enum SlotMask
	{
		kSlotMask_Head				= /*00*/ 0x0001,
		kSlotMask_Hair				= /*01*/ 0x0002,
		kSlotMask_UpperBody			= /*02*/ 0x0004,
		kSlotMask_LowerBody			= /*03*/ 0x0008,
		kSlotMask_Hand				= /*04*/ 0x0010,
		kSlotMask_Foot				= /*05*/ 0x0020,
		kSlotMask_RightRing			= /*06*/ 0x0040,
		kSlotMask_LeftRing			= /*07*/ 0x0080,
		kSlotMask_Amulet			= /*08*/ 0x0100,
		kSlotMask_Weapon			= /*09*/ 0x0200,
		kSlotMask_BackWeapon		= /*0A*/ 0x0400,
		kSlotMask_SideWeapon		= /*0B*/ 0x0800,
		kSlotMask_Quiver			= /*0C*/ 0x1000,
		kSlotMask_Shield			= /*0D*/ 0x2000,
		kSlotMask_Torch				= /*0E*/ 0x4000,
		kSlotMask_Tail				= /*0F*/ 0x8000,
		kSlotMask__MAX
	};

	enum BipedModelFlags
	{
		kBipedModelFlags_HidesRings			= /*00*/ 0x0001,
		kBipedModelFlags_HidesAmulets		= /*01*/ 0x0002,
		kBipedModelFlags_Unk02				= /*02*/ 0x0004,
		kBipedModelFlags_Unk03				= /*03*/ 0x0008,
		kBipedModelFlags_Unk04				= /*04*/ 0x0010,
		kBipedModelFlags_Unk05				= /*05*/ 0x0020,
		kBipedModelFlags_NotPlayable		= /*06*/ 0x0040,
		kBipedModelFlags_HeavyArmor			= /*07*/ 0x0080,
		kBipedModelFlags_Unk08				= /*08*/ 0x0100
	};

	// members
	//     /*00*/ void**			vtbl;
	/*04*/ UInt16					slotMask;
	/*06*/ UInt8					bipedModelFlags;
	/*07*/ UInt8					pad07;
	/*08*/ TESModel					maleBipedModel;
	/*2C*/ TESModel					femaleBipedModel;
	/*50*/ TESModel					maleGroundModel;
	/*74*/ TESModel					femaleGroundModel;
	/*98*/ TESIcon					maleIcon;
	/*B0*/ TESIcon					femaleIcon;
};

// 10
class TESEnchantableForm : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**			vtbl;
	/*04*/ EnchantmentItem*			enchantment;
	/*08*/ UInt16					enchantmentAmount;		// only valid for weapons
	/*0A*/ UInt16					pad0A;
	/*0C*/ UInt32					enchantmentType;		// init by derived class's InitializeAllComponents() fn, to values from EnchantmentItem::EnchantmentType
};

// 0C
class TESProduceForm : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**			vtbl;
	/*04*/ IngredientItem*			ingredient;
	/*08*/ UInt8					springHarvestChance;
	/*09*/ UInt8					summerHarvestChance;
	/*0A*/ UInt8					fallHarvestChance;
	/*0B*/ UInt8					winterHarvestChance;
};

// 08
class TESRaceForm : public BaseFormComponent
{
public:
	// members
	//     /*00*/ void**		vtbl;
	/*04*/ TESRace*				race;
};

// 14
class TESModelList : public BaseFormComponent
{
public:
	typedef tList<char> ModelListT;

	// members
	//     /*00*/ void**			vtbl;
	/*04*/ ModelListT				modelList;
	/*0C*/ UInt32					unk0C;				// init to 0
	/*10*/ UInt32					unk10;				// init to 0
};

// misc. utility components. not derived from BaseFormComponent

// 04
struct RGBA
{
	/*01*/ UInt8			r;
	/*02*/ UInt8			g;
	/*03*/ UInt8			b;
	/*04*/ UInt8			a;
};