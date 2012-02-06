#include "AssetSelector.h"
#include "WorkspaceManager.h"

#pragma warning(push)
#pragma optimize("", off)
#pragma warning(disable: 4005 4748)

namespace Hooks
{
	static char s_AssetSelectorReturnPath[0x200] = {0};

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
		char Buffer[0x200] = {0};

		switch (Filter)
		{
		case e_NIF:
			CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(g_APPPath.c_str(), "nif", Buffer, sizeof(Buffer));
			break;
		case e_KF:
			CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(g_APPPath.c_str(), "kf", Buffer, sizeof(Buffer));
			break;
		case e_WAV:
			CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(g_APPPath.c_str(), "wav", Buffer, sizeof(Buffer));
			break;
		case e_DDS:
			CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(g_APPPath.c_str(), "dds", Buffer, sizeof(Buffer));
			break;
		case e_SPT:
			CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(g_APPPath.c_str(), "spt", Buffer, sizeof(Buffer));
			break;
		}

		if (!strlen(Buffer))
			return 0;
		else
		{
			FORMAT_STR(s_AssetSelectorReturnPath, "%s", Buffer);
			return e_FetchPath;
		}
	}

	UInt32 __stdcall InitPathEditor(UInt32 Filter, int ID, const char* ExistingPath, HWND Dialog)
	{
		InitDialogMessageParamT<UInt32> PathEditorParam = { {0}, 0 };

		if (!ExistingPath)
			GetDlgItemText(Dialog, ID, PathEditorParam.Buffer, sizeof(PathEditorParam.Buffer));
		else
			FORMAT_STR(PathEditorParam.Buffer, "%s", ExistingPath);

		if (!DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TEXTEDIT), Dialog, (DLGPROC)TextEditDlgProc, (LPARAM)&PathEditorParam) ||
			strlen(PathEditorParam.Buffer) < 2)
		{
			return 0;
		}
		else
		{
			switch (Filter)
			{
			case e_SPT:
				FORMAT_STR(s_AssetSelectorReturnPath, "\\%s", PathEditorParam.Buffer);
				break;
			case e_KF:
				{
					std::string STLBuffer(PathEditorParam.Buffer);
					int Offset = STLBuffer.find("IdleAnims\\");
					if (Offset != -1)
						STLBuffer = STLBuffer.substr(Offset + 9);

					FORMAT_STR(s_AssetSelectorReturnPath, "\\%s", STLBuffer.c_str());
				}
				break;
			default:
				FORMAT_STR(s_AssetSelectorReturnPath, "%s", PathEditorParam.Buffer);
			}

			return e_FetchPath;
		}
	}

	UInt32 __stdcall InitPathCopier(UInt32 Filter, HWND Dialog)
	{
		InitDialogMessageParamT<UInt32> PathCopierParam = { {0}, Filter };

		if (!DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_COPYPATH), Dialog, (DLGPROC)CopyPathDlgProc, (LPARAM)&PathCopierParam) ||
			!strlen(PathCopierParam.Buffer))
		{
			return 0;
		}
		else
		{
			FORMAT_STR(s_AssetSelectorReturnPath, "%s", PathCopierParam.Buffer);
			return e_FetchPath;
		}
	}

	UInt32 __stdcall InitAssetSelectorDlg(HWND Dialog)
	{
		return DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_ASSETSEL), Dialog, (DLGPROC)AssetSelectorDlgProc);
	}

	void __stdcall InitAssetExtractor(UInt32 Filter, UInt32 PathID, const char* DefaultLookupDir, HWND Dialog)
	{
		char FullPathBuffer[MAX_PATH] = {0}, RelativePathBuffer[MAX_PATH] = {0};

		GetDlgItemText(Dialog, PathID, RelativePathBuffer, sizeof(RelativePathBuffer));
		FORMAT_STR(FullPathBuffer, "%s%s", DefaultLookupDir, RelativePathBuffer);

		if (GetFileAttributes(FullPathBuffer) != INVALID_FILE_ATTRIBUTES)
		{
			if (MessageBox(Dialog,
							"A file of the same name already exists in the Data directory. This operation will overwrite it.\n\nContinue?",
							"CSE", MB_ICONEXCLAMATION|MB_YESNO) == IDNO)
			{
				return;
			}
		}

		std::string DirPath(g_WorkspaceManager.GetWorkingDirectory()); DirPath += FullPathBuffer;
		if (DirPath.rfind("\\") != -1)
			DirPath = DirPath.substr(0, DirPath.rfind("\\") + 1);

		if (ArchiveManager::ExtractArchiveFile(FullPathBuffer, "tempaf") &&
			(SHCreateDirectoryEx(NULL, DirPath.c_str(), NULL) == ERROR_SUCCESS || GetLastError() == ERROR_FILE_EXISTS || GetLastError() == ERROR_ALREADY_EXISTS) &&
			CopyFile("tempaf", FullPathBuffer, FALSE))
		{
			DebugPrint("Extracted archived file '%s'", FullPathBuffer);
			if (MessageBox(Dialog,
						"Successfully extracted archived file! Would you like to open it ?",
						"CSE", MB_ICONINFORMATION|MB_YESNO) == IDNO)
			{
				return;
			}

			ShellExecute(NULL, "open", (LPSTR)FullPathBuffer, NULL, NULL, SW_SHOW);
		}
		else
		{
			MessageBox(Dialog,
						"Couldn't extract/copy file. Possible reasons: File not found in loaded archives, couldn't create intermediate path, insufficient permissions or an internal error.",
						"CSE", MB_ICONERROR|MB_OK);
			DebugPrint("Couldn't extract/copy archived file '%s'", FullPathBuffer);
			LogWinAPIErrorMessage(GetLastError());
		}
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
			lea		eax, s_AssetSelectorReturnPath
			jmp     [kModelPostCommonDialogRetnAddr]
		}
	}
	void __declspec(naked) AnimationPostCommonDialogHook(void)
	{
		_hhSetVar(Call, 0x004051E0);
		__asm
		{
			mov		ebx, eax

			mov     eax, [esi + 0x24]
			push    ebx
			push    eax
			lea     ecx, [ebp - 0x14]
			mov     byte ptr [ebp - 0x4], 1
			call    [_hhGetVar(Call)]

			cmp		ebx, e_FetchPath
			jz		SELECT

			lea		edx, [ebp]
			jmp		POST
		SELECT:
			lea		edx, s_AssetSelectorReturnPath
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
			lea		ecx, s_AssetSelectorReturnPath
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
			lea		eax, s_AssetSelectorReturnPath
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
			lea		ecx, s_AssetSelectorReturnPath
			jmp     [kSPTPostCommonDialogRetnAddr]
		}
	}
}

#pragma warning(pop)
#pragma optimize("", on)
