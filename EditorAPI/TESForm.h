#pragma once

#include "BaseFormComponent.h"
#include "BSTCaseInsensitiveStringMap.h"

//	EditorAPI: TESForm class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	TESForm is the base class for all object 'types' (or 'Base forms') in the game.
	It provides a common, very sophisticated interface for Serialization, Revision Control,
	Inter-form references, and editing (in the CS).

	NOTE on the CS reference-tracking:
	A Form may point to other forms through it's fields; these are it's "Form", or "Component" references.
	These list of other forms that point to this one are it's "Cross" references.
	The exception is if this form is a base for a placed TESObjectREFR - refs that point to this form are it's "Object" references.
	The notation is admittedly awkward, a result of choosing names before a clear picture of the system emerged.
	Form and Cross references are tracked individually through a global table.  Object refs are not tracked individually, but
	every instance of TESBoundObject does maintain a cell-by-cell count.
*/

class   TESFile;
class   RecordInfo;
class   TESObjectREFR;
class	TESForm;
class	FormCrossReferenceData;

typedef tList<FormCrossReferenceData>  FormCrossReferenceListT;

// replaces the vanilla cross-reference data to add reference counting
class FormCrossReferenceData
{
	TESForm*		Form;
	UInt32			Count;
public:
	void			Initialize(TESForm* Form);
	UInt32			GetReferenceCount() const;
	UInt32			IncrementRefCount();
	UInt32			DecrementRefCount();
	TESForm*		GetForm() const;

	static FormCrossReferenceData*		CreateInstance(TESForm* Reference, bool IncrementRefCount = true);
	void								DeleteInstance();
	static FormCrossReferenceData*		LookupFormInCrossReferenceList(FormCrossReferenceListT* CrossReferenceList, TESForm* CrossReferencedForm);
};

// Used by the built-in revision-control in the CS, which seems to be disabled in the public version
// This struct is public (not a subclass of TESForm) because it is also used by TESFile
// 04
struct TrackingData
{
	UInt16  date;               // 00 low byte is day of month, high byte is number of months with 1 = Jan. 2003 (Decembers are a little weird)
	UInt8   lastUser;           // 02 userID that last had this form checked out
	UInt8   currentUser;        // 03 userID that has this form checked out
};

// 24
class TESForm : public BaseFormComponent
{
public:
	typedef tList<TESForm>  VanillaFormCrossReferenceList;  // for reference tracking in the CS

	enum FormType
	{
		kFormType_None          = 0x00,
		kFormType_TES4,
		kFormType_Group,
		kFormType_GMST,
		kFormType_Global,
		kFormType_Class,
		kFormType_Faction,
		kFormType_Hair,
		kFormType_Eyes          = 0x08,
		kFormType_Race,
		kFormType_Sound,
		kFormType_Skill,
		kFormType_EffectSetting,
		kFormType_Script,
		kFormType_LandTexture,
		kFormType_Enchantment,
		kFormType_Spell         = 0x10,
		kFormType_BirthSign,
		kFormType_Activator     = 0x12,
		kFormType_Apparatus     = 0x13,
		kFormType_Armor         = 0x14,
		kFormType_Book          = 0x15,
		kFormType_Clothing      = 0x16,
		kFormType_Container     = 0x17,
		kFormType_Door          = 0x18,
		kFormType_Ingredient    = 0x19,
		kFormType_Light         = 0x1A,
		kFormType_Misc          = 0x1B,
		kFormType_Static        = 0x1C,
		kFormType_Grass         = 0x1D,
		kFormType_Tree          = 0x1E,
		kFormType_Flora         = 0x1F,
		kFormType_Furniture     = 0x20,
		kFormType_Weapon        = 0x21,
		kFormType_Ammo          = 0x22,
		kFormType_NPC           = 0x23,
		kFormType_Creature      = 0x24,
		kFormType_LeveledCreature,
		kFormType_SoulGem       = 0x26,
		kFormType_Key           = 0x27,
		kFormType_AlchemyItem   = 0x28,
		kFormType_SubSpace,
		kFormType_SigilStone,
		kFormType_LeveledItem,
		kFormType_SNDG,
		kFormType_Weather,
		kFormType_Climate,
		kFormType_Region,
		kFormType_Cell          = 0x30,
		kFormType_REFR,
		kFormType_ACHR,
		kFormType_ACRE,
		kFormType_PathGrid,
		kFormType_WorldSpace,
		kFormType_Land,
		kFormType_TLOD,
		kFormType_Road          = 0x38,
		kFormType_Topic,
		kFormType_TopicInfo,
		kFormType_Quest,
		kFormType_Idle,
		kFormType_Package,
		kFormType_CombatStyle,
		kFormType_LoadScreen,
		kFormType_LeveledSpell  = 0x40,
		kFormType_AnimObject,
		kFormType_WaterForm,
		kFormType_EffectShader,
		kFormType_TOFT          = 0x44,
		kFormType__MAX          = 0x45
	};

	static const char* FormTypeIDLongNames[kFormType__MAX];

	enum FormFlags
	{
		kFormFlags_FromMaster           = /*00*/ 0x00000001,   // form is from an esm file
		kFormFlags_FromActiveFile       = /*01*/ 0x00000002,   // form is overriden by active mod or save file
		kFormFlags_Linked               = /*03*/ 0x00000008,   // set after formids have been resolved into TESForm*
		kFormFlags_Deleted              = /*05*/ 0x00000020,   // set on deletion, not saved in CS or savegame
		kFormFlags_BorderRegion         = /*06*/ 0x00000040,   // ?? (from TES4Edit)
		kFormFlags_TurnOffFire          = /*07*/ 0x00000080,   // ?? (from TES4Edit)
		kFormFlags_CastShadows          = /*09*/ 0x00000200,   // ?? (from TES4Edit)
		kFormFlags_QuestItem            = /*0A*/ 0x00000400,   // aka Persistent Reference for TESObjectREFR
		kFormFlags_Disabled             = /*0B*/ 0x00000800,   // (TESObjectREFR)
		kFormFlags_Ignored              = /*0C*/ 0x00001000,   // (records)
		kFormFlags_Temporary            = /*0E*/ 0x00004000,   // not saved in CS, probably game as well
		kFormFlags_VisibleWhenDistant   = /*0F*/ 0x00008000,   // ?? (from TES4Edit)
		kFormFlags_OffLimits            = /*11*/ 0x00020000,   // (TESObjectCELL)
		kFormFlags_Compressed           = /*12*/ 0x00040000,   // (records)
		kFormFlags_CantWait             = /*13*/ 0x00080000,   // (TESObjectCELL/TESWorldSpace)
		kFormFlags_IgnoresFriendlyHits  = /*14*/ 0x00100000,
	};

	// 0C
	struct FormTypeInfo
	{
		UInt32          formType;   // 00 form type code, also offset in array
		const char*     shortName;  // 04 offset to 4-letter type descriptor: 'GRUP', 'ARMO', etc.
		UInt32          chunkType;  // 08 appears to be name string in byte-reversed order, used to mark form records
	};

	typedef tList<TESFile>			OverrideFileListT;

	// members
	//     /*00*/ void**            vtbl;
	/*04*/ UInt8					formType;
	/*05*/ UInt8					formPad05[3];
	/*08*/ UInt32					formFlags;
	/*0C*/ UInt32					formID;
	/*10*/ BSString					editorID;
	/*18*/ TrackingData				trackingData;
	/*1C*/ OverrideFileListT		fileList; // list of TESFiles that override this form

	// methods
	const char*						GetEditorID() const;
	bool							IsReference() const;
	bool							IsActive() const;
	bool							IsQuestItem() const;

	bool							SetEditorID(const char* EditorID);
	TESFile*						GetOverrideFile(int Index);
	void							MarkAsTemporary();
	void							SetFormID(UInt32 FormID, bool Unk02 = true);	// arg2=true to reserve formid from datahandler ?

	void							AddCrossReference(TESForm* Form);
	void							RemoveCrossReference(TESForm* Form);
	FormCrossReferenceListT*		GetCrossReferenceList(bool CreateNew = true);
	void							CleanupCrossReferenceList();

	bool							UpdateUsageInfo();
	void							SetFromActiveFile(bool State);
	void							SetDeleted(bool State);
	void							SetQuestItem(bool State);
	void							CopyFrom(TESForm* Form);
	bool							CompareTo(TESForm* Form);
	bool							LoadForm(TESFile* File);
	bool							SaveForm(TESFile* File);				// saves an empty record for deleted forms
	bool							SaveFormRecord(TESFile* File);
	void							LinkForm();
	const char*						GetTypeIDString(void);
	void							GetDataFromDialog(HWND Dialog);

	static TESForm*					CreateInstance(UInt8 TypeID);
	void							DeleteInstance(bool ReleaseMemory = true);

	static TESForm*					LookupByFormID(UInt32 FormID);
	static TESForm*					LookupByEditorID(const char* EditorID);
	static const char*				GetFormTypeIDLongName(UInt8 TypeID);

	static FormTypeInfo*																			FormTypeInfoTable;
	static ConstructionSetExtender_OverriddenClasses::BSTCaseInsensitiveStringMap<TESForm*>*		EditorIDMap;
	static ConstructionSetExtender_OverriddenClasses::NiTMapBase<UInt32, TESForm*>*					FormIDMap;
};
STATIC_ASSERT(sizeof(TESForm) == 0x24);

/*
	This class is apparently used to centralize code for certain types of dialog windows in the CS.
	It does not appear in the RTTI structure of the game code.
*/
// 24
class TESFormIDListView : public TESForm
{
public:
	// no additional members
};
STATIC_ASSERT(sizeof(TESFormIDListView) == 0x24);

/*
	TESMemContextForm is a parent class for TESObjectCELL and TESObjectREFR.  It has no members and is not polymorphic,
	so it probably has only static methods & data.  It may have something to do with the 'Memory Usage' debugging code
	used by Bethesda, and it is possible that it has no use at all in the released game.
*/
// 00
class TESMemContextForm
{
	// no members
};