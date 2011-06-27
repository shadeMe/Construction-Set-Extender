#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

#include "TESForm.h"

//	EditorAPI: ExtraData class and its derivatives.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    ExtraData is Bethesda's tool for attaching arbitrary information to other classes.
    BSExtraData is the absract node type for the linked list of extra data.
*/

class	TESPackage;
class	AlchemyItem;
class	TESKey;
class	TESGlobal;
class	TESClimate;
class	TESRegionList;
class	TESObjectREFR;
class	TESWaterForm;
class	NiNode;
class	NiLines;
class	NiAVObject;
class	ExtraDataList;

// 0C
class BSExtraData
{
public:
	enum DialogExtraTypes
	{
		kDialogExtra_Param                  = 0x00,
		kDialogExtra_LocalCopy              = 0x01,
		kDialogExtra_ColorControl           = 0x02,
		kDialogExtra_SubWindow              = 0x04,
		kDialogExtra_CurrCondItem           = 0x05,
		kDialogExtra_WorkingData            = 0x06,
		kDialogExtra_FactionData            = 0x07,
		kDialogExtra_PopupMenu              = 0x08,
		kDialogExtra_RefSelectControl       = 0x09,
		kDialogExtra_ReactionData           = 0x0A,
		kDialogExtra_PreviewControl         = 0x0B,
		kDialogExtra_FreeformFaceControl    = 0x0C,
		kDialogExtra_Data                   = 0x0D,
		kDialogExtra_NotifyInfo             = 0x0E,
		kDialogExtra_QuestStageData         = 0x0F,
		kDialogExtra_Timer                  = 0x10,
		kDialogExtra_QuestFilter            = 0x11,
		kDialogExtra__MAX                   = 0x12,
	};

	enum ExtraDataTypes
	{        
		//Unknown                           = 0x00,
		//Unknown                           = 0x01,
		kExtra_Havok                        = 0x02,
		kExtra_Cell3D                       = 0x03,
		kExtra_WaterHeight                  = 0x04,
		kExtra_CellWaterType                = 0x05,
		//Unknown                           = 0x06,
		//Unknown                           = 0x07,
		kExtra_RegionList                   = 0x08,
		kExtra_SeenData                     = 0x09, // Game only
		kExtra_EditorID                     = 0x0A, // Game only
		kExtra_CellMusicType                = 0x0B,
		kExtra_CellClimate                  = 0x0C,
		kExtra_ProcessMiddleLow             = 0x0D, // Game only
		//Unknown                           = 0x0E,
		kExtra_CellCanopyShadowMask         = 0x0F,
		kExtra_DetachTime                   = 0x10, // Game only
		kExtra_PersistentCell               = 0x11,
		kExtra_Script                       = 0x12,
		kExtra_Action                       = 0x13,
		kExtra_StartingPosition             = 0x14,
		kExtra_Anim                         = 0x15,
		kExtra_Biped                        = 0x16, // CS only
		kExtra_UsedMarkers                  = 0x17,
		kExtra_DistantData                  = 0x18,
		kExtra_RagDollData                  = 0x19,
		kExtra_ContainerChanges             = 0x1A,
		kExtra_Worn                         = 0x1B,
		kExtra_WornLeft                     = 0x1C,
		//Unknown                           = 0x1D,
		kExtra_PackageStartLocation         = 0x1E,
		kExtra_Package                      = 0x1F,
		kExtra_TresPassPackage              = 0x20, // Game only
		kExtra_RunOncePacks                 = 0x21,
		kExtra_ReferencePointer             = 0x22,
		kExtra_Follower                     = 0x23, // Game only
		kExtra_LevCreaModifier              = 0x24,
		kExtra_Ghost                        = 0x25, // Game only
		kExtra_OriginalReference            = 0x26,
		kExtra_Ownership                    = 0x27,
		kExtra_Global                       = 0x28,
		kExtra_Rank                         = 0x29,
		kExtra_Count                        = 0x2A,
		kExtra_Health                       = 0x2B,
		kExtra_Uses                         = 0x2C,
		kExtra_TimeLeft                     = 0x2D,
		kExtra_Charge                       = 0x2E,
		kExtra_Soul                         = 0x2F,
		kExtra_Light                        = 0x30,
		kExtra_Lock                         = 0x31,
		kExtra_Teleport                     = 0x32,
		kExtra_MapMarker                    = 0x33,
		//Unknown                           = 0x34,
		kExtra_LeveledCreature              = 0x35, // Game only
		kExtra_LeveledItem                  = 0x36,
		kExtra_Scale                        = 0x37,
		kExtra_Seed                         = 0x38,
		kExtra_NonActorMagicCaster          = 0x39, // Game only
		kExtra_NonActorMagicTarget          = 0x3A, // Game only
		kExtra_MasterFileCell               = 0x3B, // CS only
		//Unknown                           = 0x3C,
		kExtra_CrimeGold                    = 0x3D, // Game only
		kExtra_OblivionEntry                = 0x3E, // Game only
		kExtra_EnableStateParent            = 0x3F,
		kExtra_EnableStateChildren          = 0x40, // Game only
		kExtra_ItemDropper                  = 0x41, // Game only
		kExtra_DroppedItemList              = 0x42, // Game only
		kExtra_RandomTeleportMarker         = 0x43,
		kExtra_MerchantContainer            = 0x44,
		//Unknown                           = 0x45,
		kExtra_PersuasionPercent            = 0x46, // Game only
		kExtra_CannotWear                   = 0x47,
		kExtra_Poison                       = 0x48,
		//Unknown                           = 0x49,
		kExtra_LastFinishedSequence         = 0x4A, // Game only
		kExtra_SavedMovementData            = 0x4B, // Game only
		kExtra_NorthRotation                = 0x4C, // Game only
		kExtra_XTarget                      = 0x4D,
		kExtra_FriendHitList                = 0x4E, // Game only
		kExtra_HeadingTarget                = 0x4F,
		kExtra_BoundArmor                   = 0x50,
		kExtra_RefractionProperty           = 0x51,
		kExtra_InvestmentGold               = 0x52, // Game only
		kExtra_StartingWorldOrCell          = 0x53,
		//Unknown                           = 0x54,
		kExtra_QuickKey                     = 0x55, // Game only
		kExtra_EditorRef3DData              = 0x56, // CS only
		kExtra_EditorRefMoveData            = 0x57,
		kExtra_TravelHorse                  = 0x58,
		kExtra_InfoGeneralTopic             = 0x59, // Game only
		kExtra_HasNoRumors                  = 0x5A, // Game only
		kExtra_Sound                        = 0x5B, // Game only
		kExtra_HaggleAmount                 = 0x5C, // Game only
		kExtra__MAX                         = 0x5D,
	};

	// members
	//     /*00*/ void**			vtbl          
	/*04*/ UInt8					extraType;   // set by constructor for each derived class
	/*05*/ UInt8					extraPad05[3];
	/*08*/ BSExtraData*				extraNext;

	// methods
	virtual void					UnkVFn00();
};

// cell and position where the current package was started?
// 20
class ExtraPackageStartLocation : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESForm*				cell;		// can be worldspace or cell
	/*10*/ Vector3				position;
	/*1C*/ UInt32				pad1C;
};

// current package
// 1C
class ExtraPackage : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESPackage*			package;
	/*10*/ UInt32				unk10;
	/*14*/ UInt32				unk14;
	/*18*/ UInt32				unk18;
};

// 10
class ExtraHealth : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ float				health;
};

// 10
class ExtraUses : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ UInt32				uses;
};

// 10
class ExtraCharge : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ float				charge;
};

// 10
class ExtraSoul: public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ UInt8				soul;
	/*0D*/ UInt8				pad0D[3];
};

// used by torches, etc (implies one light per object?)
// 10
class ExtraLight : public BSExtraData
{
public:
	// 08
	struct ExtraLightData
	{
		/*00*/ NiAVObject*		light;	// probably NiLight*
		/*04*/ float			unk4;	// intensity? only seen 1.0f
	};

	// members
	//     /*00*/ BSExtraData
	/*0C*/ ExtraLightData*		data;
};

// 10
class ExtraPoison : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ AlchemyItem*			poison;
};

// 10
class ExtraMerchantContainer : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESObjectREFR*		containerRef;
};

// 10
class ExtraWaterHeight : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ float				waterHeight;
};

// 10
class ExtraTravelHorse : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESObjectREFR*		horseRef;		// Horse
};

// 10
class ExtraLock : public BSExtraData
{
public:
	ExtraLock();
	~ExtraLock();

	enum
	{
		kLockFlags_IsLocked =	/*00*/ 0x1,
		kLockFlags_Unk01	=	/*01*/ 0x2,
		kLockFlags_Leveled	=	/*02*/ 0x4
	};

	// 0C
	struct ExtraLockData
	{
		/*00*/ UInt32			lockLevel;
		/*04*/ TESKey*			key;
		/*08*/ UInt8			flags;
		/*09*/ UInt8			pad09[3];
	};
	// members
	//     /*00*/ BSExtraData
	/*0C*/ ExtraLockData*		data;
};

// 10
class ExtraOwnership : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESForm*				owner;	// either TESFaction* or TESNPC*
};

//ownership data, stored separately from ExtraOwnership
// 10
class ExtraRank	: public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ UInt32				rank;
};

//ownership data, stored separately from ExtraOwnership
// 10
class ExtraGlobal : public BSExtraData
{								
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESGlobal*			globalVar;
};

// 10
class ExtraTeleport : public BSExtraData
{
public:
	ExtraTeleport();
	~ExtraTeleport();

	// 1C
	struct ExtraTeleportData
	{
		/*00*/ TESObjectREFR*	linkedDoor;
		/*04*/ Vector3			markerPosition;
		/*10*/ Vector3			markerRotation;	// angles in radians. x generally 0, y generally -0.0
	};

	// members
	//     /*00*/ BSExtraData
	/*0C*/ ExtraTeleportData*	data;
};

// 10
class ExtraRandomTeleportMarker : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESObjectREFR *		teleportRef;
};

// 10
class ExtraCellClimate : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESClimate*			climate;
};

// 10
class ExtraScale : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ float				scale;
};

// 10
class ExtraRegionList : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESRegionList*		regionList;
};

// 10
class ExtraPersistentCell : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESObjectCELL*		cell;
};

// 10
class ExtraCellMusicType : public BSExtraData
{
public:
	enum
	{
		kMusicType_Public	= 1,
		kMusicType_Dungeon	= 2,
		kMusicType__MAX
	};

	// members
	//     /*00*/ BSExtraData
	/*0C*/ UInt8				musicType;
	/*0D*/ UInt8				pad0D[3];
};

// 14
class ExtraEnableStateParent : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESObjectREFR*		parent;
	/*10*/ UInt8				oppositeState;		// is 1 if enable state set to opposite of parent's
	/*11*/ UInt8				pad11[3];
};

// 10
class ExtraCount : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ SInt16				count;
	/*0D*/ UInt8				pad0D[2];
};

// 10
class ExtraMapMarker : public BSExtraData
{
public:
	enum
	{
		kMapMarkerFlag_Visible			=	/*00*/ 0x1,
		kMapMarkerFlag_CanTravel		=	/*01*/ 0x2
	};

	enum 
	{
		kMapMarkerType_Invalid			= 0x0,
		kMapMarkerType_Camp,
		kMapMarkerType_Cave,
		kMapMarkerType_City,
		kMapMarkerType_ElvenRuin,
		kMapMarkerType_FortRuin,
		kMapMarkerType_Mine,
		kMapMarkerType_Landmark,
		kMapMarkerType_Tavern,
		kMapMarkerType_Settlement,
		kMapMarkerType_DaedricShrine,
		kMapMarkerType_OblivionGate,
		kMapMarkerType__MAX				= 0xD
	};

	// 10
	struct ExtraMapMarkerData
	{
		/*00*/ TESFullName				fullName;
		/*0C*/ UInt16					flags;
		/*0E*/ UInt16					type;	// possibly only 8 bits, haven't checked yet
	};

	// members
	//     /*00*/ BSExtraData
	/*0C*/ ExtraMapMarkerData*	data;
};

// 18
class ExtraDistantData : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ Vector3		unk0C;		// components init to 0.97
};

// 10
class ExtraRagDollData : public BSExtraData
{
public:
	// 08
	struct ExtraRagDollDataData
	{
		/*00*/ UInt32		unk00;
		/*04*/ UInt32		unk04;
	};

	// members
	//     /*00*/ BSExtraData
	/*0C*/ ExtraRagDollDataData*	data;
};

// 10
class ExtraCellWaterType : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ TESWaterForm*	waterType;
};

// 14
class ExtraEditorRef3DData : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ NiNode*			niNode;
	/*10*/ NiLines*			selectionBox;	// present when selected in editor
};

// 10
class ExtraTimeLeft : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData
	/*0C*/ float			time;
};

// 10
class ContainerExtraData
{
public:  
	class EntryExtraData;
	typedef tList<EntryExtraData> EntryDataListT;  

	// members        
	/*00*/ EntryDataListT*			entryDataList;  // initialized to empty list by constructor
	/*04*/ TESObjectREFR*			owner;          // for actors, this doubles as a pointer to parent ref
	/*08*/ float					encumberance;	// cached total weight of contents (includes perks), -1 if needs to be recalculated
	/*0C*/ float					armorWeight;	// cached total weight of equipped armor (includes perks), -1 if needs to be recalculated

	// 0C
	class EntryExtraData
	{
	public:
		typedef tList<ExtraDataList> FormDataTableT;

		// members        
		/*00*/ FormDataTableT*		formDataTable;    // initialized to empty list by constructor
		/*04*/ SInt32				count;  // cumulative with count in base container
		/*08*/ TESForm*				form;
	};
};

// 10
class ExtraContainerChanges : public BSExtraData
{
public:
	// members
	//     /*00*/ BSExtraData          
	/*0C*/ ContainerExtraData*		data;
}; 