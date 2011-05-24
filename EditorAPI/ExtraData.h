#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

//	EditorAPI: ExtraData class and its derivatives.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    ExtraData is Bethesda's tool for attaching arbitrary information to other classes.
    BSExtraData is the absract node type for the linked list of extra data.
*/

namespace EditorAPI
{
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
	};
}