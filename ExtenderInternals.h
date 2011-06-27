#pragma once
#pragma warning(disable : 4800 4005)

#include "[Libraries]\MemoryHandler\MemoryHandler.h"
#include "[Libraries]\INI Manager\INIManager.h"
#include "[Libraries]\INI Manager\INIEditGUI.h"

#include "EditorAPI\TESEditorAPI.h"
#include "obse\PluginAPI.h"
#include "obse\NiNodes.h"
#include "obse\NiObjects.h"
#include "obse\NiRenderer.h"
#include "Console.h"

extern std::string					g_AppPath;
extern std::string					g_INIPath;
extern std::string					g_DLLPath;
extern char							g_TextBuffer[0x200];
extern bool							g_PluginPostLoad;

extern OBSEMessagingInterface*		g_msgIntfc;
extern PluginHandle					g_pluginHandle;
extern HINSTANCE					g_DLLInstance;
extern SME::INI::INIManager*		g_INIManager;
extern SME::INI::INIEditGUI*		g_INIEditGUI;

using namespace SME;
using namespace SME::MemoryHandler;

class CSEINIManager : public INI::INIManager
{
public:
	void									Initialize();
};

// simple LL template
template<typename Type> struct GenericNode
{
	Type				* data;
	GenericNode<Type>	* next;
};

class ElapsedTimeCounter;
extern ElapsedTimeCounter		g_RenderWindowTimeManager;

class BSTextureManager;
class BSRenderedTexture;
class NiDX9Renderer;

extern UInt32*					g_RenderWindowStateFlags;

class FileFinder;
class Archive;
struct ResponseEditorData;
class GameSettingCollection;
class TESPathGridPoint;
class TESPathGrid;
class TESWaterForm;
struct TESRenderUndoStack;
struct TESRenderSelection;

extern TESDataHandler**			g_TESDataHandler;
extern TES**					g_TES;

extern const HINSTANCE*			g_TESCS_Instance;

extern const DLGPROC			g_ScriptEditor_DlgProc;
extern const DLGPROC			g_FormUseReport_DlgProc;
extern const DLGPROC			g_TESDialogFormEdit_DlgProc;
extern const DLGPROC			g_TESDialogFormIDListView_DlgProc;

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
extern Setting*					g_LocalMasterPath;
extern char**					g_TESActivePluginName;
extern UInt8*					g_TESCSAllowAutoSaveFlag;
extern UInt8*					g_TESCSExittingCSFlag;
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
extern UInt8*					g_RenderWindowUpdateViewPort;
extern FileFinder**				g_FileFinder;
extern ResponseEditorData**		g_ResponseEditorData;
extern GameSettingCollection*	g_GMSTCollection;
extern void*					g_GMSTMap;			// BSTCaseInsensitiveMap<GMSTData*>
extern void**					g_IdleFormTree;		// BSTCaseInsensitiveMap<IDLE_ANIM_ROOT>*
extern TESRenderUndoStack**		g_TESRenderUndoStack;
extern TESObjectREFR**			g_TESPreviewControlRef;

extern float*					g_RenderWindowRefMovementSpeed;
extern float*					g_RenderWindowSnapGridDistance;
extern float*					g_RenderWindowRefRotationSpeed;
extern float*					g_RenderWindowSnapAngle;
extern float*					g_RenderWindowCameraRotationSpeed;
extern float*					g_RenderWindowCameraZoomSpeed;
extern float*					g_RenderWindowCameraPanSpeed;

extern UInt8*					g_RenderWindowPathGridEditModeFlag;
extern GenericNode<TESPathGridPoint>*
								g_RenderWindowSelectedPathGridPoints;

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

typedef void*			(* _FormHeap_Allocate)(UInt32 Size);
extern const _FormHeap_Allocate FormHeap_Allocate;

typedef void			(* _FormHeap_Free)(void * Pointer);
extern const _FormHeap_Free FormHeap_Free;

typedef void *			(__cdecl * _Oblivion_DynamicCast)(void * SrcObj, UInt32 Arg1, const void * FromType, const void * ToType, UInt32 Arg4);
extern const _Oblivion_DynamicCast Oblivion_DynamicCast;

typedef LRESULT			(__cdecl *_TESDialog_WriteToStatusBar)(WPARAM wParam, LPARAM lParam);
extern const _TESDialog_WriteToStatusBar TESDialog_WriteToStatusBar;

typedef UInt32			(__cdecl *_TESDialog_WritePositionToINI)(HWND Handle, CHAR* ClassName);
extern const _TESDialog_WritePositionToINI TESDialog_WritePositionToINI;

typedef bool			(__cdecl *_TESDialog_GetPositionFromINI)(const char* WindowName, tagRECT* Rect);
extern const _TESDialog_GetPositionFromINI TESDialog_GetPositionFromINI;

typedef UInt32			(__cdecl *_TESDialog_GetDialogTemplateForFormType)(UInt8 FormTypeID);
extern const _TESDialog_GetDialogTemplateForFormType TESDialog_GetDialogTemplateForFormType;

typedef void*			(__cdecl *_TESDialog_GetComboBoxSelectedItemData)(HWND ComboBox);
extern const _TESDialog_GetComboBoxSelectedItemData TESDialog_GetComboBoxSelectedItemData;

typedef bool			(__cdecl *_TESDialog_SelectTESFileCommonDialog)(HWND Parent, const char* SaveDir, bool SaveAsESM, char* Buffer, size_t Size);
extern const _TESDialog_SelectTESFileCommonDialog TESDialog_SelectTESFileCommonDialog;

typedef void			(__cdecl *_TESDialog_SetCSWindowTitleModifiedFlag)(bool unk01);
extern const _TESDialog_SetCSWindowTitleModifiedFlag TESDialog_SetCSWindowTitleModifiedFlag;

typedef TESObjectREFR* (__cdecl *_TESDialog_ShowSelectReferenceDialog)(HWND Parent, UInt32 unk01, UInt32 unk02, UInt32 unk03);
extern const _TESDialog_ShowSelectReferenceDialog	TESDialog_ShowSelectReferenceDialog;

typedef void*			(__cdecl *_TESDialog_InitializeCSWindows)();
extern const _TESDialog_InitializeCSWindows		TESDialog_InitializeCSWindows;

typedef void			(__cdecl *_TESDialog_DeinitializeCSWindows)();
extern const _TESDialog_DeinitializeCSWindows		TESDialog_DeinitializeCSWindows;

typedef void			(__cdecl *_TESDialog_AddFormToObjectWindow)(TESForm* Form);
extern const _TESDialog_AddFormToObjectWindow		TESDialog_AddFormToObjectWindow;

typedef SpellItem*		(__stdcall *_InitializeDefaultPlayerSpell)(void* Throwaway);
extern const _InitializeDefaultPlayerSpell		InitializeDefaultPlayerSpell;

typedef void			(__cdecl *_ConstructEffectSetting)(int EffectID, const char *EffectName, int School, float BaseCost, int MGEFParamA, int Flags, int ResistAV, int NoOfCounterEffects, ...);
extern const _ConstructEffectSetting	ConstructEffectSetting;

typedef void			(__cdecl *_TESDialog_AddComboBoxItem)(HWND hWnd, const char* Text, LPARAM unk3, UInt8 unk4);
extern const _TESDialog_AddComboBoxItem TESDialog_AddComboBoxItem;

typedef void			(__cdecl *_BSPrintF)(const char* format, ...);
extern const _BSPrintF		BSPrintF;

typedef void			(__cdecl *_ShowCompilerError)(ScriptBuffer* Buffer, const char* format, ...);
extern const _ShowCompilerError		ShowCompilerError;

typedef void			(__cdecl *_AutoSavePlugin)(void);
extern const _AutoSavePlugin		AutoSavePlugin;

typedef Archive*		(__cdecl *_CreateArchive)(const char* ArchiveName, UInt16 unk01, UInt8 unk02);	// pass unk01 and unk02 as 0
extern const _CreateArchive			CreateArchive;

typedef void*			(__cdecl *_TESDialog_GetListViewSelectedItemLParam)(HWND ListView);
extern const _TESDialog_GetListViewSelectedItemLParam			TESDialog_GetListViewSelectedItemLParam;

typedef TESForm*		(__cdecl *_TESForm_LookupByFormID)(UInt32 FormID);
extern const _TESForm_LookupByFormID			TESForm_LookupByFormID;

typedef TESForm*		(__cdecl *_TESForm_LookupByEditorID)(const char* EditorID);
extern const _TESForm_LookupByEditorID			TESForm_LookupByEditorID;

typedef TESForm*		(__cdecl *_TESDialog_GetDialogExtraParam)(HWND Dialog);
extern const _TESDialog_GetDialogExtraParam		TESDialog_GetDialogExtraParam;

typedef TESForm*		(__cdecl *_TESDialog_GetDialogExtraLocalCopy)(HWND Dialog);
extern const _TESDialog_GetDialogExtraLocalCopy		TESDialog_GetDialogExtraLocalCopy;

typedef void			(__cdecl *_TESDialog_ComboBoxPopulateWithRaces)(HWND ComboBox, bool NoneEntry);
extern const _TESDialog_ComboBoxPopulateWithRaces		TESDialog_ComboBoxPopulateWithRaces;

typedef void			(__cdecl *_TESDialog_ComboBoxPopulateWithForms)(HWND ComboBox, UInt8 FormType, bool ClearItems, bool AddDefaultItem);
extern const _TESDialog_ComboBoxPopulateWithForms		TESDialog_ComboBoxPopulateWithForms;

typedef void*			(__cdecl *_TESDialog_GetSelectedItemData)(HWND ComboBox);
extern const _TESDialog_GetSelectedItemData		TESDialog_GetSelectedItemData;

typedef TESObjectREFR*	(__stdcall *_DataHandler_PlaceTESBoundObjectReference)(TESBoundObject* BaseForm, NiVector3* Position, NiVector3* Rotation, float unk03);
extern const _DataHandler_PlaceTESBoundObjectReference		DataHandler_PlaceTESBoundObjectReference;



extern const UInt32			kTESChildCell_LoadCell;
extern const UInt32			kTESCellUseList_GetUseListRefHead;
extern const UInt32			kTESObjectCELL_GetParentWorldSpace;
extern const UInt32			kScript_SaveResultScript;
extern const UInt32			kScript_SaveScript;
extern const UInt32			kLinkedListNode_NewNode;		// some BSSimpleListT template initialization
extern const UInt32			kDataHandler_AddBoundObject;
extern const UInt32			kTESForm_SetFormID;
extern const UInt32			kTESForm_SetEditorID;
extern const UInt32			kTESObjectREFR_SetBaseForm;
extern const UInt32			kTESObjectREFR_SetFlagPersistent;
extern const UInt32			kExtraDataList_InitItem;
extern const UInt32			kScript_SetText;
extern const UInt32			kDataHandler_SortScripts;
extern const UInt32			kTESScriptableForm_SetScript;
extern const UInt32			kBSStringT_Set;
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
extern const UInt32			kTESIdleFormTree_AddRootNodes;
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
extern const UInt32			kTESRenderUndoStack_RecordReference;
extern const UInt32			kTESObjectREFR_PickComparator;
extern const UInt32			kLinkedListNode_CountNodes;
extern const UInt32			kDataHandler_CreateForm;
extern const UInt32			kNiTPointerMap_LookupByKey;
extern const UInt32			kNiTPointerMap_Remove;

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

extern const UInt32			kCtor_TESNPC;
extern const UInt32			kCtor_TESCreature;
extern const UInt32			kCtor_TESFurniture;
extern const UInt32			kCtor_TESObjectACTI;
extern const UInt32			kCtor_TESObjectMISC;
extern const UInt32			kCtor_TESObjectWEAP;
extern const UInt32			kCtor_TESObjectCONT;
extern const UInt32			kCtor_TESObjectREFR;
extern const UInt32			kCtor_TESObjectCLOT;
extern const UInt32			kCtor_TESQuest;
extern const UInt32			kCtor_Script;
extern const UInt32			kCtor_TESRenderSelection;
extern const UInt32			kCtor_GameSetting;

#define GetVTBL(obj)										*((UInt32*)obj)
#define CS_CAST(obj, from, to)								(to *)Oblivion_DynamicCast((void*)(obj), 0, RTTI_ ## from, RTTI_ ## to, 0)

void WaitUntilDebuggerAttached();
const char* PrintToBuffer(const char* fmt, ...);

template <typename T>
void __stdcall ToggleFlag(T* Flag, UInt32 Mask, bool State)
{
	if (State)
		*Flag |= Mask;
	else
		*Flag &= ~Mask;
}
