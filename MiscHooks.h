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


extern FormData*				UIL_FormData;
extern UseListCellItemData*		UIL_CellData;
extern bool						g_SaveAsRoutine;
extern ModEntry::Data*			g_SaveAsBuffer;
extern bool						g_QuickLoadToggle;
extern bool						g_PluginPostLoad;


extern MemHdlr					kSavePluginMasterEnum;
extern NopHdlr					kCheckIsActivePluginAnESM;// allows master files to be set as active plugins
extern NopHdlr					kTESFormGetUnUsedFormID;
extern MemHdlr					kLoadPluginsProlog;
extern MemHdlr					kLoadPluginsEpilog;
extern MemHdlr					kDataDialogPluginDescription;
extern MemHdlr					kDataDialogPluginAuthor;// allows the Author and Description fields of an ESM file to be viewed and modified correctly
extern MemHdlr					kSavePluginCommonDialog;
extern NopHdlr					kResponseEditorMic;// nops out the call to the delinquent sound struct initializer sub, fixing a crash
extern MemHdlr					kDataHandlerPostError;// fixes a crash when the CS attempts to load an unknown record/group
extern MemHdlr					kExitCS;
extern MemHdlr					kFindTextInit;
extern MemHdlr					kCSInit;
extern MemHdlr					kUseInfoListInit;
extern NopHdlr					kMissingTextureWarning;// removes the ostentatious warning
extern NopHdlr					kTopicResultScriptResetNop;// fixes the bug that clears all flags and the result script of a selected response when adding a new topic
extern MemHdlr					kTopicResultScriptReset;
extern MemHdlr					kNPCFaceGen;
extern MemHdlr					kDefaultWaterTextureFix;// gets rid of the ugly pink default water texture
extern MemHdlr					kDataDlgInit;
extern MemHdlr					kQuickLoadPluginLoadHandlerPrologue;
extern MemHdlr					kQuickLoadPluginLoadHandler;
extern MemHdlr					kMissingMasterOverride;// allows the loading of plugins with missing masters
extern MemHdlr					kAssertOverride;
extern MemHdlr					kCSWarningsDetour;
extern MemHdlr					kTextureMipMapCheck;// allows the preview of textures with mipmaps
extern NopHdlr					kAnimGroupNote;// removes the now unnecessary 'See editorWarnings file' anim group debug message
extern MemHdlr					kUnnecessaryDialogEdits;// prevents unnecessary dialog edits in active plugins should its master have a DIAL record
extern MemHdlr					kRenderWindowPopup;
extern MemHdlr					kUnnecessaryCellEdits;// prevents unnecessary cell/worldspace edits in active plugins should its master have a CELL/WRLD record ### Figure out what the function's doing
extern MemHdlr					kCustomCSWindow;
extern MemHdlr					kRaceDescriptionDirtyEdit;// prevent dirty edits occuring when you edit a race's text description and click directly to another race without switching tabs first, if the spellchecker pops up (which it will), the description for the race you were previously working on gets copied into the one you just selected.
extern MemHdlr					kPluginSave;
extern MemHdlr					kPluginLoad;
extern MemHdlr					kAddListViewItem;
extern MemHdlr					kObjectListPopulateListViewItems;
extern MemHdlr					kCellViewPopulateObjectList;
extern MemHdlr					kDoorMarkerProperties;
extern MemHdlr					kAutoLoadActivePluginOnStartup;
extern MemHdlr					kDataHandlerClearDataShadeMeRefDtor;
extern MemHdlr					kCellObjectListShadeMeRefAppend;
extern MemHdlr					kDeathToTheCloseOpenDialogsMessage;
extern MemHdlr					kTopicInfoCopyEpilog;// fixes the bug that causes the wrong topic info to be flagged as active when using the copy popup menu option
extern MemHdlr					kTopicInfoCopyProlog;
extern MemHdlr					kTESDialogPopupMenu;// hooks popup menu instantiation for new menu items

bool PatchMiscHooks(void);
void __stdcall DoCSInitHook();
void __stdcall DoExitCSHook(HWND MainWindow);
SHORT __stdcall IsControlKeyDown(void);
UInt32 __stdcall InitAssetSelectorDlg(HWND Dialog);
UInt32 __stdcall InitPathEditor(int ID, HWND Dialog);
UInt32 __stdcall InitBSAViewer(UInt32 Filter);
void __stdcall SendPingBack(UInt16 Message);


void SavePluginMasterEnumHook(void);// allows esps to be enumerated while filling the file header and provides support for the save as tool
void LoadPluginsPrologHook(void);
void LoadPluginsEpilogHook(void);
void SavePluginCommonDialogHook(void);// allows the creation of ESM files in the CS
void ExitCSHook(void);// adds fast exit to the CS
void FindTextInitHook(void);// hooks the find text window for subclassing
void CSInitHook(void);// adds a one-time only hook to the CS main windows wndproc as an alternative to WinMain()
void UseInfoListInitHook(void);// replaces the otiose "Recreate facial animation files" menu item with "Use Info Listings"
void NPCFaceGenHook(void);// fixes the facegen crash by getting the CS to correctly render the model at dialog init
void DataDlgInitHook(void);// hooks the data window for subclassing
void QuickLoadPluginLoadHandlerPrologueHook(void);// quick loading of plugins (only loads the active plugin)
void QuickLoadPluginLoadHandlerHook(void);
void AssertOverrideHook(void);// fixes crashes from assertion calls in the code and log them to the console/log instead
void CSWarningsDetourHook(void);// whisks away CS warnings to the console
void RenderWindowPopupPatchHook(void);// adds the batch ref editor to the render window's popup menu
void CustomCSWindowPatchHook(void);// keeps custom child windows of the CS main window from being closed on plugin load
void PluginSaveHook(void);// provides a callback post-plugin load/save
void PluginLoadHook(void);
void AddListViewItemHook(void);// patches various routines to check for the 'Hide UnModified Forms' flag before populating controls with forms
void AddComboBoxItemHook(void);
void AddListBoxItemHook(void);
void ObjectListPopulateListViewItemsHook(void);
void CellViewPopulateObjectListHook(void);
void DoorMarkerPropertiesHook(void);// allows the displaying of reference properties for door markers 
void AutoLoadActivePluginOnStartupHook(void);// temporary hook that allows the automatic loading of plugins on startup
void DataHandlerClearDataShadeMeRefDtorHook(void);
void CellObjectListShadeMeRefAppendHook(void);
void TopicInfoCopyEpilogHook(void);
void TESDialogPopupMenuHook(void);


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
// models
const UInt32			kModelCancelCommonDialogHookAddr = 0x0049BDB0;
const UInt32			kModelCancelCommonDialogRestoreRetnAddr = 0x0049BE6B;
const UInt32			kModelCancelCommonDialogNewFileRetnAddr = 0x0049BDBB;
const UInt32			kModelCancelCommonDialogRemoveRetnAddr = 0x0049BE5B;
const UInt32			kModelCancelCommonDialogStackOffset = 0x10;

const UInt32			kModelSelectorCommonDialogHookAddr = 0x0049BDAB;
const UInt32			kModelSelectorCommonDialogRetnAddr = 0x0049BDB0;
const UInt32			kModelSelectorCommonDialogCallAddr = 0x00446C60;
const UInt32			kModelSelectorCommonDialogFilterType = 0;
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
const UInt32			kAnimationSelectorCommonDialogFilterType = 1;
#define					kAnimationPathButtonID				 esi + 0x20

const UInt32			kAnimationPostCommonDialogHookAddr = 0x0049D93C;
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
const UInt32			kSoundSelectorCommonDialogFilterType = 2;
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
const UInt32			kTextureSelectorCommonDialogFilterType = 3;
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
const UInt32			kSPTSelectorCommonDialogFilterType = 4;
#define					kSPTPathButtonID				 edi + 0x20

const UInt32			kSPTPostCommonDialogHookAddr = 0x0049EAE3;
const UInt32			kSPTPostCommonDialogRetnAddr = 0x0049EAE9;

#define e_FetchPath			0x32

// using macros to avoid tedious redefinitions 
#define COMMON_DIALOG_SELECTOR_HOOK(name)  \
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
    }  \
}
#define COMMON_DIALOG_CANCEL_HOOK(name)  \
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

#define COMMON_DIALOG_CANCEL_PATCH(name)						WriteRelJump(k##name##CancelCommonDialogHookAddr, (UInt32)##name##CancelCommonDialogHook);
#define COMMON_DIALOG_SELECTOR_PATCH(name)						WriteRelJump(k##name##SelectorCommonDialogHookAddr, (UInt32)##name##SelectorCommonDialogHook);
#define COMMON_DIALOG_POST_PATCH(name)							WriteRelJump(k##name##PostCommonDialogHookAddr, (UInt32)##name##PostCommonDialogHook);
