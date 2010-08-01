#pragma once

// TODO: ++++++++++++++++++


struct NopData
{
	UInt32				Address;
	UInt8				Size;
};

class TESForm;
class TESObjectCELL;
class TESWorldSpace;
struct FormData;
struct UseListCellItemData;
class INISetting;

bool					PatchMiscHooks(void);
void					DoNop(const NopData* Data);
SHORT __stdcall			IsControlKeyDown(void);
void __stdcall CreateDialogParamAddress(void);

extern FormData*				UIL_FormData;
extern UseListCellItemData*		UIL_CellData;


// nop out conditional jump to allow ESP files be processed when populating the master list during a save callback
// the cs' sanity check should handle the modified code path
const NopData			kCheckLoadedPluginTypePatch = { 0x0047ECCD, 2 };
// allows master files to be set as active plugins
const NopData			kCheckIsActivePluginAnESMPatch = { 0x0040B65E, 2 };
// nops out a couple of mov instructions that are invalid when editing an active ESM
// TODO: Figure out what I'm screwing with
const NopData			kDoValidateBitArrayPatch[2] = 
												{
													{ 0x00408261, 6 },
													{ 0x0040826D, 6 }
												};
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
const UInt32			kConstructObjectUnknownRecordPatchJmpAddr = 0x00479E5B;
const UInt32			kConstructObjectUnknownRecordPatchDestAddr = 0x00479E92;
const UInt32			kDataHandlerPostErrorPatchAddr = 0x004852F0;
// adds fast exit to the CS
const UInt32			kExitCSHookAddr = 0x00419354;
const UInt32			kExitCSJumpAddr = 0x004B52C1;

void __stdcall DoExitCS(HWND MainWindow);
void ExitCSHook(void);
// makes the No button in the warning dialog box redundant
const UInt32			kEditorWarningPatchAddr = 0x004B52B0;
// hooks the find text window for subclassing
const UInt32			kFindTextInitHookAddr = 0x00419A42;
const UInt32			kFindTextInitRetnAddr = 0x00419A48;

void __stdcall DoFindTextInitHook(HWND FindTextDialog);
void FindTextInitHook(void);
// adds a one-time only hook to the CS main windows wndproc as an alternative to WinMain()
const UInt32			kCSInitHookAddr = 0x00419260;
const UInt32			kCSInitRetnAddr = 0x00419265;
const UInt32			kCSInitCallAddr = 0x006E5850;

void __stdcall DoCSInitHook();
void CSInitHook(void);
// replaces the otiose "Recreate facial animation files" menu item with "Use Info Listings"
const UInt32			kUseInfoListInitHookAddr = 0x00419833;
const UInt32			kUseInfoListInitRetnAddr = 0x00419848;

void UseInfoListInitHook(void);
void PatchUseInfoListMenu(void);

// common dialog patches - adds support for the BSA viewer, patches cancel/close behaviour
void __stdcall			ShowBSAViewer(UInt32 Filter);
// models
const UInt32			kModelCancelCommonDialogHookAddr = 0x0049BDB0;
const UInt32			kModelCancelCommonDialogRestoreRetnAddr = 0x0049BE6B;
const UInt32			kModelCancelCommonDialogNewFileRetnAddr = 0x0049BDBB;
const UInt32			kModelCancelCommonDialogRemoveRetnAddr = 0x0049BE5B;
const UInt32			kModelCancelCommonDialogStackOffset = 0x10;

const UInt32			kModelBSACommonDialogHookAddr = 0x0049BDAB;
const UInt32			kModelBSACommonDialogRetnAddr = 0x0049BDB0;
const UInt32			kModelBSACommonDialogCallAddr = 0x00446C60;
const UInt32			kModelBSACommonDialogFilterType = 1;

const UInt32			kModelPostCommonDialogHookAddr = 0x0049BDBE;
const UInt32			kModelPostCommonDialogRetnAddr = 0x0049BDC4;

void					ModelBSACommonDialogHook(void);
void					ModelPostCommonDialogHook(void);
void					ModelCancelCommonDialogHook(void);
// animations
const UInt32			kAnimationCancelCommonDialogHookAddr = 0x0049D915;
const UInt32			kAnimationCancelCommonDialogRestoreRetnAddr = 0x0049DA04;
const UInt32			kAnimationCancelCommonDialogNewFileRetnAddr = 0x0049D920;
const UInt32			kAnimationCancelCommonDialogRemoveRetnAddr = 0x0049D9F4;
const UInt32			kAnimationCancelCommonDialogStackOffset = 0x28;

const UInt32			kAnimationBSACommonDialogHookAddr = 0x0049D910;
const UInt32			kAnimationBSACommonDialogRetnAddr = 0x0049D915;
const UInt32			kAnimationBSACommonDialogCallAddr = 0x00446A30;
const UInt32			kAnimationBSACommonDialogFilterType = 2;

const UInt32			kAnimationPostCommonDialogHookAddr = 0x0049D93C;
const UInt32			kAnimationPostCommonDialogRetnAddr = 0x0049D943;

void					AnimationBSACommonDialogHook(void);
void					AnimationPostCommonDialogHook(void);
void					AnimationCancelCommonDialogHook(void);
// sounds
const UInt32			kSoundCancelCommonDialogHookAddr = 0x004A1C88;
const UInt32			kSoundCancelCommonDialogRestoreRetnAddr = 0x004A1CA7;
const UInt32			kSoundCancelCommonDialogNewFileRetnAddr = 0x004A1C8F;
const UInt32			kSoundCancelCommonDialogRemoveRetnAddr = 0x004A1C96;
const UInt32			kSoundCancelCommonDialogStackOffset = 0x10;

void					SoundCancelCommonDialogHook(void);
// textures
const UInt32			kTextureCancelCommonDialogHookAddr = 0x004A4150;
const UInt32			kTextureCancelCommonDialogRestoreRetnAddr = 0x004A425F;
const UInt32			kTextureCancelCommonDialogNewFileRetnAddr = 0x004A415B;
const UInt32			kTextureCancelCommonDialogRemoveRetnAddr = 0x004A4240;
const UInt32			kTextureCancelCommonDialogStackOffset = 0x14;

const UInt32			kTextureBSACommonDialogHookAddr = 0x004A414B;
const UInt32			kTextureBSACommonDialogRetnAddr = 0x004A4150;
const UInt32			kTextureBSACommonDialogCallAddr = 0x00446CA0;
const UInt32			kTextureBSACommonDialogFilterType = 3;

const UInt32			kTexturePostCommonDialogHookAddr = 0x004A415B;
const UInt32			kTexturePostCommonDialogRetnAddr = 0x004A4162;

void					TextureBSACommonDialogHook(void);
void					TexturePostCommonDialogHook(void);
void					TextureCancelCommonDialogHook(void);
// speed tree files
const UInt32			kSPTCancelCommonDialogHookAddr = 0x0049EAD5;
const UInt32			kSPTCancelCommonDialogRestoreRetnAddr = 0x0049EB90;
const UInt32			kSPTCancelCommonDialogNewFileRetnAddr = 0x0049EAE0;
const UInt32			kSPTCancelCommonDialogRemoveRetnAddr = 0x0049EB80;
const UInt32			kSPTCancelCommonDialogStackOffset = 0x28;

const UInt32			kSPTBSACommonDialogHookAddr = 0x0049EAD0;
const UInt32			kSPTBSACommonDialogRetnAddr = 0x0049EAD5;
const UInt32			kSPTBSACommonDialogCallAddr = 0x00446A30;
const UInt32			kSPTBSACommonDialogFilterType = 4;

const UInt32			kSPTPostCommonDialogHookAddr = 0x0049EAE3;
const UInt32			kSPTPostCommonDialogRetnAddr = 0x0049EAE9;

void					SPTBSACommonDialogHook(void);
void					SPTPostCommonDialogHook(void);
void					SPTCancelCommonDialogHook(void);

// using macros to avoid tedious redefinitions 
#define COMMON_DIALOG_CANCEL_HOOK(name)  \
void __declspec(naked) ##name##CancelCommonDialogHook(void)  \
{  \
    {  \
        __asm add		esp, [k##name##CancelCommonDialogStackOffset]  \
		__asm test		al, al  \
        __asm jz		RESTORE  \
        __asm jmp		[k##name##CancelCommonDialogNewFileRetnAddr]  \
    __asm RESTORE:  \
        __asm pushad  \
        __asm call		IsControlKeyDown  \
        __asm test		eax, eax  \
        __asm jnz		REMOVE  \
        __asm popad  \
        __asm jmp		[k##name##CancelCommonDialogRestoreRetnAddr]  \
    __asm REMOVE:  \
        __asm popad  \
        __asm jmp		[k##name##CancelCommonDialogRemoveRetnAddr]      \
    }  \
}
#define COMMON_DIALOG_BSA_HOOK(name)  \
void __declspec(naked) ##name##BSACommonDialogHook(void)  \
{  \
    {  \
        __asm mov		g_BSAReturnPath, 0  \
        __asm pushad  \
        __asm call		IsControlKeyDown  \
        __asm test		eax, eax  \
        __asm jnz		BSA  \
        __asm popad  \
		__asm call		[k##name##BSACommonDialogCallAddr]  \
        __asm jmp		[k##name##BSACommonDialogRetnAddr]  \
    __asm BSA:  \
        __asm popad  \
		__asm push		k##name##BSACommonDialogFilterType	\
		__asm call		ShowBSAViewer \
        __asm jmp		[k##name##BSACommonDialogRetnAddr]      \
    }  \
}

#define COMMON_DIALOG_CANCEL_PATCH(name)						WriteRelJump(k##name##CancelCommonDialogHookAddr, (UInt32)##name##CancelCommonDialogHook);
#define COMMON_DIALOG_BSA_PATCH(name)							WriteRelJump(k##name##BSACommonDialogHookAddr, (UInt32)##name##BSACommonDialogHook);
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

void __stdcall DoNPCFaceGenHook(HWND Dialog);
void NPCFaceGenHook(void);
// removes the Missing Master warning while saving plugins -- TODO: issues need fixing
const UInt32			kSavePluginWarnMissingMastersPatchAddr = 0x0047ED01;
// adds various hooks to allow the use of a custom workspace (LocalMasterPath) while loading masters from the default Data\ directory
const UInt32			kDataHandlerPopulateModListInitCleanupHookAddr = 0x0047E539;
const UInt32			kDataHandlerPopulateModListInitCleanupPassRetnAddr = 0x0047E53F;
const UInt32			kDataHandlerPopulateModListInitCleanupFailRetnAddr = 0x0047E54D;

void DataHandlerPopulateModListInitCleanupHook(void);

const UInt32			kDataHandlerPopulateModListFindLoopHookAddr = 0x0047E70F;
const UInt32			kDataHandlerPopulateModListFindLoopPassRetnAddr = 0x0047E570;
const UInt32			kDataHandlerPopulateModListFindLoopFailRetnAddr = 0x0047E715;

void DataHandlerPopulateModListFindLoopHook(void);

const UInt32			kDataDlgCallPopulateModListHookAddr = 0x0040B41B;
const UInt32			kDataDlgCallPopulateModListRetnAddr = 0x0040B421;
const UInt32			kDataDlgCallPopulateModListCallAddr = 0x0047E4C0;	// f_DataHandler::PopulateModList(DataHandler* this, LPSTR localMasterPath)

void __stdcall DoDataDlgCallPopulateModListHook(INISetting* Path);
void DataDlgCallPopulateModListHook(void);

const UInt32			kDataHandlerPopulateModListQuickExitHookAddr = 0x0047E4F5;
const UInt32			kDataHandlerPopulateModListQuickExitPassRetnAddr = 0x0047E4FB;
const UInt32			kDataHandlerPopulateModListQuickExitFailRetnAddr = 0x0047E980;

void DataHandlerPopulateModListQuickExitHook(void);

const UInt32			kTESCtorSkipModListPopulationHookAddr = 0x00475CCE;
const UInt32			kTESCtorSkipModListPopulationRetnAddr = 0x00475CD3;
const UInt32			kTESCtorSkipModListPopulationCallAddr = 0x0047E4C0;

void TESCtorSkipModListPopulationHook(void);
// gets rid of the ugly pink default water texture
const UInt32			kDefaultWaterTextureFixPatchAddr = 0x0047F792;

