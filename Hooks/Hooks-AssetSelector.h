#pragma once
#include "Hooks-Common.h"

namespace ConstructionSetExtender
{
	namespace Hooks
	{
		// hooks that add support for the enhanced asset selector
		void PatchAssetSelectorHooks(void);

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
			e_Close				= 0,
			e_FileBrowser,
			e_BSABrowser,
			e_EditPath,
			e_ClearPath,
			e_CopyPath,
			e_ExtractPath,
			e_FetchPath			= 0x32
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
		__asm cmp		eax, e_ExtractPath \
		__asm jz		EXTRACTP \
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
		__asm push		0 \
		__asm mov		eax, [k##name##PathButtonID] \
		__asm push		eax \
		__asm push		k##name##SelectorCommonDialogFilterType	\
		__asm call		InitPathEditor \
		__asm jmp		[k##name##SelectorCommonDialogRetnAddr]      \
		__asm COPYP: \
		__asm popad \
		__asm push		eax \
		__asm push		k##name##SelectorCommonDialogFilterType	\
		__asm call		InitPathCopier \
		__asm jmp		[k##name##SelectorCommonDialogRetnAddr]      \
		__asm EXTRACTP: \
		__asm popad \
		__asm push		eax \
		__asm mov		eax, [esp + 0x8]	\
		__asm push		eax	\
		__asm mov		eax, [k##name##PathButtonID] \
		__asm push		eax \
		__asm push		k##name##SelectorCommonDialogFilterType	\
		__asm call		InitAssetExtractor \
		__asm mov		eax, e_Close \
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

		_DeclareMemHdlr(TESDialogShowNIFFileSelect, "fixes a bug that forced NIF file selections to use the old style common dialog");
	}
}