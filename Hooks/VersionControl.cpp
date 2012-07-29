#include "VersionControl.h"
#include "TESFile.h"
#include "..\ChangeLogManager.h"

#pragma warning(push)
#pragma optimize("", off)
#pragma warning(disable: 4005 4748)

namespace ConstructionSetExtender
{
	namespace Hooks
	{
		bool g_PluginLoadOperationInProgress = false;

		_DefineHookHdlr(DataHandlerSavePluginProlog, 0x0047EC8D);
		_DefineHookHdlr(DataHandlerLoadPluginsWrapper, 0x0041BD98);
		_DefineHookHdlr(TESFormSetFromActivePlugin, 0x00497BE0);
		_DefineHookHdlr(TESFormSetDeleted, 0x00498896);
		_DefineHookHdlr(TESFormSetFormID, 0x00497F63);
		_DefineHookHdlr(TESFormSetEditorID, 0x004977E0);
		_DefineHookHdlr(TESDialogFormEditNewForm, 0x00447B7A);
		_DefineHookHdlr(ObjectWindowDragDropNewForm, 0x00421CF9);
		_DefineHookHdlr(TESDialogFormIDListViewNewForm, 0x00449417);
		_DefineHookHdlr(TESDialogCreateNewForm, 0x00413E43);
		_DefineHookHdlr(TESFormClone, 0x0049803D);
		_DefineHookHdlr(TESObjectCELLDuplicate, 0x0047B3F0);
		_DefineHookHdlr(TESObjectREFRDuplicate, 0x005474C5);
		_DefineHookHdlr(TESTopicCreate, 0x004F1805);
		_DefineHookHdlr(TESTopicInfoCreate, 0x004F07E9);
		_DefineHookHdlr(TESIdleFormCreateSibling, 0x004D4093);
		_DefineHookHdlr(TESIdleFormCreateChild, 0x004D426B);
		_DefineHookHdlr(TESDialogFormEditCopyForm, 0x00447B21);

		void PatchVersionControlHooks(void)
		{
			_MemHdlr(DataHandlerLoadPluginsWrapper).WriteJump();
			_MemHdlr(DataHandlerSavePluginProlog).WriteJump();
			_MemHdlr(TESFormSetFromActivePlugin).WriteJump();
			_MemHdlr(TESFormSetDeleted).WriteJump();
			_MemHdlr(TESFormSetFormID).WriteJump();
			_MemHdlr(TESFormSetEditorID).WriteJump();
			_MemHdlr(TESDialogFormEditNewForm).WriteJump();
			_MemHdlr(ObjectWindowDragDropNewForm).WriteJump();
			_MemHdlr(TESDialogFormIDListViewNewForm).WriteJump();
			_MemHdlr(TESDialogCreateNewForm).WriteJump();
			_MemHdlr(TESFormClone).WriteJump();
			_MemHdlr(TESObjectCELLDuplicate).WriteJump();
			_MemHdlr(TESObjectREFRDuplicate).WriteJump();
			_MemHdlr(TESTopicCreate).WriteJump();
			_MemHdlr(TESTopicInfoCreate).WriteJump();
			_MemHdlr(TESIdleFormCreateSibling).WriteJump();
			_MemHdlr(TESIdleFormCreateChild).WriteJump();
			_MemHdlr(TESDialogFormEditCopyForm).WriteJump();
		}

		void __stdcall DoDataHandlerLoadPluginsWrapperHook()
		{
			VersionControl::HandlePluginLoad();
		}

		#define _hhName		DataHandlerLoadPluginsWrapper
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0041BD9D);
			_hhSetVar(Call, 0x00484D00);
			__asm
			{
				mov		g_PluginLoadOperationInProgress, 1
				call	_hhGetVar(Call)
				mov		g_PluginLoadOperationInProgress, 0

				pushad
				call	DoDataHandlerLoadPluginsWrapperHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoDataHandlerSavePluginPrologHook(TESFile* SaveFile)
		{
			VersionControl::HandlePluginSave(SaveFile);
			g_LoadingSavingPlugins = true;
		}

		#define _hhName		DataHandlerSavePluginProlog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0047EC92);
			__asm
			{
				pushad
				push	ebx
				call	DoDataHandlerSavePluginPrologHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESFormChangeHook(TESForm* Form, UInt8 ChangeType, UInt32 Value)
		{
			if ((Form->formFlags & TESForm::kFormFlags_Temporary) == 0 && g_PluginLoadOperationInProgress == false)
			{
				VersionControl::CHANGELOG->RecordFormChange(Form, ChangeType, Value);
			}
		}

		#define _hhName		TESFormSetFromActivePlugin
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00497BE6);
			__asm
			{
				mov		eax, [esp]
				cmp		eax, 0x00497460			// don't log if called from inside TESForm::UpdateUsageInfo
				jz		EXIT

				movzx	eax, byte ptr [esp + 0x4]
				pushad
				push	eax
				push	0		// VersionControl::ChangeLogManager::kFormChange_SetActive
				push	ecx
				call	DoTESFormChangeHook
				popad
			EXIT:
				cmp     byte ptr [esp + 0x4], 0
				push    esi

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESFormSetDeleted
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0049889C);
			_hhSetVar(Jump, 0x004988B1);
			__asm
			{
				movzx	eax, bl
				pushad
				push	eax
				push	1		// VersionControl::ChangeLogManager::kFormChange_SetDeleted
				push	esi
				call	DoTESFormChangeHook
				popad

				test	bl, bl
				jz		REMOVE

				mov     edx, [esi]
				jmp		_hhGetVar(Retn)
			REMOVE:
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		TESFormSetFormID
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00497F68);
			_hhSetVar(Call, 0x00590360);
			__asm
			{
				pushad
				push	edi
				push	2		// VersionControl::ChangeLogManager::kFormChange_SetFormID
				push	esi
				call	DoTESFormChangeHook
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESFormSetEditorID
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004977E5);
			_hhSetVar(Call, 0x004051E0);
			__asm
			{
				pushad
				push	esi
				push	3		// VersionControl::ChangeLogManager::kFormChange_SetEditorID
				push	edi
				call	DoTESFormChangeHook
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoDataHandlerCreateFormHook(TESForm* Form)
		{
			VersionControl::CHANGELOG->RecordChange("%s\t[%08X]\t%s\tInstantiated",  Form->GetTypeIDString(), Form->formID, Form->editorID.c_str());
		}

		#define _hhName		TESDialogFormEditNewForm
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00447B7F);
			_hhSetVar(Call, 0x004793F0);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		ObjectWindowDragDropNewForm
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00421CFE);
			_hhSetVar(Call, 0x004793F0);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESDialogFormIDListViewNewForm
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0044941C);
			_hhSetVar(Call, 0x004793F0);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESDialogCreateNewForm
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00413E48);
			_hhSetVar(Call, 0x004793F0);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESFormClone
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00498042);
			_hhSetVar(Call, 0x004793F0);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESObjectCELLDuplicate
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0047B3F5);
			_hhSetVar(Call, 0x0053E630);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESObjectREFRDuplicate
		_hhBegin()
		{
			_hhSetVar(Retn, 0x005474CA);
			_hhSetVar(Call, 0x00541870);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESTopicCreate
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004F180A);
			_hhSetVar(Call, 0x004F12A0);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESTopicInfoCreate
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004F07EE);
			_hhSetVar(Call, 0x004F6300);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESIdleFormCreateSibling
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004D4098);
			_hhSetVar(Call, 0x004D25D0);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		TESIdleFormCreateChild
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004D4270);
			_hhSetVar(Call, 0x004D25D0);
			__asm
			{
				call	_hhGetVar(Call)
				pushad
				push	eax
				call	DoDataHandlerCreateFormHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESFormCopyHook(TESForm* To, TESForm* From)
		{
	//		VersionControl::CHANGELOG->RecordChange(To, From);
		}

		#define _hhName		TESDialogFormEditCopyForm
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00447B27);
			__asm
			{
				pushad
				push	ebx
				push	esi
				call	DoTESFormCopyHook
				popad

				mov     eax, [edx + 0xB8]
				jmp		_hhGetVar(Retn)
			}
		}
	}
}

#pragma warning(pop)
#pragma optimize("", on)