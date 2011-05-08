#pragma once
#include "ExtenderInternals.h"

// 0C
class FormEditParam
{
public:
	UInt8		typeID;					// 00
	UInt8		pad01[3];
	TESForm*	form;					// 04

	FormEditParam(const char* EditorID);
	FormEditParam(UInt32 FormID);
};

// 20
class Subwindow
{
public:
	Subwindow();
	~Subwindow();

	GenericNode<HWND>     controls;		// 00
    HWND                  hDialog;		// 08	handle of parent dialog window
    HINSTANCE             hInstance;	// 0C	module instance of dialog template
	POINT                 position;		// 10	position of subwindow within parent dialog
	HWND                  hContainer;	// 18	handle of container control (e.g. Tab Control)
    HWND                  hSubwindow;	// 1C	handle of subwindow, if created
};

// 2C
struct ResponseEditorData
{
	struct VoiceRecorderData
	{
		HWND				recorderDlg;		// 00
	};

	const char*				editorTitle;		// 00
	UInt32					maxResponseLength;	// 04
	DialogResponse*			selectedResponse;	// 08
	DialogResponse*			responseLocalCopy;	// 0C
	VoiceRecorderData*		recorderData;		// 10
	TESTopic*				parentTopic;		// 14
	TESTopicInfo*			infoLocalCopy;		// 18
	TESTopicInfo*			selectedInfo;		// 1C
	TESQuest*				selectedQuest;		// 20
	GenericNode<TESRace*>	voicedRaces;		// 24
};

// 12C
struct ScriptEditorData
{
	RECT					editorBounds;				// 00
	Script*					currentScript;				// 10
	UInt8					newScript;					// 14	set to 1 when a new script is created, used to remove the new script when it's discarded 
	UInt8					pad14[3];					// 15
	HMENU					editorMenu;					// 18
	HWND					editorStatusBar;			// 1C
	HWND					editorToolBar;				// 20
	HWND					scriptableFormList;			// 24	handle to the TESScriptableForm combo box, passed as the lParam during init
	char*					findTextQuery;				// 28
	UInt32					unk2C[(0x12C - 0x2C) >> 2];	// 2C	never initialized, the size is probably a typo (intended size 0x2C) made by the tool coder
};

enum
{
	kTESObjectREFRSpecialFlags_3DInvisible				= 1 << 31,			// bits (only?) used in the runtime to mark modifications
	kTESObjectREFRSpecialFlags_Children3DInvisible		= 1 << 30,
	kTESObjectREFRSpecialFlags_Frozen					= 1 << 29,
};

enum
{
	kNiNodeSpecialFlags_DontUncull						= 1 << 15
};

enum
{
	kRenderWindowState_SnapToGrid					= 0x1,
	kRenderWindowState_SnapToAngle					= 0x2,
	kRenderWindowState_AllowRenderWindowCellLoads	= 0x4,
	kRenderWindowState_SkipInitialCellLoad			= 0x10,
	kRenderWindowState_UseCSDiscAsSource			= 0x20,		// deprecated
	kRenderWindowState_UseWorld						= 0x40,
};

class FormEnumerationWrapper
{
public:
	static void __stdcall ReinitializeFormLists();
	static bool GetUnmodifiedFormHiddenState();	// returns true when hidden
	static bool GetDeletedFormHiddenState();
	static bool __stdcall GetShouldEnumerateForm(TESForm* Form);
	static bool __stdcall PerformListViewPrologCheck(UInt32 CallAddress);
	static void ToggleUnmodifiedFormVisibility();
	static void	ToggleDeletedFormVisibility();

	static void __stdcall ResetFormVisibility(void);
};

class TESDialogWindowHandleCollection
{
	typedef std::vector<HWND>			_HandleCollection;
	_HandleCollection					WindowHandles;

	_HandleCollection::iterator			FindHandle(HWND Handle);
public:
	void								AddHandle(HWND Handle) { WindowHandles.push_back(Handle); }
	bool								RemoveHandle(HWND Handle);
	bool								GetHandleExists(HWND Handle) { return FindHandle(Handle) != WindowHandles.end(); }
	void								ClearHandles(void) { WindowHandles.clear(); }
};

extern TESDialogWindowHandleCollection	g_CustomMainWindowChildrenDialogs,		// used to keep them from being closed during a plugin load event
										g_DragDropSupportDialogs;				// keeps track of custom dialogs/controls that allow form (drag-)dropping

TESObjectREFR*				ShowReferencePickDialog(HWND Parent);
UInt32						GetFormDialogTemplate(const char* FormType);
UInt32						GetFormDialogTemplate(UInt8 FormTypeID);
void						LoadReferenceParentCell(const char* EditorID);
void						LoadReferenceParentCell(UInt32 FormID);
void						ShowFormEditDialog(const char* EditorID, const char* FormType);
void						ShowFormEditDialog(const char* EditorID, UInt8 FormType);
void						ShowFormEditDialog(UInt32 FormID, const char* FormType);
void						ShowFormEditDialog(UInt32 FormID, UInt8 FormType);
void						ResetRenderWindow();
void						InstantitateCustomScriptEditor(const char* ScriptEditorID);
void						InstantitateCustomScriptEditor(UInt32 ScriptFormID);

class CSStartupManager
{
public:
	static void				LoadStartupPlugin();
	static void				LoadStartupScript();
	static void				LoadStartupWorkspace();
};

enum
{
	// CS Main Dialogs
	kDialogTemplate_About						= 100,
	kDialogTemplate_Temp						= 102,
	kDialogTemplate_ObjectWindow				= 122,
	kDialogTemplate_CellEdit					= 125,
	kDialogTemplate_Data						= 162,
	kDialogTemplate_Preferences					= 169,
	kDialogTemplate_CellView					= 175,
	kDialogTemplate_RenderWindow				= 176,
	kDialogTemplate_PreviewWindow				= 181,			// accessible through View > Preview Window
	kDialogTemplate_ScriptEdit					= 188,
	kDialogTemplate_SearchReplace				= 198,
	kDialogTemplate_LandscapeEdit				= 203,
	kDialogTemplate_FindText					= 233,
	kDialogTemplate_RegionEditor				= 250,
	kDialogTemplate_HeightMapEditor				= 295,
	kDialogTemplate_IdleAnimations				= 302,
	kDialogTemplate_AIPackages					= 303,
	kDialogTemplate_AdjustExteriorCells			= 306,
	kDialogTemplate_OpenWindows					= 307,
	kDialogTemplate_DistantLODExport			= 317,
	kDialogTemplate_FilteredDialog				= 3235,
	kDialogTemplate_CreateLocalMaps				= 3249,

	// TESBoundObject/FormEdit Dialogs
	kDialogTemplate_Weapon						= 135,
	kDialogTemplate_Armor						= 136,
	kDialogTemplate_Clothing					= 137,
	kDialogTemplate_MiscItem					= 138,
	kDialogTemplate_Static						= 140,
	kDialogTemplate_Reference					= 141,
	kDialogTemplate_Apparatus					= 143,
	kDialogTemplate_Book						= 144,
	kDialogTemplate_Container					= 145,
	kDialogTemplate_Activator					= 147,
	kDialogTemplate_AIForm						= 154,
	kDialogTemplate_Light						= 156,
	kDialogTemplate_Potion						= 165,
	kDialogTemplate_Enchantment					= 166,
	kDialogTemplate_LeveledCreature				= 168,
	kDialogTemplate_Sound						= 190,
	kDialogTemplate_Door						= 196,
	kDialogTemplate_LeveledItem					= 217,
	kDialogTemplate_LandTexture					= 230,
	kDialogTemplate_SoulGem						= 261,
	kDialogTemplate_Ammo						= 262,
	kDialogTemplate_Spell						= 279,
	kDialogTemplate_Flora						= 280,
	kDialogTemplate_Tree						= 287,
	kDialogTemplate_CombatStyle					= 305,
	kDialogTemplate_Water						= 314,
	kDialogTemplate_NPC							= 3202,
	kDialogTemplate_Creature					= 3206,
	kDialogTemplate_Grass						= 3237,
	kDialogTemplate_Furniture					= 3239,
	kDialogTemplate_LoadingScreen				= 3246,
	kDialogTemplate_Ingredient					= 3247,
	kDialogTemplate_LeveledSpell				= 3250,
	kDialogTemplate_AnimObject					= 3251,
	kDialogTemplate_Subspace					= 3252,
	kDialogTemplate_EffectShader				= 3253,
	kDialogTemplate_SigilStone					= 3255,

	// TESFormIDListView Dialogs
	kDialogTemplate_Faction						= 157,
	kDialogTemplate_Race						= 159,
	kDialogTemplate_Class						= 160,
	kDialogTemplate_Skill						= 161,
	kDialogTemplate_EffectSetting				= 163,
	kDialogTemplate_GameSetting					= 170,
	kDialogTemplate_Globals						= 192,
	kDialogTemplate_Birthsign					= 223,
	kDialogTemplate_Climate						= 285,
	kDialogTemplate_Worldspace					= 286,
	kDialogTemplate_Hair						= 289,
	kDialogTemplate_Quest						= 3225,
	kDialogTemplate_Eyes						= 3228,

	// Misc Dialogs
	kDialogTemplate_StringEdit					= 174,
	kDialogTemplate_SelectForm					= 189,
	kDialogTemplate_SoundPick					= 195,
	kDialogTemplate_UseReport					= 220,
	kDialogTemplate_DialogNameConflicts			= 227,
	kDialogTemplate_Package						= 243,
	kDialogTemplate_FileInUse					= 244,
	kDialogTemplate_RegionSystemTester			= 247,			// could be deprecated
	kDialogTemplate_EffectItem					= 267,
	kDialogTemplate_TESFileDetails				= 180,
	kDialogTemplate_SelectWorldspace			= 291,
	kDialogTemplate_FunctionParams				= 300,
	kDialogTemplate_ConfirmFormUserChanges		= 301,
	kDialogTemplate_ClimateChanceWarning		= 310,
	kDialogTemplate_Progress					= 3166,
	kDialogTemplate_ChooseReference				= 3224,
	kDialogTemplate_SelectTopic					= 3226,
	kDialogTemplate_ResponseEditor				= 3231,
	kDialogTemplate_SelectAudioFormat			= 107,
	kDialogTemplate_SelectAudioCaptureDevice	= 134,
	kDialogTemplate_SelectQuests				= 3234,
	kDialogTemplate_SelectQuestsEx				= 3236,
	kDialogTemplate_SoundRecording				= 3241,

	// Subwindows
	kDialogTemplate_AIPackageLocationData		= 240,
	kDialogTemplate_AIPackageTargetData			= 241,
	kDialogTemplate_AIPackageTimeData			= 242,
	kDialogTemplate_RegionEditorWeatherData		= 251,
	kDialogTemplate_RegionEditorObjectsData		= 252,
	kDialogTemplate_RegionEditorGeneral			= 253,
	kDialogTemplate_Reference3DData				= 254,
	kDialogTemplate_RegionEditorMapData			= 256,
	kDialogTemplate_ReferenceLockData			= 257,
	kDialogTemplate_ReferenceTeleportData		= 258,
	kDialogTemplate_ReferenceOwnershipData		= 259,
	kDialogTemplate_ReferenceExtraData			= 260,
	kDialogTemplate_IdleConditionData			= 263,
	kDialogTemplate_RaceBodyData				= 264,
	kDialogTemplate_RaceGeneral					= 265,
	kDialogTemplate_RaceTextData				= 266,
	kDialogTemplate_CellLightingData			= 283,
	kDialogTemplate_CellGeneral					= 284,
	kDialogTemplate_NPCStatsData				= 288,
	kDialogTemplate_RegionEditorObjectsExtraData
												= 309,
	kDialogTemplate_WeatherGeneral				= 311,
	kDialogTemplate_WeatherPecipitationData		= 312,
	kDialogTemplate_WeatherSoundData			= 313,
	kDialogTemplate_HeightMapEditorBrushData	= 3167,			// doubtful
	kDialogTemplate_HeightMapEditorToolBar		= 3169,
	kDialogTemplate_HeightMapEditorOverview		= 3180,
	kDialogTemplate_HeightMapEditorPreview		= 3193,
	kDialogTemplate_HeightMapEditorColorData	= 3201,
	kDialogTemplate_ActorFactionData			= 3203,
	kDialogTemplate_ActorInventoryData			= 3204,
	kDialogTemplate_ActorBlank					= 3205,
	kDialogTemplate_CreatureStatsData			= 3208,
	kDialogTemplate_FactionInterfactionData		= 3209,
	kDialogTemplate_PreferencesRenderWindow		= 3210,
	kDialogTemplate_PreferencesMovement			= 3211,
	kDialogTemplate_PreferencesMisc				= 3212,
	kDialogTemplate_RegionEditorLandscapeData	= 3214,
	kDialogTemplate_ActorAnimationData			= 3215,
	kDialogTemplate_NPCFaceData					= 3217,
	kDialogTemplate_ActorBlankEx				= 3219,			// spell list ?
	kDialogTemplate_RegionEditorGrassData		= 3220,
	kDialogTemplate_PreferencesShader			= 3221,
	kDialogTemplate_PreferencesLOD				= 3222,
	kDialogTemplate_ReferenceMapMarkerData		= 3223,
	kDialogTemplate_QuestGeneral				= 3227,
	kDialogTemplate_RaceFaceData				= 3229,
	kDialogTemplate_DialogData					= 151,
	kDialogTemplate_PreferencesPreviewMovement	= 3230,
	kDialogTemplate_ConditionData				= 3232,
	kDialogTemplate_RegionEditorSoundData		= 3233,
	kDialogTemplate_QuestStageData				= 3240,
	kDialogTemplate_CreatureSoundData			= 3242,
	kDialogTemplate_CellInteriorData			= 3244,
	kDialogTemplate_QuestTargetData				= 3245,
	kDialogTemplate_ReferenceSelectRefData		= 3248,
	kDialogTemplate_CreatureBloodData			= 3254,
	kDialogTemplate_NPCFaceAdvancedData			= 3256,
	kDialogTemplate_WeatherHDRData				= 3257,
	kDialogTemplate_ReferenceLeveledCreatureData
												= 3259,

	// Deprecated
	kDialogTemplate_SelectModel					= 110,
	kDialogTemplate_IngredientEx				= 139,
	kDialogTemplate_AnimGroup					= 155,
	kDialogTemplate_BodyPart					= 153,
	kDialogTemplate_FactionRankDisposition		= 158,
	kDialogTemplate_FindObject					= 205,
	kDialogTemplate_PathGrid					= 208,
	kDialogTemplate_JournalPreview				= 222,
	kDialogTemplate_ExtDoorTeleport				= 224,			// haven't seen it around
	kDialogTemplate_SoundGen					= 228,
	kDialogTemplate_IntDoorTeleport				= 229,			// same here
	kDialogTemplate_IntNorthMarker				= 231,
	kDialogTemplate_VersionControl				= 238,
	kDialogTemplate_VersionControlCheckInProbs	= 255,
	kDialogTemplate_MemoryUsage					= 304,
	kDialogTemplate_TextureUse					= 316,
	kDialogTemplate_CreatureSoundEx				= 3243,
	kDialogTemplate_NPCFaceAdvancedDataEx		= 3216,

	// Unknown
	kDialogTemplate_Unk235						= 235,
	kDialogTemplate_Dialog						= 308,			// looks like a base formIDListView template of sorts
	kDialogTemplate_Progress3238				= 3238,			// could be version control related/ convert ESM for xBox tool related
	kDialogTemplate_Preview3258					= 3258,
	kDialogTemplate_Preview315					= 315
};