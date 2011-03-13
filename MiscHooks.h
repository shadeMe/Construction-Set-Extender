#pragma once
#include "obse/GameData.h"
#include "Hooks_Common.h"

class TESForm;
class TESObjectCELL;
class TESWorldSpace;
struct FormData;
struct UseListCellItemData;
class INISetting;
struct NopData;

extern bool						g_QuickLoadToggle;
extern bool						g_PluginPostLoad;
extern HFONT					g_CSDefaultFont;


extern MemHdlr					kSavePluginMasterEnum;// allows esps to be enumerated while filling the file header and provides support for the save as tool
extern NopHdlr					kCheckIsActivePluginAnESM;// allows master files to be set as active plugins
extern NopHdlr					kTESFormGetUnUsedFormID;
extern MemHdlr					kLoadPluginsProlog;
extern MemHdlr					kLoadPluginsEpilog;
extern MemHdlr					kDataDialogPluginDescription;// allows the Author and Description fields of an ESM file to be viewed and modified correctly
extern MemHdlr					kDataDialogPluginAuthor;
extern MemHdlr					kSavePluginCommonDialog;// allows the creation of ESM files in the CS
extern NopHdlr					kResponseEditorMic;// nops out the call to the delinquent sound struct initializer sub, fixing a crash
extern MemHdlr					kDataHandlerPostError;// fixes a crash when the CS attempts to load an unknown record/group
extern MemHdlr					kExitCS;// adds fast exit to the CS
extern MemHdlr					kFindTextInit;// hooks the find text window for subclassing
extern MemHdlr					kCSInit;// adds an one-time only hook to the CS main windows wndproc as an alternative to WinMain()
extern MemHdlr					kMessagePumpInit;// prevents the premature calling of DoCSInitHook when the cs enters an idle state through a message loop
extern MemHdlr					kUseInfoListInit;// replaces the redundant "Recreate facial animation files" menu item with "Use Info Listings"
extern NopHdlr					kMissingTextureWarning;// removes the ostentatious warning
extern MemHdlr					kTopicResultScriptReset;// fixes the bug that clears all flags and the result script of a selected response when adding a new topic
extern MemHdlr					kNPCFaceGen;// fixes the facegen crash by getting the CS to correctly render the model at dialog init
extern MemHdlr					kDefaultWaterTextureFix;// gets rid of the ugly pink default water texture
extern MemHdlr					kDataDlgInit;// hooks the data window for subclassing
extern MemHdlr					kQuickLoadPluginLoadHandlerPrologue;// adds support for the quick loading of plugins (only loads the active plugin)
extern MemHdlr					kQuickLoadPluginLoadHandler;
extern MemHdlr					kMissingMasterOverride;// allows the loading of plugins with missing masters
extern MemHdlr					kAssertOverride;// fixes crashes from assertion calls in the code and log them to the console/log instead
extern MemHdlr					kTextureMipMapCheck;// allows the preview of textures with mipmaps
extern NopHdlr					kAnimGroupNote;// removes the now unnecessary 'See editorWarnings file' anim group debug message
extern MemHdlr					kUnnecessaryDialogEdits;// prevents unnecessary dialog edits in active plugins should its master have a DIAL record
extern MemHdlr					kRenderWindowPopup;// adds the batch ref editor to the render window's popup menu
extern MemHdlr					kUnnecessaryCellEdits;// prevents unnecessary cell/worldspace edits in active plugins should its master have a CELL/WRLD record ### Figure out what the function's doing
extern MemHdlr					kCustomCSWindow;// keeps custom child windows of the CS main window from being closed on plugin load
extern MemHdlr					kRaceDescriptionDirtyEdit;// prevent dirty edits occuring when you edit a race's text description and click directly to another race without switching tabs first, if the spellchecker pops up (which it will), the description for the race you were previously working on gets copied into the one you just selected.
extern MemHdlr					kPluginSave;// provides a callback post-plugin load/save
extern MemHdlr					kPluginLoad;
extern MemHdlr					kAddListViewItem;// patches various routines to check for the 'Hide UnModified Forms' flag before populating controls with forms
extern MemHdlr					kObjectListPopulateListViewItems;
extern MemHdlr					kCellViewPopulateObjectList;
extern MemHdlr					kDoorMarkerProperties;// allows the displaying of reference properties for door markers 
extern MemHdlr					kAutoLoadActivePluginOnStartup;// temporary hook that allows the automatic loading of plugins on startup
extern MemHdlr					kDataHandlerClearDataShadeMeRefDtor;
extern MemHdlr					kCellObjectListShadeMeRefAppend;
extern MemHdlr					kDeathToTheCloseOpenDialogsMessage;// gets rid of it
extern MemHdlr					kTopicInfoCopyEpilog;// fixes the bug that causes the wrong topic info to be flagged as active when using the copy popup menu option
extern MemHdlr					kTopicInfoCopyProlog;
extern MemHdlr					kTESDialogPopupMenu;// hooks popup menu instantiation for new menu items
extern MemHdlr					kResponseWindowLipButtonPatch;// adds suport for the 'fixed' lipsync tool
extern MemHdlr					kResponseWindowInit;// hooks the response editor dialog for subclassing
extern MemHdlr					kNumericEditorID;// displays a warning when editorIDs start with an integer
extern MemHdlr					kDataHandlerConstructSpecialForms;// initializes easter egg forms
extern MemHdlr					kResultScriptSaveForm;// prevents a crash that occurs when a result script has local variable declarations
extern MemHdlr					kDataDlgZOrder;// prevents the data dlg from staying ontop of all other windows
extern MemHdlr					kFormIDListViewInit;// changes the text of the OK and CANCEL buttons
extern MemHdlr					kFormIDListViewSaveChanges;// prevents the OK button from closing the list view dialog
extern MemHdlr					kFormIDListViewItemChange;// displays a confirmation message before comitting changes made to the active item when switching to another
extern MemHdlr					kFormIDListViewSelectItem;// fixes a bug that prevents the correct selection of newly created list view items
extern MemHdlr					kFormIDListViewDuplicateSelection;// increments the item index returned by TESDialog::LookupListViewItemByData when duplicating forms from the popup menu
extern MemHdlr					kTESRaceCopyHairEyeDataInit;// adds buttons to the face data tab page of the TESRace formIDListView dialog
extern MemHdlr					kTESRaceCopyHairEyeDataMessageHandler;// handles the WM_COMMAND messages sent by the newly added controls
extern NopHdlr					kTESDialogSubwindowEnumChildCallback;// patches the TESDialogSubWindow::EnumChildWindowsCallback function to keep it from overwriting the subwindow object's container member
extern MemHdlr					kTESObjectREFRDoCopyFrom;// patches the TESObjectREFR::Copy handler to fully duplicate extradata from the source
extern NopHdlr					kLODLandTextureMipMapLevelA;// patches the LOD texture generator to generate the full mip chain for diffuse maps
extern MemHdlr					kLODLandTextureMipMapLevelB;// patches the LOD texture generator to generate the full mip chain for normal maps
extern MemHdlr					kLODLandTextureResolution;// quadruples the resolution of lod landscape diffuse maps
extern MemHdlr					kDataHandlerSaveFormToFile;// allows records in esp masters to be overridden with deleted records
extern MemHdlr					kTESFileUpdateHeader;// prevents TESFile::UpdateHeader from continuing for locked files
extern MemHdlr					kTESObjectREFRGet3DData;// selectively culls reference nodes depending on the presence of various visibiity flags

bool PatchMiscHooks(void);
void PatchMessageHandler(void);
void __stdcall DoCSInitHook();
void __stdcall DoExitCSHook(HWND MainWindow);
UInt32 __stdcall IsControlKeyDown(void);
UInt32 __stdcall InitAssetSelectorDlg(HWND Dialog);
UInt32 __stdcall InitPathEditor(int ID, HWND Dialog);
UInt32 __stdcall InitBSAViewer(UInt32 Filter);
void __stdcall SendPingBack(UInt16 Message);
void __stdcall MessageHandlerOverride(const char* Message);


void SavePluginMasterEnumHook(void);
void LoadPluginsPrologHook(void);
void LoadPluginsEpilogHook(void);
void SavePluginCommonDialogHook(void);
void ExitCSHook(void);
void FindTextInitHook(void);
void CSInitHook(void);
void MessagePumpInitHook(void);
void UseInfoListInitHook(void);
void NPCFaceGenHook(void);
void DataDlgInitHook(void);
void QuickLoadPluginLoadHandlerPrologueHook(void);
void QuickLoadPluginLoadHandlerHook(void);
void AssertOverrideHook(void);
void CSWarningsDetourHook(void);
void RenderWindowPopupPatchHook(void);
void CustomCSWindowPatchHook(void);
void PluginSaveHook(void);
void PluginLoadHook(void);
void AddListViewItemHook(void);
void AddComboBoxItemHook(void);
void AddListBoxItemHook(void);
void ObjectListPopulateListViewItemsHook(void);
void CellViewPopulateObjectListHook(void);
void DoorMarkerPropertiesHook(void);
void AutoLoadActivePluginOnStartupHook(void);
void DataHandlerClearDataShadeMeRefDtorHook(void);
void CellObjectListShadeMeRefAppendHook(void);
void TopicInfoCopyEpilogHook(void);
void TESDialogPopupMenuHook(void);
void ResponseWindowInitHook(void);
void NumericEditorIDHook(void);
void DataHandlerConstructSpecialFormsHook(void);
void ResultScriptSaveFormHook(void);
void FormIDListViewInitHook(void);
void FormIDListViewItemChangeHook(void);
void FormIDListViewSelectItemHook(void);
void FormIDListViewDuplicateSelectionHook(void);
void TESRaceCopyHairEyeDataInitHook(void);
void TESRaceCopyHairEyeDataMessageHandlerHook(void);
void FormIDListViewSaveChangesHook(void);
void TESObjectREFRDoCopyFromHook(void);
void LODLandTextureMipMapLevelBHook(void);
void LODLandTextureResolutionHook(void);
void DataHandlerSaveFormToFileHook(void);
void TESFileUpdateHeaderHook(void);
void TESObjectREFRGet3DDataHook(void);


void ModelSelectorCommonDialogHook(void);
void ModelPostCommonDialogHook(void);
void ModelCancelCommonDialogHook(void);
void AnimationSelectorCommonDialogHook(void);
void AnimationPostCommonDialogHook(void);
void AnimationCancelCommonDialogHook(void);
void SoundSelectorCommonDialogHook(void);
void SoundPostCommonDialogHook(void);
void SoundCancelCommonDialogHook(void);
void TextureSelectorCommonDialogHook(void);
void TexturePostCommonDialogHook(void);
void TextureCancelCommonDialogHook(void);
void SPTSelectorCommonDialogHook(void);
void SPTPostCommonDialogHook(void);
void SPTCancelCommonDialogHook(void);



// common dialog patches - adds support for the BSA viewer, path editing, patches cancel/close behaviour
enum AssetSelectorFilter
{
	e_NIF = 0,
	e_KF,
	e_WAV,
	e_DDS,
	e_SPT
};
// models
const UInt32			kModelCancelCommonDialogHookAddr = 0x0049BDB0;
const UInt32			kModelCancelCommonDialogRestoreRetnAddr = 0x0049BE6B;
const UInt32			kModelCancelCommonDialogNewFileRetnAddr = 0x0049BDBB;
const UInt32			kModelCancelCommonDialogRemoveRetnAddr = 0x0049BE5B;
const UInt32			kModelCancelCommonDialogStackOffset = 0x10;

const UInt32			kModelSelectorCommonDialogHookAddr = 0x0049BDAB;
const UInt32			kModelSelectorCommonDialogRetnAddr = 0x0049BDB0;
const UInt32			kModelSelectorCommonDialogCallAddr = 0x00446C60;
const UInt32			kModelSelectorCommonDialogFilterType = e_NIF;
#define					kModelPathButtonID				 edi + 0x20

const UInt32			kModelPostCommonDialogHookAddr = 0x0049BDBE;
const UInt32			kModelPostCommonDialogRetnAddr = 0x0049BDC4;

// animations
const UInt32			kAnimationCancelCommonDialogHookAddr = 0x0049D915;
const UInt32			kAnimationCancelCommonDialogRestoreRetnAddr = 0x0049DA04;
const UInt32			kAnimationCancelCommonDialogNewFileRetnAddr = 0x0049D920;
const UInt32			kAnimationCancelCommonDialogRemoveRetnAddr = 0x0049D9F4;
const UInt32			kAnimationCancelCommonDialogStackOffset = 0x28;

const UInt32			kAnimationSelectorCommonDialogHookAddr = 0x0049D910;
const UInt32			kAnimationSelectorCommonDialogRetnAddr = 0x0049D915;
const UInt32			kAnimationSelectorCommonDialogCallAddr = 0x00446A30;
const UInt32			kAnimationSelectorCommonDialogFilterType = e_KF;
#define					kAnimationPathButtonID				 esi + 0x20

const UInt32			kAnimationPostCommonDialogHookAddr = 0x0049D92B;
const UInt32			kAnimationPostCommonDialogRetnAddr = 0x0049D943;

// sounds
const UInt32			kSoundCancelCommonDialogHookAddr = 0x004A1C88;
const UInt32			kSoundCancelCommonDialogRestoreRetnAddr = 0x004A1CA7;
const UInt32			kSoundCancelCommonDialogNewFileRetnAddr = 0x004A1C8F;
const UInt32			kSoundCancelCommonDialogRemoveRetnAddr = 0x004A1C96;
const UInt32			kSoundCancelCommonDialogStackOffset = 0x10;

const UInt32			kSoundSelectorCommonDialogHookAddr = 0x004A1C83;
const UInt32			kSoundSelectorCommonDialogRetnAddr = 0x004A1C88;
const UInt32			kSoundSelectorCommonDialogCallAddr = 0x004431A0;
const UInt32			kSoundSelectorCommonDialogFilterType = e_WAV;
#define					kSoundPathButtonID				 1451

const UInt32			kSoundPostCommonDialogHookAddr = 0x004A1C8F;
const UInt32			kSoundPostCommonDialogRetnAddr = 0x004A1C9B;

// textures
const UInt32			kTextureCancelCommonDialogHookAddr = 0x004A4150;
const UInt32			kTextureCancelCommonDialogRestoreRetnAddr = 0x004A425F;
const UInt32			kTextureCancelCommonDialogNewFileRetnAddr = 0x004A415B;
const UInt32			kTextureCancelCommonDialogRemoveRetnAddr = 0x004A4240;
const UInt32			kTextureCancelCommonDialogStackOffset = 0x14;

const UInt32			kTextureSelectorCommonDialogHookAddr = 0x004A414B;
const UInt32			kTextureSelectorCommonDialogRetnAddr = 0x004A4150;
const UInt32			kTextureSelectorCommonDialogCallAddr = 0x00446CA0;
const UInt32			kTextureSelectorCommonDialogFilterType = e_DDS;
#define					kTexturePathButtonID				 esi + 0x10

const UInt32			kTexturePostCommonDialogHookAddr = 0x004A415B;
const UInt32			kTexturePostCommonDialogRetnAddr = 0x004A4162;

// speed tree files
const UInt32			kSPTCancelCommonDialogHookAddr = 0x0049EAD5;
const UInt32			kSPTCancelCommonDialogRestoreRetnAddr = 0x0049EB90;
const UInt32			kSPTCancelCommonDialogNewFileRetnAddr = 0x0049EAE0;
const UInt32			kSPTCancelCommonDialogRemoveRetnAddr = 0x0049EB80;
const UInt32			kSPTCancelCommonDialogStackOffset = 0x28;

const UInt32			kSPTSelectorCommonDialogHookAddr = 0x0049EAD0;
const UInt32			kSPTSelectorCommonDialogRetnAddr = 0x0049EAD5;
const UInt32			kSPTSelectorCommonDialogCallAddr = 0x00446A30;
const UInt32			kSPTSelectorCommonDialogFilterType = e_SPT;
#define					kSPTPathButtonID				 edi + 0x20

const UInt32			kSPTPostCommonDialogHookAddr = 0x0049EAE3;
const UInt32			kSPTPostCommonDialogRetnAddr = 0x0049EAE9;

enum AssetSelectorResult
{
	e_Close = 0,
	e_FileBrowser,
	e_BSABrowser,
	e_EditPath,
	e_ClearPath,
	e_CopyPath,
	e_FetchPath = 0x32
};

#define DefineCommonDialogPrologHandler(name)  \
void __declspec(naked) ##name##SelectorCommonDialogHook(void)  \
{  \
    {  \
		__asm mov		eax, [esp] \
        __asm pushad  \
		__asm push		eax \
        __asm call		InitAssetSelectorDlg  \
		__asm cmp		eax, e_Close \
		__asm jz		CLOSE \
        __asm cmp		eax, e_FileBrowser  \
        __asm jz		FILEB  \
		__asm cmp		eax, e_BSABrowser \
		__asm jz		BSAB \
		__asm cmp		eax, e_EditPath \
		__asm jz		EDITP \
		__asm cmp		eax, e_CopyPath \
		__asm jz		COPYP \
        __asm popad  \
		__asm mov		eax, e_ClearPath \
        __asm jmp		[k##name##SelectorCommonDialogRetnAddr]  \
	__asm CLOSE: \
		__asm popad \
		__asm mov		eax, e_Close \
        __asm jmp		[k##name##SelectorCommonDialogRetnAddr]  \
	__asm FILEB: \
		__asm popad \
		__asm call		[k##name##SelectorCommonDialogCallAddr]  \
        __asm jmp		[k##name##SelectorCommonDialogRetnAddr]  \
    __asm BSAB:  \
        __asm popad  \
		__asm push		k##name##SelectorCommonDialogFilterType	\
		__asm call		InitBSAViewer \
        __asm jmp		[k##name##SelectorCommonDialogRetnAddr]      \
	__asm EDITP: \
		__asm popad \
		__asm push		eax \
		__asm mov		eax, [k##name##PathButtonID] \
		__asm push		eax \
		__asm call		InitPathEditor \
        __asm jmp		[k##name##SelectorCommonDialogRetnAddr]      \
	__asm COPYP: \
		__asm popad \
		__asm push		eax \
		__asm push		k##name##SelectorCommonDialogFilterType	\
		__asm call		InitPathCopier \
        __asm jmp		[k##name##SelectorCommonDialogRetnAddr]      \
    }  \
}
#define DefineCommonDialogCancelHandler(name)  \
void __declspec(naked) ##name##CancelCommonDialogHook(void)  \
{  \
    {  \
        __asm add		esp, [k##name##CancelCommonDialogStackOffset]  \
		__asm cmp		eax, e_ClearPath \
		__asm jz		REMOVE \
		__asm test		al, al  \
        __asm jz		RESTORE  \
        __asm jmp		[k##name##CancelCommonDialogNewFileRetnAddr]  \
    __asm RESTORE:  \
        __asm jmp		[k##name##CancelCommonDialogRestoreRetnAddr]  \
    __asm REMOVE:  \
        __asm jmp		[k##name##CancelCommonDialogRemoveRetnAddr]      \
    }  \
}

#define PatchCommonDialogCancelHandler(name)							WriteRelJump(k##name##CancelCommonDialogHookAddr, (UInt32)##name##CancelCommonDialogHook);
#define PatchCommonDialogPrologHandler(name)							WriteRelJump(k##name##SelectorCommonDialogHookAddr, (UInt32)##name##SelectorCommonDialogHook);
#define PatchCommonDialogEpilogHandler(name)							WriteRelJump(k##name##PostCommonDialogHookAddr, (UInt32)##name##PostCommonDialogHook);
