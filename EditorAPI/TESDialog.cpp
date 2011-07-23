#include "TESForm.h"
#include "TESDialog.h"

const HINSTANCE*					g_TESCS_Instance = (HINSTANCE*)0x00A0AF1C;
const DLGPROC						g_ScriptEditor_DlgProc = (DLGPROC)0x004FE760;
const DLGPROC						g_FormUseReport_DlgProc = (DLGPROC)0x00433FE0;
const DLGPROC						g_TESDialogFormEdit_DlgProc = (DLGPROC)0x00447580;
const DLGPROC						g_TESDialogFormIDListView_DlgProc = (DLGPROC)0x00448820;
const DLGPROC						g_TESDialogSelectReference_DlgProc = (DLGPROC)0x0044D470;

HWND*								g_HWND_RenderWindow = (HWND*)0x00A0AF28;
HWND*								g_HWND_ObjectWindow = (HWND*)0x00A0AF44;
HWND*								g_HWND_CellView = (HWND*)0x00A0AF4C;
HWND*								g_HWND_CSParent = (HWND*)0x00A0AF20;
HWND*								g_HWND_AIPackagesDlg = (HWND*)0x00A0AFD8;
HWND*								g_HWND_ObjectWindow_FormList = (HWND*)0x00A0BAA0;
HWND*								g_HWND_ObjectWindow_Tree = (HWND*)0x00A0BAA4;
HWND*								g_HWND_MainToolbar = (HWND*)0x00A0AFD0;
HWND*								g_HWND_QuestWindow = (HWND*)0x00A0B034;
HWND*								g_HWND_LandscapeEdit = (HWND*)0x00A0AF54;
HWND*								g_HWND_CellView_ObjectList = (HWND*)0x00A0AA00;
HWND*								g_HWND_CellView_CellList = (HWND*)0x00A0AA34;

char**								g_TESActivePluginName = (char**)0x00A0AF00;
UInt8*								g_TESCSAllowAutoSaveFlag = (UInt8*)0x00A0B628;
UInt8*								g_TESCSExittingCSFlag = (UInt8*)0x00A0B63C;
UInt8*								g_Flag_ObjectWindow_MenuState = (UInt8*)0x00A0AF40;
UInt8*								g_Flag_CellView_MenuState = (UInt8*)0x00A0AF48;
HMENU*								g_RenderWindowPopup = (HMENU*)0x00A0BC40;
ResponseEditorData**				g_ResponseEditorData = (ResponseEditorData**)0x00A10E2C;
RECT*								g_CellViewObjListBounds = (RECT*)0x00A0AA04;
RECT*								g_CellViewCellNameStaticBounds = (RECT*)0x00A0A9F0;
RECT*								g_CellViewDlgBounds = (RECT*)0x00A0AA38;

FormEditParam::FormEditParam(const char* EditorID)
{
	form = TESForm::LookupByEditorID(EditorID);
	typeID = form->formType;
}

FormEditParam::FormEditParam(UInt32 FormID)
{
	form = TESForm::LookupByFormID(FormID);
	typeID = form->formType;
}

UInt32 TESDialog::WritePositionToINI( HWND Handle, const char* WindowClassName )
{
	return cdeclCall<UInt32>(0x00417510, Handle, WindowClassName);
}

bool TESDialog::GetPositionFromINI( const char* WindowClassName, LPRECT OutRect )
{
	return cdeclCall<bool>(0x004176D0, WindowClassName, OutRect);
}

LRESULT TESDialog::WriteToStatusBar( WPARAM wParam, LPARAM lParam )
{
	return cdeclCall<UInt32>(0x00431310, wParam, lParam);
}

void TESDialog::InitializeCSWindows()
{
	cdeclCall<UInt32>(0x00430980);
}

void TESDialog::DeinitializeCSWindows()
{
	cdeclCall<UInt32>(0x00431220);
}

void TESDialog::SetCSWindowTitleModifiedFlag( bool State )
{
	cdeclCall<UInt32>(0x004306F0, State);
}

void TESDialog::AutoSave()
{
	cdeclCall<UInt32>(0x004307C0);
}

UInt32 TESDialog::GetDialogTemplateForFormType( UInt8 FormTypeID )
{
	return cdeclCall<UInt32>(0x00442050, FormTypeID);
}

TESObjectREFR* TESDialog::ShowSelectReferenceDialog( HWND Parent, TESObjectREFR* DefaultSelection )
{
	return cdeclCall<TESObjectREFR*>(0x0044D660, Parent, DefaultSelection, 0x00545B10, 0);
}

BSExtraData* TESDialog::GetDialogExtraByType( HWND Dialog, UInt16 Type )
{
	return cdeclCall<BSExtraData*>(0x00442990, Dialog, Type);
}

TESForm* TESDialog::GetDialogExtraParam( HWND Dialog )
{
	return cdeclCall<TESForm*>(0x004429D0, Dialog);
}

TESForm* TESDialog::GetDialogExtraLocalCopy( HWND Dialog )
{
	return cdeclCall<TESForm*>(0x004429B0, Dialog);
}

bool TESDialog::GetIsWindowDragDropRecipient( UInt16 FormType, HWND hWnd )
{
	return cdeclCall<bool>(0x004433C0, FormType, hWnd);
}

bool TESDialog::SelectTESFileCommonDialog( HWND Parent, const char* SaveDir, bool SaveAsESM, char* FileNameOut, size_t OutSize )
{
	return cdeclCall<bool>(0x00446D40, Parent, SaveDir, SaveAsESM, FileNameOut, OutSize);
}

void TESComboBox::AddItem( HWND hWnd, const char* Text, void* Data, bool ResizeDroppedWidth )
{
	cdeclCall<UInt32>(0x00403540, hWnd, Text, Data, ResizeDroppedWidth);
}

void* TESComboBox::GetSelectedItemData( HWND hWnd )
{
	return cdeclCall<void*>(0x00403690, hWnd);
}

void TESComboBox::PopulateWithForms( HWND hWnd, UInt8 FormType, bool ClearItems, bool AddDefaultItem )
{
	cdeclCall<UInt32>(0x004456F0, hWnd, FormType, ClearItems, AddDefaultItem);
}

void* TESListView::GetSelectedItemData( HWND hWnd )
{
	return cdeclCall<void*>(0x00403C40, hWnd);
}