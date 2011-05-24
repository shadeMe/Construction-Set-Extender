#include "AssetSelector.h"

namespace Hooks
{
	const char* g_AssetSelectorReturnPath = NULL;

	void PatchAssetSelectorHooks(void)
	{
		PatchCommonDialogCancelHandler(Model);
		PatchCommonDialogCancelHandler(Animation);
		PatchCommonDialogCancelHandler(Sound);
		PatchCommonDialogCancelHandler(Texture);
		PatchCommonDialogCancelHandler(SPT);

		PatchCommonDialogPrologHandler(Model);
		PatchCommonDialogPrologHandler(Animation);
		PatchCommonDialogPrologHandler(Sound);
		PatchCommonDialogPrologHandler(Texture);
		PatchCommonDialogPrologHandler(SPT);

		PatchCommonDialogEpilogHandler(Model);
		PatchCommonDialogEpilogHandler(Animation);
		PatchCommonDialogEpilogHandler(Sound);
		PatchCommonDialogEpilogHandler(Texture);
		PatchCommonDialogEpilogHandler(SPT);
	}

	DefineCommonDialogCancelHandler(Model)
	DefineCommonDialogCancelHandler(Animation)
	DefineCommonDialogCancelHandler(Sound)
	DefineCommonDialogCancelHandler(Texture)
	DefineCommonDialogCancelHandler(SPT)

	UInt32 __stdcall InitBSAViewer(UInt32 Filter)
	{
		switch (Filter)
		{
		case e_NIF:
			g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "nif");
			break;
		case e_KF:
			g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "kf");
			break;
		case e_WAV:
			g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "wav");
			break;
		case e_DDS:
			g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "dds");
			break;
		case e_SPT:
			g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "spt");
			break;
		}

		if (!g_AssetSelectorReturnPath)
			return 0;
		else
			return e_FetchPath;
	}

	UInt32 __stdcall InitPathEditor(int ID, const char* ExistingPath, HWND Dialog)
	{
		if (!ExistingPath)
			GetDlgItemText(Dialog, ID, g_TextBuffer, sizeof(g_TextBuffer));
		else
			PrintToBuffer("%s", ExistingPath);

		g_AssetSelectorReturnPath = (const char*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TEXTEDIT), Dialog, (DLGPROC)TextEditDlgProc, (LPARAM)g_TextBuffer);

		if (!g_AssetSelectorReturnPath)
			return 0;
		else
			return e_FetchPath;
	}

	UInt32 __stdcall InitPathCopier(UInt32 Filter, HWND Dialog)
	{
		g_AssetSelectorReturnPath = (const char*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_COPYPATH), Dialog, (DLGPROC)CopyPathDlgProc, (LPARAM)Filter);
		if (!g_AssetSelectorReturnPath)
			return 0;
		else
			return e_FetchPath;
	}

	UInt32 __stdcall InitAssetSelectorDlg(HWND Dialog)
	{
		return DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_ASSETSEL), Dialog, (DLGPROC)AssetSelectorDlgProc);
	}

	DefineCommonDialogPrologHandler(Model)
	DefineCommonDialogPrologHandler(Animation)
	DefineCommonDialogPrologHandler(Sound)
	DefineCommonDialogPrologHandler(Texture)
	DefineCommonDialogPrologHandler(SPT)

	void __declspec(naked) ModelPostCommonDialogHook(void)
	{
		__asm
		{
			cmp		eax, e_FetchPath
			jz		SELECT

			lea		eax, [esp + 0x14]
			jmp     [kModelPostCommonDialogRetnAddr]
		SELECT:
			mov		eax, g_AssetSelectorReturnPath
			jmp     [kModelPostCommonDialogRetnAddr]
		}
	}
	void __declspec(naked) AnimationPostCommonDialogHook(void)
	{
		__asm
		{
			mov		ebx, eax

			mov     eax, [esi + 0x24]
			push    ebx
			push    eax
			lea     ecx, [ebp - 0x14]
			mov     byte ptr [ebp - 0x4], 1
			call    kBSStringT_Set

			cmp		ebx, e_FetchPath
			jz		SELECT

			lea		edx, [ebp]
			jmp		POST
		SELECT:
			mov		edx, g_AssetSelectorReturnPath
		POST:
			push	edx
			lea		ecx, [esp + 0x24]
			jmp     [kAnimationPostCommonDialogRetnAddr]
		}
	}
	void __declspec(naked) SoundPostCommonDialogHook(void)
	{
		__asm
		{
			cmp		eax, e_FetchPath
			jz		SELECT

			lea		ecx, [esp + 8]
			push	ecx
			jmp     [kSoundPostCommonDialogRetnAddr]
		SELECT:
			mov		ecx, g_AssetSelectorReturnPath
			push	ecx
			jmp     [kSoundPostCommonDialogRetnAddr]
		}
	}
	void __declspec(naked) TexturePostCommonDialogHook(void)
	{
		__asm
		{
			cmp		eax, e_FetchPath
			jz		SELECT

			lea		eax, [ebp]
			jmp		POST
		SELECT:
			mov		eax, g_AssetSelectorReturnPath
		POST:
			push	eax
			lea		ecx, [ebp - 0x14]
			jmp     [kTexturePostCommonDialogRetnAddr]
		}
	}
	void __declspec(naked) SPTPostCommonDialogHook(void)
	{
		__asm
		{
			cmp		eax, e_FetchPath
			jz		SELECT

			lea		ecx, [esp + 0x14]
			jmp     [kSPTPostCommonDialogRetnAddr]
		SELECT:
			mov		ecx, g_AssetSelectorReturnPath
			jmp     [kSPTPostCommonDialogRetnAddr]
		}
	}
}