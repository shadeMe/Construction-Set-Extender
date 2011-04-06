#pragma once
#pragma warning(disable : 4800; disable : 4005)

#include "UtilityBox.h"
#include "obse_editor/EditorAPI.h"
#include "obse/GameData.h"
#include "obse/Script.h"
#include "obse/PluginAPI.h"
#include "obse/GameObjects.h"
#include "obse/NiNodes.h"
#include "obse/NiObjects.h"
#include "obse/NiRenderer.h"
#include "Console.h"
#include <D3dx9tex.h>

#include "[Libraries]\INI Manager\INIManager.h"
#include "[Libraries]\INI Manager\INIEditGUI.h"

extern std::fstream					g_DEBUG_LOG;
extern std::string					g_AppPath;
extern std::string					g_INIPath;
extern char							g_Buffer[0x200];

extern OBSEMessagingInterface*		g_msgIntfc;
extern PluginHandle					g_pluginHandle;
extern HINSTANCE					g_DLLInstance;
extern SME::INI::INIManager*		g_INIManager;
extern SME::INI::INIEditGUI*		g_INIEditGUI;

class EditorAllocator
{
	static EditorAllocator*													Singleton;

	struct SEAlloc {
		HWND																RichEditControl;
		HWND																ListBoxControl;
		UInt32																Index;

																			SEAlloc(HWND REC, HWND LBC, UInt32 IDX): 
																			RichEditControl(REC), ListBoxControl(LBC), Index(IDX) {};
	};

	typedef std::map<HWND, SEAlloc*>										AlMap;
	AlMap																	AllocationMap;
	UInt32																	NextIndex;
	UInt32																	LastContactedEditor;
public:
	static EditorAllocator*													GetSingleton(void);
	UInt32																	TrackNewEditor(HWND EditorDialog);
	void																	DeleteTrackedEditor(UInt32 TrackedEditorIndex);
	void																	DeleteAllTrackedEditors(void);
	void																	DestroyVanillaDialogs(void);

	HWND																	GetTrackedREC(HWND TrackedEditorDialog);
	HWND																	GetTrackedLBC(HWND TrackedEditorDialog);
	UInt32																	GetTrackedIndex(HWND TrackedEditorDialog);
	HWND																	GetTrackedDialog(UInt32 TrackedEditorIndex);
	UInt32																	GetTrackedEditorCount() { return AllocationMap.size(); }
	UInt32																	GetLastContactedEditor() { return LastContactedEditor; }
	void																	SetLastContactedEditor(UInt32 TrackedEditorIndex) { LastContactedEditor = TrackedEditorIndex; }
};

#define EDAL																EditorAllocator::GetSingleton()

class CSEINIManager : public SME::INI::INIManager
{
public:
	void									Initialize();
};


				
// 0C+?
class TESDialogInitParam
{
public:
	UInt8		TypeID;					// 00
	UInt8		Pad01[3];				
	TESForm*	Form;					// 04
	UInt32		unk08;					// 08

	TESDialogInitParam(const char* EditorID);
	TESDialogInitParam(UInt32 FormID);
};

// 08
class TESCellUseData
{
public:
	TESCellUseData();
	~TESCellUseData();

	TESObjectCELL*		Cell;		// 00
	UInt32				Count;		// 04	
};

class TESCellUseList;

// 08		### partial
class INISetting
{
public:
	INISetting();
	~INISetting();

	union							// 00
	{
		int			iData;
		UInt32		uData;
		float		fData;
		char*		sData;
	};

	const char*		Name;			// 04
};

// 24
class TESFormIDListView
{
public:
	TESFormIDListView();
	~TESFormIDListView();

	// bases
	TESForm					Form;

	// no members
};

// 2C		### partial - look into the common settings class
class GameSetting
{
public:
	GameSetting();
	~GameSetting();

	// bases
	TESFormIDListView		listView;

	//members
	union								// 24
	{
		int					iData;
		float				fData;
		const char*			sData;
	};

	const char*				settingID;	// 28
};

class GameSettingCollection;

template<typename Type> struct GenericNode
{
	Type				* data;
	GenericNode<Type>	* next;
};

// 18 
struct TESRenderSelection
{
	struct SelectedObjectsEntry
	{
		TESForm*				Data;
		SelectedObjectsEntry*	Prev;
		SelectedObjectsEntry*	Next;
	};

	SelectedObjectsEntry*	RenderSelection;	// 00
	UInt32					SelectionCount;		// 04
	float					x, y, z;			// 08 sum of position vectors of selected refr's
	float					unk14;				// 14 init to 0.0
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

// A0
class TESTopicInfo : public TESForm
{
public:
	TESTopicInfo();
	~TESTopicInfo();

	struct TopicListEntry
	{
		TESTopic*			data;
		TopicListEntry*		next;
	};

	struct LinkedTopics
	{
		TopicListEntry		topicsLinkedFrom;
		TopicListEntry		topicsLinkedTo;		// doubles as choices for appropriate infotype
	};

	struct ResponseEntry
	{
		// 24
		struct Data 
		{
			enum 
			{
				kEmotionType_Neutral = 0,
				kEmotionType_Anger,
				kEmotionType_Disgust,
				kEmotionType_Fear,
				kEmotionType_Sad,
				kEmotionType_Happy,
				kEmotionType_Surprise,
			};

			UInt32			emotionType;				// 00
			UInt32			emotionValue;				// 04
			UInt32			unk08;						// 08
			UInt32			unk0C;						// 0C
			String			responseText;				// 10
			String			actorNotes;					// 18	
			UInt32			unk20;						// 20
		};

		Data*				data;
		ResponseEntry*		next;		
	};

	 enum
	 {
		kInfoType_Topic = 0,
		kInfoType_Conversation,
		kInfoType_Combat,
		kInfoType_Persuasion,
		kInfoType_Detection,
		kInfoType_Service,
		kInfoType_Miscellaneous
	 };

	 enum
	 {
		kFlags_Goodbye = 0x0001,
		kFlags_Random = 0x0002,
		kFlags_SayOnce = 0x0004,
		kFlags_Unk008 = 0x0008,
		kFlags_InfoRefusal = 0x0010,
		kFlags_RandomEnd = 0x0020,
		kFlags_RunforRumors = 0x0040
	 };

	TESTopic*			unk024;			// 24 - always NULL ?
	ConditionEntry		conditions;		// 28
	UInt16				unk30;			// 30 - init to -1. used to determine previous info	?
	UInt16				infotype;		// 32 
	UInt8				flags;			// 34
	UInt8				flagsPad[3];	// 35
	TopicListEntry		addedTopics;	// 38
	LinkedTopics*		linkedTopics;	// 40
	ResponseEntry		responses;		// 44
	Script				resultScript;	// 4C
};

typedef TESTopicInfo::ResponseEntry::Data DialogResponse;

// 2C
struct ResponseEditorData
{
	struct VoiceRecorderData
	{
		HWND			recorderDlg;			// 00	
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

// 280		### partial
class Archive
{
public:
	Archive();
	~Archive();

	// 22+? (nothing beyond 0x22 initialized)
	struct Unk154
	{
		UInt32					unk00;			// initialized to 'BSA'
		UInt32					unk04;			// initialized to 0x67
		UInt32					unk08;			// initalized to 0x24
		UInt32					unk0C;
		UInt32					unk10;
		UInt32					unk14;
		UInt32					unk18;
		UInt32					unk1C;
		UInt16					unk20;			// flags of some sort
	};

	// bases
	BSFile						bsfile;			// 000

	//members
	Unk154						unk154;			// 154
	UInt32						unk176;			// 176
	UInt32						unk194;			// 194	bitfield
	LPCRITICAL_SECTION			archiveCS;		// 200
};

// ### partial
union GMSTData
{
	int				i;
	UInt32			u;
	float			f;
	const char*		s;	
};
struct GMSTMap_Key_Comparer
{
	bool operator()(const char* Key1, const char* Key2) const {
		return _stricmp(Key1, Key2) < 0;
	}
};

typedef std::map<const char*, GMSTData*, GMSTMap_Key_Comparer>		_DefaultGMSTMap;
extern _DefaultGMSTMap			g_DefaultGMSTMap;

class BSTextureManager;
class BSRenderedTexture;
class NiDX9Renderer;
typedef ModEntry::Data			TESFile;

enum 
{
	kTESObjectREFRSpecialFlags_3DInvisible				= 1 << 31,			// bits (only?) used in the runtime to mark modifications
	kTESObjectREFRSpecialFlags_Children3DInvisible		= 1 << 30,
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
extern UInt32*					g_RenderWindowStateFlags;

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


extern const HINSTANCE*			g_TESCS_Instance;

extern const DLGPROC			g_ScriptEditor_DlgProc;
extern const DLGPROC			g_UseReport_DlgProc;
extern const DLGPROC			g_TESDialog_DlgProc;
extern const DLGPROC			g_TESDialogListView_DlgProc;

extern HWND*					g_HWND_RenderWindow;
extern HWND*					g_HWND_ObjectWindow;
extern HWND*					g_HWND_CellView;
extern HWND*					g_HWND_CSParent;
extern HWND*					g_HWND_AIPackagesDlg;
extern HWND*					g_HWND_ObjectWindow_FormList;
extern HWND*					g_HWND_ObjectWindow_Tree;
extern HWND*					g_HWND_MainToolbar;
extern HWND*					g_HWND_QuestWindow;

extern TBBUTTON*				g_MainToolbarButtonArray;
extern INISetting*				g_LocalMasterPath;
extern char**					g_TESActivePluginName;
extern UInt8*					g_WorkingFileFlag;
extern UInt8*					g_ActiveChangesFlag;

extern TESWaterForm**			g_DefaultWater;
extern TESRenderSelection**		g_TESRenderSelectionPrimary;
extern HMENU*					g_RenderWindowPopup;
extern void*					g_ScriptCompilerUnkObj;
extern TESObjectREFR**			g_PlayerRef;
extern GameSettingCollection*	g_GMSTCollection;
extern void*					g_GMSTMap;			// BSTCaseInsensitiveMap<GMSTData*>*
extern GenericNode<Archive>**	g_LoadedArchives;
extern ResponseEditorData**		g_ResponseEditorData;
extern UInt8*					g_Flag_ObjectWindow_MenuState;
extern UInt8*					g_Flag_CellView_MenuState;
extern CRITICAL_SECTION*		g_ExtraListCS;
extern TESSound**				g_FSTSnowSneak;
extern BSTextureManager**		g_TextureManager;
extern NiDX9Renderer**			g_CSRenderer;
extern UInt8*					g_Flag_RenderWindowUpdateViewPort;

extern TESForm**				g_DoorMarker;
extern TESForm**				g_NorthMarker;
extern TESForm**				g_TravelMarker;
extern TESForm**				g_MapMarker;
extern TESForm**				g_HorseMarker;

extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture32x; 
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture64x; 
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture128x;
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture512x; 
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture1024x;
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture2048x;

extern BSRenderedTexture**		g_LODBSTexture32x;
extern BSRenderedTexture**		g_LODBSTexture64x;
extern BSRenderedTexture**		g_LODBSTexture128x;
extern BSRenderedTexture**		g_LODBSTexture512x;
extern BSRenderedTexture**		g_LODBSTexture1024x;
extern BSRenderedTexture**		g_LODBSTexture2048x;


typedef LRESULT (__cdecl *_WriteToStatusBar)(WPARAM wParam, LPARAM lParam);
extern const _WriteToStatusBar WriteToStatusBar;

typedef UInt32			(__cdecl *_WritePositionToINI)(HWND Handle, CHAR* ClassName);
extern const _WritePositionToINI WritePositionToINI;

typedef bool			(__cdecl *_GetPositionFromINI)(const char* WindowName, tagRECT* Rect);
extern const _GetPositionFromINI GetPositionFromINI;

typedef UInt32			(__cdecl *_GetTESDialogTemplateForType)(UInt8 FormTypeID);
extern const _GetTESDialogTemplateForType GetTESDialogTemplateForType;

typedef void*			(__cdecl *_GetComboBoxItemData)(HWND ComboBox);
extern const _GetComboBoxItemData GetComboBoxItemData;

typedef bool			(__cdecl *_SelectTESFileCommonDialog)(HWND Parent, const char* SaveDir, bool SaveAsESM, char* Buffer, size_t Size);
extern const _SelectTESFileCommonDialog SelectTESFileCommonDialog;

typedef void			(__cdecl *_sub_4306F0)(bool unk01);
extern const _sub_4306F0 sub_4306F0;

typedef TESObjectREFR* (__cdecl *_ChooseRefWrapper)(HWND Parent, UInt32 unk01, UInt32 unk02, UInt32 unk03);
extern const _ChooseRefWrapper	ChooseRefWrapper;

typedef void*			(__cdecl *_InitializeCSWindows)();
extern const _InitializeCSWindows		InitializeCSWindows;

typedef void			(__cdecl *_DeInitializeCSWindows)();
extern const _DeInitializeCSWindows		DeInitializeCSWindows;

typedef void			(__cdecl *_AddFormToObjectWindow)(TESForm* Form);
extern const _AddFormToObjectWindow		AddFormToObjectWindow;

typedef SpellItem*		(__stdcall *_InitializeDefaultPlayerSpell)(void* Throwaway);
extern const _InitializeDefaultPlayerSpell		InitializeDefaultPlayerSpell;

typedef void (__cdecl *_ConstructEffectSetting)(int EffectID, const char *EffectName, int School, float BaseCost, int MGEFParamA, int Flags, int ResistAV, int NoOfCounterEffects, ...);
extern const _ConstructEffectSetting	ConstructEffectSetting;

typedef void (__cdecl *_TESDialog_AddComboBoxItem)(HWND hWnd, const char* Text, LPARAM unk3, UInt8 unk4);
extern const _TESDialog_AddComboBoxItem TESDialog_AddComboBoxItem;

typedef void			(__cdecl *_BSPrintF)(const char* format, ...);
extern const _BSPrintF		BSPrintF;

typedef void			(__cdecl *_ShowCompilerError)(ScriptBuffer* Buffer, const char* format, ...);
extern const _ShowCompilerError		ShowCompilerErrorEx;

typedef void			(__cdecl *_AutoSavePlugin)(void);
extern const _AutoSavePlugin		AutoSavePlugin;

typedef Archive*		(__cdecl *_CreateArchive)(const char* ArchiveName, UInt16 unk01, UInt8 unk02);	// pass unk01 and unk02 as 0
extern const _CreateArchive			CreateArchive;

typedef void*			(__cdecl *_TESDialog_GetListViewSelectedItemLParam)(HWND ListView);
extern const _TESDialog_GetListViewSelectedItemLParam			TESDialog_GetListViewSelectedItemLParam;

typedef TESForm*			(__cdecl *_TESForm_LookupByFormID)(UInt32 FormID);
extern const _TESForm_LookupByFormID			TESForm_LookupByFormID;

typedef TESForm*			(__cdecl *_TESDialog_GetDialogExtraParam)(HWND Dialog);
extern const _TESDialog_GetDialogExtraParam		TESDialog_GetDialogExtraParam;

typedef TESForm*			(__cdecl *_TESDialog_GetDialogExtraLocalCopy)(HWND Dialog);
extern const _TESDialog_GetDialogExtraLocalCopy		TESDialog_GetDialogExtraLocalCopy;

typedef void			(__cdecl *_TESDialog_ComboBoxPopulateWithRaces)(HWND ComboBox, bool NoneEntry);
extern const _TESDialog_ComboBoxPopulateWithRaces		TESDialog_ComboBoxPopulateWithRaces;

typedef void			(__cdecl *_TESDialog_ComboBoxPopulateWithForms)(HWND ComboBox, UInt8 FormType, bool ClearItems, bool AddDefaultItem);
extern const _TESDialog_ComboBoxPopulateWithForms		TESDialog_ComboBoxPopulateWithForms;

typedef void*			(__cdecl *_TESDialog_GetSelectedItemData)(HWND ComboBox);
extern const _TESDialog_GetSelectedItemData		TESDialog_GetSelectedItemData;

typedef TESObjectREFR*			(__stdcall *_DataHandler_PlaceTESBoundObjectReference)(TESBoundObject* BaseForm, NiVector3* Position, NiVector3* Rotation, float unk03);
extern const _DataHandler_PlaceTESBoundObjectReference		DataHandler_PlaceTESBoundObjectReference;

extern const void *			RTTI_TESCellUseList;

extern const UInt32			kTESChildCell_LoadCell;
extern const UInt32			kTESForm_GetObjectUseList;
extern const UInt32			kTESCellUseList_GetUseListRefHead;
extern const UInt32			kTESObjectCELL_GetParentWorldSpace;
extern const UInt32			kScript_SaveResultScript;
extern const UInt32			kScript_SaveScript;
extern const UInt32			kLinkedListNode_NewNode;		// some BSTSimpleList template initialization
extern const UInt32			kDataHandler_AddBoundObject;
extern const UInt32			kTESForm_SetFormID;
extern const UInt32			kTESForm_SetEditorID;
extern const UInt32			kTESObjectREFR_SetBaseForm;
extern const UInt32			kTESObjectREFR_SetFlagPersistent;
extern const UInt32			kExtraDataList_InitItem;
extern const UInt32			kScript_SetText;
extern const UInt32			kDataHandler_SortScripts;
extern const UInt32			kTESScriptableForm_SetScript;
extern const UInt32			kBSString_Set;
extern const UInt32			kExtraDataList_CopyListForReference;
extern const UInt32			kExtraDataList_CopyList;
extern const UInt32			kGMSTMap_Add;	// NiTPointerMap<const char*, GMSTData>
extern const UInt32			kBSTextureManager_CreateBSRenderedTexture;
extern const UInt32			kTESForm_GetOverrideFile;
extern const UInt32			kTESForm_AddReference;
extern const UInt32			kTESQuest_SetStartEnabled;
extern const UInt32			kTESQuest_SetAllowedRepeatedStages;
extern const UInt32			kTESObjectCELL_GetIsInterior;
extern const UInt32			kTESBipedModelForm_GetIsPlayable;
extern const UInt32			kTESRenderSelection_ClearSelection;
extern const UInt32			kTESRenderSelection_AddFormToSelection;
extern const UInt32			kTESRenderSelection_Free;
extern const UInt32			kTESForm_SaveFormRecord;
extern const UInt32			kTESFile_GetIsESM;

extern const UInt32			kBaseExtraList_GetExtraDataByType;
extern const UInt32			kBaseExtraList_ModExtraEnableStateParent;
extern const UInt32			kBaseExtraList_ModExtraOwnership;
extern const UInt32			kBaseExtraList_ModExtraGlobal;
extern const UInt32			kBaseExtraList_ModExtraRank;
extern const UInt32			kBaseExtraList_ModExtraCount;
extern const UInt32			kTESObjectREFR_ModExtraHealth;
extern const UInt32			kTESObjectREFR_ModExtraCharge;
extern const UInt32			kTESObjectREFR_ModExtraTimeLeft;
extern const UInt32			kTESObjectREFR_ModExtraSoul;
extern const UInt32			kTESObjectREFR_SetExtraEnableStateParent_OppositeState;
extern const UInt32			kTESObjectREFR_GetExtraRef3DData;
extern const UInt32			kTESObjectREFR_RemoveExtraTeleport;

extern const UInt32			kVTBL_TESObjectREFR;
extern const UInt32			kVTBL_TESForm;
extern const UInt32			kVTBL_TESTopicInfo;
extern const UInt32			kVTBL_TESQuest;
extern const UInt32			kVTBL_TESNPC;
extern const UInt32			kVTBL_TESCreature;
extern const UInt32			kVTBL_TESFurniture;
extern const UInt32			kVTBL_TESObjectACTI;
extern const UInt32			kVTBL_TESObjectMISC;
extern const UInt32			kVTBL_TESObjectWEAP;
extern const UInt32			kVTBL_TESObjectCONT;
extern const UInt32			kVTBL_TESObjectCLOT;
extern const UInt32			kVTBL_SpellItem;
extern const UInt32			kVTBL_Script;
extern const UInt32			kVTBL_MessageHandler;

extern const UInt32			kTESNPC_Ctor;
extern const UInt32			kTESCreature_Ctor;
extern const UInt32			kTESFurniture_Ctor;
extern const UInt32			kTESObjectACTI_Ctor;
extern const UInt32			kTESObjectMISC_Ctor;
extern const UInt32			kTESObjectWEAP_Ctor;
extern const UInt32			kTESObjectCONT_Ctor;
extern const UInt32			kTESObjectREFR_Ctor;
extern const UInt32			kTESObjectCLOT_Ctor;
extern const UInt32			kTESQuest_Ctor;
extern const UInt32			kScript_Ctor;
extern const UInt32			kTESRenderSelection_Ctor;


TESObjectREFR*				ChooseReferenceDlg(HWND Parent);
UInt32						GetDialogTemplate(const char* FormType);
UInt32						GetDialogTemplate(UInt8 FormTypeID);
void						RemoteLoadRef(const char* EditorID);
void						RemoteLoadRef(UInt32 FormID);
void						LoadFormIntoView(const char* EditorID, const char* FormType);
void						LoadFormIntoView(const char* EditorID, UInt8 FormType);
void						LoadFormIntoView(UInt32 FormID, const char* FormType);
void						LoadFormIntoView(UInt32 FormID, UInt8 FormType);
void						LoadStartupPlugin();
void						InitializeDefaultGMSTMap();
void						LoadedMasterArchives();
void						UnloadLoadedCell();
void						SpawnCustomScriptEditor(const char* ScriptEditorID);
void						SpawnCustomScriptEditor(UInt32 ScriptFormID);

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

class RenderTimeManager
{
	LARGE_INTEGER				ReferenceFrame;
	LARGE_INTEGER				FrameBuffer;
	LARGE_INTEGER				TimerFrequency;
	long double					TimePassed;					// in seconds
public:

	RenderTimeManager()
	{
		QueryPerformanceCounter(&ReferenceFrame);
		QueryPerformanceFrequency(&TimerFrequency);
	}

	void								Update(void);
	long double							GetTimePassedSinceLastFrame(void) { return TimePassed; }
};

extern RenderTimeManager		g_RenderTimeManager;

class RenderWindowTextPainter
{
	static RenderWindowTextPainter*		Singleton;

	RenderWindowTextPainter();

	class RenderChannelBase
	{
	protected:
		LPD3DXFONT						Font;
		D3DCOLOR						Color;
		RECT							DrawArea;

		bool							Valid;

		RenderChannelBase(INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, DWORD Color, RECT* DrawArea)
		{
			this->Color = Color;

			this->DrawArea.left = DrawArea->left;
			this->DrawArea.right = DrawArea->right;
			this->DrawArea.top = DrawArea->top;
			this->DrawArea.bottom = DrawArea->bottom;

			this->Valid = false;
			if (FAILED(D3DXCreateFont((*g_CSRenderer)->device, FontHeight, FontWidth, FontWeight, 0, FALSE, 
						DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
						DEFAULT_PITCH|FF_DONTCARE, (LPCTSTR)FontFace, &Font)))
			{
				DebugPrint("Failed to create font for RenderChannelBase!");
				return;
			}
			this->Valid = true;
		}
	public:
		virtual void					Render() = 0;
		virtual void					Release()
		{
			if (Valid == false)
				return;

			Font->Release();
		}
		bool							GetIsValid() { return Valid; }
	};

	class StaticRenderChannel : public RenderChannelBase
	{
		std::string						TextToRender;
	public:
		StaticRenderChannel(INT FontHeight, 
							INT FontWidth, 
							UINT FontWeight, 
							const char* FontFace, 
							DWORD Color, 
							RECT* DrawArea) : RenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea) {}
		
		virtual void					Render();
		void							Queue(const char* Text);
		UInt32							GetQueueSize() { return (TextToRender.length() < 1); }
	};

	class DynamicRenderChannel : public RenderChannelBase
	{
		long double						TimeLeft;

		struct QueueTask
		{
			std::string					Text;
			long double					RemainingTime;

			QueueTask(const char* Text, long double SecondsToDisplay) :  Text(Text), RemainingTime(SecondsToDisplay) {}
		};

		std::queue<QueueTask*>			DrawQueue;
	public:
		DynamicRenderChannel(INT FontHeight, 
							INT FontWidth, 
							UINT FontWeight, 
							const char* FontFace, 
							DWORD Color, 
							RECT* DrawArea) : RenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea), TimeLeft(0) {}

		virtual void					Render();
		virtual void					Release();
		void							Queue(const char* Text, long double SecondsToDisplay);
		UInt32							GetQueueSize() { return DrawQueue.size(); }
	};

	StaticRenderChannel*					RenderChannel1;
	DynamicRenderChannel*					RenderChannel2;
	bool									Valid;
public:

	enum
	{
		kRenderChannel_1 = 0,			// static
		kRenderChannel_2				// dynamic
	};

	static RenderWindowTextPainter*		GetSingleton(void);

	bool								Initialize();			// must be called during init
	bool								Recreate() { Release(); return Initialize(); }
	void								Render();
	void								Release();
	void								QueueDrawTask(UInt8 Channel, const char* Text, long double SecondsToDisplay);

	UInt32								GetRenderChannelQueueSize(UInt8 Channel);
	
};
#define RENDERTEXT								RenderWindowTextPainter::GetSingleton()
#define PrintToRender(message, duration)		RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_2, message, duration)

class RenderSelectionGroupManager
{
	typedef std::map<TESObjectCELL*,
					std::vector<TESRenderSelection*>>	_RenderSelectionGroupMap;

	_RenderSelectionGroupMap					SelectionGroupMap;

	TESObjectREFR*								GetRefAtSelectionIndex(TESRenderSelection* Selection, UInt32 Index);

	std::vector<TESRenderSelection*>*			GetCellExists(TESObjectCELL* Cell);
	TESRenderSelection*							AllocateNewSelection(TESRenderSelection* Selection);
	TESRenderSelection*							GetTrackedSelection(TESObjectCELL* Cell, TESRenderSelection* Selection);			// returns the tracked copy of the source selection
	void										UntrackSelection(TESObjectCELL* Cell, TESRenderSelection* TrackedSelection);		// pass GetTrackedSelection's result
public:
	bool										AddGroup(TESObjectCELL* Cell, TESRenderSelection* Selection);
	bool										RemoveGroup(TESObjectCELL* Cell, TESRenderSelection* Selection);

	TESRenderSelection*							GetRefSelectionGroup(TESObjectREFR* Ref, TESObjectCELL* Cell);
	void										Clear();
};

extern RenderSelectionGroupManager		g_RenderSelectionGroupManager;

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