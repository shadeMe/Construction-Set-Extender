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

extern std::string					g_AppPath;
extern std::string					g_INIPath;
extern char							g_Buffer[0x200];
extern bool							g_PluginPostLoad;

extern OBSEMessagingInterface*		g_msgIntfc;
extern PluginHandle					g_pluginHandle;
extern HINSTANCE					g_DLLInstance;
extern SME::INI::INIManager*		g_INIManager;
extern SME::INI::INIEditGUI*		g_INIEditGUI;

using namespace SME::MemoryHandler;

class CSEINIManager : public SME::INI::INIManager
{
public:
	void									Initialize();
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

template<typename Type> struct GenericNode
{
	Type				* data;
	GenericNode<Type>	* next;
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

class BSTextureManager;
class BSRenderedTexture;
class NiDX9Renderer;
typedef ModEntry::Data			TESFile;

extern UInt32*					g_RenderWindowStateFlags;

class FileFinder;
class Archive;
struct ResponseEditorData;
class GameSettingCollection;

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
extern GenericNode<Archive>**	g_LoadedArchives;
extern TESWaterForm**			g_DefaultWater;
extern TESRenderSelection**		g_TESRenderSelectionPrimary;
extern HMENU*					g_RenderWindowPopup;
extern void*					g_ScriptCompilerUnkObj;
extern TESObjectREFR**			g_PlayerRef;
extern UInt8*					g_Flag_ObjectWindow_MenuState;
extern UInt8*					g_Flag_CellView_MenuState;
extern CRITICAL_SECTION*		g_ExtraListCS;
extern TESSound**				g_FSTSnowSneak;
extern BSTextureManager**		g_TextureManager;
extern NiDX9Renderer**			g_CSRenderer;
extern UInt8*					g_Flag_RenderWindowUpdateViewPort;
extern FileFinder**				g_FileFinder;
extern ResponseEditorData**		g_ResponseEditorData;
extern GameSettingCollection*	g_GMSTCollection;
extern void*					g_GMSTMap;			// BSTCaseInsensitiveMap<GMSTData*>*

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

extern LPDIRECT3DTEXTURE9		g_LODD3DTexture256x;
extern BSRenderedTexture*		g_LODBSTexture256x;
extern LPDIRECT3DTEXTURE9		g_LODD3DTexture4096x;
extern BSRenderedTexture*		g_LODBSTexture4096x;
extern LPDIRECT3DTEXTURE9		g_LODD3DTexture8192x;
extern BSRenderedTexture*		g_LODBSTexture8192x;

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

typedef void			(__cdecl *_TESDialog_SetCSWindowTitleModifiedFlag)(bool unk01);
extern const _TESDialog_SetCSWindowTitleModifiedFlag TESDialog_SetCSWindowTitleModifiedFlag;

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
extern const UInt32			kTESForm_GetFormReferenceList;
extern const UInt32			kTESForm_CleanupFormReferenceList;
extern const UInt32			kLinkedListNode_RemoveNode;
extern const UInt32			kLinkedListNode_GetIsDangling;
extern const UInt32			kLinkedListNode_Cleanup;
extern const UInt32			kLinkedListNode_GetData;

extern const UInt32			kTESQuest_SetStartEnabled;
extern const UInt32			kTESQuest_SetAllowedRepeatedStages;
extern const UInt32			kTESObjectCELL_GetIsInterior;
extern const UInt32			kTESBipedModelForm_GetIsPlayable;
extern const UInt32			kTESRenderSelection_ClearSelection;
extern const UInt32			kTESRenderSelection_AddFormToSelection;
extern const UInt32			kTESRenderSelection_Free;
extern const UInt32			kTESForm_SaveFormRecord;
extern const UInt32			kTESFile_GetIsESM;
extern const UInt32			kTESFile_Dtor;
extern const UInt32			kDataHandler_PopulateModList;
extern const UInt32			kTESRenderSelection_RemoveFormFromSelection;
extern const UInt32			kTESForm_SetTemporary;

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
extern const UInt32			kVTBL_FileFinder;

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
extern const UInt32			kGameSetting_Ctor;

#define GetVTBL(obj)		*((UInt32*)obj)