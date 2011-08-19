#include "Dialog.h"
#include "..\CSDialogs.h"
#include "..\Achievements.h"
#include "..\WindowManager.h"

namespace Hooks
{
	bool		g_QuickLoadToggle = false;
	HANDLE		g_CSESplashImage = NULL;

	_DefineNopHdlr(ResponseEditorMic, 0x00407F3D, 5);
	_DefineHookHdlr(FindTextInit, 0x00419A42);
	_DefineHookHdlr(UseInfoListInit, 0x00419833);
	_DefineJumpHdlr(TopicResultScriptReset, 0x004F49A0, 0x004F49FA);
	_DefineHookHdlr(NPCFaceGen, 0x004D76AC);
	_DefineHookHdlr(DataDlgInit, 0x0040C6D7);
	_DefineHookHdlr(CustomCSWindow, 0x004311E5);
	_DefinePatchHdlr(RaceDescriptionDirtyEdit, 0x0049405C);
	_DefineHookHdlr(AddListViewItem, 0x004038F0);
	_DefineHookHdlr(AddComboBoxItem, 0x00403540);
	_DefineHookHdlr(ObjectListPopulateListViewItems, 0x00413980);
	_DefineHookHdlr(CellViewPopulateObjectList, 0x004087C0);
	_DefineHookHdlr(CellObjectListShadeMeRefAppend, 0x00445128);
	_DefinePatchHdlr(DeathToTheCloseOpenDialogsMessage, 0x0041BAA7);
	_DefineHookHdlr(TESDialogPopupMenu, 0x004435A6);
	_DefineJumpHdlr(ResponseWindowLipButtonPatch, 0x004EC0E7, 0x004EC0F7);
	_DefineHookHdlr(ResponseWindowInit, 0x004EBA81);
	_DefineJumpHdlr(DataDlgZOrder, 0x0040C530, 0x0040C552);
	_DefineHookHdlr(FormIDListViewInit, 0x00448A8A);
	_DefineHookHdlr(FormIDListViewSaveChanges, 0x0044957A);
	_DefineHookHdlr(FormIDListViewItemChange, 0x00448DEC);
	_DefineHookHdlr(FormIDListViewSelectItem, 0x00403B3D);
	_DefineHookHdlr(FormIDListViewDuplicateSelection, 0x004492AE);
	_DefineHookHdlr(TESRaceCopyHairEyeDataInit, 0x004E9735);
	_DefineHookHdlr(TESRaceCopyHairEyeDataMessageHandler, 0x004E8FE1);
	_DefineHookHdlr(TESDialogGetIsWindowDragDropRecipient, 0x004433FF);
	_DefineHookHdlr(AboutDialog, 0x00441CC5);
	_DefineNopHdlr(TESQuestStageResultScript, 0x004E234D, 2);
	_DefineHookHdlr(TESNPCUpdatePreviewControl, 0x0048C598);
	_DefineHookHdlr(TESParametersFillAndInitSelectionComboBoxOwners, 0x0045942F);
	_DefineHookHdlr(SearchReplaceDialog, 0x004448FD);
	_DefineHookHdlr(ObjectWindowPopulateFormListInvalidate, 0x00421EE1);
	_DefineHookHdlr(ObjectWindowResize, 0x004204C2);
	_DefineHookHdlr(ObjectWindowPopulateFormListFilter, 0x00414D7C);
	_DefineHookHdlr(LandscapeTextureUseDlg, 0x004138D9);
	_DefineHookHdlr(BuildResultScriptEditButton, 0x004DED70);
	_DefineHookHdlr(DialogEditorCommandMessageCallback, 0x004F1DCD);
	_DefineHookHdlr(TESQuestCommandMessageCallback, 0x004E286A);
	_DefineHookHdlr(CellViewWindowResizeFix, 0x00409813);
	_DefineHookHdlr(CellViewWindowResizeA, 0x004096DA);
	_DefineHookHdlr(CellViewWindowResizeB, 0x0040972E);
	_DefineHookHdlr(CellViewWindowPopulateObjectListFilter, 0x00409054);
	_DefineHookHdlr(TESSoundPlayFile, 0x005047B0);
	_DefineHookHdlr(FormEditDialogTitle, 0x00447877);
	_DefineHookHdlr(FindTextFormEnumerationA, 0x00444211);
	_DefineHookHdlr(FindTextFormEnumerationB, 0x0044417B);
	_DefineHookHdlr(FindTextFormEnumerationC, 0x00444430);

	void PatchDialogHooks(void)
	{
		_MemHdlr(FindTextInit).WriteJump();
		_MemHdlr(UseInfoListInit).WriteJump();
		_MemHdlr(DataDlgInit).WriteJump();
		_MemHdlr(NPCFaceGen).WriteJump();
		_MemHdlr(CustomCSWindow).WriteJump();
		_MemHdlr(AddListViewItem).WriteJump();
		_MemHdlr(ObjectListPopulateListViewItems).WriteJump();
		_MemHdlr(CellViewPopulateObjectList).WriteJump();
		_MemHdlr(TopicResultScriptReset).WriteJump();
		_MemHdlr(RaceDescriptionDirtyEdit).WriteUInt8(0xEB);
		_MemHdlr(MissingTextureWarning).WriteNop();
		_MemHdlr(ResponseEditorMic).WriteNop();
		_MemHdlr(CellObjectListShadeMeRefAppend).WriteJump();
		_MemHdlr(DeathToTheCloseOpenDialogsMessage).WriteUInt8(0xEB);
		_MemHdlr(TESDialogPopupMenu).WriteJump();
		_MemHdlr(ResponseWindowLipButtonPatch).WriteJump();
		_MemHdlr(ResponseWindowInit).WriteJump();
		_MemHdlr(DataDlgZOrder).WriteJump();
		_MemHdlr(FormIDListViewInit).WriteJump();
		_MemHdlr(FormIDListViewSaveChanges).WriteJump();
		_MemHdlr(FormIDListViewItemChange).WriteJump();
		_MemHdlr(FormIDListViewSelectItem).WriteJump();
		_MemHdlr(FormIDListViewDuplicateSelection).WriteJump();
		_MemHdlr(TESRaceCopyHairEyeDataInit).WriteJump();
		_MemHdlr(TESRaceCopyHairEyeDataMessageHandler).WriteJump();
		_MemHdlr(TESDialogGetIsWindowDragDropRecipient).WriteJump();
		_MemHdlr(AboutDialog).WriteJump();
		_MemHdlr(TESQuestStageResultScript).WriteNop();
		_MemHdlr(TESNPCUpdatePreviewControl).WriteJump();
		_MemHdlr(TESParametersFillAndInitSelectionComboBoxOwners).WriteJump();
		_MemHdlr(SearchReplaceDialog).WriteJump();
		_MemHdlr(ObjectWindowPopulateFormListInvalidate).WriteJump();
		_MemHdlr(ObjectWindowResize).WriteJump();
		_MemHdlr(ObjectWindowPopulateFormListFilter).WriteJump();
		_MemHdlr(LandscapeTextureUseDlg).WriteJump();
		_MemHdlr(BuildResultScriptEditButton).WriteJump();
		_MemHdlr(DialogEditorCommandMessageCallback).WriteJump();
		_MemHdlr(TESQuestCommandMessageCallback).WriteJump();
		_MemHdlr(CellViewWindowResizeFix).WriteJump();
		_MemHdlr(CellViewWindowResizeA).WriteJump();
		_MemHdlr(CellViewWindowResizeB).WriteJump();
		_MemHdlr(CellViewWindowPopulateObjectListFilter).WriteJump();
		_MemHdlr(TESSoundPlayFile).WriteJump();
		_MemHdlr(FormEditDialogTitle).WriteJump();
		_MemHdlr(FindTextFormEnumerationA).WriteJump();
		_MemHdlr(FindTextFormEnumerationB).WriteJump();
		_MemHdlr(FindTextFormEnumerationC).WriteJump();
	}

	void __stdcall DoFindTextInitHook(HWND FindTextDialog)
	{
		g_FindTextOrgWindowProc = (WNDPROC)SetWindowLong(FindTextDialog, GWL_WNDPROC, (LONG)FindTextDlgSubClassProc);
	}

	#define _hhName		FindTextInit
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00419A48);
		__asm
		{
			call	CreateDialogParamAddress
			call	[g_WindowHandleCallAddr]			// CreateDialogParamA

			pushad
			push	eax
			call	DoFindTextInitHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		UseInfoListInit
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00419848);
		_asm	pushad
		CLIWrapper::Interfaces::USE->ShowUseInfoListDialog(NULL);
		__asm
		{
			popad
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoNPCFaceGenHook(HWND Dialog)
	{
		SendMessageA(Dialog, WM_COMMAND, 1014, 0);
	}

	#define _hhName		NPCFaceGen
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004D76B1);
		_hhSetVar(Call, 0x0049C230);
		__asm
		{
			call	[_hhGetVar(Call)]
			push	esi
			call	DoNPCFaceGenHook
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoDataDlgInitHook(HWND DataDialog)
	{
		// create new controls
		RECT DlgRect;
		GetClientRect(DataDialog, &DlgRect);

		HWND QuickLoadCheckBox = CreateWindowEx(0,
												"BUTTON",
												"Quick-Load Plugin",
												BS_AUTOCHECKBOX|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
												DlgRect.right - 141, DlgRect.bottom - 82, 142, 15,
												DataDialog,
												(HMENU)DATA_QUICKLOAD,
												GetModuleHandle(NULL),
												NULL),
			 StartupPluginBtn = CreateWindowEx(0,
												"BUTTON",
												"Set As Startup Plugin",
												WS_CHILD|WS_VISIBLE|WS_TABSTOP,
												DlgRect.right - 141, DlgRect.bottom - 64, 130, 20,
												DataDialog,
												(HMENU)DATA_SETSTARTUPPLUGIN,
												GetModuleHandle(NULL),
												NULL);

		CheckDlgButton(DataDialog, DATA_QUICKLOAD, (!g_QuickLoadToggle ? BST_UNCHECKED : BST_CHECKED));

		SendMessage(QuickLoadCheckBox, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
		SendMessage(StartupPluginBtn, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);

		g_DataDlgOrgWindowProc = (WNDPROC)SetWindowLong(DataDialog, GWL_WNDPROC, (LONG)DataDlgSubClassProc);
		SetWindowPos(DataDialog, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}

	#define _hhName		DataDlgInit
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0040C6DC);
		_hhSetVar(Call, 0x00404A90);
		__asm
		{
			call	[_hhGetVar(Call)]

			pushad
			push	esi
			call	DoDataDlgInitHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	bool __stdcall DoCustomCSWindowPatchHook(HWND Window)
	{
		if (g_CustomMainWindowChildrenDialogs.GetHandleExists(Window))
			return false;
		else
			return true;
	}

	#define _hhName		CustomCSWindow
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004311EF);
		__asm
		{
			mov		edi, [g_HWND_CSParent]
			cmp		eax, edi
			jnz		FAIL
			xor		edi, edi
			pushad
			push	esi
			call	DoCustomCSWindowPatchHook
			test	eax, eax
			jz		FAIL
			popad
			mov		edi, esi
		FAIL:
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		AddListViewItem
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004038F7);
		_hhSetVar(Exit, 0x0040396E);
		__asm
		{
			mov		eax, [esp]
			sub		eax, 5
			pushad
			push	eax
			call	FormEnumerationWrapper::PerformListViewPrologCheck
			test	al, al
			jz		SKIP
			popad

			mov		eax, [esp + 8]
			pushad
			push	eax
			call	FormEnumerationWrapper::GetShouldEnumerateForm
			test	al, al
			jz		EXIT
		SKIP:
			popad

			mov		ecx, [esp + 0x10]
			or		edx, 0x0FFFFFFFF
			jmp		[_hhGetVar(Retn)]
		EXIT:
			popad
			jmp		[_hhGetVar(Exit)]
		}
	}

	#define _hhName		AddComboBoxItem
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00403548);
		_hhSetVar(Exit, 0x004035F4);
		__asm
		{
			pushad
			push	[esp + 0xC]
			call	FormEnumerationWrapper::GetShouldEnumerateForm
			test	al, al
			jz		EXIT
			popad

			sub		esp, 8
			push	esi
			mov		esi, [esp + 0x10]
			jmp		[_hhGetVar(Retn)]
		EXIT:
			popad
			jmp		[_hhGetVar(Exit)]
		}
	}

	#define _hhName		ObjectListPopulateListViewItems
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0041398A);
		_hhSetVar(Exit, 0x00413A50);
		__asm
		{
			mov		eax, [esp + 8]
			pushad
			push	eax
			call	FormEnumerationWrapper::GetShouldEnumerateForm
			test	al, al
			jz		EXIT2
			popad

			sub		esp, 0x28
			mov		eax, [0x00A0AF40]		// object window menu item checked state
			cmp		eax, 0

			jmp		[_hhGetVar(Retn)]
		EXIT2:
			popad
			jmp		[_hhGetVar(Exit)]
		}
	}

	#define _hhName		CellViewPopulateObjectList
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004087D3);
		_hhSetVar(Exit, 0x004088AF);
		__asm
		{
			mov		eax, [esp + 8]

			sub		esp, 0x28
			push	esi
			mov		esi, eax
			push	edi
			xor		edi, edi
			cmp		esi, edi

			jz		EXIT2

			pushad
			push	eax
			call	FormEnumerationWrapper::GetShouldEnumerateForm
			test	al, al
			jz		EXIT1
			popad

			jmp		[_hhGetVar(Retn)]
		EXIT1:
			popad
		EXIT2:
			jmp		[_hhGetVar(Exit)]
		}
	}

	void __stdcall AppendShadeMeRefToComboBox(HWND hWnd)
	{
		TESForm* Ref = TESForm::LookupByEditorID("TheShadeMeRef");
		sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "'shadeMe' 'TheShadeMeRef'");
		TESComboBox::AddItem(hWnd, g_TextBuffer, Ref, 1);
	}

	#define _hhName		CellObjectListShadeMeRefAppend
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0044512D);
		__asm
		{
			pushad
			push	edx
			call	AppendShadeMeRefToComboBox
			popad

			call	TESComboBox::AddItem
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall InsertFormListPopupMenuItems(HMENU Menu, TESForm* SelectedForm)
	{
		InsertMenu(Menu, -1, MF_BYPOSITION|MF_SEPARATOR, NULL, NULL);
		InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_SETFORMID, "Set FormID");
		InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_MARKUNMODIFIED, "Mark As Unmodified");
		InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_UNDELETE, "Undelete");
		InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_JUMPTOUSEINFOLIST, "Jump To Central Use Info List");
		InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_SHOWOVERRIDES, "Show Override List");

		if (GetFormDialogTemplate(SelectedForm->formType) == 1 && SelectedForm->IsReference() == 0)
		{
			InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_ADDTOTAG, "Add to Active Tag");
		}

		InsertMenu(Menu, -1, MF_BYPOSITION|MF_SEPARATOR, NULL, NULL);
		if (SelectedForm->IsReference())
		{
			InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_EDITBASEFORM, "Edit Base Form");
			InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_TOGGLEVISIBILITY, "Toggle Visibility");
			InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_TOGGLECHILDRENVISIBILITY, "Toggle Children Visibility");
		}
	}

	void __stdcall HandleHookedPopup(HWND Parent, int MenuIdentifier, TESForm* SelectedObject)
	{
		switch (MenuIdentifier)
		{
		case POPUP_SETFORMID:
		case POPUP_MARKUNMODIFIED:
		case POPUP_JUMPTOUSEINFOLIST:
		case POPUP_UNDELETE:
		case POPUP_EDITBASEFORM:
		case POPUP_TOGGLEVISIBILITY:
		case POPUP_TOGGLECHILDRENVISIBILITY:
		case POPUP_ADDTOTAG:
		case POPUP_SHOWOVERRIDES:
			EvaluatePopupMenuItems(Parent, MenuIdentifier, SelectedObject);
			break;
		default:
			SendMessage(Parent, WM_COMMAND, (WPARAM)MenuIdentifier, NULL);
			break;
		}
	}

	void __stdcall RemoveFormListPopupMenuItems(HMENU Menu)
	{
		DeleteMenu(Menu, POPUP_SETFORMID, MF_BYCOMMAND);
		DeleteMenu(Menu, POPUP_MARKUNMODIFIED, MF_BYCOMMAND);
		DeleteMenu(Menu, POPUP_JUMPTOUSEINFOLIST, MF_BYCOMMAND);
		DeleteMenu(Menu, POPUP_UNDELETE, MF_BYCOMMAND);
		DeleteMenu(Menu, POPUP_EDITBASEFORM, MF_BYCOMMAND);
		DeleteMenu(Menu, POPUP_TOGGLEVISIBILITY, MF_BYCOMMAND);
		DeleteMenu(Menu, POPUP_TOGGLECHILDRENVISIBILITY, MF_BYCOMMAND);
		DeleteMenu(Menu, POPUP_ADDTOTAG, MF_BYCOMMAND);
		DeleteMenu(Menu, POPUP_SHOWOVERRIDES, MF_BYCOMMAND);
		DeleteMenu(Menu, GetMenuItemCount(Menu) - 1, MF_BYPOSITION);
		DeleteMenu(Menu, GetMenuItemCount(Menu) - 1, MF_BYPOSITION);
	}

	#define _hhName		TESDialogPopupMenu
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004435C3);
		static HWND ParentHWND = NULL;
		__asm
		{
			pushad
			call	TrackPopupMenuAddress
			popad

			mov		eax, [esp + 0x18]
			mov		ParentHWND, eax
			push	0
			push	eax
			mov		eax, [esp + 0x1C]
			mov		ecx, [eax + 4]
			mov		edx, [eax]
			push	0
			push	ecx
			push	edx

			test	ebx, ebx
			jz		SKIP

			pushad
			push	ebx
			push	esi
			call	InsertFormListPopupMenuItems
			popad

			push	0x102
			push	esi
			call	g_WindowHandleCallAddr

			pushad
			push	esi
			call	RemoveFormListPopupMenuItems
			popad

			push	ebx
			push	eax
			push	ParentHWND
			call	HandleHookedPopup
			jmp		[_hhGetVar(Retn)]
		SKIP:
			push	2
			push	esi
			call	g_WindowHandleCallAddr

			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoResponseWindowInitHook(HWND hWnd)
	{
		g_ResponseWndOrgWindowProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)ResponseWndSubClassProc);

		ShowWindow(GetDlgItem(hWnd, 2220), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, 2221), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, 2222), SW_HIDE);

		EnableWindow(GetDlgItem(hWnd, 2379), FALSE);
		EnableWindow(GetDlgItem(hWnd, 2380), FALSE);
		EnableWindow(GetDlgItem(hWnd, 1016), TRUE);

		CheckRadioButton(hWnd, 2379, 2380, 2379);
		SetWindowText(GetDlgItem(hWnd, 2379), "From MP3");

		SetWindowText(GetDlgItem(hWnd, 2223), "Copy External File");
		SetWindowPos(GetDlgItem(hWnd, 2223), HWND_TOP, 150, 550, 105, 20, SWP_NOZORDER|SWP_SHOWWINDOW);
	}

	#define _hhName		ResponseWindowInit
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004EBA92);
		__asm
		{
			push	ebp
			call	DoResponseWindowInitHook
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoFormIDListViewInitHook(HWND hWnd)
	{
		if (hWnd != *g_HWND_QuestWindow)
		{
			SetWindowText(GetDlgItem(hWnd, 1), "Apply");
			SetWindowText(GetDlgItem(hWnd, 2), "Close");
		}
	}

	#define _hhName		FormIDListViewInit
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00448A94);
		_hhSetVar(Byte, 0x00A0BE45);
		__asm
		{
			pushad
			push	esi
			call	DoFormIDListViewInitHook
			popad

			push	5
			push	esi
			mov		[_hhGetVar(Byte)], 0

			jmp		[_hhGetVar(Retn)]
		}
	}

	UInt32 __stdcall DoFormIDListViewSaveChangesHookProlog(HWND Parent)
	{
		return Parent != *g_HWND_QuestWindow;
	}

	void __stdcall DoFormIDListViewSaveChangesEffectSettingHook(TESForm* Form)
	{
		if (Form->formType == TESForm::kFormType_EffectSetting)
			Achievements::UnlockAchievement(Achievements::kAchievement_Magister);
	}

	void __stdcall DoFormIDListViewSaveChangesHookEpilog(HWND Parent)
	{
		if (IsWindowEnabled(GetDlgItem(Parent, 1)))
		{
			TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(Parent);
			TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(Parent);

			if (WorkingCopy)
			{
				thisVirtualCall<UInt32>(0x118, LocalCopy, Parent);		// GetDataFromDialog
				if (WorkingCopy->CompareTo(LocalCopy))
				{
					if (WorkingCopy->UpdateUsageInfo())
					{
						WorkingCopy->SetFromActiveFile(true);
						WorkingCopy->CopyFrom(LocalCopy);

						DoFormIDListViewSaveChangesEffectSettingHook(WorkingCopy);
					}
				}
			}
		}
	}

	#define _hhName		FormIDListViewSaveChanges
	_hhBegin()
	{
		_hhSetVar(QuestRetn, 0x00449580);
		_hhSetVar(ExitRetn, 0x00448BF0);
		__asm
		{
			push	esi
			call	TESDialog::GetDialogExtraParam

			pushad
			push	esi
			call	DoFormIDListViewSaveChangesHookProlog
			test	eax, eax
			jnz		NOTQUEST
			popad

			jmp		[_hhGetVar(QuestRetn)]
		NOTQUEST:
			popad

			pushad
			push	esi
			call	DoFormIDListViewSaveChangesHookEpilog
			popad

			jmp		[_hhGetVar(ExitRetn)]
		}
	}

	int __stdcall DoFormIDListViewItemChangeHook(HWND Parent)
	{
		return MessageBox(Parent, "Save Changes made to the active form?", "CSE", MB_YESNO);
	}

	#define _hhName		FormIDListViewItemChange
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00448DF4);
		__asm
		{
			pushad
			push	esi
			call	DoFormIDListViewItemChangeHook
			cmp		eax, IDYES
			jnz		REVERT

			push	edi
			call	DoFormIDListViewSaveChangesEffectSettingHook
			popad

			mov		eax, [edi]
			mov		edx, [eax + 0x104]

			jmp		[_hhGetVar(Retn)]
		REVERT:
			popad

			mov		eax, 0x004494C9
			jmp		eax
		}
	}

	void __stdcall DoFormIDListViewSelectItemHook(HWND ListView, int ItemIndex)
	{
		SetFocus(ListView);
		ListView_SetItemState(ListView, ItemIndex, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
	}

	#define _hhName		FormIDListViewSelectItem
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00403B8A);
		__asm
		{
			pushad
			push	ebx
			push	esi
			call	DoFormIDListViewSelectItemHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		FormIDListViewDuplicateSelection
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004492B7);
		__asm
		{
			add		eax, 1
			push	eax
			push	0x810
			push	esi
			call	edi

			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoTESRaceCopyHairEyeDataInitHook(Subwindow* RaceDialogSubWindow, UInt8 Template)
	{
		if (Template == 2)
		{
			HWND Parent = RaceDialogSubWindow->hDialog;

			HWND CopyHairButton = CreateWindowEx(0,
													"BUTTON",
													"Copy Hair From Race",
													WS_CHILD|WS_VISIBLE|WS_TABSTOP,
													RaceDialogSubWindow->position.x + 175 - 42, RaceDialogSubWindow->position.y + 285, 165, 25,
													Parent,
													(HMENU)RACE_COPYHAIR,
													GetModuleHandle(NULL),
													NULL);

			HWND CopyEyesButton = CreateWindowEx(0,
													"BUTTON",
													"Copy Eyes From Race",
													WS_CHILD|WS_VISIBLE|WS_TABSTOP,
													RaceDialogSubWindow->position.x + 175 + 130, RaceDialogSubWindow->position.y + 285, 165, 25,
													Parent,
													(HMENU)RACE_COPYEYES,
													GetModuleHandle(NULL),
													NULL);

			SendMessage(CopyHairButton, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
			SendMessage(CopyEyesButton, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);

			RaceDialogSubWindow->controls.AddAt((HWND*)CopyHairButton, eListEnd);
			RaceDialogSubWindow->controls.AddAt((HWND*)CopyEyesButton, eListEnd);
		}
	}

	#define _hhName		TESRaceCopyHairEyeDataInit
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004E973A);
		_hhSetVar(Call, 0x004E83B0);
		__asm
		{
			call	[_hhGetVar(Call)]

			pushad
			push	ebx
			push	edi
			call	DoTESRaceCopyHairEyeDataInitHook
			popad
			jmp		[_hhGetVar(Retn)]
		}
	}

	UInt32 __stdcall DoTESRaceCopyHairEyeDataMessageHandlerHook(HWND Dialog, INT Identifier, TESRace* WorkingRace)
	{
		switch (Identifier)
		{
		case RACE_COPYEYES:
		case RACE_COPYHAIR:
			TESForm* Selection = (TESForm*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TESCOMBOBOX), Dialog, (DLGPROC)TESComboBoxDlgProc, (LPARAM)TESForm::kFormType_Race);
			if (Selection)
			{
				TESRace* SelectedRace = CS_CAST(Selection, TESForm, TESRace);

				if (WorkingRace)
				{
					int Count = 0;
					if (Identifier == RACE_COPYEYES)
					{
						tList<TESEyes>* Source = &SelectedRace->eyeList;
						tList<TESEyes>* Destination = &WorkingRace->eyeList;

						for (tList<TESEyes>::Iterator Itr = Source->Begin(); !Itr.End() && Itr.Get(); ++Itr)
						{
							if (Destination->IndexOf(Itr.Get()) == -1)
							{
								Destination->AddAt(Itr.Get(), eListEnd);
								Count++;
							}
						}

						sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "Copied %d eye forms from race '%s'.", Count, SelectedRace->editorID.c_str());
						MessageBox(Dialog, g_TextBuffer, "CSE", MB_OK);
					}
					else if (Identifier == RACE_COPYHAIR)
					{
						tList<TESHair>* Source = &SelectedRace->hairList;
						tList<TESHair>* Destination = &WorkingRace->hairList;

						for (tList<TESHair>::Iterator Itr = Source->Begin(); !Itr.End() && Itr.Get(); ++Itr)
						{
							if (Destination->IndexOf(Itr.Get()) == -1)
							{
								Destination->AddAt(Itr.Get(), eListEnd);
								Count++;
							}
						}

						sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "Copied %d hair forms from race '%s'.", Count, SelectedRace->editorID.c_str());
						MessageBox(Dialog, g_TextBuffer, "CSE", MB_OK);
					}
				}
			}
			return 1;
		}

		return 0;
	}

	#define _hhName		TESRaceCopyHairEyeDataMessageHandler
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004E8FF2);
		_hhSetVar(Jump, 0x004E8E6D);
		__asm
		{
			movzx	eax, di
			add		eax, 0x0FFFFF78C
			cmp		eax, 0x24
			ja		DEFAULT

			jmp		[_hhGetVar(Retn)]
		DEFAULT:
			pushad
			push	ebp
			push	edi
			push	esi
			call	DoTESRaceCopyHairEyeDataMessageHandlerHook
			test	eax, eax
			jnz		HANDLED
			popad

			jmp		[_hhGetVar(Jump)]
		HANDLED:
			popad

			xor		al, al
			mov		ecx, 0x004E9D5B
			jmp		ecx
		}
	}

	bool __stdcall DoTESDialogGetIsWindowDragDropRecipientHook(HWND Handle)
	{
		return g_DragDropSupportDialogs.GetHandleExists(Handle);
	}

	#define _hhName		TESDialogGetIsWindowDragDropRecipient
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00443409);
		_hhSetVar(Jump, 0x0044341F);
		__asm
		{
			pushad
			call	SendMessageAddress
			popad

			call	[g_WindowHandleCallAddr]
			test	eax, eax
			jz		FAIL

			jmp		[_hhGetVar(Retn)]
		FAIL:
			pushad
			push	edi
			call	DoTESDialogGetIsWindowDragDropRecipientHook
			test	al, al
			jnz		FIX
			popad

			jmp		[_hhGetVar(Jump)]
		FIX:
			popad
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoAboutDialogHook(HWND Dialog)
	{
		if (g_CSESplashImage == NULL)
			g_CSESplashImage = LoadImage(g_DLLInstance, MAKEINTRESOURCE(BITMAP_SPLASH), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

		HWND PictureControl = GetDlgItem(Dialog, 1963);
		SendMessage(PictureControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_CSESplashImage);
		Static_SetText(GetDlgItem(Dialog, -1), "Elder Scrolls Construction Set IV | Construction Set Extender");

		DWORD FileVersionHandle = 0,
			  FileVersionSize = GetFileVersionInfoSize(g_DLLPath.c_str(), &FileVersionHandle);

		if (FileVersionSize)
		{
			char* Buffer = new char[FileVersionSize];
			char VersionString[0x100] = {0};
			void* VersionStringPtr = NULL;

			GetFileVersionInfo(g_DLLPath.c_str(), FileVersionHandle, FileVersionSize, Buffer);
			VerQueryValue(Buffer, "\\StringFileInfo\\040904b0\\ProductVersion", &VersionStringPtr, (PUINT)FileVersionHandle);
			sprintf_s(VersionString, sizeof(VersionString), "Version %s", VersionStringPtr);

			std::string ReplacedString(VersionString);
			std::replace(ReplacedString.begin(), ReplacedString.end(), ',', '.');

			SetDlgItemText(Dialog, 1580, (LPCSTR)ReplacedString.c_str());

			delete [] Buffer;
		}
	}

	#define _hhName		AboutDialog
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00441CCB);
		__asm
		{
			pushad
			push	esi
			call	DoAboutDialogHook
			popad

			call	ShowWindowAddress
			call	g_WindowHandleCallAddr
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoTESNPCUpdatePreviewControlHook(HWND Dialog)
	{
		BSExtraData* xData = TESDialog::GetDialogExtraByType(Dialog, 6);
		if (xData)
		{
			DialogExtraWorkingData* xWorkingData = CS_CAST(xData, BSExtraData, DialogExtraWorkingData);
			TESForm* ExistingTempRef = (TESForm*)xWorkingData->localCopy;
			if (ExistingTempRef)
				ExistingTempRef->DeleteInstance();

			xWorkingData->localCopy = NULL;
		}
	}

	#define _hhName		TESNPCUpdatePreviewControl
	_hhBegin()
	{
		_hhSetVar(Call, 0x0048AAC0);
		_hhSetVar(Retn, 0x0048C59D);
		__asm
		{
			pushad
			push	esi
			call	DoTESNPCUpdatePreviewControlHook
			popad

			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESParametersFillAndInitSelectionComboBoxOwners
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00459437);
		__asm
		{
			call	TESComboBox::PopulateWithForms
			add		esp, 0x10

			movzx	eax, word ptr [ebp + 0]				// get parameterID
			lea		eax, [eax + eax * 4]
			mov		eax, 0x9F3624[eax * 8]
			mov		eax, [eax + edi + 4]

			cmp		eax, 0x24							// check if parameterID == kParamType_Owner
			jz		FIX

			jmp		[_hhGetVar(Retn)]
		FIX:
			push	0
			push	0
			push	0x6
			push	esi
			call	TESComboBox::PopulateWithForms
			add		esp, 0x10

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		SearchReplaceDialog
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0044490A);
		__asm
		{
			pushad
			call	MessageBoxAddress
			popad

			call	[g_WindowHandleCallAddr]
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoObjectWindowPopulateFormListInvalidateHook(bool RedrawState)
	{
		if (!RedrawState)
			SendMessage(*g_HWND_ObjectWindow_FormList, WM_SETREDRAW, FALSE, NULL);
		else
		{
			SendMessage(*g_HWND_ObjectWindow_FormList, WM_SETREDRAW, TRUE, NULL);
			RedrawWindow(*g_HWND_ObjectWindow_FormList, NULL, NULL, RDW_ERASE|RDW_FRAME|RDW_INVALIDATE|RDW_ALLCHILDREN);
		}
	}

	#define _hhName		ObjectWindowPopulateFormListInvalidate
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00421EE6);
		_hhSetVar(Call, 0x00414C90);
		__asm
		{
			pushad
			push	0
			call	DoObjectWindowPopulateFormListInvalidateHook
			popad

			call	[_hhGetVar(Call)]

			pushad
			push	1
			call	DoObjectWindowPopulateFormListInvalidateHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		ObjectWindowResize
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004204CB);
		__asm
		{
			sub		esi, g_ObjectWindowTreePosOffset.y
			push	esi
			push	eax
			push	g_ObjectWindowTreePosOffset.y
			push	g_ObjectWindowTreePosOffset.x
			push	ecx
			call	edi
			add		esi, g_ObjectWindowTreePosOffset.y
			jmp		[_hhGetVar(Retn)]
		}
	}

	bool __stdcall FilterFormList(TESForm* Form, UInt8 ParentWindowID)
	{
		std::string FilterString;

		if (ParentWindowID == 1)
			FilterString = g_ObjectWindowFilterStr;
		else if (ParentWindowID == 2)
			FilterString = g_CellViewWindowFilterStr;

		if (!Form->editorID.c_str())
		{
			TESObjectREFR* Ref = CS_CAST(Form, TESForm, TESObjectREFR);
			if (Ref)
				Form = Ref->baseForm;
		}

		if (FilterString == "")
			return true;
		else if (Form && Form->editorID.c_str())
		{
			std::string LowerCase(Form->editorID.c_str()); MakeLower(LowerCase);
			if (LowerCase.find(FilterString) != std::string::npos)
				return true;
			else
				return false;
		}
		else
			return false;
	}

	#define _hhName		ObjectWindowPopulateFormListFilter
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00414D81);
		_hhSetVar(Jump, 0x00414D8C);
		__asm
		{
			pushad
			push	1		// object window filter string
			push	edi
			call	FilterFormList
			test	al, al
			jz		SKIP
			popad

			push	edi
			mov		edi, [esp + 0x1C]
			jmp		[_hhGetVar(Retn)]
		SKIP:
			popad
			jmp		[_hhGetVar(Jump)]
		}
	}

	void __stdcall DoLandscapeTextureUseDlgHook(HWND Handle)
	{
		g_LandscapeTextureUseOrgWindowProc = (WNDPROC)SetWindowLong(Handle, GWL_WNDPROC, (LONG)LandscapeTextureUseSubClassProc);
	}

	#define _hhName		LandscapeTextureUseDlg
	_hhBegin()
	{
		__asm
		{
			push    eax
			push    0x13C
			push    ecx

			pushad
			call	CreateDialogParamAddress
			popad

			call	[g_WindowHandleCallAddr]
			push	eax
			call	DoLandscapeTextureUseDlgHook
			retn
		}
	}

	void __stdcall DoBuildResultScriptEditButtonHook(HWND Dialog, int Template)
	{
		BSExtraData* xExtra = TESDialog::GetDialogExtraByType(Dialog, BSExtraData::kDialogExtra_SubWindow);
		DialogExtraSubWindow* xSubwindow = CS_CAST(xExtra, BSExtraData, DialogExtraSubWindow);

		if (xSubwindow)
		{
			Subwindow* DialogSubwindow = xSubwindow->subWindow;
			HWND EditResultScriptButton =  NULL;
			POINT ButtonPosition = DialogSubwindow->position;

			if (Template == 1)		// quest stage
			{
				ButtonPosition.x += 532 + 50;
				ButtonPosition.y += 136;
			}
			else if (Template > 2)	// dialog editor
			{
				ButtonPosition.x += 190 + 50;
				ButtonPosition.y += 360;
			}
			else
				return;

			EditResultScriptButton =  CreateWindowEx(0,
													"BUTTON",
													"...",
													WS_CHILD|WS_VISIBLE|WS_TABSTOP,
													ButtonPosition.x, ButtonPosition.y, 23, 15,
													Dialog,
													(HMENU)QUEST_EDITRESULTSCRIPT,
													GetModuleHandle(NULL),
													NULL);

			SendMessage(EditResultScriptButton, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
			DialogSubwindow->controls.AddAt((HWND*)EditResultScriptButton, eListEnd);
		}
	}

	#define _hhName		BuildResultScriptEditButton
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004DED77);
		__asm
		{
			mov     eax, [esp + 0x8]
			mov		ecx, [esp + 0x4]

			pushad
			push	eax
			push	ecx
			call	DoBuildResultScriptEditButtonHook
			popad

			cmp     eax, 9
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall HandleEditResultScriptMessages(HWND Dialog, int Identifier)
	{
		switch (Identifier)
		{
		case QUEST_EDITRESULTSCRIPT:
			if (IsWindowEnabled(GetDlgItem(Dialog, 1444)))
			{
				if (DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_EDITRESULTSCRIPT), Dialog, EditResultScriptDlgProc, (LPARAM)Dialog))
					SendMessage(Dialog, WM_COMMAND, 1591, NULL);		// compile result script
			}
			break;
		}
	}

	#define _hhName		DialogEditorCommandMessageCallback
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004F1DD7);
		__asm
		{
			movzx   esi, cx
			mov     eax, esi

			pushad
			push	eax
			push	edi
			call	HandleEditResultScriptMessages
			popad

			cmp     eax, 0x66C
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		TESQuestCommandMessageCallback
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004E2874);
		__asm
		{
			mov     ecx, [esp + 0x0B4]
			movzx   eax, cx

			pushad
			push	eax
			push	esi
			call	HandleEditResultScriptMessages
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	UInt32 __stdcall DoCellViewWindowResizeFixHook()
	{
		return GetWindowLong(*g_HWND_CellView_ObjectList, GWL_STYLE);
	}

	#define _hhName		CellViewWindowResizeFix
	_hhBegin()
	{
		static bool s_DoOnce = false;

		_hhSetVar(Retn, 0x0040981F);
		_hhSetVar(Jump, 0x00409C13);
		__asm
		{
			mov		al, s_DoOnce
			test	al, al
			jnz		SKIP

			mov		s_DoOnce, 1
			push	0x00A0AA38
			push	esi
			pushad
			call	GetClientRectAddress
			popad
			call	g_WindowHandleCallAddr

			jmp		[_hhGetVar(Retn)]
		SKIP:
			call	DoCellViewWindowResizeFixHook
			jmp		[_hhGetVar(Jump)]
		}
	}

	#define _hhName		CellViewWindowResizeA
	_hhBegin()
	{
		_hhSetVar(Retn, 0x004096FA);
		__asm
		{
			sub		eax, g_CellViewWindowObjListPosOffset.y
			push    eax
			mov		eax, [g_CellViewObjListBounds]
			mov     eax, [eax + 4]			// top
			add		eax, g_CellViewWindowObjListPosOffset.y
			lea     edx, [esi + ecx]
			mov		ecx, [g_CellViewObjListBounds]
			mov     ecx, [ecx]				// left
			push    edx
			push	eax
			mov     eax, g_HWND_CellView_ObjectList
			mov		eax, [eax]
			lea     edx, [esi + ecx]
			push    edx
			push    0
			push    eax
			push    edi
			call    ebx

			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoCellViewWindowResizeBHook(void)
	{
		HWND CellNameStatic = GetDlgItem(*g_HWND_CellView, 1163);
		HWND FilterEditBox = GetDlgItem(*g_HWND_CellView, OBJCELL_FILTEREDIT);
		HWND FilterLabel = GetDlgItem(*g_HWND_CellView, OBJCELL_FILTERLBL);
		HWND XCoordLbl = GetDlgItem(*g_HWND_CellView, CELLVIEW_XLBL);
		HWND YCoordLbl = GetDlgItem(*g_HWND_CellView, CELLVIEW_YLBL);
		HWND XCoordEditBox = GetDlgItem(*g_HWND_CellView, CELLVIEW_XEDIT);
		HWND YCoordEditBox = GetDlgItem(*g_HWND_CellView, CELLVIEW_YEDIT);
		HWND GoButton = GetDlgItem(*g_HWND_CellView, CELLVIEW_GOBTN);

		RECT CellNameBounds = {0}, ObjListBounds = {0};
		GetClientRect(CellNameStatic, &CellNameBounds);
		GetClientRect(*g_HWND_CellView_ObjectList, &ObjListBounds);

		MoveWindow(FilterLabel,
			CellNameBounds.right * 1.5,
			CellNameBounds.bottom + g_CellViewWindowObjListPosOffset.y * 1.5,
			25,
			20,
			TRUE);
		MoveWindow(FilterEditBox,
				CellNameBounds.right * 1.5 + 30 ,
				CellNameBounds.bottom + g_CellViewWindowObjListPosOffset.y * 1.5,
				(ObjListBounds.right - ObjListBounds.left) / 2 - 32,
				20,
				TRUE);

		MoveWindow(XCoordLbl,
				CellNameBounds.right,
				CellNameBounds.bottom + g_CellViewWindowObjListPosOffset.y * 1.5,
				10,
				20,
				TRUE);
		MoveWindow(XCoordEditBox,
				CellNameBounds.right + 15,
				CellNameBounds.bottom + g_CellViewWindowObjListPosOffset.y * 1.5,
				30,
				20,
				TRUE);
		MoveWindow(YCoordLbl,
				CellNameBounds.right + 15 + 35,
				CellNameBounds.bottom + g_CellViewWindowObjListPosOffset.y * 1.5,
				10,
				20,
				TRUE);
		MoveWindow(YCoordEditBox,
				CellNameBounds.right + 15 + 35 + 15,
				CellNameBounds.bottom + g_CellViewWindowObjListPosOffset.y * 1.5,
				30,
				20,
				TRUE);

		MoveWindow(GoButton,
				CellNameBounds.right + 15 + 35 + 15 + 42,
				CellNameBounds.bottom + g_CellViewWindowObjListPosOffset.y * 1.5,
				35,
				20,
				TRUE);
	}

	#define _hhName		CellViewWindowResizeB
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00409734);
		__asm
		{
			push    eax
			push    edi
			call    ebx
			push    1

			pushad
			call	DoCellViewWindowResizeBHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		CellViewWindowPopulateObjectListFilter
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00409059);
		_hhSetVar(Call, 0x004087C0);
		__asm
		{
			pushad
			push	2		// cell view window filter string
			push	eax
			call	FilterFormList
			test	al, al
			jz		SKIP
			popad

			call	[_hhGetVar(Call)]
			jmp		[_hhGetVar(Retn)]
		SKIP:
			popad
			jmp		[_hhGetVar(Retn)]
		}
	}

	static char s_SoundSamplerTempPath[MAX_PATH] = {0};

	void __stdcall DoTESSoundPlayFileHook(const char* FilePath)
	{
		TESSound::DeinitializeSoundSampler();
		ZeroMemory(s_SoundSamplerTempPath, sizeof(s_SoundSamplerTempPath));
		DeleteFile("tempsf");

		if (GetFileAttributes(FilePath) != INVALID_FILE_ATTRIBUTES)
			return;

		if (ArchiveManager::ExtractArchiveFile(FilePath, "tempsf"))
		{
			sprintf_s(s_SoundSamplerTempPath, sizeof(s_SoundSamplerTempPath), "tempsf");
		}
	}

	#define _hhName		TESSoundPlayFile
	_hhBegin()
	{
		_hhSetVar(Retn, 0x005047B6);
		__asm
		{
			mov		eax, [esp + 4]

			pushad
			push	eax
			call	DoTESSoundPlayFileHook
			popad

			lea		eax, s_SoundSamplerTempPath
			mov		al, [eax]
			test	al, al
			jz		EXIT

			lea		eax, s_SoundSamplerTempPath
			mov		[esp + 4], eax
		EXIT:
			sub		esp, 0x310
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoFormEditDialogTitleHook(HWND Dialog, TESForm* Form)
	{
		if (Form && Form->editorID.c_str())
		{
			char BufferA[0x200] = {0}, BufferB[0x200] = {0};
			GetWindowText(Dialog, BufferA, sizeof(BufferA));
			sprintf_s(BufferB, sizeof(BufferB), "%s [%s]", BufferA, Form->editorID.c_str());
			SetWindowText(Dialog, BufferB);
		}
	}

	#define _hhName		FormEditDialogTitle
	_hhBegin()
	{
		_hhSetVar(Retn, 0x0044788B);
		__asm
		{
			mov     ecx, [ebx + 4]

			pushad
			push	ecx
			push	edi
			call	DoFormEditDialogTitleHook
			popad

			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		FindTextFormEnumerationA
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00444217);
		_hhSetVar(Jump, 0x00444237);
		__asm
		{
			mov		[esp + 0x14], edi
			mov		ecx, esi
			pushad
			push	eax
			call	FormEnumerationWrapper::GetShouldEnumerateForm
			test	al, al
			jz		SKIP

			popad
			jmp		[_hhGetVar(Retn)]
		SKIP:
			popad
			jmp		[_hhGetVar(Jump)]
		}
	}

	#define _hhName		FindTextFormEnumerationB
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00444184);
		_hhSetVar(Jump, 0x004441A1);
		__asm
		{
			mov     [esp + 0x34], ebp
			mov     ecx, edi

			pushad
			push	eax
			call	FormEnumerationWrapper::GetShouldEnumerateForm
			test	al, al
			jz		SKIP

			popad
			add     ebp, 1
			jmp		[_hhGetVar(Retn)]
		SKIP:
			popad
			jmp		[_hhGetVar(Jump)]
		}
	}

	#define _hhName		FindTextFormEnumerationC
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00444439);
		_hhSetVar(Jump, 0x00444456);
		__asm
		{
			mov     [esp + 0x38], ebp
			mov     ecx, ebx

			pushad
			push	eax
			call	FormEnumerationWrapper::GetShouldEnumerateForm
			test	al, al
			jz		SKIP

			popad
			add     ebp, 1
			jmp		[_hhGetVar(Retn)]
		SKIP:
			popad
			jmp		[_hhGetVar(Jump)]
		}
	}
}