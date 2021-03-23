#include "TESForm.h"
#include "TESDialog.h"
#include "[Common]\CLIWrapper.h"
#include "Hooks\Hooks-Dialog.h"
#include "Achievements.h"

TESFile**					DataDialog::ActivePlugin = (TESFile**)0x00A0AA7C;
TESFile**					DataDialog::CurrentSelection = (TESFile**)0x00A0AA78;


tList<HWND>*				TESDialog::OpenDialogWindows = (tList<HWND>*)0x00A0B55C;
bool						TESDialog::PackageCellDragDropInProgress = false;
UInt8*						TESDialog::TESFormIDListViewDragDropInProgress = (UInt8*)0x00A0BE45;
UInt8*						TESDialog::ObjectWindowDragDropInProgress = (UInt8*)0x00A0BA68;
void*						TESDialog::LandscapeTextureSortComparator = (void*)0x0041E7D0;

HINSTANCE*					TESCSMain::Instance = (HINSTANCE*)0x00A0AF1C;
HWND*						TESCSMain::WindowHandle = (HWND*)0x00A0AF20;
HWND*						TESCSMain::MainToolbarHandle = (HWND*)0x00A0AFD0;
HMENU*						TESCSMain::MainMenuHandle = (HMENU*)0x00A0B630;
char**						TESCSMain::ActivePluginNameBuffer = (char**)0x00A0AF00;
UInt8*						TESCSMain::AllowAutoSaveFlag = (UInt8*)0x00A0B628;
UInt8*						TESCSMain::ExittingCSFlag = (UInt8*)0x00A0B63C;
UInt8*						TESCSMain::UnsavedChangesFlag = (UInt8*)0x00A0B13C;

const char*					TESCSMain::INIFilePath = (const char*)0x00A0ABB8;
HIMAGELIST*					TESCSMain::BoundObjectIcons = (HIMAGELIST*)0x00A0B158;
char**						TESCSMain::FileSelectionBuffer = (char**)0x00A0AF00;
char*						TESCSMain::ProfileFolderPath = (char*)0x00A89E68;

HWND*						TESObjectWindow::WindowHandle = (HWND*)0x00A0AF44;
HWND*						TESObjectWindow::FormListHandle = (HWND*)0x00A0BAA0;
HWND*						TESObjectWindow::TreeViewHandle = (HWND*)0x00A0BAA4;
HWND*						TESObjectWindow::SplitterHandle = (HWND*)0x00A0BA6C;
UInt8*						TESObjectWindow::Initialized = (UInt8*)0x00A0AF40;
UInt32*						TESObjectWindow::CurrentTreeNode = (UInt32*)0x00A0B6F8;
int*						TESObjectWindow::SortColumnArray = (int*)0x00A0B668;
DLGPROC						TESObjectWindow::DialogProc = (DLGPROC)0x00420240;
TESObjectWindow::TreeEntryInfo**
							TESObjectWindow::TreeEntryArray = (TESObjectWindow::TreeEntryInfo**)0x00A0B700;
HWND						TESObjectWindow::PrimaryObjectWindowHandle = nullptr;

HWND*						TESCellViewWindow::WindowHandle = (HWND*)0x00A0AF4C;
HWND*						TESCellViewWindow::ObjectListHandle = (HWND*)0x00A0AA00;
HWND*						TESCellViewWindow::CellListHandle = (HWND*)0x00A0AA34;
UInt8*						TESCellViewWindow::MainMenuState = (UInt8*)0x00A0AF48;
TESObjectCELL**				TESCellViewWindow::CurrentCellSelection = (TESObjectCELL**)0x00A0A9DC;
int*						TESCellViewWindow::ObjectListSortColumn = (int*)0x00A0A9D4;

ResponseEditorData**		ResponseEditorData::EditorCache = (ResponseEditorData**)0x00A10E2C;

HWND*						TESPreviewWindow::WindowHandle = (HWND*)0x00A0A71C;
TESPreviewControl**			TESPreviewWindow::PreviewControl = (TESPreviewControl**)0x00A0A730;
DLGPROC						TESPreviewWindow::DialogProc = (DLGPROC)0x00402F30;
TESObjectSTAT**				TESPreviewWindow::PreviewStatic = (TESObjectSTAT**)0x00A0A728;
TESObjectREFR**				TESPreviewWindow::PreviewRef = (TESObjectREFR**)0x00A0A72C;
DWORD*						TESPreviewWindow::InitialTickCount = (DWORD*)0x00A0A724;
HWND*						TESPreviewWindow::AnimationListHandle = (HWND*)0x00A0A720;

TESObjectSelection**		TESObjectSelection::PrimaryInstance = (TESRenderSelection**)0x00A0AF60;

SelectTopicWindowData**		SelectTopicWindowData::Singleton = (SelectTopicWindowData**)0x00A10EF0;
SelectQuestWindowData**		SelectQuestWindowData::Singleton = (SelectQuestWindowData**)0x00A10988;

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

FormEditParam::FormEditParam(TESForm* Form)
{
	form = Form;
	typeID = Form->formType;
}

UInt32 TESDialog::WriteBoundsToINI(HWND Handle, const char* WindowClassName)
{
	return cdeclCall<UInt32>(0x00417510, Handle, WindowClassName);
}

bool TESDialog::ReadBoundsFromINI(const char* WindowClassName, LPRECT OutRect)
{
	return cdeclCall<bool>(0x004176D0, WindowClassName, OutRect);
}

LRESULT TESCSMain::WriteToStatusBar(int PanelIndex, const char* Message)
{
	return cdeclCall<UInt32>(0x00431310, PanelIndex, Message);
}

void TESCSMain::InitializeCSWindows()
{
	cdeclCall<UInt32>(0x00430980);
}

void TESCSMain::DeinitializeCSWindows()
{
	cdeclCall<UInt32>(0x00431220);
}

void TESCSMain::SetTitleModified(bool State)
{
	cdeclCall<UInt32>(0x004306F0, State);
}

void TESCSMain::AutoSave()
{
	cdeclCall<UInt32>(0x004307C0);
}

void TESCSMain::InvokeMainMenuTool(int Identifier)
{
	SendMessage(*WindowHandle, WM_COMMAND, Identifier, NULL);
}

bool TESCSMain::ConfirmUnsavedChanges()
{
	return cdeclCall<bool>(0x00430EB0);
}

UInt32 TESDialog::GetDialogTemplateForFormType(UInt8 FormTypeID)
{
	// handle types unaccounted for in the vanilla function
	switch (FormTypeID)
	{
	case TESForm::kFormType_Package:
		return kDialogTemplate_Package;
	default:
		return cdeclCall<UInt32>(0x00442050, FormTypeID);
	}
}

BSExtraData* TESDialog::GetDialogExtraByType(HWND Dialog, UInt16 Type)
{
	return cdeclCall<BSExtraData*>(0x00442990, Dialog, Type);
}

TESForm* TESDialog::GetDialogExtraParam(HWND Dialog)
{
	return cdeclCall<TESForm*>(0x004429D0, Dialog);
}

TESForm* TESDialog::GetDialogExtraLocalCopy(HWND Dialog)
{
	return cdeclCall<TESForm*>(0x004429B0, Dialog);
}

bool TESDialog::GetIsWindowDragDropRecipient(UInt16 FormType, HWND hWnd)
{
	return cdeclCall<bool>(0x004433C0, FormType, hWnd);
}

bool TESDialog::SelectTESFileCommonDialog(HWND Parent, const char* SaveDir, bool SaveAsESM, char* FileNameOut, size_t OutSize)
{
	return cdeclCall<bool>(0x00446D40, Parent, SaveDir, SaveAsESM, FileNameOut, OutSize);
}

HWND TESDialog::ShowFormEditDialog(TESForm* Form)
{
	bool FormIDListViewForm = false;
	DLGPROC WndProc = GetFormEditDlgProc(Form, FormIDListViewForm);

	if (WndProc == nullptr || GetDialogTemplateForFormType(Form->formType) == 0)
		return nullptr;

	FormEditParam InitData(Form);
	HWND Dialog = BGSEEUI->ModelessDialog(*TESCSMain::Instance,
										  (LPSTR)GetDialogTemplateForFormType(InitData.typeID),
										  *TESCSMain::WindowHandle,
										  WndProc,
										  (LPARAM)&InitData,
										  true);

	if (FormIDListViewForm)
	{
		// attempt to select the form
		HWND ListView = GetDlgItem(Dialog, kFormList_TESFormIDListView);
		if (ListView)
		{
			int Index = TESListView::GetItemByData(ListView, Form);
			if (Index != -1)
			{
				TESListView::SetSelectedItem(ListView, Index);
				TESListView::ScrollToItem(ListView, Index);
			}
		}
	}

	return Dialog;
}

void TESDialog::ShowScriptEditorDialog(TESForm* InitScript)
{
	Script* AuxScript = CS_CAST(InitScript, TESForm, Script);
	componentDLLInterface::ScriptData* Data = nullptr;

	if (AuxScript)
		Data = new componentDLLInterface::ScriptData(AuxScript);

	RECT ScriptEditorLoc;
	TESDialog::ReadBoundsFromINI("Script Edit", &ScriptEditorLoc);
	cse::cliWrapper::interfaces::SE->InstantiateEditor(Data,
																		   ScriptEditorLoc.left,
																		   ScriptEditorLoc.top,
																		   ScriptEditorLoc.right,
																		   ScriptEditorLoc.bottom);

	cse::achievements::kPowerUser->UnlockTool(cse::achievements::AchievementPowerUser::kTool_ScriptEditor);
}

DLGPROC TESDialog::GetFormEditDlgProc(TESForm* Form, bool& FormIDListViewForm)
{
	FormIDListViewForm = false;

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
	case TESForm::kFormType_Package:
	case TESForm::kFormType_Sound:
	case TESForm::kFormType_LandTexture:
	case TESForm::kFormType_CombatStyle:
	case TESForm::kFormType_LoadScreen:
	case TESForm::kFormType_WaterForm:
	case TESForm::kFormType_AnimObject:
	case TESForm::kFormType_SubSpace:
	case TESForm::kFormType_EffectShader:
	case TESForm::kFormType_SigilStone:
	case TESForm::kFormType_REFR:
		return (DLGPROC)0x00447580;
	case TESForm::kFormType_Faction:
	case TESForm::kFormType_Race:
	case TESForm::kFormType_Class:
	case TESForm::kFormType_Skill:
	case TESForm::kFormType_EffectSetting:
	case TESForm::kFormType_GMST:
	case TESForm::kFormType_Global:
	case TESForm::kFormType_BirthSign:
	case TESForm::kFormType_Climate:
	case TESForm::kFormType_WorldSpace:
	case TESForm::kFormType_Hair:
	case TESForm::kFormType_Eyes:
	case TESForm::kFormType_Weather:
		FormIDListViewForm = true;
		return (DLGPROC)0x00448820;
	default:
		return nullptr;
	}
}

HWND TESDialog::ShowUseReportDialog(TESForm* Form)
{
	return BGSEEUI->ModelessDialog(*TESCSMain::Instance, (LPSTR)TESDialog::kDialogTemplate_UseReport, *TESCSMain::WindowHandle, (DLGPROC)0x00433FE0, (LPARAM)Form, true);
}

void TESDialog::ResetFormListControls()
{
	TESCSMain::DeinitializeCSWindows();

	SendMessage(*TESCellViewWindow::WindowHandle, 0x40E, 1, 1);			// for worldspaces
	SendMessage(*TESPackage::WindowHandle, 0x41A, 0, 0);				// for AI packages

	TESCSMain::InitializeCSWindows();
}

float TESDialog::GetDlgItemFloat(HWND Dialog, int ID)
{
	return cdeclCall<float>(0x00404A80, Dialog, ID);
}

void TESDialog::ShowDialogPopupMenu(HMENU Menu, POINT* Coords, HWND Parent, LPARAM Data)
{
	cdeclCall<void>(0x00443520, Menu, Coords, Parent, Data);
}

void TESDialog::SetDlgItemFloat(HWND Dialog, int ID, float Value, int DecimalPlaces)
{
	cdeclCall<void>(0x00404A90, Dialog, ID, Value, DecimalPlaces);
}

void TESDialog::ClampDlgEditField(HWND EditControl, float Min, float Max, bool NoDecimals /*= false*/, UInt32 DecimalPlaces /*= 2*/)
{
	cdeclCall<void>(0x004042C0, EditControl, Min, Max, NoDecimals, DecimalPlaces);
}

BaseExtraList* TESDialog::GetDialogExtraDataList(HWND Dialog)
{
	return cdeclCall<BaseExtraList*>(0x00442980, Dialog);
}

bool TESDialog::CallFormDialogMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LONG* outLong)
{
	return cdeclCall<bool>(0x00442BD0, hWnd, uMsg, wParam, lParam, outLong);
}

HWND TESDialog::GetActiveFormEditDialog(TESForm* Form)
{
	return cdeclCall<HWND>(0x004469D0, Form);
}

BaseExtraList* TESDialog::CreateDialogExtraDataList(HWND Dialog)
{
	return cdeclCall<BaseExtraList*>(0x004428C0, Dialog);
}

void TESDialog::AddDialogToOpenList(HWND Dialog)
{
	thisCall<void>(0x004E3900, OpenDialogWindows, Dialog);
}

void TESDialog::RemoveDialogFromOpenList(HWND Dialog)
{
	thisCall<void>(0x00452AE0, OpenDialogWindows, Dialog);
}

void TESDialog::DestroyDialogExtraDataList(HWND Dialog)
{
	cdeclCall<void>(0x00442950, Dialog);
}

bool TESDialog::ShowFileSelect(HWND parent, const char* relativePath, const char* filter, const char* title, const char* defaultExtension, LPOFNHOOKPROC proc, bool fileMustExist, bool saveVsOpen, char* filenameBuffer, UInt32 bufferSize)
{
	return cdeclCall<bool>(0x00446A30, parent, relativePath, filter, title, defaultExtension, proc, fileMustExist, saveVsOpen, filenameBuffer, bufferSize);
}

void TESComboBox::AddItem(HWND hWnd, const char* Text, void* Data, bool ResizeDroppedWidth)
{
	cdeclCall<UInt32>(0x00403540, hWnd, Text, Data, ResizeDroppedWidth);
}

void* TESComboBox::GetSelectedItemData(HWND hWnd)
{
	return cdeclCall<void*>(0x00403690, hWnd);
}

void TESComboBox::PopulateWithForms(HWND hWnd, UInt8 FormType, bool ClearItems, bool AddDefaultItem)
{
	cdeclCall<UInt32>(0x004456F0, hWnd, FormType, ClearItems, AddDefaultItem);
}

void TESComboBox::SetSelectedItemByData(HWND hWnd, void* Data)
{
	cdeclCall<void>(0x004036E0, hWnd, Data);
}

void TESComboBox::ClearItems(HWND hWnd)
{
	ComboBox_ResetContent(hWnd);
}

void* TESListView::GetSelectedItemData(HWND hWnd)
{
	return cdeclCall<void*>(0x00403C40, hWnd);
}

void* TESListView::GetItemData(HWND hWnd, int Index)
{
	return cdeclCall<void*>(0x00403A30, hWnd, Index);
}

void TESListView::SetSelectedItem(HWND hWnd, int Index)
{
	cdeclCall<void>(0x00403B10, hWnd, Index);
}

int TESListView::GetItemByData(HWND hWnd, void* Data)
{
	return cdeclCall<int>(0x004039E0, hWnd, Data);
}

void TESListView::ScrollToItem(HWND hWnd, int Index)
{
	cdeclCall<void>(0x00403BA0, hWnd, Index);
}

void TESListView::InsertItem(HWND hWnd, void* Data, bool ImageCallback /*= false*/, int Index /*= -1*/)
{
	cdeclCall<void>(0x004038F0, hWnd, Data, ImageCallback, Index);
}

void TESListView::AddColumnHeader(HWND hWnd, int Index, const char* Name, int Width, UInt32 Format /*= LVCFMT_LEFT*/)
{
	cdeclCall<void>(0x00403810, hWnd, Index, Name, Width, Format);
}

void TESListView::ClearColumnHeaders(HWND hWnd)
{
	ClearItems(hWnd);
	while (ListView_DeleteColumn(hWnd, 0))
		;//
}

void TESListView::ClearItems(HWND hWnd)
{
	ListView_DeleteAllItems(hWnd);
}

UInt32 TESListView::GetSelectedItemCount(HWND hWnd)
{
	int Selection = -1, Count = 0;
	do
	{
		Selection = ListView_GetNextItem(hWnd, Selection, LVNI_SELECTED);
		if (Selection != -1)
			Count++;
	} while (Selection != -1);

	return Count;
}
void TESPreviewWindow::SetSourceObject(TESBoundObject* Object)
{
	if (Object)
		cdeclCall<void>(0x00402BC0, Object);
}

void TESPreviewWindow::HandleResize(HWND PreviewWindow)
{
	cdeclCall<void>(0x004025A0, PreviewWindow);
}

void TESPreviewWindow::Initialize(HWND PreviewWindow)
{
	cdeclCall<void>(0x00402910, PreviewWindow);
}

void TESPreviewWindow::Deinitialize(HWND PreviewWindow)
{
	cdeclCall<void>(0x00402B00, PreviewWindow);
}

void TESPreviewWindow::Display(TESBoundObject* Object)
{
	if (*WindowHandle == nullptr)
		SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_PreviewWindow, NULL);

	if (Object)
		SetSourceObject(Object);
}

bool Subwindow::Build(UInt32 TemplateID)
{
	return cdeclCall<bool>(0x00404EC0, TemplateID, this);
}

void Subwindow::TearDown(void)
{
	thisCall<void>(0x00405340, this);
}

Subwindow* Subwindow::CreateInstance(void)
{
	Subwindow* NewInstance = (Subwindow*)FormHeap_Allocate(sizeof(Subwindow));
	ZeroMemory(NewInstance, sizeof(Subwindow));
	return NewInstance;
}

void Subwindow::DeleteInstance(void)
{
	FormHeap_Free(this);
}

void TESObjectSelection::AddToSelection(TESForm* Form, bool AddSelectionBox)
{
	thisCall<UInt32>(0x00512730, this, Form, AddSelectionBox);
}

void TESObjectSelection::RemoveFromSelection(TESForm* Form, bool RemoveSelectionBox)
{
	thisCall<UInt32>(0x00512830, this, Form, RemoveSelectionBox);
}

void TESObjectSelection::ClearSelection(bool RemoveSelectionBox)
{
	thisCall<UInt32>(0x00511C20, this, RemoveSelectionBox);
}

TESObjectSelection* TESObjectSelection::CreateInstance(TESObjectSelection* Source)
{
	TESObjectSelection* NewInstance = (TESObjectSelection*)FormHeap_Allocate(sizeof(TESObjectSelection));
	thisCall<UInt32>(0x00511A20, NewInstance);

	if (Source && Source->selectionCount)
	{
		for (SelectedObjectsEntry* Itr = Source->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			NewInstance->AddToSelection(Itr->Data);
	}

	return NewInstance;
}

void TESObjectSelection::DeleteInstance()
{
	this->ClearSelection(false);
	FormHeap_Free(this);
}

void TESObjectSelection::CalculatePositionVectorSum(void)
{
	thisCall<void>(0x00511A70, this);
}

bool TESObjectSelection::HasObject(TESForm* Form)
{
	return thisCall<bool>(0x00511CC0, this, Form);
}

void TESObjectSelection::CalculateBounds(void)
{
	thisCall<void>(0x00511B20, this);
}

void TESFileFormListWindow::Show(HWND Parent, TESFile* File)
{
	if (Parent == nullptr)
		Parent = *TESCSMain::WindowHandle;

	BGSEEUI->ModalDialog(*TESCSMain::Instance, MAKEINTRESOURCE(TESDialog::kDialogTemplate_TESFileDetails), Parent, (DLGPROC)0x00410280, (LPARAM)File, true);
}

void TESCellViewWindow::SetCellSelection(TESObjectCELL* Cell)
{
	cdeclCall<void>(0x00409070, Cell);
}

void TESCellViewWindow::RefreshObjectList(void)
{
	SendMessage(*WindowHandle, 0x40F, NULL, NULL);
}

void TESCellViewWindow::RefreshCellList(bool RefreshWorldspaces /*= false*/)
{
	// the vanilla handler only updates when the no. of list view items is not equal to the current worldspace's exterior/interior cell count
	SendMessage(*CellListHandle, LVM_DELETEALLITEMS, NULL, NULL);
	SendMessage(*WindowHandle, 0x40E, RefreshWorldspaces, NULL);
}

void TESCellViewWindow::UpdateCurrentWorldspace()
{
	cdeclCall<void>(0x004086E0);
}

void TESCellViewWindow::SetCurrentCell(Vector3* Position, bool ReloadObjectList)
{
	cdeclCall<void>(0x00409170, Position, (UInt32)ReloadObjectList);
}

void TESObjectWindow::RefreshFormList(void)
{
	SendMessage(*TESObjectWindow::WindowHandle, 0x41A, NULL, NULL);
}

void TESObjectWindow::SetSplitterEnabled(HWND Splitter, bool State)
{
	cdeclCall<void>(0x004044D0, Splitter, State);
}

void TESObjectWindow::PerformLimitedInit(HWND ObjectWindow)
{
	SME_ASSERT(ObjectWindow == *WindowHandle);

	BGSEEUI->GetInvalidationManager()->Push(*TreeViewHandle);
	BGSEEUI->GetInvalidationManager()->Push(*FormListHandle);
	{
		cdeclCall<void>(0x00420130, *TESObjectWindow::TreeViewHandle);
		int TreeIndex = cdeclCall<int>(0x0041FA00,
			*TESObjectWindow::TreeViewHandle,
			SendMessage(*TESObjectWindow::TreeViewHandle, TVM_GETNEXTITEM, 0, 0),
			0, 0, 0);
		SendMessage(*TESObjectWindow::TreeViewHandle, TVM_SELECTITEM, 9u, TreeIndex);
		thisCall<void>(0x00414C90, TESObjectWindow::TreeEntryArray[0], 0);
		SendMessage(*TESObjectWindow::FormListHandle, LVM_SORTITEMS, 1, (LPARAM)cse::hooks::ObjectWindowFormListComparator);
		SetWindowLong(*TESObjectWindow::FormListHandle,
			GWL_STYLE,
			GetWindowLong(*TESObjectWindow::FormListHandle, GWL_STYLE) | LVS_SHAREIMAGELISTS);
		SendMessage(*TESObjectWindow::FormListHandle, LVM_SETIMAGELIST, 1u, (LPARAM)*TESCSMain::BoundObjectIcons);
		cdeclCall<void>(0x00404F30, *TESObjectWindow::SplitterHandle, *TESObjectWindow::TreeViewHandle, *TESObjectWindow::FormListHandle);
		SendMessage(*TESObjectWindow::FormListHandle, LVM_SETEXTENDEDLISTVIEWSTYLE, 0x421u, 0x421u);
	}
	BGSEEUI->GetInvalidationManager()->Pop(*FormListHandle);
	BGSEEUI->GetInvalidationManager()->Pop(*TreeViewHandle);
	BGSEEUI->GetInvalidationManager()->Redraw(ObjectWindow);
}

void TESObjectWindow::PerformLimitedDeinit(HWND ObjectWindow)
{
	SME_ASSERT(ObjectWindow == *WindowHandle);

	int FirstNode = SendMessage(*TESObjectWindow::TreeViewHandle, TVM_GETNEXTITEM, 0, 0);
	cdeclCall<void>(0x0041FE40, *TESObjectWindow::TreeViewHandle, FirstNode);
	SendMessage(*TESObjectWindow::TreeViewHandle, TVM_DELETEITEM, 0, -65536);
	SendMessage(*TESObjectWindow::FormListHandle, LVM_DELETEALLITEMS, NULL, NULL);
}

int CALLBACK TreeViewSortComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TESObjectWindow::TreeViewItemData* Item1 = (TESObjectWindow::TreeViewItemData*)lParam1;
	TESObjectWindow::TreeViewItemData* Item2 = (TESObjectWindow::TreeViewItemData*)lParam2;

	return _stricmp(Item1->name, Item2->name);
}

void TESObjectWindow::UpdateTreeChildren(HWND ObjectWindow)
{
	SME_ASSERT(ObjectWindow == *WindowHandle);

	// the window subclasser's windows hook has an out-sized performance penalty for
	// insertions in tree view controls (probably also has to do with the way it's used in this dialog)
	// so we have to temporarily suspend the hook when this code is executing
	BGSEEUI->GetInvalidationManager()->Push(*TreeViewHandle);
	BGSEEUI->GetInvalidationManager()->Push(*FormListHandle);
	BGSEEUI->GetSubclasser()->SuspendHooks();
	{
		for (int i = 0; i < TESObjectWindow::TreeEntryInfo::kTreeEntryCount; i++)
		{
			TESObjectWindow::TreeEntryInfo* Info = TESObjectWindow::TreeEntryArray[i];
			for (TESObjectWindow::TreeEntryInfo::FormListT::Iterator Itr = Info->formList.Begin();
				 Itr.Get() && !Itr.End();
				 ++Itr)
			{
				TESForm* Form = Itr.Get();
				cdeclCall<void>(0x00422310, Form);
			}
		}

		TVSORTCB SortData = { 0 };
		SortData.hParent = nullptr;
		SortData.lpfnCompare = TreeViewSortComparator;

		TreeView_SortChildrenCB(*TESObjectWindow::TreeViewHandle, &SortData, nullptr);

		for (HTREEITEM i = TreeView_GetRoot(*TESObjectWindow::TreeViewHandle); i; i = TreeView_GetNextItem(*TESObjectWindow::TreeViewHandle, i, TVGN_NEXT))
		{
			SortData.hParent = i;
			SortData.lpfnCompare = TreeViewSortComparator;

			TreeView_SortChildrenCB(*TESObjectWindow::TreeViewHandle, &SortData, nullptr);
		}
	}
	BGSEEUI->GetSubclasser()->ResumeHooks();
	BGSEEUI->GetInvalidationManager()->Pop(*FormListHandle);
	BGSEEUI->GetInvalidationManager()->Pop(*TreeViewHandle);
	BGSEEUI->GetInvalidationManager()->Redraw(ObjectWindow);
}

bool TESObjectWindow::GetMinimized(void)
{
	return (GetMenuState(*TESCSMain::MainMenuHandle, 40199, MF_BYCOMMAND) & MF_CHECKED) == false;
}

void TESObjectWindow::InitializeSplitter(HWND Splitter, HWND TreeView, HWND ListView)
{
	cdeclCall<void>(0x00404F30, Splitter, TreeView, ListView);
}

void* TESTreeView::GetItemData(HWND hWnd, HTREEITEM Item)
{
	return cdeclCall<void*>(0x0041F990, hWnd, Item);
}

void SelectTopicWindowData::RefreshListView(HWND Dialog)
{
	cdeclCall<void>(0x004EFFB0, Dialog, this, nullptr);
}

void SelectQuestWindowData::RefreshListView(HWND Dialog)
{
	cdeclCall<void>(0x004DEE30, Dialog, this, nullptr);
}

bool RefSelectControlDefaultComparator(TESObjectREFR* Ref, void* UserData)
{
	UInt32 OnlyPersistent = (UInt32)UserData;
	if (Ref == nullptr || Ref->IsDeleted() || Ref->IsTemporary() || (OnlyPersistent && Ref->IsQuestItem() == false))
		return false;
	else
		return true;
}

TESObjectREFR* RefSelectControl::ShowSelectReferenceDialog(HWND Parent, TESObjectREFR* DefaultSelection, bool OnlyPersistent)
{
	TESObjectREFR* Selection = cdeclCall<TESObjectREFR*>(0x0044D660, Parent, DefaultSelection, RefSelectControlDefaultComparator, (UInt32)OnlyPersistent);
	if (Parent)
	{
		// usual kind of black magic one needs to perform because Windows
		SetForegroundWindow(*TESCSMain::WindowHandle);
		SetForegroundWindow(Parent);
		SetActiveWindow(Parent);
	}

	return Selection;
}

