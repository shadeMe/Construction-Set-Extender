#include "TESForm.h"
#include "TESDialog.h"
#include "PathGridUndoManager.h"
#include "[Common]\CLIWrapper.h"

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
HWND*								g_HWND_PreviewWindow = (HWND*)0x00A0A71C;

HMENU*								g_HMENU_MainMenu = (HMENU*)0x00A0B630;

char**								g_TESActivePluginName = (char**)0x00A0AF00;
UInt8*								g_TESCSAllowAutoSaveFlag = (UInt8*)0x00A0B628;
UInt8*								g_TESCSExittingCSFlag = (UInt8*)0x00A0B63C;
UInt8*								g_Flag_ObjectWindow_MenuState = (UInt8*)0x00A0AF40;
UInt8*								g_Flag_CellView_MenuState = (UInt8*)0x00A0AF48;
ResponseEditorData**				g_ResponseEditorData = (ResponseEditorData**)0x00A10E2C;

using namespace ConstructionSetExtender;

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

FormEditParam::FormEditParam( TESForm* Form )
{
	form = Form;
	typeID = Form->formType;
}

UInt32 TESDialog::WritePositionToINI( HWND Handle, const char* WindowClassName )
{
	return cdeclCall<UInt32>(0x00417510, Handle, WindowClassName);
}

bool TESDialog::GetPositionFromINI( const char* WindowClassName, LPRECT OutRect )
{
	return cdeclCall<bool>(0x004176D0, WindowClassName, OutRect);
}

LRESULT TESDialog::WriteToStatusBar( int PanelIndex, const char* Message )
{
	return cdeclCall<UInt32>(0x00431310, PanelIndex, Message);
}

void TESDialog::InitializeCSWindows()
{
	cdeclCall<UInt32>(0x00430980);
}

void TESDialog::DeinitializeCSWindows()
{
	cdeclCall<UInt32>(0x00431220);
}

void TESDialog::SetMainWindowTitleModified( bool State )
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

HWND TESDialog::ShowFormEditDialog( TESForm* Form )
{
	if (GetIsFormEditDialogCompatible(Form) == false)
		return NULL;

	FormEditParam InitData(Form);
	return BGSEEUI->ModelessDialog(*g_TESCS_Instance,
							(LPSTR)GetDialogTemplateForFormType(InitData.typeID),
							*g_HWND_CSParent,
							g_TESDialogFormEdit_DlgProc,
							(LPARAM)&InitData,
							true);
}

void TESDialog::ShowScriptEditorDialog( TESForm* InitScript )
{
	Script* AuxScript = CS_CAST(InitScript, TESForm, Script);
	tagRECT ScriptEditorLoc;
	ComponentDLLInterface::ScriptData* Data = NULL;

	if (AuxScript)
		Data = new ComponentDLLInterface::ScriptData(AuxScript);

	TESDialog::GetPositionFromINI("Script Edit", &ScriptEditorLoc);
	CLIWrapper::Interfaces::SE->InstantiateEditor(Data, ScriptEditorLoc.left, ScriptEditorLoc.top, ScriptEditorLoc.right, ScriptEditorLoc.bottom);
}

void TESDialog::ResetRenderWindow()
{
	UInt8 ObjWndState = *g_Flag_ObjectWindow_MenuState, CellWndState = *g_Flag_CellView_MenuState;

	*g_Flag_ObjectWindow_MenuState = 0;
	*g_Flag_CellView_MenuState = 0;

	SendMessage(*g_HWND_RenderWindow, 0x419, 6, 1);
	SendMessage(*g_HWND_RenderWindow, 0x419, 5, 0);
	InvalidateRect(*g_HWND_RenderWindow, 0, 1);

	TESDialog::DeinitializeCSWindows();
	TESDialog::InitializeCSWindows();

	*g_Flag_ObjectWindow_MenuState = ObjWndState;
	*g_Flag_CellView_MenuState = CellWndState;
}

bool TESDialog::GetIsFormEditDialogCompatible( TESForm* Form )
{
	switch (Form->formType)
	{
	case TESForm::kFormType_Activator:
	case TESForm::kFormType_Apparatus:
	case TESForm::kFormType_Armor:
	case TESForm::kFormType_Book:
	case TESForm::kFormType_Clothing:
	case TESForm::kFormType_Container:
	case TESForm::kFormType_Door:
	case TESForm::kFormType_Ingredient:
	case TESForm::kFormType_Light:
	case TESForm::kFormType_Misc:
	case TESForm::kFormType_Key:
	case TESForm::kFormType_SoulGem:
	case TESForm::kFormType_Static:
	case TESForm::kFormType_Grass:
	case TESForm::kFormType_Tree:
	case TESForm::kFormType_Flora:
	case TESForm::kFormType_Furniture:
	case TESForm::kFormType_Ammo:
	case TESForm::kFormType_Weapon:
	case TESForm::kFormType_NPC:
	case TESForm::kFormType_Creature:
	case TESForm::kFormType_LeveledCreature:
	case TESForm::kFormType_Spell:
	case TESForm::kFormType_Enchantment:
	case TESForm::kFormType_AlchemyItem:
	case TESForm::kFormType_LeveledItem:
	case TESForm::kFormType_LeveledSpell:
	case TESForm::kFormType_Sound:
	case TESForm::kFormType_LandTexture:
	case TESForm::kFormType_CombatStyle:
	case TESForm::kFormType_LoadScreen:
	case TESForm::kFormType_WaterForm:
	case TESForm::kFormType_AnimObject:
	case TESForm::kFormType_SubSpace:
	case TESForm::kFormType_EffectShader:
	case TESForm::kFormType_SigilStone:
		return true;
	default:
		return false;
	}
}

void TESDialog::RedrawRenderWindow()
{
	if (*g_RenderWindowPathGridEditModeFlag)
	{
		PathGridUndoManager::Instance.SetCanReset(false);
		SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);
		SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);
		PathGridUndoManager::Instance.SetCanReset(true);
	}
	else
	{
		*g_RenderWindowUpdateViewPortFlag = 1;
	}
}

HWND TESDialog::ShowUseReportDialog( TESForm* Form )
{
	return BGSEEUI->ModelessDialog(*g_TESCS_Instance, (LPSTR)TESDialog::kDialogTemplate_UseReport, NULL, g_FormUseReport_DlgProc, (LPARAM)Form, true);
}

void TESDialog::ResetFormListControls()
{
	TESDialog::DeinitializeCSWindows();

	SendMessage(*g_HWND_CellView, 0x40E, 1, 1);			// for worldspaces
	SendMessage(*g_HWND_AIPackagesDlg, 0x41A, 0, 0);	// for AI packages

	TESDialog::InitializeCSWindows();
}

float TESDialog::GetDlgItemFloat( HWND Dialog, int ID )
{
	return cdeclCall<float>(0x00404A80, Dialog, ID);
}

void TESDialog::ShowDialogPopupMenu( HMENU Menu, POINT* Coords, HWND Parent, LPARAM Data )
{
	cdeclCall<void>(0x00443520, Menu, Coords, Parent, Data);
}

void TESDialog::SetDlgItemFloat( HWND Dialog, int ID, float Value, int DecimalPlaces )
{
	cdeclCall<void>(0x00404A90, Dialog, ID, Value, DecimalPlaces);
}

void TESDialog::UpdatePreviewWindows( bool RefreshRenderWindow /*= true*/ )
{
	cdeclCall<void>(0x00446F30, RefreshRenderWindow);
}

void TESDialog::ClampDlgEditField( HWND EditControl, float Min, float Max, bool NoDecimals /*= false*/, UInt32 DecimalPlaces /*= 2*/ )
{
	cdeclCall<void>(0x004042C0, EditControl, Min, Max, NoDecimals, DecimalPlaces);
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

void* TESListView::GetItemData( HWND hWnd, int Index )
{
	return cdeclCall<void*>(0x00403A30, hWnd, Index);
}

void TESListView::SetSelectedItem( HWND hWnd, int Index )
{
	cdeclCall<void>(0x00403B10, hWnd, Index);
}

void TESPreviewWindow::Initialize( TESBoundObject* Object )
{
	if (*g_HWND_PreviewWindow == NULL)
		SendMessage(*g_HWND_CSParent, WM_COMMAND, 40121, NULL);

	if (Object)
		cdeclCall<void>(0x00402BC0, Object);
}

bool Subwindow::Build( UInt32 TemplateID )
{
	return cdeclCall<bool>(0x00404EC0, TemplateID, this);
}

void Subwindow::TearDown( void )
{
	thisCall<void>(0x00405340, this);
}

Subwindow* Subwindow::CreateInstance( void )
{
	Subwindow* NewInstance = (Subwindow*)FormHeap_Allocate(sizeof(Subwindow));
	ZeroMemory(NewInstance, sizeof(Subwindow));
	return NewInstance;
}

void Subwindow::DeleteInstance( void )
{
	FormHeap_Free(this);
}
