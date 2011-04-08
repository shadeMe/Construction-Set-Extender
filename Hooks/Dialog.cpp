#include "Dialog.h"

bool g_QuickLoadToggle = false;

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
_DefineNopHdlr(TESDialogSubwindowEnumChildCallback, 0x00404E69, 3);
_DefineHookHdlr(TESDialogGetIsWindowDragDropRecipient, 0x004433FF);


void PatchDialogHooks(void)
{
	_MemoryHandler(FindTextInit).WriteJump();
	_MemoryHandler(UseInfoListInit).WriteJump();
	_MemoryHandler(DataDlgInit).WriteJump();
	_MemoryHandler(NPCFaceGen).WriteJump();
	_MemoryHandler(CustomCSWindow).WriteJump();
	_MemoryHandler(AddListViewItem).WriteJump();
	_MemoryHandler(ObjectListPopulateListViewItems).WriteJump();
	_MemoryHandler(CellViewPopulateObjectList).WriteJump();
	_MemoryHandler(TopicResultScriptReset).WriteJump();
	_MemoryHandler(RaceDescriptionDirtyEdit).WriteUInt8(0xEB);
	_MemoryHandler(MissingTextureWarning).WriteNop();
	_MemoryHandler(ResponseEditorMic).WriteNop(); 
	_MemoryHandler(CellObjectListShadeMeRefAppend).WriteJump();
	_MemoryHandler(DeathToTheCloseOpenDialogsMessage).WriteUInt8(0xEB);
	_MemoryHandler(TESDialogPopupMenu).WriteJump();
	_MemoryHandler(ResponseWindowLipButtonPatch).WriteJump();
	_MemoryHandler(ResponseWindowInit).WriteJump();
	_MemoryHandler(DataDlgZOrder).WriteJump();
	_MemoryHandler(FormIDListViewInit).WriteJump();
	_MemoryHandler(FormIDListViewSaveChanges).WriteJump();
	_MemoryHandler(FormIDListViewItemChange).WriteJump();
	_MemoryHandler(FormIDListViewSelectItem).WriteJump();
	_MemoryHandler(FormIDListViewDuplicateSelection).WriteJump();
	_MemoryHandler(TESRaceCopyHairEyeDataInit).WriteJump();
	_MemoryHandler(TESRaceCopyHairEyeDataMessageHandler).WriteJump();
	_MemoryHandler(TESDialogGetIsWindowDragDropRecipient).WriteJump();
}

void __stdcall DoFindTextInitHook(HWND FindTextDialog)
{
	g_FindTextOrgWindowProc = (WNDPROC)SetWindowLong(FindTextDialog, GWL_WNDPROC, (LONG)FindTextDlgSubClassProc);
}

_BeginHookHdlrFn(FindTextInit)
{
	_DeclareHookHdlrFnVariable(FindTextInit, Retn, 0x00419A48);
	__asm
	{
		call	CreateDialogParamAddress
		call	[g_WindowHandleCallAddr]			// CreateDialogParamA

		pushad
		push	eax	
		call	DoFindTextInitHook
		popad

		jmp		[_HookHdlrFnVariable(FindTextInit, Retn)]
	}
}

_BeginHookHdlrFn(UseInfoListInit)
{
	_DeclareHookHdlrFnVariable(UseInfoListInit, Retn, 0x00419848);
	__asm
	{
		push	0
		call	CLIWrapper::UseInfoList::OpenUseInfoBox
		jmp		[_HookHdlrFnVariable(UseInfoListInit, Retn)]
	}
}

void __stdcall DoNPCFaceGenHook(HWND Dialog)
{
	SendMessageA(Dialog, WM_COMMAND, 1014, 0);
}

_BeginHookHdlrFn(NPCFaceGen)     
{
	_DeclareHookHdlrFnVariable(NPCFaceGen, Retn, 0x004D76B1);
	_DeclareHookHdlrFnVariable(NPCFaceGen, Call, 0x0049C230);
    __asm
    {
		call	[_HookHdlrFnVariable(NPCFaceGen, Call)]
		push	esi
		call	DoNPCFaceGenHook
		jmp		[_HookHdlrFnVariable(NPCFaceGen, Retn)]
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

_BeginHookHdlrFn(DataDlgInit)
{
	_DeclareHookHdlrFnVariable(DataDlgInit, Retn, 0x0040C6DC);
	_DeclareHookHdlrFnVariable(DataDlgInit, Call, 0x00404A90);
	__asm
	{
		call	[_HookHdlrFnVariable(DataDlgInit, Call)]

		pushad
		push	esi
		call	DoDataDlgInitHook
		popad

		jmp		[_HookHdlrFnVariable(DataDlgInit, Retn)]
	}
}

bool __stdcall DoCustomCSWindowPatchHook(HWND Window)
{
	if (g_CustomMainWindowChildrenDialogs.GetHandleExists(Window))
		return false;
	else
		return true;
}

_BeginHookHdlrFn(CustomCSWindow)
{
	_DeclareHookHdlrFnVariable(CustomCSWindow, Retn, 0x004311EF);
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
		jmp		[_HookHdlrFnVariable(CustomCSWindow, Retn)]
	}

}

_BeginHookHdlrFn(AddListViewItem)
{
	_DeclareHookHdlrFnVariable(AddListViewItem, Retn, 0x004038F7);
	_DeclareHookHdlrFnVariable(AddListViewItem, Exit, 0x0040396E);
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
		jmp		[_HookHdlrFnVariable(AddListViewItem, Retn)]
	EXIT:
		popad
		jmp		[_HookHdlrFnVariable(AddListViewItem, Exit)]
    }
}

_BeginHookHdlrFn(AddComboBoxItem)
{
	_DeclareHookHdlrFnVariable(AddComboBoxItem, Retn, 0x00403548);
	_DeclareHookHdlrFnVariable(AddComboBoxItem, Exit, 0x004035F4);
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
		jmp		[_HookHdlrFnVariable(AddComboBoxItem, Retn)]
	EXIT:
		popad
		jmp		[_HookHdlrFnVariable(AddComboBoxItem, Exit)]
    }
}

_BeginHookHdlrFn(ObjectListPopulateListViewItems)
{
	_DeclareHookHdlrFnVariable(ObjectListPopulateListViewItems, Retn, 0x0041398A);
	_DeclareHookHdlrFnVariable(ObjectListPopulateListViewItems, Exit, 0x00413A50);
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

		jmp		[_HookHdlrFnVariable(ObjectListPopulateListViewItems, Retn)]
	EXIT2:
		popad
		jmp		[_HookHdlrFnVariable(ObjectListPopulateListViewItems, Exit)]
	}
}

_BeginHookHdlrFn(CellViewPopulateObjectList)
{
	_DeclareHookHdlrFnVariable(CellViewPopulateObjectList, Retn, 0x004087D3);
	_DeclareHookHdlrFnVariable(CellViewPopulateObjectList, Exit, 0x004088AF);
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

		jmp		[_HookHdlrFnVariable(CellViewPopulateObjectList, Retn)]
	EXIT1:
		popad
	EXIT2:
		jmp		[_HookHdlrFnVariable(CellViewPopulateObjectList, Exit)]
	}
}

void __stdcall AppendShadeMeRefToComboBox(HWND hWnd)
{
	TESForm* Ref = GetFormByID("TheShadeMeRef");
	sprintf_s(g_Buffer, sizeof(g_Buffer), "'shadeMe' 'TheShadeMeRef'");
	TESDialog_AddComboBoxItem(hWnd, g_Buffer, (LPARAM)Ref, 1);
}

_BeginHookHdlrFn(CellObjectListShadeMeRefAppend)
{
	_DeclareHookHdlrFnVariable(CellObjectListShadeMeRefAppend, Retn, 0x0044512D);
	__asm
	{
		pushad
		push	edx
		call	AppendShadeMeRefToComboBox
		popad

		call	TESDialog_AddComboBoxItem
		jmp		[_HookHdlrFnVariable(CellObjectListShadeMeRefAppend, Retn)]
	}
}

void __stdcall InsertFormListPopupMenuItems(HMENU Menu, TESForm* SelectedForm)
{
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_SEPARATOR, NULL, NULL);
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_SETFORMID, "Set FormID");
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_MARKUNMODIFIED, "Mark As Unmodified");
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_UNDELETE, "Undelete");
	InsertMenu(Menu, -1, MF_BYPOSITION|MF_STRING, POPUP_JUMPTOUSEINFOLIST, "Jump To Central Use Info List");
	if (GetDialogTemplate(SelectedForm->typeID) == 1 && SelectedForm->IsReference() == 0)
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
	DeleteMenu(Menu, GetMenuItemCount(Menu) - 1, MF_BYPOSITION);
	DeleteMenu(Menu, GetMenuItemCount(Menu) - 1, MF_BYPOSITION);
}


_BeginHookHdlrFn(TESDialogPopupMenu)
{
	_DeclareHookHdlrFnVariable(TESDialogPopupMenu, Retn, 0x004435C3);
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
		jmp		[_HookHdlrFnVariable(TESDialogPopupMenu, Retn)]
	SKIP:
		push	2
		push	esi
		call	g_WindowHandleCallAddr

		jmp		[_HookHdlrFnVariable(TESDialogPopupMenu, Retn)]
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

_BeginHookHdlrFn(ResponseWindowInit)
{
	_DeclareHookHdlrFnVariable(ResponseWindowInit, Retn, 0x004EBA92);
	__asm
	{
		push	ebp
		call	DoResponseWindowInitHook
		jmp		[_HookHdlrFnVariable(ResponseWindowInit, Retn)]
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

_BeginHookHdlrFn(FormIDListViewInit)
{
	_DeclareHookHdlrFnVariable(FormIDListViewInit, Retn, 0x00448A94);
	_DeclareHookHdlrFnVariable(FormIDListViewInit, Byte, 0x00A0BE45);
	__asm
	{
		pushad
		push	esi
		call	DoFormIDListViewInitHook
		popad

		push	5
		push	esi
		mov		[_HookHdlrFnVariable(FormIDListViewInit, Byte)], 0

		jmp		[_HookHdlrFnVariable(FormIDListViewInit, Retn)]
	}
}

UInt32 __stdcall DoFormIDListViewSaveChangesHookProlog(HWND Parent)
{
	return Parent != *g_HWND_QuestWindow;
}

void __stdcall DoFormIDListViewSaveChangesHookEpilog(HWND Parent)
{
	if (IsWindowEnabled(GetDlgItem(Parent, 1)))
	{
		TESForm* LocalCopy = TESDialog_GetDialogExtraLocalCopy(Parent);
		TESForm* WorkingCopy = TESDialog_GetDialogExtraParam(Parent);

		if (WorkingCopy)
		{
			thisVirtualCall(*((UInt32*)LocalCopy), 0x118, LocalCopy, Parent);						// GetDataFromDialog
			if (thisVirtualCall(*((UInt32*)WorkingCopy), 0xBC, WorkingCopy, LocalCopy))				// CompareTo
			{
				if (thisVirtualCall(*((UInt32*)WorkingCopy), 0x104, WorkingCopy, LocalCopy))		// UpdateUsageInfo
				{
					thisVirtualCall(*((UInt32*)WorkingCopy), 0x94, LocalCopy, 1);					// SetFromActiveFile
					thisVirtualCall(*((UInt32*)WorkingCopy), 0xB8, WorkingCopy, LocalCopy);			// CopyFrom
				}
			}
		}
	}
}

_BeginHookHdlrFn(FormIDListViewSaveChanges)
{
	_DeclareHookHdlrFnVariable(FormIDListViewSaveChanges, QuestRetn, 0x00449580);
	_DeclareHookHdlrFnVariable(FormIDListViewSaveChanges, ExitRetn, 0x00448BF0);	
	__asm
	{
		push	esi
		call	TESDialog_GetDialogExtraParam

		pushad
		push	esi
		call	DoFormIDListViewSaveChangesHookProlog
		test	eax, eax
		jnz		NOTQUEST
		popad

		jmp		[_HookHdlrFnVariable(FormIDListViewSaveChanges, QuestRetn)]
	NOTQUEST:
		popad

		pushad
		push	esi
		call	DoFormIDListViewSaveChangesHookEpilog
		popad

		jmp		[_HookHdlrFnVariable(FormIDListViewSaveChanges, ExitRetn)]
	}
}

int __stdcall DoFormIDListViewItemChangeHook(HWND Parent)
{
	return MessageBox(Parent, "Save Changes made to the active form?", "CSE", MB_YESNO);
}

_BeginHookHdlrFn(FormIDListViewItemChange)
{
	_DeclareHookHdlrFnVariable(FormIDListViewItemChange, Retn, 0x00448DF4);
	__asm
	{
		pushad
		push	esi
		call	DoFormIDListViewItemChangeHook
		cmp		eax, IDYES
		jnz		REVERT
		popad

		mov		eax, [edi]
		mov		edx, [eax + 0x104]

		jmp		[_HookHdlrFnVariable(FormIDListViewItemChange, Retn)]
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

_BeginHookHdlrFn(FormIDListViewSelectItem)
{
	_DeclareHookHdlrFnVariable(FormIDListViewSelectItem, Retn, 0x00403B8A);
	__asm
	{
		pushad
		push	ebx
		push	esi
		call	DoFormIDListViewSelectItemHook
		popad

		jmp		[_HookHdlrFnVariable(FormIDListViewSelectItem, Retn)]
	}
}

_BeginHookHdlrFn(FormIDListViewDuplicateSelection)
{
	_DeclareHookHdlrFnVariable(FormIDListViewDuplicateSelection, Retn, 0x004492B7);
	__asm
	{
		add		eax, 1
		push	eax
		push	0x810
		push	esi
		call	edi

		jmp		[_HookHdlrFnVariable(FormIDListViewDuplicateSelection, Retn)]
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

		thisCall(kLinkedListNode_NewNode, &RaceDialogSubWindow->controls, CopyHairButton);
		thisCall(kLinkedListNode_NewNode, &RaceDialogSubWindow->controls, CopyEyesButton);
	}
}

_BeginHookHdlrFn(TESRaceCopyHairEyeDataInit)
{
	_DeclareHookHdlrFnVariable(TESRaceCopyHairEyeDataInit, Retn, 0x004E973A);
	_DeclareHookHdlrFnVariable(TESRaceCopyHairEyeDataInit, Call, 0x004E83B0);
	__asm
	{
		call	[_HookHdlrFnVariable(TESRaceCopyHairEyeDataInit, Call)]

		pushad
		push	ebx
		push	edi
		call	DoTESRaceCopyHairEyeDataInitHook
		popad
		jmp		[_HookHdlrFnVariable(TESRaceCopyHairEyeDataInit, Retn)]
	}
}

UInt32 __stdcall DoTESRaceCopyHairEyeDataMessageHandlerHook(HWND Dialog, INT Identifier, TESRace* WorkingRace)
{
	switch (Identifier)
	{
	case RACE_COPYEYES:
	case RACE_COPYHAIR:
		TESForm* Selection = (TESForm*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TESCOMBOBOX), Dialog, (DLGPROC)TESComboBoxDlgProc, (LPARAM)kFormType_Race);
		if (Selection)
		{
			TESRace* SelectedRace = CS_CAST(Selection, TESForm, TESRace);

			if (WorkingRace)
			{
				int Count = 0;
				if (Identifier == RACE_COPYEYES)
				{
					GenericNode<TESEyes>* Source = (GenericNode<TESEyes>*)&SelectedRace->eyes;
					tList<TESEyes>* Destination = &WorkingRace->eyes;

					for (;Source && Source->data; Source = Source->next)
					{
						if (Destination->IndexOf(Source->data) == -1)
						{
							thisCall(kLinkedListNode_NewNode, Destination, Source->data);
							Count++;
						}
					}

					sprintf_s(g_Buffer, sizeof(g_Buffer), "Copied %d eye forms from race '%s'.", Count, SelectedRace->editorData.editorID.m_data);
					MessageBox(Dialog, g_Buffer, "CSE", MB_OK);
				}
				else if (Identifier == RACE_COPYHAIR)
				{
					GenericNode<TESHair>* Source = (GenericNode<TESHair>*)&SelectedRace->hairs;
					tList<TESHair>* Destination = &WorkingRace->hairs;

					for (;Source && Source->data; Source = Source->next)
					{
						if (Destination->IndexOf(Source->data) == -1)
						{
							thisCall(kLinkedListNode_NewNode, Destination, Source->data);
							Count++;
						}
					}

					sprintf_s(g_Buffer, sizeof(g_Buffer), "Copied %d hair forms from race '%s'.", Count, SelectedRace->editorData.editorID.m_data);
					MessageBox(Dialog, g_Buffer, "CSE", MB_OK);
				}
			}
		}
		return 1;
	}

	return 0;
}

_BeginHookHdlrFn(TESRaceCopyHairEyeDataMessageHandler)
{
	_DeclareHookHdlrFnVariable(TESRaceCopyHairEyeDataMessageHandler, Retn, 0x004E8FF2);
	_DeclareHookHdlrFnVariable(TESRaceCopyHairEyeDataMessageHandler, Jump, 0x004E8E6D);
	__asm
	{
		movzx	eax, di
		add		eax, 0FFFFF78Ch
		cmp		eax, 0x24 
		ja		DEFAULT

		jmp		[_HookHdlrFnVariable(TESRaceCopyHairEyeDataMessageHandler, Retn)]
	DEFAULT:
		pushad
		push	ebp
		push	edi
		push	esi
		call	DoTESRaceCopyHairEyeDataMessageHandlerHook
		test	eax, eax
		jnz		HANDLED
		popad

		jmp		[_HookHdlrFnVariable(TESRaceCopyHairEyeDataMessageHandler, Jump)]
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

_BeginHookHdlrFn(TESDialogGetIsWindowDragDropRecipient)
{
	_DeclareHookHdlrFnVariable(TESDialogGetIsWindowDragDropRecipient, Retn, 0x00443409);
	_DeclareHookHdlrFnVariable(TESDialogGetIsWindowDragDropRecipient, Jump, 0x0044341F);
	__asm
	{
		pushad
		call	SendMessageAddress
		popad

		call	[g_WindowHandleCallAddr]
		test	eax, eax
		jz		FAIL

		jmp		[_HookHdlrFnVariable(TESDialogGetIsWindowDragDropRecipient, Retn)]
	FAIL:
		pushad
		push	edi
		call	DoTESDialogGetIsWindowDragDropRecipientHook
		test	al, al
		jnz		FIX
		popad

		jmp		[_HookHdlrFnVariable(TESDialogGetIsWindowDragDropRecipient, Jump)]
	FIX:
		popad
		jmp		[_HookHdlrFnVariable(TESDialogGetIsWindowDragDropRecipient, Retn)]
	}
}