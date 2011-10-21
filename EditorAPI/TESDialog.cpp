#include "TESForm.h"
#include "TESDialog.h"
#include "WorkspaceManager.h"
#include "Hooks\ScriptEditor.h"
#include "Hooks\TESFile.h"
#include "PathGridUndoManager.h"

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
UInt16*								g_TESFormIDListViewFormIDColumnWidth = (UInt16*)0x009EA32A;

TESDialogWindowHandleCollection		g_CustomMainWindowChildrenDialogs;
TESDialogWindowHandleCollection		g_DragDropSupportDialogs;

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
	if (!GetIsFormEditDialogCompatible(Form))
		return NULL;

	FormEditParam InitData(Form);
	return CreateDialogParamA(*g_TESCS_Instance,
							(LPCSTR)GetDialogTemplateForFormType(InitData.typeID),
							*g_HWND_CSParent,
							g_TESDialogFormEdit_DlgProc,
							(LPARAM)&InitData);
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
		g_PathGridUndoManager.SetCanReset(false);
		SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);
		SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);
		g_PathGridUndoManager.SetCanReset(true);
	}
	else
	{
		*g_RenderWindowUpdateViewPortFlag = 1;
		SendMessage(*g_HWND_RenderWindow, WM_TIMER, 1, NULL);
	}
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

void CSStartupManager::LoadStartupPlugin()
{
	if (g_INIManager->GetINIInt("LoadPluginOnStartup", "Extender::General"))
	{
		Hooks::kAutoLoadActivePluginOnStartup.WriteJump();

		const char* PluginName = g_INIManager->GetINIStr("StartupPluginName", "Extender::General");
		TESFile* File = _DATAHANDLER->LookupPluginByName(PluginName);

		if (File)
		{
			DebugPrint("Loading plugin '%s'", PluginName);
			CONSOLE->Indent();

			if (_stricmp(PluginName, "Oblivion.esm"))
				ToggleFlag(&File->fileFlags, TESFile::kFileFlag_Active, true);

			ToggleFlag(&File->fileFlags, TESFile::kFileFlag_Loaded, true);
			SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CD1, 0);

			CONSOLE->Exdent();
		}
		else if (strlen(PluginName) >= 1)
		{
			DebugPrint("Couldn't load plugin '%s' on startup - It doesn't exist!", PluginName);
		}

		Hooks::kAutoLoadActivePluginOnStartup.WriteBuffer();
	}
}

void CSStartupManager::LoadStartupScript()
{
	if (g_INIManager->GetINIInt("OpenScriptWindowOnStartup", "Extender::General"))
	{
		const char* ScriptID = g_INIManager->GetINIStr("StartupScriptEditorID", "Extender::General");
		if (strcmp(ScriptID, ""))
			TESDialog::ShowScriptEditorDialog(TESForm::LookupByEditorID(ScriptID));
		else
			TESDialog::ShowScriptEditorDialog(NULL);
	}
}

void CSStartupManager::LoadStartupWorkspace()
{
	if (g_INIManager->GetINIInt("SetWorkspaceOnStartup", "Extender::General"))
		g_WorkspaceManager.SelectWorkspace(g_INIManager->GetINIStr("DefaultWorkspacePath", "Extender::General"));
}

void __stdcall FormEnumerationWrapper::ReinitializeFormLists()
{
	TESDialog::DeinitializeCSWindows();

	SendMessage(*g_HWND_CellView, 0x40E, 1, 1);			// for worldspaces
	SendMessage(*g_HWND_AIPackagesDlg, 0x41A, 0, 0);	// for AI packages

	TESDialog::InitializeCSWindows();
}

bool FormEnumerationWrapper::GetUnmodifiedFormHiddenState()	// returns true when hidden
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
	UInt32 State = GetMenuState(ViewMenu, MAIN_VIEW_MODIFIEDRECORDS, MF_BYCOMMAND);

	return (State & MF_CHECKED);
}

bool FormEnumerationWrapper::GetDeletedFormHiddenState()
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
	UInt32 State = GetMenuState(ViewMenu, MAIN_VIEW_DELETEDRECORDS, MF_BYCOMMAND);

	return (State & MF_CHECKED);
}

bool __stdcall FormEnumerationWrapper::GetShouldEnumerateForm(TESForm* Form)
{
	if (GetUnmodifiedFormHiddenState() && (Form->formFlags & TESForm::kFormFlags_FromActiveFile) == 0)
		return false;		// skip addition
	else if (GetDeletedFormHiddenState() && (Form->formFlags & TESForm::kFormFlags_Deleted))
		return false;
	else
		return true;
}

bool __stdcall FormEnumerationWrapper::PerformListViewPrologCheck(UInt32 CallAddress)
{
	switch (CallAddress)
	{
	case 0x00445C88:
	case 0x00445DC8:
	case 0x00445E6E:
	case 0x00452FA8:
	case 0x00440FBD:
	case 0x0040A4BF:
	case 0x00412F7A:
	case 0x0043FDFF:
	case 0x00442576:
	case 0x00452409:
	case 0x00560DC2:
	case 0x00445E12:
	case 0x00445D81:
	case 0x004F00C3:
		return 1;
	default:
		return 0;
	}
}

void FormEnumerationWrapper::ToggleUnmodifiedFormVisibility()
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
	if (GetUnmodifiedFormHiddenState())
		CheckMenuItem(ViewMenu, MAIN_VIEW_MODIFIEDRECORDS, MF_UNCHECKED);
	else
		CheckMenuItem(ViewMenu, MAIN_VIEW_MODIFIEDRECORDS, MF_CHECKED);

	ReinitializeFormLists();
}
void FormEnumerationWrapper::ToggleDeletedFormVisibility()
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
	if (GetDeletedFormHiddenState())
		CheckMenuItem(ViewMenu, MAIN_VIEW_DELETEDRECORDS, MF_UNCHECKED);
	else
		CheckMenuItem(ViewMenu, MAIN_VIEW_DELETEDRECORDS, MF_CHECKED);

	ReinitializeFormLists();
}

void __stdcall FormEnumerationWrapper::ResetFormVisibility(void)
{
	if (GetUnmodifiedFormHiddenState())
		ToggleUnmodifiedFormVisibility();
	if (GetDeletedFormHiddenState())
		ToggleDeletedFormVisibility();
}

TESDialogWindowHandleCollection::HandleCollectionT::iterator TESDialogWindowHandleCollection::FindHandle(HWND Handle)
{
	for (HandleCollectionT::iterator Itr = WindowHandles.begin(); Itr != WindowHandles.end(); Itr++)
	{
		if (*Itr == Handle)
			return Itr;
	}

	return WindowHandles.end();
}

bool TESDialogWindowHandleCollection::RemoveHandle(HWND Handle)
{
	HandleCollectionT::iterator Match = FindHandle(Handle);
	if (Match != WindowHandles.end())
	{
		WindowHandles.erase(Match);
		return true;
	}
	else
		return false;
}