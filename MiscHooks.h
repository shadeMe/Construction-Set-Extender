#pragma once
#include "obse/GameData.h"
#include "UtilityBox.h"

class TESForm;
class TESObjectCELL;
class TESWorldSpace;
struct FormData;
struct UseListCellItemData;
class INISetting;
struct NopData;


bool					PatchMiscHooks(void);
void __stdcall			DoCSInitHook();
void __stdcall			DoExitCSHook(HWND MainWindow);

SHORT __stdcall			IsControlKeyDown(void);
void __stdcall			CreateDialogParamAddress(void);

extern FormData*				UIL_FormData;
extern UseListCellItemData*		UIL_CellData;
extern bool						g_SaveAsRoutine;
extern ModEntry::Data*			g_SaveAsBuffer;
extern bool						g_QuickLoadToggle;
extern FARPROC					g_WindowHandleCallAddr;
extern bool						g_PluginPostLoad;



// allows esps to be enumerated while filling the file header and provides support for the save as tool
const UInt32			kSavePluginMasterEnumHookAddr = 0x0047ECC6;
const UInt32			kSavePluginMasterEnumRetnPassAddr = 0x0047ECCF;
const UInt32			kSavePluginMasterEnumRetnFailAddr = 0x0047ECEB;

void					SavePluginMasterEnumHook(void);
// allows master files to be set as active plugins
const NopData			kCheckIsActivePluginAnESMPatch = { 0x0040B65E, 2 };
const NopData			kTESFormGetUnUsedFormIDPatch = { 0x00486C08, 2};
const UInt32			kLoadPluginsPrologHookAddr = 0x00485252;
const UInt32			kLoadPluginsPrologRetnAddr = 0x00485257;
const UInt32			kLoadPluginsPrologCallAddr = 0x00431310;

void LoadPluginsPrologHook(void);

const UInt32			kLoadPluginsEpilogHookAddr = 0x004856B2;
const UInt32			kLoadPluginsEpilogRetnAddr = 0x004856B7;
const UInt32			kLoadPluginsEpilogCallAddr = 0x0047DA60;

void LoadPluginsEpilogHook(void);
// allows the Author and Description fields of an ESM file to be viewed and modified correctly
const UInt32			kDataDialogPluginDescriptionPatchAddr = 0x0040CAB6;
const UInt32			kDataDialogPluginAuthorPatchAddr = 0x0040CAFE;
// allows the creation of ESM files in the CS
const UInt32			kSavePluginCommonDialogHookAddr = 0x00446D51;
const UInt32			kSavePluginCommonDialogESMRetnAddr = 0x00446D58;
const UInt32			kSavePluginCommonDialogESPRetnAddr = 0x00446D69;

void					SavePluginCommonDialogHook(void);
// nops out the call to the delinquent sound struct initializer sub, fixing a crash
const NopData			kResponseEditorMicPatch = { 0x00407F3D, 5 };
// fixes a crash when the CS attempts to load an unknown record/group
const UInt32			kDataHandlerPostErrorPatchAddr = 0x004852F0;
// adds fast exit to the CS
const UInt32			kExitCSHookAddr = 0x00419354;
const UInt32			kExitCSJumpAddr = 0x004B52C1;

void ExitCSHook(void);
// makes the No button in the warning dialog box redundant
const UInt32			kEditorWarningPatchAddr = 0x004B52B0;
// hooks the find text window for subclassing
const UInt32			kFindTextInitHookAddr = 0x00419A42;
const UInt32			kFindTextInitRetnAddr = 0x00419A48;

void FindTextInitHook(void);
// adds a one-time only hook to the CS main windows wndproc as an alternative to WinMain()
const UInt32			kCSInitHookAddr = 0x00419260;
const UInt32			kCSInitRetnAddr = 0x00419265;
const UInt32			kCSInitCallAddr = 0x006E5850;
const UInt8				kCSInitCodeBuffer[5] = { 0xE8, 0xEB, 0xC5, 0x2C, 0 };	// overwritten code

void CSInitHook(void);
// replaces the otiose "Recreate facial animation files" menu item with "Use Info Listings"
const UInt32			kUseInfoListInitHookAddr = 0x00419833;
const UInt32			kUseInfoListInitRetnAddr = 0x00419848;

void UseInfoListInitHook(void);
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

void					ModelSelectorCommonDialogHook(void);
void					ModelPostCommonDialogHook(void);
void					ModelCancelCommonDialogHook(void);
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


void					AnimationSelectorCommonDialogHook(void);
void					AnimationPostCommonDialogHook(void);
void					AnimationCancelCommonDialogHook(void);
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

void					SoundSelectorCommonDialogHook(void);
void					SoundPostCommonDialogHook(void);
void					SoundCancelCommonDialogHook(void);
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

void					TextureSelectorCommonDialogHook(void);
void					TexturePostCommonDialogHook(void);
void					TextureCancelCommonDialogHook(void);
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

void					SPTSelectorCommonDialogHook(void);
void					SPTPostCommonDialogHook(void);
void					SPTCancelCommonDialogHook(void);

UInt32 __stdcall InitAssetSelectorDlg(HWND Dialog);
UInt32 __stdcall InitPathEditor(int ID, HWND Dialog);
UInt32 __stdcall InitBSAViewer(UInt32 Filter);

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

// removes the ostentatious warning
const NopData			kMissingTextureWarningPatch = { 0x0044F3AF, 14 };
// fixes the bug that clears all flags and the result script of a selected response when adding a new topic
const NopData			kTopicResultScriptResetPatch = { 0x004F49A0, 90 };

const UInt32			kTopicResultScriptResetHookAddr = 0x004F49A0;
const UInt32			kTopicResultScriptResetRetnAddr = 0x004F49FA;
// fixes the facegen crash by getting the CS to correctly render the model at dialog init
const UInt32			kNPCFaceGenHookAddr = 0x004D76AC;
const UInt32			kNPCFaceGenRetnAddr = 0x004D76B1;
const UInt32			kNPCFaceGenCallAddr = 0x0049C230;

void NPCFaceGenHook(void);
// gets rid of the ugly pink default water texture
const UInt32			kDefaultWaterTextureFixPatchAddr = 0x0047F792;
// hooks the data window for subclassing
const UInt32			kDataDlgInitHookAddr = 0x0040C6D7;
const UInt32			kDataDlgInitRetnAddr = 0x0040C6DC;
const UInt32			kDataDlgInitCallAddr = 0x00404A90;

void DataDlgInitHook(void);
// quick loading of plugins (only loads the active plugin)
const UInt32			kQuickLoadPluginLoadHandlerPrologueHookAddr = 0x0040D073;
const UInt32			kQuickLoadPluginLoadHandlerPrologueCallAddr = 0x0040CA30;
const UInt32			kQuickLoadPluginLoadHandlerPrologueRetnAddr = 0x0040D078;

void QuickLoadPluginLoadHandlerPrologueHook(void);

const UInt32			kQuickLoadPluginLoadHandlerHookAddr = 0x004852E5;
const UInt32			kQuickLoadPluginLoadHandlerCallAddr = 0x00484A60;		// f_DataHandler::LoadTESFile
const UInt32			kQuickLoadPluginLoadHandlerRetnAddr = 0x004852EE;
const UInt32			kQuickLoadPluginLoadHandlerSkipAddr = 0x004852F0;

void QuickLoadPluginLoadHandlerHook(void);
// allows the loading of plugins with missing masters
const UInt32			kMissingMasterOverridePatchAddr = 0x00484FC9;
const UInt32			kMissingMasterOverrideJumpAddr = 0x00484E8E;
// fixes crashes from assertion calls in the code and log them to the console/log instead
const UInt32			kAssertOverrideHookAddr = 0x004B5670;
const UInt32			kAssertOverrideRetnAddr = 0x004B575E;

void AssertOverrideHook(void);
// whisks away CS warnings to the console
const UInt32			kCSWarningsDetourHookAddr = 0x004B5140;
const UInt32			kCSWarningsDetourRetnAddr = 0x004B5146;

void CSWarningsDetourHook(void);
// allows the preview of textures with mipmaps
const UInt32			kTextureMipMapCheckPatchAddr = 0x0044F49B;
// removes the now unnecessary 'See editorWarnings file' anim group debug message
const NopData			kAnimGroupNotePatch = { 0x004CA21D, 5 };
// prevents unnecessary dialog edits in active plugins should its master have a DIAL record
const UInt32			kUnnecessaryDialogEditsPatchAddr = 0x004EDFF7;
// adds the batch ref editor to the render window's popup menu
const UInt32			kRenderWindowPopupPatchHookAddr = 0x004297CE;
const UInt32			kRenderWindowPopupPatchRetnAddr = 0x004297D3;

void RenderWindowPopupPatchHook(void);
// prevents unnecessary cell/worldspace edits in active plugins should its master have a CELL/WRLD record
// ### Figure out what the function's doing
const UInt32			kUnnecessaryCellEditsPatchAddr = 0x005349A5;
// keeps custom child windows of the CS main window from being closed on plugin load
const UInt32			kCustomCSWindowPatchHookAddr = 0x004311E5;
const UInt32			kCustomCSWindowPatchRetnAddr = 0x004311EF;

void CustomCSWindowPatchHook(void);
// prevent dirty edits occuring when you edit a race's text description and click directly to another race without switching tabs first, if the spellchecker pops up (which it will), the description for the race you were previously working on gets copied into the one you just selected.
const UInt32			kRaceDescriptionDirtyEditPatchAddr = 0x0049405C;
// provides a callback post-plugin load/save
void __stdcall SendPingBack(UInt16 Message);

const UInt32			kPluginSaveHookAddr	=	0x0041BBCD;
const UInt32			kPluginSaveRetnAddr	=	0x0041BBD3;

void PluginSaveHook(void);

const UInt32			kPluginLoadHookAddr	=	0x0041BEFA;
const UInt32			kPluginLoadRetnAddr	=	0x0041BEFF;

void PluginLoadHook(void);
// patches various routines to check for the 'Hide UnModified Forms' flag before populating controls with forms
bool __stdcall			PerformControlPopulationPrologCheck(TESForm* Form);

const UInt32			kAddListViewItemHookAddr = 0x004038F0;
const UInt32			kAddListViewItemRetnAddr = 0x004038F7;
const UInt32			kAddListViewItemExitAddr = 0x0040396E;

void AddListViewItemHook(void);

const UInt32			kAddComboBoxItemHookAddr = 0x00403540;
const UInt32			kAddComboBoxItemRetnAddr = 0x00403548;
const UInt32			kAddComboBoxItemExitAddr = 0x004035F4;

void AddComboBoxItemHook(void);

const UInt32			kObjectListPopulateListViewItemsHookAddr = 0x00413980;
const UInt32			kObjectListPopulateListViewItemsRetnAddr = 0x0041398A;
const UInt32			kObjectListPopulateListViewItemsExitAddr = 0x00413A50;

void ObjectListPopulateListViewItemsHook(void);

const UInt32			kCellViewPopulateObjectListHookAddr = 0x004087C0;
const UInt32			kCellViewPopulateObjectListRetnAddr = 0x004087D3;
const UInt32			kCellViewPopulateObjectListExitAddr = 0x004088AF;

void CellViewPopulateObjectListHook(void);
// allows the displaying of reference properties for door markers 
const UInt32			kDoorMarkerPropertiesHookAddr = 0x00429EA9;
const UInt32			kDoorMarkerPropertiesPropertiesAddr = 0x00429EB1;
const UInt32			kDoorMarkerPropertiesTeleportAddr = 0x00429EE8;

void DoorMarkerPropertiesHook(void);