#include "AssetSelector.h"
#include "CSEWorkspaceManager.h"
#include "Construction Set Extender_Resource.h"

#pragma warning(push)
#pragma optimize("", off)
#pragma warning(disable: 4005 4748)

namespace ConstructionSetExtender
{
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
				CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(BGSEEMAIN->GetAPPPath(), "nif", Buffer, sizeof(Buffer));
				break;
			case e_KF:
				CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(BGSEEMAIN->GetAPPPath(), "kf", Buffer, sizeof(Buffer));
				break;
			case e_WAV:
				CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(BGSEEMAIN->GetAPPPath(), "wav", Buffer, sizeof(Buffer));
				break;
			case e_DDS:
				CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(BGSEEMAIN->GetAPPPath(), "dds", Buffer, sizeof(Buffer));
				break;
			case e_SPT:
				CLIWrapper::Interfaces::BSA->ShowBSAViewerDialog(BGSEEMAIN->GetAPPPath(), "spt", Buffer, sizeof(Buffer));
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
			UIManager::InitDialogMessageParamT<UInt32> PathEditorParam = { {0}, 0 };

			if (!ExistingPath)
				GetDlgItemText(Dialog, ID, PathEditorParam.Buffer, sizeof(PathEditorParam.Buffer));
			else
				FORMAT_STR(PathEditorParam.Buffer, "%s", ExistingPath);

			if (DialogBoxParam(BGSEEMAIN->GetExtenderHandle(),
								MAKEINTRESOURCE(IDD_TEXTEDIT),
								Dialog,
								(DLGPROC)UIManager::TextEditDlgProc,
								(LPARAM)&PathEditorParam) == 0 ||
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
			UIManager::InitDialogMessageParamT<UInt32> PathCopierParam = { {0}, Filter };

			if (DialogBoxParam(BGSEEMAIN->GetExtenderHandle(),
								MAKEINTRESOURCE(IDD_COPYPATH),
								Dialog,
								(DLGPROC)UIManager::CopyPathDlgProc,
								(LPARAM)&PathCopierParam) == 0 ||
				strlen(PathCopierParam.Buffer) == 0)
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
			return DialogBox(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_ASSETSELECTOR), Dialog, (DLGPROC)UIManager::AssetSelectorDlgProc);
		}

		void __stdcall InitAssetExtractor(UInt32 Filter, UInt32 PathID, const char* DefaultLookupDir, HWND Dialog)
		{
			char FullPathBuffer[MAX_PATH] = {0}, RelativePathBuffer[MAX_PATH] = {0};

			GetDlgItemText(Dialog, PathID, RelativePathBuffer, sizeof(RelativePathBuffer));
			FORMAT_STR(FullPathBuffer, "%s%s", DefaultLookupDir, RelativePathBuffer);

			if (GetFileAttributes(FullPathBuffer) != INVALID_FILE_ATTRIBUTES)
			{
				if (BGSEEUI->MsgBoxW(Dialog, MB_YESNO, "A file of the same name already exists in the Data directory. This operation will overwrite it.\n\nContinue?") == IDNO)
				{
					return;
				}
			}

			std::string DirPath(BGSEEWORKSPACE->GetCurrentWorkspace());
			DirPath += FullPathBuffer;

			if (DirPath.rfind("\\") != -1)
				DirPath = DirPath.substr(0, DirPath.rfind("\\") + 1);

			if (ArchiveManager::ExtractArchiveFile(FullPathBuffer, "tempaf") &&
				(SHCreateDirectoryEx(NULL, DirPath.c_str(), NULL) == ERROR_SUCCESS || GetLastError() == ERROR_FILE_EXISTS || GetLastError() == ERROR_ALREADY_EXISTS) &&
				CopyFile("tempaf", FullPathBuffer, FALSE))
			{
				BGSEECONSOLE_MESSAGE("Extracted archived file '%s'", FullPathBuffer);
				if (BGSEEUI->MsgBoxI(Dialog, MB_YESNO, "Successfully extracted archived file! Would you like to open it?") == IDNO)
				{
					return;
				}

				ShellExecute(NULL, "open", (LPSTR)FullPathBuffer, NULL, NULL, SW_SHOW);
			}
			else
			{
				BGSEEUI->MsgBoxE(Dialog, 0,
								"Couldn't extract/copy file. Possible reasons: File not found in loaded archives, couldn't create intermediate path, insufficient permissions or an internal error.");

				BGSEECONSOLE_ERROR("Couldn't extract/copy archived file '%s'", FullPathBuffer);
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
}

#pragma warning(pop)
#pragma optimize("", on)
