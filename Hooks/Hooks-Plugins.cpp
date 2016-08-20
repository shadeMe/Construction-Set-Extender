#include "Hooks-Plugins.h"
#include "Hooks-Misc.h"
#include "Hooks-Dialog.h"
#include "WorkspaceManager.h"
#include "CustomDialogProcs.h"

#pragma warning(push)
/*#pragma optimize("", off)*/
#pragma warning(disable: 4005 4748)

namespace cse
{
	namespace hooks
	{
		_DefineHookHdlr(SavePluginMasterEnum, 0x0047ECC6);
		_DefineNopHdlr(CheckIsActivePluginAnESM, 0x0040B65E, 2);
		_DefineNopHdlr(TESFormGetUnUsedFormID, 0x00486C08, 2);
		_DefineHookHdlr(LoadPluginsProlog, 0x00485252);
		_DefineHookHdlr(LoadPluginsEpilog, 0x004856B2);
		_DefineHookHdlr(PostPluginLoad, 0x0041BEFA);
		_DefinePatchHdlr(DataDialogPluginDescription, 0x0040CAB6);
		_DefinePatchHdlr(DataDialogPluginAuthor, 0x0040CAFE);
		_DefineHookHdlr(SavePluginCommonDialog, 0x00446D51);
		_DefineJumpHdlr(MissingMasterOverride, 0x00484FC9, 0x00484E8E);
		_DefinePatchHdlr(DataHandlerPostError, 0x004852F0);
		_DefineHookHdlrWithBuffer(AutoLoadActivePluginOnStartup, 0x0041A26A, 6, 0x8B, 0x0D, 0x44, 0xB6, 0xA0, 0x0);
		_DefineHookHdlr(DataHandlerSaveFormToFile, 0x00479181);
		_DefineHookHdlr(TESFileUpdateHeader, 0x004894D0);
		_DefineHookHdlr(DataHandlerSavePluginEpilog, 0x0047F136);
		_DefineHookHdlr(TESFileUpdateHeaderFlagBit, 0x00489570);
		_DefineHookHdlr(TESObjectCELLSaveReferencesProlog, 0x00538860);
		_DefineHookHdlr(TESObjectCELLSaveReferencesEpilog, 0x005389DB);
		_DefineHookHdlr(DataHandlerSavePluginResetA, 0x0047EBBF);
		_DefineHookHdlr(DataHandlerSavePluginResetB, 0x0047EC09);
		_DefineHookHdlr(DataHandlerSavePluginResetC, 0x0047EC83);
		_DefineNopHdlr(DataHandlerSavePluginOverwriteESM, 0x0047EB6F, 2);
		_DefineHookHdlr(DataHandlerSavePluginRetainTimeStamps, 0x0041BB12);
		_DefineHookHdlr(DataDialogUpdateCurrentTESFileHeaderA, 0x0040CE60);
		_DefineHookHdlr(DataDialogUpdateCurrentTESFileHeaderB, 0x0040D0BF);
		_DefineHookHdlr(DataHandlerAutoSaveA, 0x00481F81);
		_DefineHookHdlr(DataHandlerAutoSaveB, 0x00481FC1);
		_DefineHookHdlr(DataDlgCancelled, 0x0041A289);

		void PatchTESFileHooks(void)
		{
			_MemHdlr(LoadPluginsProlog).WriteJump();
			_MemHdlr(LoadPluginsEpilog).WriteJump();
			_MemHdlr(PostPluginLoad).WriteJump();
			_MemHdlr(SavePluginCommonDialog).WriteJump();
			_MemHdlr(SavePluginMasterEnum).WriteJump();
			_MemHdlr(MissingMasterOverride).WriteJump();
			_MemHdlr(DataHandlerPostError).WriteUInt8(0xEB);
			_MemHdlr(DataDialogPluginDescription).WriteUInt8(0xEB);
			_MemHdlr(DataDialogPluginAuthor).WriteUInt8(0xEB);
			_MemHdlr(CheckIsActivePluginAnESM).WriteNop();
			_MemHdlr(TESFormGetUnUsedFormID).WriteNop();
			_MemHdlr(DataHandlerSaveFormToFile).WriteJump();
			_MemHdlr(TESFileUpdateHeader).WriteJump();
			_MemHdlr(DataHandlerSavePluginEpilog).WriteJump();
			_MemHdlr(TESFileUpdateHeaderFlagBit).WriteJump();
			_MemHdlr(TESObjectCELLSaveReferencesProlog).WriteJump();
			_MemHdlr(TESObjectCELLSaveReferencesEpilog).WriteJump();
			_MemHdlr(DataHandlerSavePluginResetA).WriteJump();
			_MemHdlr(DataHandlerSavePluginResetB).WriteJump();
			_MemHdlr(DataHandlerSavePluginResetC).WriteJump();
			_MemHdlr(DataHandlerSavePluginOverwriteESM).WriteNop();
			_MemHdlr(DataHandlerSavePluginRetainTimeStamps).WriteJump();
			_MemHdlr(DataHandlerAutoSaveA).WriteJump();
			_MemHdlr(DataHandlerAutoSaveB).WriteJump();
			_MemHdlr(DataDlgCancelled).WriteJump();
			_MemHdlr(DataDialogUpdateCurrentTESFileHeaderA).WriteJump();
			_MemHdlr(DataDialogUpdateCurrentTESFileHeaderB).WriteJump();
		}

		bool __stdcall InitTESFileSaveDlg()
		{
			return BGSEEUI->ModalDialog(BGSEEMAIN->GetExtenderHandle(),
										MAKEINTRESOURCE(IDD_TESFILESAVE),
										BGSEEUI->GetMainWindow(),
										(DLGPROC)uiManager::TESFileSaveDlgProc);
		}

		#define _hhName		SavePluginCommonDialog
		_hhBegin()
		{
			_hhSetVar(ESMRetn, 0x00446D58);
			_hhSetVar(ESPRetn, 0x00446D69);
			_asm
			{
				pushad
				call	InitTESFileSaveDlg
				test	eax, eax
				jnz		ESM

				popad
				jmp		_hhGetVar(ESPRetn)
			ESM:
				popad
				jmp		_hhGetVar(ESMRetn)
			}
		}

		static HWND				s_LoadIdleWindow = nullptr;
		static char				s_NumericIDWarningBuffer[0x10] = {0};

		void __stdcall DoLoadPluginsPrologHook(void)
		{
			TESFile* ActiveFile = _DATAHANDLER->activeFile;
			if (ActiveFile && (ActiveFile->fileFlags & TESFile::kFileFlag_Master))
				SME::MiscGunk::ToggleFlag(&ActiveFile->fileFlags, TESFile::kFileFlag_Master, 0);

			s_LoadIdleWindow = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), BGSEEUI->GetMainWindow(), nullptr, NULL);
			Static_SetText(GetDlgItem(s_LoadIdleWindow, -1), "Loading Plugins\nPlease Wait");
		}

		#define _hhName		LoadPluginsProlog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00485257);
			_hhSetVar(Call, 0x00431310);
			__asm
			{
				pushad
				call	DoLoadPluginsPrologHook
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoLoadPluginsEpilogHook(void)
		{
			DestroyWindow(s_LoadIdleWindow);
		}

		#define _hhName		LoadPluginsEpilog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004856B7);
			_hhSetVar(Call, 0x0047DA60);
			__asm
			{
				pushad
				call	DoLoadPluginsEpilogHook
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoPostPluginLoadHook(bool State)
		{
			if (State == false)
			{
				if (TESObjectWindow::GetMinimized() == false)
					BGSEEUI->GetInvalidationManager()->Push(*TESObjectWindow::WindowHandle);
			}
			else
			{
				if (TESObjectWindow::GetMinimized() == false)
					BGSEEUI->GetInvalidationManager()->Pop(*TESObjectWindow::WindowHandle);

				SetActiveWindow(*TESCSMain::WindowHandle);				// to make sure none of its child dialogs are hidden behind it
			}
		}

		#define _hhName	PostPluginLoad
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0041BEFF);
			_hhSetVar(Call, 0x00430980);
			__asm
			{
				pushad
				push	0
				call	DoPostPluginLoadHook
				popad

				call	_hhGetVar(Call)

				pushad
				push	1
				call	DoPostPluginLoadHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		bool __stdcall DoSavePluginMasterEnumHook(TESFile* CurrentFile)
		{
			if ((CurrentFile->fileFlags & TESFile::kFileFlag_Loaded) == 0)
				return false;
			else if ((CurrentFile->fileFlags & TESFile::kFileFlag_Master) == 0 && settings::plugins::kSaveLoadedESPsAsMasters.GetData().i == 0)
				return false;
			else
				return true;
		}

		#define _hhName		SavePluginMasterEnum
		_hhBegin()
		{
			_hhSetVar(RetnPass, 0x0047ECCF);
			_hhSetVar(RetnFail, 0x0047ECEB);
			__asm
			{
				pushad
				push	ecx
				call	DoSavePluginMasterEnumHook
				test	al, al
				jz		SKIP

				popad
				jmp		_hhGetVar(RetnPass)
			SKIP:
				popad
				jmp		_hhGetVar(RetnFail)
			}
		}

		#define _hhName		AutoLoadActivePluginOnStartup
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0041A284);
			__asm
			{
				mov		eax, 1
				jmp		_hhGetVar(Retn)
			}
		}

		bool __stdcall DoTESFileUpdateHeaderHook(TESFile* Plugin)
		{
			char Buffer[0x200] = {0};
			FORMAT_STR(Buffer, "%s%s", Plugin->filePath, Plugin->fileName);

			HANDLE TempFile = CreateFile(Buffer, GENERIC_READ|GENERIC_WRITE, NULL, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (TempFile == INVALID_HANDLE_VALUE)
			{
				BGSEECONSOLE_ERROR("Couldn't open TESFile '%s'!", Buffer);
				BGSEEUI->MsgBoxE(nullptr,
								MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND|MB_OK,
								"Couldn't open TESFile '%s' for read/write access.\n\nError logged to the console.", Plugin->fileName);
				return false;
			}
			else
			{
				CloseHandle(TempFile);
				return true;
			}
		}

		#define _hhName		TESFileUpdateHeader
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004894D6);
			_hhSetVar(Exit, 0x00489570);
			__asm
			{
				MOV_LARGEFS0_EAX
				pushad
				xor		eax, eax
				push	ecx
				call	DoTESFileUpdateHeaderHook
				test	al, al
				jz		EXIT

				popad
				jmp		_hhGetVar(Retn)
			EXIT:
				popad
				jmp		_hhGetVar(Exit)
			}
		}

		void __stdcall DoDataHandlerSavePluginEpilogHook(void)
		{
			TESFile* ActiveFile = _DATAHANDLER->activeFile;
			SME::MiscGunk::ToggleFlag(&ActiveFile->fileFlags, TESFile::kFileFlag_Master, 0);
		}

		#define _hhName		DataHandlerSavePluginEpilog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0047F13B);
			_hhSetVar(Call, 0x00431310);
			__asm
			{
				call	_hhGetVar(Call)

				pushad
				call	DoDataHandlerSavePluginEpilogHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESFileUpdateHeaderFlagBitHook(TESFile* Plugin)
		{
			TESFile* ActiveFile = _DATAHANDLER->activeFile;

			if (ActiveFile)
				ActiveFile->SetMaster(false);
		}

		#define _hhName		TESFileUpdateHeaderFlagBit
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0048957B);
			__asm
			{
				MOV_LARGEFS0_ECX
				pushad
				push	esi
				call	DoTESFileUpdateHeaderFlagBitHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		bool __stdcall DoDataHandlerSaveFormToFileHook(TESForm* Form)
		{
			TESFile* OverrideFile = Form->GetOverrideFile(-1);

			if (!OverrideFile || OverrideFile == _DATAHANDLER->activeFile)
				return false;
			else
				return true;
		}

		#define _hhName		DataHandlerSaveFormToFile
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00479187);
			_hhSetVar(Jump, 0x0047919E);
			__asm
			{
				test	byte ptr [esi + 0x8], 1
				jz		FAIL
			EXIT:
				jmp		_hhGetVar(Retn)	// TESForm::SaveForm
			FAIL:
				pushad
				xor		eax, eax
				push	esi
				call	DoDataHandlerSaveFormToFileHook
				test	al, al
				jnz		FIX
				popad

				jmp		_hhGetVar(Jump)	// TESForm::SaveFormRecord
			FIX:
				popad
				jmp		EXIT
			}
		}

		bool __stdcall DoTESObjectCELLSaveReferencesPrologHook(TESObjectREFR* Reference, TESFile* SaveFile)
		{
			TESFile* SourceFile = Reference->GetOverrideFile(0);
			TESFile* ActiveFile = Reference->GetOverrideFile(-1);

			if (SourceFile == ActiveFile && ActiveFile == SaveFile)
				return false;
			else
				return true;
		}

		#define _hhName		TESObjectCELLSaveReferencesProlog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00538869);
			_hhSetVar(Jump, 0x0053886B);
			_hhSetVar(Call, 0x00485B00);
			__asm
			{
				call	_hhGetVar(Call)		// TESFile_GetIsESM
				test	al, al
				jnz		PASS

				mov		eax, [esp + 0x44]		// TESFile* SaveFile
				pushad
				push	eax
				push	esi
				call	DoTESObjectCELLSaveReferencesPrologHook
				test	al, al
				jnz		FIX
				popad

				jmp		_hhGetVar(Retn)
			FIX:
				popad
			PASS:
				jmp		_hhGetVar(Jump)
			}
		}

		bool __stdcall DoTESObjectCELLSaveReferencesEpilogHook(TESObjectREFR* Reference, TESFile* SaveFile)
		{
			if ((Reference->formFlags & TESForm::kFormFlags_Deleted))
			{
				TESFile* SourceFile = Reference->GetOverrideFile(0);
				TESFile* ActiveFile = Reference->GetOverrideFile(-1);

				if ((SourceFile == ActiveFile && ActiveFile == SaveFile) ||
					(SourceFile == nullptr && ActiveFile == nullptr))
				{
					return false;
				}
				else if ((Reference->baseForm->formFlags & TESForm::kFormFlags_Deleted))
				{
					Reference->SaveForm(SaveFile);		// call SaveForm to dump an empty record
					return false;
				}
				else
					return true;
			}
			else
				return true;
		}

		#define _hhName		TESObjectCELLSaveReferencesEpilog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x005389E1);
			_hhSetVar(Call, 0x00494950);
			__asm
			{
				pushad
				push	eax
				push	ecx
				call	DoTESObjectCELLSaveReferencesEpilogHook
				test	al, al
				jz		EXIT
				popad

				push	eax
				call	_hhGetVar(Call)		// TESForm_SaveFormRecord
				jmp		_hhGetVar(Retn)
			EXIT:
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoDataHandlerSavePluginResetHook(void)
		{
			ZeroMemory(*TESCSMain::ActivePluginNameBuffer, 0x104);
		}

		#define _hhName		DataHandlerSavePluginResetA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0047F156);
			__asm
			{
				pushad
				call	DoDataHandlerSavePluginResetHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		DataHandlerSavePluginResetB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0047F156);
			__asm
			{
				pushad
				call	DoDataHandlerSavePluginResetHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		DataHandlerSavePluginResetC
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0047F156);
			__asm
			{
				pushad
				call	DoDataHandlerSavePluginResetHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall PreserveTESFileTimeStamp(TESFile* File, bool State)
		{
			static FILETIME s_SavedTimestamp = {0};

			if (File == nullptr)
				return;
			else if (settings::plugins::kPreventTimeStampChanges.GetData().i == 0)
				return;

			char Buffer[0x200] = {0};
			FORMAT_STR(Buffer, "%s\\%s", File->filePath, File->fileName);	// will only work with files in the current workspace

			HANDLE SaveFile = CreateFile(Buffer, GENERIC_READ|GENERIC_WRITE, NULL, nullptr, OPEN_EXISTING, 0, nullptr);
			if (SaveFile == INVALID_HANDLE_VALUE)
			{
				// check if the file's from the base workspace
				FORMAT_STR(Buffer, "%s\\%s\\%s", BGSEEWORKSPACE->GetDefaultWorkspace(), File->filePath, File->fileName);
				SaveFile = CreateFile(Buffer, GENERIC_READ|GENERIC_WRITE, NULL, nullptr, OPEN_EXISTING, 0, nullptr);

				if (SaveFile == INVALID_HANDLE_VALUE)
					return;
			}

			if (State)
			{
				ZeroMemory(&s_SavedTimestamp, sizeof(FILETIME));
				GetFileTime(SaveFile, nullptr, nullptr, &s_SavedTimestamp);
			}
			else
			{
				if (s_SavedTimestamp.dwHighDateTime || s_SavedTimestamp.dwLowDateTime)
					SetFileTime(SaveFile, nullptr, nullptr, &s_SavedTimestamp);
			}

			CloseHandle(SaveFile);
		}

		bool __stdcall DoDataHandlerSavePluginRetainTimeStampsHook(bool State)
		{
			if (_DATAHANDLER->activeFile == nullptr)
				return false;
			else
				PreserveTESFileTimeStamp(_DATAHANDLER->activeFile, State);

			return true;
		}

		#define	_hhName		DataHandlerSavePluginRetainTimeStamps
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0041BB17);
			_hhSetVar(Call, 0x0047E9B0);
			__asm
			{
				pushad
				push	1		// save
				call	DoDataHandlerSavePluginRetainTimeStampsHook
				test	al, al
				jz		NEWFILE
				popad

				call	_hhGetVar(Call)

				pushad
				push	0		// restore
				call	DoDataHandlerSavePluginRetainTimeStampsHook
				popad

				jmp		_hhGetVar(Retn)
			NEWFILE:
				popad
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __cdecl DoDataDialogUpdateCurrentTESFileHeaderHook(HWND Dialog, TESFile* File)
		{
			PreserveTESFileTimeStamp(File, true);
			cdeclCall<void>(0x0040CC60, Dialog, File);
			PreserveTESFileTimeStamp(File, false);
		}

		#define	_hhName		DataDialogUpdateCurrentTESFileHeaderA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0040CE65);
			__asm
			{
				call	DoDataDialogUpdateCurrentTESFileHeaderHook
				jmp		_hhGetVar(Retn)
			}
		}

		#define	_hhName		DataDialogUpdateCurrentTESFileHeaderB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0040D0C4);
			__asm
			{
				call	DoDataDialogUpdateCurrentTESFileHeaderHook
				jmp		_hhGetVar(Retn)
			}
		}

		#define	_hhName		DataHandlerAutoSaveA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00481F86);
			_hhSetVar(CallActive, 0x00485BB0);
			_hhSetVar(CallLoaded, 0x00485B70);
			__asm
			{
				call	_hhGetVar(CallActive)

				push	0
				mov		ecx, ebp
				call	_hhGetVar(CallLoaded)

				jmp		_hhGetVar(Retn)
			}
		}

		#define	_hhName		DataHandlerAutoSaveB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00481FC6);
			_hhSetVar(CallActive, 0x00485BB0);
			_hhSetVar(CallLoaded, 0x00485B70);
			__asm
			{
				call	_hhGetVar(CallActive)

				push	1
				mov		ecx, ebp
				call	_hhGetVar(CallLoaded)

				jmp		_hhGetVar(Retn)
			}
		}

		#define	_hhName		DataDlgCancelled
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0041A28F);
			_hhSetVar(Jump, 0x0041BF22);
			__asm
			{
				jz		SKIP
				jmp		_hhGetVar(Retn)
			SKIP:
				push	0		// to correct the stack pointer
				jmp		_hhGetVar(Jump)
			}
		}
	}
}

#pragma warning(pop)
/*#pragma optimize("", on)*/