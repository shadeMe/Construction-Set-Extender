#include "CSEUIManager.h"
#include "CSEMain.h"
#include "Construction Set Extender_Resource.h"
#include "CSEAuxiliaryViewport.h"
#include "[Common]\CLIWrapper.h"
#include "CSEWorkspaceManager.h"
#include "CSERenderWindowPainter.h"
#include "CSERenderWindowFlyCamera.h"
#include "CSEAchievements.h"
#include "CSEHallOfFame.h"
#include "Hooks\Hooks-AssetSelector.h"
#include "Hooks\Hooks-Dialog.h"
#include "Hooks\Hooks-Renderer.h"
#include "Hooks\Hooks-LOD.h"
#include "CSERenderSelectionGroupManager.h"
#include "CSEPathGridUndoManager.h"
#include "CSInterop.h"
#include "CSEGlobalClipboard.h"
#include "CSEFormUndoStack.h"
#include "CSEDialogImposterManager.h"
#include "CSEObjectPaletteManager.h"

#include <BGSEEToolBox.h>
#include <BGSEEScript\CodaVM.h>

namespace ConstructionSetExtender
{
	namespace UIManager
	{
#define ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID				0x99

		CSEFilterableFormListManager												CSEFilterableFormListManager::Instance;
		CSEFilterableFormListManager::FilterableWindowData::WindowTimerMapT			CSEFilterableFormListManager::FilterableWindowData::FilterTimerTable;
		CSEFilterableFormListManager::FilterableWindowData::FormListFilterDataMapT	CSEFilterableFormListManager::FilterableWindowData::FormListDataTable;

		CSEFilterableFormListManager::FilterableWindowData::FilterableWindowData( HWND Parent, HWND EditBox, HWND FormList, HWND Label, int TimerPeriod ) :
			ParentWindow(Parent),
			FilterEditBox(EditBox),
			FormListView(FormList),
			FormListWndProc(NULL),
			FilterLabel(Label),
			FilterString(""),
			TimerPeriod(TimerPeriod),
			TimeCounter(-1),
			Enabled(true)
		{
			SME_ASSERT(ParentWindow && FilterEditBox && FormListView);

			HookFormList();
			CreateTimer();
			Flags = kFlags_SearchEditorID | kFlags_SearchDescription | kFlags_SearchName;
		}

		CSEFilterableFormListManager::FilterableWindowData::~FilterableWindowData()
		{
			UnhookFormList();
			DestroyTimer();
		}

		LRESULT CALLBACK CSEFilterableFormListManager::FilterableWindowData::FormListSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			SME_ASSERT(FormListDataTable.count(hWnd));
			FilterableWindowData* UserData = FormListDataTable[hWnd];

			switch (uMsg)
			{
			case LVM_INSERTITEMA:
			case LVM_INSERTITEMW:
				LVITEM* Item = (LVITEM*)lParam;

				if (UserData->Enabled && Item->lParam)
				{
					TESForm* Form = (TESForm*)Item->lParam;
					if (Form)
					{
						if (UserData->FilterForm(Form) == false)
							return -1;
					}
				}

				break;
			}

			return CallWindowProc(UserData->FormListWndProc, hWnd, uMsg, wParam, lParam);
		}

#define IDC_CSEFILTERABLEFORMLIST_REGEX				9009
#define IDC_CSEFILTERABLEFORMLIST_EDITORID			9010
#define IDC_CSEFILTERABLEFORMLIST_NAME				9011
#define IDC_CSEFILTERABLEFORMLIST_DESCRIPTION		9012
#define IDC_CSEFILTERABLEFORMLIST_FORMID			9013

		bool CSEFilterableFormListManager::FilterableWindowData::HandleMessages( UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == FilterEditBox)
				{
					TimeCounter = 0;
				}

				break;
			case WM_RBUTTONUP:
				{
					POINT CursorLoc = { 0 };
					GetCursorPos(&CursorLoc);

					HWND WindowFromPoint = SME::UIHelpers::WinSpy::WindowFromPointEx(CursorLoc, FALSE);
					if (WindowFromPoint == FilterLabel)
						HandlePopupMenu(ParentWindow, CursorLoc.x, CursorLoc.y);
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID:
					if (TimeCounter && TimeCounter != -1 && TimeCounter >= TimerPeriod * 1.5)
					{
						char Buffer[0x200] = {0};
						GetWindowText(FilterEditBox, (LPSTR)Buffer, sizeof(Buffer));

						if (strlen(Buffer))
						{
							FilterString = Buffer;

							if (HasRegEx() == false)
								SME::StringHelpers::MakeLower(FilterString);
						}
						else
						{
							FilterString = "";
						}

						TimeCounter = -1;
						return true;
					}
					else if (TimeCounter != -1)
					{
						TimeCounter += TimerPeriod;
					}

					break;
				}

				break;
			}

			return false;
		}

		bool CSEFilterableFormListManager::FilterableWindowData::FilterForm(TESForm* Form)
		{
			SME_ASSERT(Form);

			// fallthrough if no filters are active
			if (HasEditorID() == false && HasName() == false && HasDescription() == false && HasFormID() == false)
				return true;
			else if (FilterString.length() == 0)
				return true;

			std::string EditorID, FullName, Description, FormIDStr;
			TESFullName* FullNameCmpt = NULL;
			TESDescription* DescriptionCmpt = NULL;

			if (Form->IsReference())
			{
				TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
				if (Reference->baseForm)
				{
					FullNameCmpt = CS_CAST(Reference->baseForm, TESForm, TESFullName);
					DescriptionCmpt = CS_CAST(Reference->baseForm, TESForm, TESDescription);
				}
			}

			if (Form->IsReference() == false && FullNameCmpt == NULL)
				FullNameCmpt = CS_CAST(Form, TESForm, TESFullName);

			if (Form->IsReference() == false && DescriptionCmpt == NULL)
				DescriptionCmpt = CS_CAST(Form, TESForm, TESDescription);

			if (FullNameCmpt && FullNameCmpt->name.c_str())
				FullName = FullNameCmpt->name.c_str();

			if (DescriptionCmpt && DescriptionCmpt->description.c_str())
				Description = DescriptionCmpt->description.c_str();

			if (Form->editorID.c_str())
				EditorID = Form->GetEditorID();
			else if (Form->IsReference())
			{
				TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
				if (Reference->baseForm && Reference->baseForm->GetEditorID())
					EditorID = Reference->baseForm->GetEditorID();
			}

			char Buffer[50] = { 0 };
			FORMAT_STR(Buffer, "%08x", Form->formID);
			FormIDStr = Buffer;

			bool CanAdd = false;

			if (HasRegEx())
			{
				try
				{
					std::regex Expr(FilterString);
					std::smatch Results;

					while (true)
					{
						if (HasEditorID() && std::regex_search(EditorID, Results, Expr))
						{
							CanAdd = true;
							break;
						}

						if (HasName() && std::regex_search(FullName, Results, Expr))
						{
							CanAdd = true;
							break;
						}

						if (HasDescription() && std::regex_search(Description, Results, Expr))
						{
							CanAdd = true;
							break;
						}

						if (HasFormID() && std::regex_search(FormIDStr, Results, Expr))
						{
							CanAdd = true;
							break;
						}

						break;
					}
				}
				catch (std::exception& e)
				{
					BGSEECONSOLE_MESSAGE("An error occurred while matching the regular expresssion filter string. Exception - %s", e.what());
					FilterString.clear();
				}
			}
			else
			{
				SME::StringHelpers::MakeLower(EditorID);
				SME::StringHelpers::MakeLower(FullName);
				SME::StringHelpers::MakeLower(Description);
				SME::StringHelpers::MakeLower(FormIDStr);

				while (true)
				{
					if (HasEditorID() && EditorID.find(FilterString) != std::string::npos)
					{
						CanAdd = true;
						break;
					}

					if (HasName() && FullName.find(FilterString) != std::string::npos)
					{
						CanAdd = true;
						break;
					}

					if (HasDescription() && Description.find(FilterString) != std::string::npos)
					{
						CanAdd = true;
						break;
					}

					if (HasFormID() && FormIDStr.find(FilterString) != std::string::npos)
					{
						CanAdd = true;
						break;
					}

					break;
				}
			}

			return CanAdd;
		}

		void CSEFilterableFormListManager::FilterableWindowData::HandlePopupMenu(HWND Parent, int X, int Y)
		{
			HMENU Popup = CreatePopupMenu();
			MENUITEMINFO Item = { 0 };
			Item.cbSize = sizeof(MENUITEMINFO);
			Item.fMask = MIIM_STATE | MIIM_TYPE | MIIM_ID;
			Item.fType = MFT_STRING;

			Item.fState = MFS_ENABLED;
			Item.dwTypeData = "Use Regular Expressions";
			Item.cch = strlen((const char*)Item.dwTypeData);
			if (HasRegEx())
				Item.fState |= MFS_CHECKED;
			Item.wID = IDC_CSEFILTERABLEFORMLIST_REGEX;
			InsertMenuItem(Popup, -1, TRUE, &Item);

			InsertMenu(Popup, -1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);

			Item.fState = MFS_ENABLED;
			Item.dwTypeData = "Search EditorID";
			Item.cch = strlen((const char*)Item.dwTypeData);
			if (HasEditorID())
				Item.fState |= MFS_CHECKED;
			Item.wID = IDC_CSEFILTERABLEFORMLIST_EDITORID;
			InsertMenuItem(Popup, -1, TRUE, &Item);

			Item.fState = MFS_ENABLED;
			Item.dwTypeData = "Search Name";
			Item.cch = strlen((const char*)Item.dwTypeData);
			if (HasName())
				Item.fState |= MFS_CHECKED;
			Item.wID = IDC_CSEFILTERABLEFORMLIST_NAME;
			InsertMenuItem(Popup, -1, TRUE, &Item);

			Item.fState = MFS_ENABLED;
			Item.dwTypeData = "Search Description";
			Item.cch = strlen((const char*)Item.dwTypeData);
			if (HasDescription())
				Item.fState |= MFS_CHECKED;
			Item.wID = IDC_CSEFILTERABLEFORMLIST_DESCRIPTION;
			InsertMenuItem(Popup, -1, TRUE, &Item);

			Item.fState = MFS_ENABLED;
			Item.dwTypeData = "Search FormID";
			Item.cch = strlen((const char*)Item.dwTypeData);
			if (HasFormID())
				Item.fState |= MFS_CHECKED;
			Item.wID = IDC_CSEFILTERABLEFORMLIST_FORMID;
			InsertMenuItem(Popup, -1, TRUE, &Item);

			switch (TrackPopupMenu(Popup, TPM_RETURNCMD, X, Y, NULL, Parent, NULL))
			{
			case IDC_CSEFILTERABLEFORMLIST_REGEX:
				SME::MiscGunk::ToggleFlag(&Flags, kFlags_RegEx, HasRegEx() == false);
				break;
			case IDC_CSEFILTERABLEFORMLIST_EDITORID:
				SME::MiscGunk::ToggleFlag(&Flags, kFlags_SearchEditorID, HasEditorID() == false);
				break;
			case IDC_CSEFILTERABLEFORMLIST_NAME:
				SME::MiscGunk::ToggleFlag(&Flags, kFlags_SearchName, HasName() == false);
				break;
			case IDC_CSEFILTERABLEFORMLIST_DESCRIPTION:
				SME::MiscGunk::ToggleFlag(&Flags, kFlags_SearchDescription, HasDescription() == false);
				break;
			case IDC_CSEFILTERABLEFORMLIST_FORMID:
				SME::MiscGunk::ToggleFlag(&Flags, kFlags_SearchFormID, HasFormID() == false);
				break;
			}

			DestroyMenu(Popup);
		}

		void CSEFilterableFormListManager::FilterableWindowData::SetEnabledState(bool State)
		{
			Enabled = State;
		}

		bool CSEFilterableFormListManager::FilterableWindowData::operator==(HWND FilterEditBox)
		{
			return this->FilterEditBox == FilterEditBox;
		}

		void CSEFilterableFormListManager::FilterableWindowData::CreateTimer(void) const
		{
			if (FilterTimerTable.count(ParentWindow) == 0)
			{
				FilterTimerTable[ParentWindow] = 1;
				SetTimer(ParentWindow, ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID, TimerPeriod, NULL);
			}
			else
				FilterTimerTable[ParentWindow]++;
		}

		void CSEFilterableFormListManager::FilterableWindowData::DestroyTimer(void) const
		{
			SME_ASSERT(FilterTimerTable.count(ParentWindow));

			if (--FilterTimerTable[ParentWindow] == 0)
			{
				KillTimer(ParentWindow, ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID);
				FilterTimerTable.erase(ParentWindow);
			}
		}

		void CSEFilterableFormListManager::FilterableWindowData::HookFormList(void)
		{
			FormListWndProc = (WNDPROC)SetWindowLongPtr(FormListView, GWL_WNDPROC, (LONG)FormListSubclassProc);
			FormListDataTable[FormListView] = this;
		}

		void CSEFilterableFormListManager::FilterableWindowData::UnhookFormList(void)
		{
			SetWindowLongPtr(FormListView, GWL_WNDPROC, (LONG)FormListWndProc);
			FormListDataTable.erase(FormListView);
		}

		CSEFilterableFormListManager::CSEFilterableFormListManager() :
			ActiveFilters()
		{
			;//
		}

		CSEFilterableFormListManager::~CSEFilterableFormListManager()
		{
			for each (auto Itr in ActiveFilters)
				delete Itr;

			ActiveFilters.clear();
		}

		bool CSEFilterableFormListManager::Register(HWND FilterEdit, HWND FilterLabel, HWND FormList, HWND ParentWindow, int TimePeriod /*= 500*/)
		{
			SME_ASSERT(ParentWindow && FormList);
			SME_ASSERT(FilterEdit && FilterLabel);

			if (Lookup(FilterEdit) == NULL)
			{
				ActiveFilters.push_back(new FilterableWindowData(ParentWindow, FilterEdit, FormList, FilterLabel, TimePeriod));
				return true;
			}

			return false;
		}

		void CSEFilterableFormListManager::Unregister(HWND FilterEdit)
		{
			SME_ASSERT(FilterEdit);

			for (auto Itr = ActiveFilters.begin(); Itr != ActiveFilters.end(); Itr++)
			{
				if (**Itr == FilterEdit)
				{
					delete *Itr;
					ActiveFilters.erase(Itr);
					return;
				}
			}
		}

		bool CSEFilterableFormListManager::HandleMessages(HWND FilterEdit, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			SME_ASSERT(FilterEdit);

			FilterableWindowData* Data = Lookup(FilterEdit);
			if (Data)
			{
				return Data->HandleMessages(uMsg, wParam, lParam);
			}

			return false;
		}

		void CSEFilterableFormListManager::SetEnabledState(HWND FilterEdit, bool State)
		{
			SME_ASSERT(FilterEdit);

			FilterableWindowData* Data = Lookup(FilterEdit);
			if (Data)
			{
				return Data->SetEnabledState(State);
			}
		}

		CSEFilterableFormListManager::FilterableWindowData* CSEFilterableFormListManager::Lookup(HWND FilterEdit)
		{
			for each (auto Itr in ActiveFilters)
			{
				if (*Itr == FilterEdit)
					return Itr;
			}

			return NULL;
		}

		CSEFormEnumerationManager		CSEFormEnumerationManager::Instance;

		CSEFormEnumerationManager::CSEFormEnumerationManager() :
			VisibilityDeletedForms(true),
			VisibilityUnmodifiedForms(true)
		{
			;//
		}

		CSEFormEnumerationManager::~CSEFormEnumerationManager()
		{
			;//
		}

		inline bool CSEFormEnumerationManager::GetVisibleDeletedForms( void ) const
		{
			return VisibilityDeletedForms == true;
		}

		inline bool CSEFormEnumerationManager::GetVisibleUnmodifiedForms( void ) const
		{
			return VisibilityUnmodifiedForms == true;
		}

		bool CSEFormEnumerationManager::ToggleVisibilityUnmodifiedForms( void )
		{
			VisibilityUnmodifiedForms = (VisibilityUnmodifiedForms == false);
			TESDialog::ResetFormListControls();

			return VisibilityUnmodifiedForms;
		}

		bool CSEFormEnumerationManager::ToggleVisibilityDeletedForms( void )
		{
			VisibilityDeletedForms = (VisibilityDeletedForms == false);
			TESDialog::ResetFormListControls();

			return VisibilityDeletedForms;
		}

		bool CSEFormEnumerationManager::GetShouldEnumerate( TESForm* Form )
		{
			if (Form && (Form->formFlags & TESForm::kFormFlags_FromActiveFile) == 0 && GetVisibleUnmodifiedForms() == false)
				return false;
			else if (Form && (Form->formFlags & TESForm::kFormFlags_Deleted) && GetVisibleDeletedForms() == false)
				return false;
			else
				return true;
		}

		void CSEFormEnumerationManager::ResetVisibility( void )
		{
			VisibilityDeletedForms = true;
			VisibilityUnmodifiedForms = true;
		}

		int CSEFormEnumerationManager::CompareActiveForms( TESForm* FormA, TESForm* FormB, int OriginalResult )
		{
			int Result = OriginalResult;
			bool Enabled = Settings::Dialogs::kSortFormListsByActiveForm.GetData().i;

			if (FormA && FormB)
			{
				bool ActiveFormA = (FormA->formFlags & TESForm::kFormFlags_FromActiveFile);
				bool ActiveFormB = (FormB->formFlags & TESForm::kFormFlags_FromActiveFile);

				if (Enabled)
				{
					if (ActiveFormA == true && ActiveFormB == false)
						Result = -1;
					else if (ActiveFormA == false && ActiveFormB == true)
						Result = 1;
				}
			}

			return Result;
		}

		CSECellViewExtraData::~CSECellViewExtraData()
		{
			;//
		}

		CSECellViewExtraData::CSECellViewExtraData() :
			BGSEditorExtender::BGSEEWindowExtraData(),
			RefFilterEditBox(),
			RefFilterLabel(),
			XLabel(),
			YLabel(),
			XEdit(),
			YEdit(),
			GoBtn(),
			CellFilterEditBox(),
			CellFilterLabel()
		{
			;//
		}

		int CALLBACK CSECellViewExtraData::CustomFormListComparator( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
		{
			int Result = 0;

			TESObjectREFR* FormA = (TESObjectREFR*)lParam1;
			TESObjectREFR* FormB = (TESObjectREFR*)lParam2;

			if (FormA && FormB)
			{
				switch ((UInt32)abs(lParamSort))
				{
				case kExtraRefListColumn_Persistent:
					if ((bool)(FormA->formFlags & TESForm::kFormFlags_QuestItem) == true &&
						(bool)(FormB->formFlags & TESForm::kFormFlags_QuestItem) == false)
					{
						Result = -1;
					}
					else if ((bool)(FormA->formFlags & TESForm::kFormFlags_QuestItem) == false &&
						(bool)(FormB->formFlags & TESForm::kFormFlags_QuestItem) == true)
					{
						Result = 1;
					}

					break;
				case kExtraRefListColumn_Disabled:
					if ((bool)(FormA->formFlags & TESForm::kFormFlags_Disabled) == true &&
						(bool)(FormB->formFlags & TESForm::kFormFlags_Disabled) == false)
					{
						Result = -1;
					}
					else if ((bool)(FormA->formFlags & TESForm::kFormFlags_Disabled) == false &&
						(bool)(FormB->formFlags & TESForm::kFormFlags_Disabled) == true)
					{
						Result = 1;
					}

					break;
				case kExtraRefListColumn_VWD:
					if ((bool)(FormA->formFlags & TESForm::kFormFlags_VisibleWhenDistant) == true &&
						(bool)(FormB->formFlags & TESForm::kFormFlags_VisibleWhenDistant) == false)
					{
						Result = -1;
					}
					else if ((bool)(FormA->formFlags & TESForm::kFormFlags_VisibleWhenDistant) == false &&
						(bool)(FormB->formFlags & TESForm::kFormFlags_VisibleWhenDistant) == true)
					{
						Result = 1;
					}

					break;
				case kExtraRefListColumn_EnableParent:
					{
						BSExtraData* AxData = FormA->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
						BSExtraData* BxData = FormB->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);

						if (AxData && BxData == NULL)
							Result = -1;
						else if (AxData == NULL && BxData)
							Result = 1;
						else if (AxData && BxData)
						{
							ExtraEnableStateParent* AxParent = CS_CAST(AxData, BSExtraData, ExtraEnableStateParent);
							ExtraEnableStateParent* BxParent = CS_CAST(BxData, BSExtraData, ExtraEnableStateParent);
							SME_ASSERT(AxParent->parent && BxParent->parent);

							if (AxParent->parent->formID < BxParent->parent->formID)
								Result = -1;
							else if (AxParent->parent->formID > BxParent->parent->formID)
								Result = 1;
						}
					}

					break;
				case kExtraRefListColumn_Count:
					{
						BSExtraData* AxData = FormA->extraData.GetExtraDataByType(BSExtraData::kExtra_Count);
						BSExtraData* BxData = FormB->extraData.GetExtraDataByType(BSExtraData::kExtra_Count);

						if (AxData && BxData == NULL)
							Result = -1;
						else if (AxData == NULL && BxData)
							Result = 1;
						else if (AxData && BxData)
						{
							ExtraCount* AxCount = CS_CAST(AxData, BSExtraData, ExtraCount);
							ExtraCount* BxCount = CS_CAST(BxData, BSExtraData, ExtraCount);

							if (AxCount->count < BxCount->count)
								Result = -1;
							else if (AxCount->count > BxCount->count)
								Result = 1;
						}
					}

					break;
				}

				if (lParamSort < 0)
					Result *= -1;

				Result = CSEFormEnumerationManager::Instance.CompareActiveForms(FormA, FormB, Result);
			}

			return Result;
		}

		CSEDialogExtraFittingsData::CSEDialogExtraFittingsData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			LastCursorPos.x = LastCursorPos.y = 0;
			LastCursorPosWindow = NULL;
			QuickViewTriggered = false;

			AssetControlToolTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
												TTS_ALWAYSTIP|TTS_NOPREFIX,
												CW_USEDEFAULT, CW_USEDEFAULT,
												CW_USEDEFAULT, CW_USEDEFAULT,
												NULL, NULL, NULL, NULL);
			ZeroMemory(&AssetControlToolData, sizeof(AssetControlToolData));
			AssetControlToolData.cbSize = sizeof(AssetControlToolData);
			LastTrackedTool = NULL;
			TrackingToolTip = false;
		}

		CSEDialogExtraFittingsData::~CSEDialogExtraFittingsData()
		{
			if (AssetControlToolTip)
				DestroyWindow(AssetControlToolTip);
		}

		CSEMainWindowMiscData::CSEMainWindowMiscData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			ToolbarExtras = Subwindow::CreateInstance();
		}

		CSEMainWindowMiscData::~CSEMainWindowMiscData()
		{
			if (ToolbarExtras)
			{
				ToolbarExtras->TearDown();
				ToolbarExtras->DeleteInstance();
			}
		}

		CSEMainWindowToolbarData::CSEMainWindowToolbarData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			SettingTODSlider = false;
		}

		CSEMainWindowToolbarData::~CSEMainWindowToolbarData()
		{
			;//
		}

		CSERenderWindowMiscData::CSERenderWindowMiscData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			TunnelingKeyMessage = false;
		}

		CSERenderWindowMiscData::~CSERenderWindowMiscData()
		{
			;//
		}

		CSETESFormEditData::CSETESFormEditData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			Buffer = NULL;
		}

		CSETESFormEditData::~CSETESFormEditData()
		{
			if (Buffer)
				Buffer->DeleteInstance();
		}

		void CSETESFormEditData::FillBuffer( TESForm* Parent )
		{
			SME_ASSERT(Buffer == NULL);

			Buffer = TESForm::CreateTemporaryCopy(Parent);
		}

		bool CSETESFormEditData::HasChanges( TESForm* Parent )
		{
			SME_ASSERT(Buffer && Buffer->formType == Parent->formType);

			return Buffer->CompareTo(Parent);
		}

		CSEFaceGenWindowData::CSEFaceGenWindowData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			TunnelingTabSelectMessage = false;
			AllowPreviewUpdates = true;
			VoicePlaybackFilePath = "";
		}

		CSEFaceGenWindowData::~CSEFaceGenWindowData()
		{
			;//
		}

		LRESULT CALLBACK FindTextDlgSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			static bool kDraggingForms = false;

			switch (uMsg)
			{
			case WM_MOUSEMOVE:
				if (kDraggingForms)
				{
					POINT Pos = {0};
					GetCursorPos(&Pos);
					HWND Window = WindowFromPoint(Pos);

					if (_RENDERSEL->selectionList && _RENDERSEL->selectionList->Data)
					{
						if (TESDialog::GetIsWindowDragDropRecipient(_RENDERSEL->selectionList->Data->formType, Window))
						{
							SetCursor(LoadCursor(*TESCSMain::Instance, (LPCSTR)0xBA));
						}
						else
						{
							SetCursor(LoadCursor(*TESCSMain::Instance, (LPCSTR)0xB8));
						}
					}
				}

				break;
			case WM_LBUTTONUP:
				if (kDraggingForms)
				{
					POINT Pos = {0};
					GetCursorPos(&Pos);
					HWND Window = WindowFromPoint(Pos);

					if (_RENDERSEL->selectionList && _RENDERSEL->selectionList->Data)
					{
						if (TESDialog::GetIsWindowDragDropRecipient(_RENDERSEL->selectionList->Data->formType, Window))
						{
							HWND Parent = GetParent(Window);
							if (Parent == NULL || Parent == *TESCSMain::WindowHandle)
								Parent = Window;

							SendMessage(Parent, TESDialog::kWindowMessage_HandleDragDrop, NULL, (LPARAM)&Pos);
						}
						else
							_RENDERSEL->ClearSelection();

						kDraggingForms = false;
						SetCursor(LoadCursor(NULL, IDC_ARROW));
						ReleaseCapture();
					}
				}

				break;
			case WM_NOTIFY:
				switch (((LPNMHDR)lParam)->code)
				{
				case LVN_BEGINDRAG:
					{
						NMLISTVIEW* Data = (NMLISTVIEW*)lParam;
						std::list<TESForm*> Selection;

						switch (Data->hdr.idFrom)
						{
						case kFindTextListView_Objects:
							{
								int Index = -1;
								while ((Index = ListView_GetNextItem(Data->hdr.hwndFrom, Index, LVNI_SELECTED)) != -1)
								{
									TESForm* Form = (TESForm*)TESListView::GetItemData(Data->hdr.hwndFrom, Index);
									SME_ASSERT(Form);

									Selection.push_back(Form);
								}
							}

							break;
						}

						kDraggingForms = false;
						if (Selection.size())
						{
							_RENDERSEL->ClearSelection(true);

							for (std::list<TESForm*>::iterator Itr = Selection.begin(); Itr != Selection.end(); ++Itr)
								_RENDERSEL->AddToSelection(*Itr);

							kDraggingForms = true;
							SetCursor(LoadCursor(*TESCSMain::Instance, (LPCSTR)0xB8));
							SetCapture(hWnd);
						}
					}

					break;
				case LVN_ITEMACTIVATE:
					NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;
					TESForm* Form = (TESForm*)TESListView::GetItemData(Data->hdr.hwndFrom, Data->iItem);

					if (Data->hdr.idFrom == kFindTextListView_Objects && Form)
					{
						switch (Form->formType)
						{
						case TESForm::kFormType_Script:
							TESDialog::ShowScriptEditorDialog(Form);
							break;
						case TESForm::kFormType_REFR:
							_TES->LoadCellIntoViewPort((CS_CAST(Form, TESForm, TESObjectREFR))->GetPosition(), CS_CAST(Form, TESForm, TESObjectREFR));
							break;
						default:
							TESDialog::ShowFormEditDialog(Form);
							break;
						}
					}

					break;
				}

				break;
			case WM_INITDIALOG:
				break;
			case WM_DESTROY:
				break;
			}

			return DlgProcResult;
		}

#define WM_DATADLG_RECURSEMASTERS				(WM_USER + 2002)
		// lParam = TESFile*

		LRESULT CALLBACK DataDlgSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			TESFile* ActiveTESFile = *((TESFile**)0x00A0AA7C);
			HWND PluginList = GetDlgItem(hWnd, kDataDlg_PluginFileList);

			switch (uMsg)
			{
			case WM_DATADLG_RECURSEMASTERS:
				{
					Return = true;

					TESFile* PluginFile = (TESFile*)lParam;

					if (PluginFile)
					{
						for (int i = 0; i < PluginFile->masterCount; i++)
							SendMessage(hWnd, WM_DATADLG_RECURSEMASTERS, NULL, (LPARAM)PluginFile->masterFiles[i]);

						PluginFile->SetLoaded(true);
					}
				}

				break;
			case WM_INITDIALOG:
				{
					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 305;
					ListView_SetColumn(PluginList, 0, &ColumnData);

					ColumnData.cx = 65;
					ListView_SetColumn(PluginList, 1, &ColumnData);

					bool LoadStartupPlugin = Settings::Startup::kLoadPlugin.GetData().i;
					if (LoadStartupPlugin)
						CheckDlgButton(hWnd, IDC_CSE_DATA_LOADSTARTUPPLUGIN, BST_CHECKED);
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case NM_CUSTOMDRAW:
						if (wParam == kDataDlg_PluginFileList)
						{
							NMLVCUSTOMDRAW* DrawData = (NMLVCUSTOMDRAW*)lParam;

							switch (DrawData->nmcd.dwDrawStage)
							{
							case CDDS_PREPAINT:
								{
									SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
									DlgProcResult = TRUE;
									Return = true;
								}

								break;
							case CDDS_ITEMPREPAINT:
								{
									UInt32 PluginIndex = (UInt32)DrawData->nmcd.lItemlParam;
									TESFile* CurrentFile = _DATAHANDLER->LookupPluginByIndex(PluginIndex);

									if (CurrentFile)
									{
										bool Update = true;

										if (CurrentFile->IsMaster())
										{
											DrawData->clrTextBk = RGB(242, 247, 243);
											DrawData->clrText = RGB(0, 0, 0);
										}

										if (CurrentFile == ActiveTESFile)
										{
											DrawData->clrTextBk = RGB(227, 183, 251);
											DrawData->clrText = RGB(0, 0, 0);
										}
										else if (!_stricmp(Settings::Startup::kPluginName.GetData().s, CurrentFile->fileName))
										{
											DrawData->clrTextBk = RGB(248, 227, 186);
											DrawData->clrText = RGB(0, 0, 0);
										}
										else if (CurrentFile->authorName.c_str() &&
												!_stricmp(CurrentFile->authorName.c_str(), "shadeMe"))
										{
											DrawData->clrTextBk = RGB(249, 255, 255);
											DrawData->clrText = RGB(0, 0, 0);
										}
										else
										{
											Update = false;
										}

										if (Update)
										{
											SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NEWFONT);
											DlgProcResult = TRUE;
											Return = true;
										}
									}
								}

								break;
							}
						}

						break;
					case LVN_KEYDOWN:
						if (wParam == kDataDlg_ParentMasterFileList)
						{
							NMLVKEYDOWN* KeyData = (NMLVKEYDOWN*)lParam;

							switch (KeyData->wVKey)
							{
							case VK_DELETE:		// delete master
								{
									TESFile* Selection = (TESFile*)TESListView::GetSelectedItemData(KeyData->hdr.hwndFrom);
									if (Selection)
									{
										if (BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "You are about to remove a master file from the selected plugin.\n\nAre you sure you'd like to proceed?") == IDNO)
											Return = true;
									}
								}

								break;
							case VK_INSERT:		// insert master
								{
									;//
								}

								break;
							}
						}

						break;
					}
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_DATA_LOADSTARTUPPLUGIN:
					{
						Settings::Startup::kLoadPlugin.SetInt((IsDlgButtonChecked(hWnd, IDC_CSE_DATA_LOADSTARTUPPLUGIN) == BST_CHECKED));
					}

					break;
				case IDC_CSE_DATA_SETSTARTUPPLUGIN:
					{
						int SelectedItem = ListView_GetNextItem(PluginList, -1, LVNI_SELECTED);

						if (SelectedItem != -1)
						{
							LVITEM SelectedPluginItem;
							char PluginNameBuffer[0x200] = {0};

							SelectedPluginItem.iItem = SelectedItem;
							SelectedPluginItem.iSubItem = 0;
							SelectedPluginItem.mask = LVIF_TEXT;
							SelectedPluginItem.pszText = PluginNameBuffer;
							SelectedPluginItem.cchTextMax = sizeof(PluginNameBuffer);

							if (ListView_GetItem(PluginList, &SelectedPluginItem) == TRUE)
							{
								Settings::Startup::kPluginName.SetString(PluginNameBuffer);

								BGSEEUI->MsgBoxI(hWnd, 0, "Startup plugin set to '%s'.", PluginNameBuffer);
								BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
							}
						}
					}

					break;
				case 1:		// OK
					if (CLIWrapper::Interfaces::SE->GetOpenEditorCount())
					{
						if (BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "There are open script windows. Are you sure you'd like to proceed?") == IDNO)
							Return = true;
					}
					else if (ActiveTESFile != NULL && !_stricmp(ActiveTESFile->fileName, "oblivion.esm"))
					{
						if (BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "You have set Oblvion.esm as an active file. Are you absolutely sure this is the end of the world?") == IDNO)
							Return = true;
						else if (BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "What you're about to do is akin to using the Osterhagen Key.\n\nThis is the Point Of No Return. Proceed?") == IDNO)
						{
							Return = true;
							BGSEEACHIEVEMENTS->Unlock(Achievements::kChicken);
						}
						else
							BGSEEACHIEVEMENTS->Unlock(Achievements::kFearless);
					}

					if (Return == false)
					{
						CSEFormEnumerationManager::Instance.ResetVisibility();
						CLIWrapper::Interfaces::SE->CloseAllOpenEditors();
						ObjectPalette::CSEObjectPaletteManager::Instance.Close();

						if (ActiveTESFile)
							SendMessage(hWnd, WM_DATADLG_RECURSEMASTERS, NULL, (LPARAM)ActiveTESFile);
						else
						{
							int Selection = -1;
							do
							{
								Selection = ListView_GetNextItem(PluginList, Selection, LVNI_SELECTED);
								if (Selection == -1)
									break;

								UInt32 PluginIndex = (UInt32)TESListView::GetItemData(PluginList, Selection);
								TESFile* CurrentFile = _DATAHANDLER->LookupPluginByIndex(PluginIndex);

								if (CurrentFile && CurrentFile->IsLoaded())
									SendMessage(hWnd, WM_DATADLG_RECURSEMASTERS, NULL, (LPARAM)CurrentFile);
							}
							while (true);
						}
					}

					break;
				}

				break;
			case WM_DESTROY:
				TESFile* ActiveFile = _DATAHANDLER->activeFile;					// required for correct ESM handling

				if (ActiveFile)
					ActiveFile->SetMaster(false);

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK MainWindowMenuInitSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITMENUPOPUP:
				{
					if (HIWORD(lParam) == FALSE)
					{
						HMENU Popup = (HMENU)wParam;

						for (int i = 0, j = GetMenuItemCount(Popup); i < j; i++ )
						{
							MENUITEMINFO CurrentItem = {0};
							CurrentItem.cbSize = sizeof(MENUITEMINFO);
							CurrentItem.fMask = MIIM_ID|MIIM_STATE;

							if (GetMenuItemInfo(Popup, i, TRUE, &CurrentItem) == TRUE)
							{
								bool UpdateItem = true;
								bool CheckItem = false;

								switch (CurrentItem.wID)
								{
								case TESCSMain::kMainMenu_World_EditCellPathGrid:
									if (*TESRenderWindow::PathGridEditFlag)
										CheckItem = true;

									break;
								case IDC_MAINMENU_SAVEOPTIONS_SAVEESPMASTERS:
									if (Settings::Plugins::kSaveLoadedESPsAsMasters.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS:
									if (Settings::Plugins::kPreventTimeStampChanges.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_SAVEOPTIONS_CREATEBACKUPBEFORESAVING:
									if (Settings::VersionControl::kBackupOnSave.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_CONSOLE:
									if (BGSEECONSOLE->GetVisible())
										CheckItem = true;

									break;
								case IDC_MAINMENU_AUXVIEWPORT:
									if (AUXVIEWPORT->GetVisible())
										CheckItem = true;

									break;
								case IDC_MAINMENU_HIDEUNMODIFIEDFORMS:
									if (CSEFormEnumerationManager::Instance.GetVisibleUnmodifiedForms() == false)
										CheckItem = true;

									break;
								case IDC_MAINMENU_HIDEDELETEDFORMS:
									if (CSEFormEnumerationManager::Instance.GetVisibleDeletedForms() == false)
										CheckItem = true;

									break;
								case IDC_MAINMENU_SORTACTIVEFORMSFIRST:
									if (Settings::Dialogs::kSortFormListsByActiveForm.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_COLORIZEACTIVEFORMS:
									if (Settings::Dialogs::kColorizeActiveForms.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_COLORIZEFORMOVERRIDES:
									if (Settings::Dialogs::kColorizeFormOverrides.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_CODABACKGROUNDER:
									if (CODAVM->GetBackgrounderState())
										CheckItem = true;

									break;
								case IDC_MAINMENU_INITIALLYDISABLEDREFERENCES:
									if (TESRenderWindow::ShowInitiallyDisabledRefs)
										CheckItem = true;

									break;
								case IDC_MAINMENU_CHILDREFERENCESOFTHEDISABLED:
									if (TESRenderWindow::ShowInitiallyDisabledRefChildren)
										CheckItem = true;

									break;
								case IDC_MAINMENU_MULTIPLEPREVIEWWINDOWS:
									if (PreviewWindowImposterManager::Instance.GetEnabled())
										CheckItem = true;

									break;
								default:
									UpdateItem = false;
									break;
								}

								if (UpdateItem)
								{
									if (CheckItem)
									{
										CurrentItem.fState &= ~MFS_UNCHECKED;
										CurrentItem.fState |= MFS_CHECKED;
									}
									else
									{
										CurrentItem.fState &= ~MFS_CHECKED;
										CurrentItem.fState |= MFS_UNCHECKED;
									}

									CurrentItem.fMask = MIIM_STATE;
									SetMenuItemInfo(Popup, i, TRUE, &CurrentItem);
								}
							}
						}
					}

					Return = true;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK MainWindowMenuSelectSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_COMMAND:
				Return = true;

				switch (LOWORD(wParam))
				{
				case TESCSMain::kMainMenu_View_PreviewWindow:
					if (PreviewWindowImposterManager::Instance.GetEnabled())
						BGSEEUI->MsgBoxI("Use the Object Window's context menu to preview objects when multiple preview windows are enabled.");
					else
						Return = false;

					break;
				case TESCSMain::kMainMenu_Help_Contents:
				case TESCSMain::kMainMenu_Character_ExportDialogue:
					{
						if (Achievements::kOldestTrickInTheBook->GetUnlocked() == false)
						{
							ShellExecute(NULL, "open", "http://www.youtube.com/watch?v=oHg5SJYRHA0", NULL, NULL, SW_SHOWNORMAL);
							BGSEEACHIEVEMENTS->Unlock(Achievements::kOldestTrickInTheBook);
						}
						else
							Return = false;
					}

					break;
				case IDC_MAINMENU_SAVEAS:
					{
						if (_DATAHANDLER->activeFile == NULL)
						{
							BGSEEUI->MsgBoxE("An active plugin must be set before using this tool.");
							break;
						}

						*TESCSMain::AllowAutoSaveFlag = 0;

						char FileName[0x104] = {0};

						if (TESDialog::SelectTESFileCommonDialog(hWnd,
																INISettingCollection::Instance->LookupByName("sLocalMasterPath:General")->value.s,
																0,
																FileName,
																0x104))
						{
							TESFile* SaveAsBuffer = _DATAHANDLER->activeFile;

							SaveAsBuffer->SetActive(false);
							SaveAsBuffer->SetLoaded(false);

							_DATAHANDLER->activeFile = NULL;

							if (SendMessage(hWnd, 0x40C, NULL, (LPARAM)FileName))
							{
								TESCSMain::SetTitleModified(false);
							}
							else
							{
								_DATAHANDLER->activeFile = SaveAsBuffer;

								SaveAsBuffer->SetActive(true);
								SaveAsBuffer->SetLoaded(true);
							}

							BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						}

						*TESCSMain::AllowAutoSaveFlag = 1;
					}

					break;
				case IDC_MAINMENU_CONSOLE:
					BGSEECONSOLE->ToggleVisibility();

					break;
				case IDC_MAINMENU_HIDEDELETEDFORMS:
					CSEFormEnumerationManager::Instance.ToggleVisibilityDeletedForms();
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_HIDEUNMODIFIEDFORMS:
					CSEFormEnumerationManager::Instance.ToggleVisibilityUnmodifiedForms();
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_CSEPREFERENCES:
					BGSEEMAIN->ShowPreferencesGUI();

					break;
				case IDC_MAINMENU_LAUNCHGAME:
					{
						std::string AppPath = BGSEEMAIN->GetAPPPath();
						AppPath += "\\";

						IFileStream SteamLoader;
						if (SteamLoader.Open((std::string(AppPath + "obse_steam_loader.dll")).c_str()) == false)
							AppPath += "obse_loader.exe";
						else
							AppPath += "Oblivion.exe";

						ShellExecute(NULL, "open", (LPCSTR)AppPath.c_str(), NULL, NULL, SW_SHOW);
						BGSEEACHIEVEMENTS->Unlock(Achievements::kLazyBum);
					}

					break;
				case IDC_MAINMENU_CREATEGLOBALSCRIPT:
					BGSEEUI->ModelessDialog(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_GLOBALSCRIPT), hWnd, (DLGPROC)CreateGlobalScriptDlgProc);
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_TAGBROWSER:
					CLIWrapper::Interfaces::TAG->ShowTagBrowserDialog(NULL);
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_SETWORKSPACE:
					BGSEEWORKSPACE->SelectCurrentWorkspace();
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_TOOLS:
					BGSEETOOLBOX->ShowToolListMenu(BGSEEMAIN->GetExtenderHandle(), hWnd);
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_CODAMANAGEGLOBALDATASTORE:
					CODAVM->ShowGlobalStoreEditDialog(BGSEEMAIN->GetExtenderHandle(), hWnd);

					break;
				case IDC_MAINMENU_CODABACKGROUNDER:
					CODAVM->ToggleBackgrounderState();

					break;
				case IDC_MAINMENU_SAVEOPTIONS_SAVEESPMASTERS:
					Settings::Plugins::kSaveLoadedESPsAsMasters.ToggleData();

					break;
				case IDC_MAINMENU_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS:
					Settings::Plugins::kPreventTimeStampChanges.ToggleData();

					break;
				case IDC_MAINMENU_AUXVIEWPORT:
					AUXVIEWPORT->ToggleVisibility();

					break;
				case IDC_MAINMENU_USEINFOLISTING:
					CLIWrapper::Interfaces::USE->ShowUseInfoListDialog(NULL);

					break;
				case IDC_MAINMENU_BATCHLIPGENERATOR:
					{
						if (CSIOM->GetInitialized() == false)
						{
							BGSEEUI->MsgBoxE("The CSInteropManager is not initialized!");
							break;
						}

						bool SkipInactiveTopicInfos = false;
						bool OverwriteExisting = false;

						if (BGSEEUI->MsgBoxI(hWnd,
											MB_YESNO,
											"Only process active topic infos?") == IDYES)
						{
							SkipInactiveTopicInfos = true;
						}

						if (BGSEEUI->MsgBoxI(hWnd,
											MB_YESNO,
											"Overwrite existing LIP files?") == IDYES)
						{
							OverwriteExisting = true;
						}

						HWND IdleWindow = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), hWnd, NULL, NULL);
						IFileStream ExistingFile;
						int BatchGenCounter = 0, FailedCounter = 0;
						bool HasError = false;

						for (tList<TESTopic>::Iterator ItrTopic = _DATAHANDLER->topics.Begin(); ItrTopic.End() == false && ItrTopic.Get(); ++ItrTopic)
						{
							TESTopic* Topic = ItrTopic.Get();
							SME_ASSERT(Topic);

							for (TESTopic::TopicDataListT::Iterator ItrTopicData = Topic->topicData.Begin();
																	ItrTopicData.End() == false && ItrTopicData.Get();
																	++ItrTopicData)
							{
								TESQuest* Quest = ItrTopicData->parentQuest;
								SME_ASSERT(Quest);

								for (int i = 0; i < ItrTopicData->questInfos.numObjs; i++)
								{
									TESTopicInfo* Info = ItrTopicData->questInfos.data[i];
									SME_ASSERT(Info);

									TESFile* OverrideFile = Info->GetOverrideFile(-1);

									if (OverrideFile)
									{
										if (SkipInactiveTopicInfos == false || (Info->formFlags & TESForm::kFormFlags_FromActiveFile))
										{
											for (tList<TESRace>::Iterator ItrRace = _DATAHANDLER->races.Begin();
																		ItrRace.End() == false && ItrRace.Get();
																		++ItrRace)
											{
												TESRace* Race = ItrRace.Get();
												SME_ASSERT(Race);

												int ResponseCounter = 1;
												for (TESTopicInfo::ResponseListT::Iterator ItrResponse = Info->responseList.Begin();
																						ItrResponse.End() == false && ItrResponse.Get();
																						++ItrResponse)
												{
													TESTopicInfo::ResponseData* Response = ItrResponse.Get();
													SME_ASSERT(Response);

													char VoiceFilePath[MAX_PATH] = {0};

													for (int j = 0; j < 2; j++)
													{
														const char* Sex = "M";
														if (j)
															Sex = "F";

														FORMAT_STR(VoiceFilePath, "Data\\Sound\\Voice\\%s\\%s\\%s\\%s_%s_%08X_%u",
																				OverrideFile->fileName,
																				Race->name.c_str(),
																				Sex,
																				Quest->editorID.c_str(),
																				Topic->editorID.c_str(),
																				(Info->formID & 0xFFFFFF),
																				ResponseCounter);

														std::string MP3Path(VoiceFilePath); MP3Path += ".mp3";
														std::string WAVPath(VoiceFilePath); WAVPath += ".wav";
														std::string LIPPath(VoiceFilePath); LIPPath += ".lip";

														if (ExistingFile.Open(MP3Path.c_str()) ||
															ExistingFile.Open(WAVPath.c_str()))
														{
															if (OverwriteExisting || ExistingFile.Open(LIPPath.c_str()) == false)
															{
																if (CSIOM->GenerateLIPSyncFile(VoiceFilePath, Response->responseText.c_str()))
																	BatchGenCounter++;
																else
																{
																	HasError = true;
																	FailedCounter++;
																}
															}
														}
													}

													ResponseCounter++;
												}
											}
										}
									}
								}
							}
						}

						DestroyWindow(IdleWindow);
						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

						if (HasError)
							BGSEEUI->MsgBoxW("Batch generation completed with some errors!\n\nGenerated: %d files\nFailed: %d Files", BatchGenCounter, FailedCounter);
						else
							BGSEEUI->MsgBoxI("Batch generation completed successfully!\n\nGenerated: %d files.", BatchGenCounter);
					}

					break;
				case IDC_MAINMENU_SAVEOPTIONS_CREATEBACKUPBEFORESAVING:
					Settings::VersionControl::kBackupOnSave.ToggleData();

					break;
				case IDC_MAINMENU_SORTACTIVEFORMSFIRST:
					Settings::Dialogs::kSortFormListsByActiveForm.ToggleData();

					break;
				case IDC_MAINMENU_COLORIZEACTIVEFORMS:
					Settings::Dialogs::kColorizeActiveForms.ToggleData();

					break;
				case IDC_MAINMENU_COLORIZEFORMOVERRIDES:
					Settings::Dialogs::kColorizeFormOverrides.ToggleData();

					break;
				case IDC_MAINMENU_GLOBALCLIPBOARDCONTENTS:
					BGSEECLIPBOARD->DisplayContents();

					break;
				case IDC_MAINMENU_PASTEFROMGLOBALCLIPBOARD:
					BGSEECLIPBOARD->Paste();

					break;
				case IDC_MAINMENU_GLOBALUNDO:
					BGSEEUNDOSTACK->PerformUndo();

					break;
				case IDC_MAINMENU_GLOBALREDO:
					BGSEEUNDOSTACK->PerformRedo();

					break;
				case IDC_MAINMENU_PURGELOADEDRESOURCES:
					{
#ifdef NDEBUG
						BGSEEUI->MsgBoxI("This feature has not been implemented yet.");
						break;
#endif

#if 0
						BGSEECONSOLE_MESSAGE("Purging resources...");
						BGSEECONSOLE->Indent();
						PROCESS_MEMORY_COUNTERS_EX MemCounter = {0};
						UInt32 RAMUsage = 0;

						GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter));
						RAMUsage = MemCounter.WorkingSetSize / (1024 * 1024);
						BGSEECONSOLE_MESSAGE("Current RAM Usage: %d MB", RAMUsage);

						TESRenderWindow::Reset();
						thisCall<void>(0x004763A0, _TES, 0, 0);
						thisCall<void>(0x00476190, _TES, 0);

						thisCall<void>(0x00474760, _MODELLOADER);
						FormHeap_Free(_MODELLOADER);

						_MODELLOADER = (ModelLoader*)FormHeap_Allocate(0x1C);
						thisCall<void>(0x00474CD0, _MODELLOADER);
						BGSEECONSOLE_MESSAGE("Resources purged!");

						GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter));
						RAMUsage = MemCounter.WorkingSetSize / (1024 * 1024);
						BGSEECONSOLE_MESSAGE("Current RAM Usage: %d MB", RAMUsage);
						BGSEECONSOLE->Exdent();
#endif
					}

					break;
				case IDC_MAINMENU_INITIALLYDISABLEDREFERENCES:
					TESRenderWindow::ShowInitiallyDisabledRefs = (TESRenderWindow::ShowInitiallyDisabledRefs == false);

					break;
				case IDC_MAINMENU_CHILDREFERENCESOFTHEDISABLED:
					TESRenderWindow::ShowInitiallyDisabledRefChildren = (TESRenderWindow::ShowInitiallyDisabledRefChildren == false);

					break;
				case IDC_MAINMENU_CODAOPENSCRIPTREPOSITORY:
					CODAVM->OpenScriptRepository();

					break;
				case IDC_MAINMENU_SPAWNEXTRAOBJECTWINDOW:
					ObjectWindowImposterManager::Instance.SpawnImposter();

					break;
				case IDC_MAINMENU_MULTIPLEPREVIEWWINDOWS:
					if (PreviewWindowImposterManager::Instance.GetEnabled())
					{
						PreviewWindowImposterManager::Instance.SetEnabled(false);
						Settings::Dialogs::kMultiplePreviewWindows.SetInt(0);
					}
					else
					{
						PreviewWindowImposterManager::Instance.SetEnabled(true);
						Settings::Dialogs::kMultiplePreviewWindows.SetInt(1);
					}

					break;
				case IDC_MAINMENU_OBJECTPALETTE:
					ObjectPalette::CSEObjectPaletteManager::Instance.Show();

					break;
				default:
					Return = false;

					break;
				}

				break;
			}

			return DlgProcResult;
		}

#define ID_PATHGRIDTOOLBARBUTTION_TIMERID		0x99
#define WM_MAINWINDOW_INITEXTRADATA				(WM_USER + 2003)
#define WM_MAINTOOLBAR_SETTOD					(WM_USER + 2004)
		// wParam = position

		LRESULT CALLBACK MainWindowMiscSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					SetTimer(hWnd, ID_PATHGRIDTOOLBARBUTTION_TIMERID, 500, NULL);
					Return = true;
				}

				break;
			case WM_DESTROY:
				{
					KillTimer(hWnd, ID_PATHGRIDTOOLBARBUTTION_TIMERID);

					CSEMainWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowMiscData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(CSEMainWindowMiscData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_MAINWINDOW_INITEXTRADATA:
				{
					CSEMainWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowMiscData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSEMainWindowMiscData();

						xData->ToolbarExtras->hInstance = BGSEEMAIN->GetExtenderHandle();
						xData->ToolbarExtras->hDialog = *TESCSMain::MainToolbarHandle;
						xData->ToolbarExtras->hContainer = *TESCSMain::MainToolbarHandle;
						xData->ToolbarExtras->position.x = 485;
						xData->ToolbarExtras->position.y = 0;

						if (xData->ToolbarExtras->Build(IDD_TOOLBAREXTRAS) == false)
						{
							BGSEECONSOLE_ERROR("Couldn't build main window toolbar subwindow!");
						}
						else
						{
							BGSEEUI->GetSubclasser()->RegisterRegularWindowSubclass(*TESCSMain::MainToolbarHandle, MainWindowToolbarSubClassProc);
							SendMessage(*TESCSMain::MainToolbarHandle, WM_INITDIALOG, NULL, NULL);

							HWND TODSlider = GetDlgItem(hWnd, IDC_TOOLBAR_TODSLIDER);
							HWND TODEdit = GetDlgItem(hWnd, IDC_TOOLBAR_TODCURRENT);

							TESDialog::ClampDlgEditField(TODEdit, 0.0, 24.0);

							SendMessage(TODSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 23));
							SendMessage(TODSlider, TBM_SETLINESIZE, NULL, 1);
							SendMessage(TODSlider, TBM_SETPAGESIZE, NULL, 4);

							SendMessage(*TESCSMain::MainToolbarHandle, WM_MAINTOOLBAR_SETTOD, _TES->GetSkyTOD() * 4.0, NULL);
						}

						ExtraData->Add(xData);
					}
				}

				break;
			case WM_TIMER:
				DlgProcResult = TRUE;
				Return = true;

				switch (wParam)
				{
				case TESCSMain::kTimer_Autosave:				// autosave timer, needs to be handled here as the org wndproc doesn't compare the timerID
					if ((*TESCSMain::AllowAutoSaveFlag) != 0 && (*TESCSMain::ExittingCSFlag) == 0)
					{
						TESCSMain::AutoSave();
					}

					break;
				case ID_PATHGRIDTOOLBARBUTTION_TIMERID:
					{
						TBBUTTONINFO PathGridData = {0};
						PathGridData.cbSize = sizeof(TBBUTTONINFO);
						PathGridData.dwMask = TBIF_STATE;

						SendMessage(*TESCSMain::MainToolbarHandle, TB_GETBUTTONINFO, TESCSMain::kToolbar_PathGridEdit, (LPARAM)&PathGridData);
						if ((PathGridData.fsState & TBSTATE_CHECKED) == false && *TESRenderWindow::PathGridEditFlag)
						{
							PathGridData.fsState |= TBSTATE_CHECKED;
							SendMessage(*TESCSMain::MainToolbarHandle, TB_SETBUTTONINFO, TESCSMain::kToolbar_PathGridEdit, (LPARAM)&PathGridData);
						}
					}

					break;
				}

				break;
			case 0x40C:				// save handler
				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK MainWindowToolbarSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND TODSlider = GetDlgItem(hWnd, IDC_TOOLBAR_TODSLIDER);
			HWND TODEdit = GetDlgItem(hWnd, IDC_TOOLBAR_TODCURRENT);

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					CSEMainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowToolbarData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSEMainWindowToolbarData();
						ExtraData->Add(xData);
					}
				}

				break;
			case WM_DESTROY:
				{
					CSEMainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowToolbarData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(CSEMainWindowToolbarData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_COMMAND:
				{
					CSEMainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowToolbarData, ExtraData);
					SME_ASSERT(xData);

					if (HIWORD(wParam) == EN_CHANGE &&
						LOWORD(wParam) == IDC_TOOLBAR_TODCURRENT &&
						xData->SettingTODSlider == false)
					{
						xData->SettingTODSlider = true;
						float TOD = TESDialog::GetDlgItemFloat(hWnd, IDC_TOOLBAR_TODCURRENT);
						SendMessage(hWnd, WM_MAINTOOLBAR_SETTOD, TOD * 4.0, NULL);
						xData->SettingTODSlider = false;
					}
				}

				break;
			case WM_HSCROLL:
				{
					bool BreakOut = true;

					switch (LOWORD(wParam))
					{
					case TB_BOTTOM:
					case TB_ENDTRACK:
					case TB_LINEDOWN:
					case TB_LINEUP:
					case TB_PAGEDOWN:
					case TB_PAGEUP:
					case TB_THUMBPOSITION:
					case TB_THUMBTRACK:
					case TB_TOP:
						if ((HWND)lParam == TODSlider)
						{
							BreakOut = false;
						}

						break;
					}

					if (BreakOut)
						break;
				}
			case WM_MAINTOOLBAR_SETTOD:
				{
					if (uMsg != WM_HSCROLL)
						SendDlgItemMessage(hWnd, IDC_TOOLBAR_TODSLIDER, TBM_SETPOS, TRUE, (LPARAM)wParam);

					int Position = SendMessage(TODSlider, TBM_GETPOS, NULL, NULL);
					float TOD = Position / 4.0;

					if (TOD > 24.0f)
						TOD = 24.0f;

					_TES->SetSkyTOD(TOD);

					CSEMainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowToolbarData, ExtraData);
					SME_ASSERT(xData);

					if (xData->SettingTODSlider == false)
						TESDialog::SetDlgItemFloat(hWnd, IDC_TOOLBAR_TODCURRENT, TOD, 2);

					TESPreviewControl::UpdatePreviewWindows();
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK RenderWindowMenuInitSelectSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = TRUE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITMENUPOPUP:
				{
					if (HIWORD(lParam) == FALSE)
					{
						HMENU Popup = (HMENU)wParam;

						for (int i = 0, j = GetMenuItemCount(Popup); i < j; i++ )
						{
							MENUITEMINFO CurrentItem = {0};
							CurrentItem.cbSize = sizeof(MENUITEMINFO);
							CurrentItem.fMask = MIIM_ID|MIIM_STATE;

							if (GetMenuItemInfo(Popup, i, TRUE, &CurrentItem) == TRUE)
							{
								bool UpdateItem = true;
								bool CheckItem = false;
								bool DisableItem = false;
								char NewItemText[0x200] = {0};

								switch (CurrentItem.wID)
								{
								case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
									if (TESRenderWindow::FreezeInactiveRefs)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_COPLANARDROP:
									if (Settings::Renderer::kCoplanarRefDrops.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_SWITCHCNY:
									if (Settings::Renderer::kSwitchCAndY.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_STATICCAMERAPIVOT:
									if (Settings::Renderer::kFixedCameraPivot.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_MOUSEREFENABLED:
									if (Settings::RenderWindowPainter::kShowMouseRef.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_MOUSEREFCTRLMODIFIED:
									if (Settings::RenderWindowPainter::kMouseRefCtrlModified.GetData().i)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_USEALTERNATEMOVEMENTSETTINGS:
									if (TESRenderWindow::UseAlternateMovementSettings)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_SAVEEXTERIORSNAPSHOT:
									FORMAT_STR(NewItemText, "Save Current Exterior Cell Snapshot");

									if (*TESRenderWindow::ActiveCell == NULL || _TES->currentInteriorCell)
										DisableItem = true;
									else
									{
										FORMAT_STR(NewItemText, "Save Exterior Cell %i,%i Snapshot",
												(*TESRenderWindow::ActiveCell)->cellData.coords->x,
												(*TESRenderWindow::ActiveCell)->cellData.coords->y);
									}

									break;
								default:
									UpdateItem = false;
									break;
								}

								if (UpdateItem)
								{
									if (CheckItem)
									{
										CurrentItem.fState &= ~MFS_UNCHECKED;
										CurrentItem.fState |= MFS_CHECKED;
									}
									else
									{
										CurrentItem.fState &= ~MFS_CHECKED;
										CurrentItem.fState |= MFS_UNCHECKED;
									}

									if (DisableItem)
									{
										CurrentItem.fState &= ~MFS_ENABLED;
										CurrentItem.fState |= MFS_DISABLED;
									}
									else
									{
										CurrentItem.fState &= ~MFS_DISABLED;
										CurrentItem.fState |= MFS_ENABLED;
									}

									CurrentItem.fMask = MIIM_STATE;

									if (NewItemText[0] != 0)
									{
										CurrentItem.fMask |= MIIM_STRING;
										CurrentItem.fType |= MFT_STRING;
										CurrentItem.dwTypeData = NewItemText;
										CurrentItem.cch = strlen(NewItemText);
									}

									SetMenuItemInfo(Popup, i, TRUE, &CurrentItem);
								}
							}
						}
					}

					Return = true;
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_RENDERWINDOWCONTEXT_USEALTERNATEMOVEMENTSETTINGS:
					{
						TESRenderWindow::UseAlternateMovementSettings = (TESRenderWindow::UseAlternateMovementSettings == false);

						if (TESRenderWindow::UseAlternateMovementSettings == false)
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Using vanilla movement settings");
						else
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Using alternate movement settings");

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_STATICCAMERAPIVOT:
					{
						Settings::Renderer::kFixedCameraPivot.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_UNLOADCURRENTCELLS:
					TESRenderWindow::Reset();

					break;
				case IDC_RENDERWINDOWCONTEXT_SWITCHCNY:
					{
						Settings::Renderer::kSwitchCAndY.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
					TESRenderWindow::FreezeInactiveRefs = (TESRenderWindow::FreezeInactiveRefs == false);

					if (TESRenderWindow::FreezeInactiveRefs)
						RenderWindowPainter::RenderChannelNotifications->Queue(4, "Inactive references frozen");
					else
						RenderWindowPainter::RenderChannelNotifications->Queue(4, "Inactive references thawed");

					break;
				case IDC_RENDERWINDOWCONTEXT_INVERTSELECTION:
					if (*TESRenderWindow::PathGridEditFlag == 0)
					{
						CellObjectListT Refs;
						if (TESRenderWindow::GetActiveCellObjects(Refs))
						{
							TESRenderSelection* Buffer = TESRenderSelection::CreateInstance(_RENDERSEL);
							_RENDERSEL->ClearSelection(true);

							for (CellObjectListT::iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
							{
								TESObjectREFR* Ref = *Itr;

								if (Buffer->HasObject(Ref) == false)
									_RENDERSEL->AddToSelection(Ref, true);
							}

							Buffer->DeleteInstance();
							TESRenderWindow::Redraw();
						}
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_BATCHREFERENCEEDITOR:
					{
						CellObjectListT Refs;
						UInt32 RefCount = TESRenderWindow::GetActiveCellObjects(Refs);

						if (RefCount > 1)
						{
							int i = 0;
							ComponentDLLInterface::CellObjectData* RefData = new ComponentDLLInterface::CellObjectData[RefCount];
							ComponentDLLInterface::BatchRefData* BatchData = new ComponentDLLInterface::BatchRefData();

							for (CellObjectListT::iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr, ++i)
							{
								TESObjectREFR* ThisRef = *Itr;
								ComponentDLLInterface::CellObjectData* ThisRefData = &RefData[i];

								ThisRefData->EditorID = (!ThisRef->editorID.c_str())?ThisRef->baseForm->editorID.c_str():ThisRef->editorID.c_str();
								ThisRefData->FormID = ThisRef->formID;
								ThisRefData->TypeID = ThisRef->baseForm->formType;
								ThisRefData->Flags = ThisRef->formFlags;
								ThisRefData->Selected = false;

								for (TESRenderSelection::SelectedObjectsEntry* j = _RENDERSEL->selectionList; j != 0; j = j->Next)
								{
									if (j->Data && j->Data == ThisRef)
									{
										ThisRefData->Selected = true;
										break;
									}
								}

								ThisRefData->ParentForm = ThisRef;
							}

							BatchData->CellObjectListHead = RefData;
							BatchData->ObjectCount = RefCount;

							if (CLIWrapper::Interfaces::BE->ShowBatchRefEditorDialog(BatchData))
							{
								for (UInt32 k = 0; k < RefCount; k++)
								{
									TESObjectREFR* ThisRef = (TESObjectREFR*)RefData[k].ParentForm;
									ComponentDLLInterface::CellObjectData* ThisRefData = &RefData[k];
									bool Modified = false;

									if (ThisRefData->Selected)
									{
										if (BatchData->World3DData.UsePosX())	ThisRef->position.x = BatchData->World3DData.PosX, Modified = true;
										if (BatchData->World3DData.UsePosY())	ThisRef->position.y = BatchData->World3DData.PosY, Modified = true;
										if (BatchData->World3DData.UsePosZ())	ThisRef->position.z = BatchData->World3DData.PosZ, Modified = true;

										if (BatchData->World3DData.PosChanged())
											ThisRef->SetPosition(ThisRef->position.x, ThisRef->position.y, ThisRef->position.z);

										if (BatchData->World3DData.UseRotX())	ThisRef->rotation.x = BatchData->World3DData.RotX * PI / 180, Modified = true;
										if (BatchData->World3DData.UseRotY())	ThisRef->rotation.y = BatchData->World3DData.RotY * PI / 180, Modified = true;
										if (BatchData->World3DData.UseRotZ())	ThisRef->rotation.z = BatchData->World3DData.RotZ * PI / 180, Modified = true;

										if (BatchData->World3DData.RotChanged())
											ThisRef->SetRotation(ThisRef->rotation.x, ThisRef->rotation.y, ThisRef->rotation.z, true);

										if (BatchData->World3DData.UseScale())	ThisRef->SetScale(BatchData->World3DData.Scale), Modified = true;

										if (BatchData->Flags.UsePersistent() &&
											ThisRef->baseForm->formType != TESForm::kFormType_NPC &&
											ThisRef->baseForm->formType != TESForm::kFormType_Creature)
										{
											ThisRef->SetQuestItem(BatchData->Flags.Persistent);
											Modified = true;
										}

										if (BatchData->Flags.UseDisabled())
											SME::MiscGunk::ToggleFlag(&ThisRef->formFlags,
											TESForm::kFormFlags_Disabled,
											BatchData->Flags.Disabled), Modified = true;

										if (BatchData->Flags.UseVWD())
											SME::MiscGunk::ToggleFlag(&ThisRef->formFlags,
											TESForm::kFormFlags_VisibleWhenDistant,
											BatchData->Flags.VWD), Modified = true;

										if (BatchData->EnableParent.UseEnableParent())
										{
											TESObjectREFR* Parent = (TESObjectREFR*)BatchData->EnableParent.Parent;
											if (Parent != ThisRef)
											{
												ThisRef->extraData.ModExtraEnableStateParent(Parent);
												ThisRef->SetExtraEnableStateParentOppositeState(BatchData->EnableParent.OppositeState);
												Modified = true;
											}
										}

										if (BatchData->Ownership.UseOwnership() &&
											ThisRef->baseForm->formType != TESForm::kFormType_NPC &&
											ThisRef->baseForm->formType != TESForm::kFormType_Creature)
										{
											ThisRef->extraData.ModExtraGlobal(NULL);
											ThisRef->extraData.ModExtraRank(-1);
											ThisRef->extraData.ModExtraOwnership(NULL);

											TESForm* Owner = (TESForm*)BatchData->Ownership.Owner;
											ThisRef->extraData.ModExtraOwnership(Owner);

											if (BatchData->Ownership.UseNPCOwner())
												ThisRef->extraData.ModExtraGlobal((TESGlobal*)BatchData->Ownership.Global);
											else
												ThisRef->extraData.ModExtraRank(BatchData->Ownership.Rank);

											Modified = true;
										}

										if (BatchData->Extra.UseCharge())		ThisRef->ModExtraCharge((float)BatchData->Extra.Charge), Modified = true;
										if (BatchData->Extra.UseHealth())		ThisRef->ModExtraHealth((float)BatchData->Extra.Health), Modified = true;
										if (BatchData->Extra.UseTimeLeft())		ThisRef->ModExtraTimeLeft((float)BatchData->Extra.TimeLeft), Modified = true;
										if (BatchData->Extra.UseSoulLevel())	ThisRef->ModExtraSoul(BatchData->Extra.SoulLevel), Modified = true;
										if (BatchData->Extra.UseCount())
										{
											switch (ThisRef->baseForm->formType)
											{
											case TESForm::kFormType_Apparatus:
											case TESForm::kFormType_Armor:
											case TESForm::kFormType_Book:
											case TESForm::kFormType_Clothing:
											case TESForm::kFormType_Ingredient:
											case TESForm::kFormType_Misc:
											case TESForm::kFormType_Weapon:
											case TESForm::kFormType_Ammo:
											case TESForm::kFormType_SoulGem:
											case TESForm::kFormType_Key:
											case TESForm::kFormType_AlchemyItem:
											case TESForm::kFormType_SigilStone:
												ThisRef->extraData.ModExtraCount(BatchData->Extra.Count), Modified = true;

												break;
											case TESForm::kFormType_Light:
												TESObjectLIGH* Light = CS_CAST(ThisRef->baseForm, TESForm, TESObjectLIGH);
												if (Light)
												{
													if (Light->IsCarriable())
														ThisRef->extraData.ModExtraCount(BatchData->Extra.Count), Modified = true;
												}

												break;
											}
										}
									}

									if (Modified)
									{
										ThisRef->SetFromActiveFile(true);
									}
								}

								BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
							}

							delete BatchData;
						}

						break;
					}
				case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
				case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
					{
						CellObjectListT Refs;
						TESRenderWindow::GetActiveCellObjects(Refs);

						for (CellObjectListT::iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
						{
							TESObjectREFR* Ref = *Itr;

							switch (LOWORD(wParam))
							{
							case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
								if (Ref->GetInvisible())
									Ref->ToggleInvisiblity();

								if (Ref->GetChildrenInvisible())
									Ref->ToggleChildrenInvisibility();

								break;
							case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
								Ref->SetFrozenState(false);

								break;
							}
						}

						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Reset visibility flags on the active cell's references");

							break;
						case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Thawed all of the active cell's references");

							break;
						}

						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						TESRenderWindow::Redraw();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
				case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
				case IDC_RENDERWINDOWCONTEXT_FREEZE:
				case IDC_RENDERWINDOWCONTEXT_THAW:
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);

						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
							Ref->ToggleInvisiblity();

							break;
						case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
							Ref->ToggleChildrenInvisibility();

							break;
						case IDC_RENDERWINDOWCONTEXT_FREEZE:
							Ref->SetFrozenState(true);

							break;
						case IDC_RENDERWINDOWCONTEXT_THAW:
							Ref->SetFrozenState(false);

							break;
						}
					}

					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
					TESRenderWindow::Redraw();
					Return = true;

					break;
				case IDC_RENDERWINDOWCONTEXT_GROUP:
				case IDC_RENDERWINDOWCONTEXT_UNGROUP:
					if (_RENDERSEL->selectionCount > 1)
					{
						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_GROUP:
							if (CSERenderSelectionGroupManager::Instance.AddGroup(_RENDERSEL) == false)
							{
								BGSEEUI->MsgBoxW(hWnd, 0,
												"Couldn't add current selection to a new group.\n\nCheck the console for more details.");
							}
							else
								RenderWindowPainter::RenderChannelNotifications->Queue(4, "Created new selection group");

							break;
						case IDC_RENDERWINDOWCONTEXT_UNGROUP:
							if (CSERenderSelectionGroupManager::Instance.RemoveGroup(_RENDERSEL) == false)
							{
								BGSEEUI->MsgBoxW(hWnd, 0,
												"Couldn't dissolve the current selection's group.\n\nCheck the console for more details.");
							}
							else
								RenderWindowPainter::RenderChannelNotifications->Queue(4, "Removed selection group");

							break;
						}

						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_ALIGNTOX:
				case IDC_RENDERWINDOWCONTEXT_ALIGNTOY:
				case IDC_RENDERWINDOWCONTEXT_ALIGNTOZ:
					if (_RENDERSEL->selectionCount > 1)
					{
						// record the op twice, otherwise the thingy will crash on undo for some reason
						_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_Unk03, _RENDERSEL->selectionList);
						_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_Unk03, _RENDERSEL->selectionList);

						TESObjectREFR* AlignRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList->Next; Itr && Itr->Data; Itr = Itr->Next)
						{
							TESObjectREFR* ThisRef = CS_CAST(Itr->Data, TESForm, TESObjectREFR);

							switch (LOWORD(wParam))
							{
							case IDC_RENDERWINDOWCONTEXT_ALIGNTOX:
								ThisRef->position.x = AlignRef->position.x;
								break;
							case IDC_RENDERWINDOWCONTEXT_ALIGNTOY:
								ThisRef->position.y = AlignRef->position.y;
								break;
							case IDC_RENDERWINDOWCONTEXT_ALIGNTOZ:
								ThisRef->position.z = AlignRef->position.z;
								break;
							}

							ThisRef->SetFromActiveFile(true);
							ThisRef->UpdateNiNode();
						}

						RenderWindowPainter::RenderChannelNotifications->Queue(2, "Selection aligned to %08X", AlignRef->formID);
						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						TESRenderWindow::Redraw();

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_COPLANARDROP:
					{
						Settings::Renderer::kCoplanarRefDrops.ToggleData();

						if (Settings::Renderer::kCoplanarRefDrops.GetData().i)
							RenderWindowPainter::RenderChannelNotifications->Queue(6, "Enabled co-planar dropping");
						else
							RenderWindowPainter::RenderChannelNotifications->Queue(6, "Disabled co-planar dropping");

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_COPYTOGLOBALCLIPBOARD:
					{
						CSEFormListBuilder Buffer;

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
							Buffer.Add(Itr->Data);

						Buffer.Copy();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_PASTEFROMGLOBALCLIPBOARD:
					{
						BGSEECLIPBOARD->Paste();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_MOUSEREFENABLED:
					{
						Settings::RenderWindowPainter::kShowMouseRef.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_MOUSEREFCTRLMODIFIED:
					{
						Settings::RenderWindowPainter::kMouseRefCtrlModified.ToggleData();
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_SAVEEXTERIORSNAPSHOT:
					{
						SME_ASSERT(*TESRenderWindow::ActiveCell && _TES->currentInteriorCell == NULL);

						TESLODTextureGenerator::SaveExteriorSnapshot(*TESRenderWindow::ActiveCell,
																	Settings::Renderer::kExteriorSnapshotResolution.GetData().i,
																	NULL) ;
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

#define IDT_RENDERWINDOW_TITLEBARUPDATE			0x154

		LRESULT CALLBACK RenderWindowMiscSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = TRUE;
			Return = false;

			UInt8* YKeyState = (UInt8*)0x00A0BC1E;
			float* UnkRotFactor = (float*)0x00A0BAC4;

			static Vector3 kCameraStaticPivot;

			switch (uMsg)
			{
			case WM_RENDERWINDOW_UPDATEFOV:
				{
					Return = true;

					float CameraFOV = Settings::Renderer::kCameraFOV.GetData().f;
					if (CameraFOV > 120.0f)
						CameraFOV = 120.0f;
					else if (CameraFOV < 50.0f)
						CameraFOV = 50.0f;

					TESRender::SetCameraFOV(_PRIMARYRENDERER->primaryCamera, CameraFOV);
					memcpy(&TESRenderWindow::CameraFrustumBuffer, &_PRIMARYRENDERER->primaryCamera->m_kViewFrustum, sizeof(NiFrustum));
				}

				break;
			case WM_INITDIALOG:
				{
					CSERenderWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSERenderWindowMiscData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSERenderWindowMiscData();
						ExtraData->Add(xData);
					}

					SetTimer(hWnd, IDT_RENDERWINDOW_TITLEBARUPDATE, 2000, NULL);
				}

				break;
			case WM_DESTROY:
				{
					CSERenderWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSERenderWindowMiscData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(CSERenderWindowMiscData::kTypeID);
						delete xData;
					}

					KillTimer(hWnd, IDT_RENDERWINDOW_TITLEBARUPDATE);
				}

				break;
			case WM_RENDERWINDOW_GETCAMERASTATICPIVOT:
				{
					SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (LONG_PTR)&kCameraStaticPivot);
					DlgProcResult = TRUE;
					Return = true;
				}

				break;
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_RenderWindow, NULL);
				Return = true;

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case IDT_RENDERWINDOW_TITLEBARUPDATE:
					{
						char Buffer[0x100] = {0};
						GetWindowText(hWnd, Buffer, sizeof(Buffer));
						if (!_stricmp("Render Window", Buffer))
							break;

						TESObjectCELL* CurrentCell = _TES->currentInteriorCell;
						if (CurrentCell == NULL)
							CurrentCell = *TESRenderWindow::ActiveCell;

						if (CurrentCell)
						{
							if (CurrentCell->GetIsInterior())
							{
								FORMAT_STR(Buffer, "%s (%08X)", CurrentCell->GetEditorID(), CurrentCell->formID);
							}
							else
							{
								FORMAT_STR(Buffer, "%s %d,%d (%08X)", CurrentCell->GetEditorID(), CurrentCell->cellData.coords->x,
										CurrentCell->cellData.coords->y, CurrentCell->formID);
							}

							SetWindowText(hWnd, Buffer);
						}

						Return = true;
					}

					break;
				case TESRenderWindow::kTimer_ViewportUpdate:			// update timer
					static bool SetTimerPeriod = true;
					if (SetTimerPeriod)
					{
						SetTimerPeriod = false;
						UInt32 Period = Settings::Renderer::kUpdatePeriod.GetData().i;
						if (Period == 0 || Period >= 100)
							Period = 50;

						SetTimer(hWnd, TESRenderWindow::kTimer_ViewportUpdate, Period, NULL);
					}

					if (TESLODTextureGenerator::GeneratorState != TESLODTextureGenerator::kLODDiffuseMapGeneratorState_NotInUse)
					{
						// prevent the OS from triggering the screen-saver/switching to standby mode
						SetThreadExecutionState(ES_CONTINUOUS|ES_DISPLAY_REQUIRED|ES_SYSTEM_REQUIRED);
					}

					if (BGSEditorExtender::BGSEERenderWindowFlyCamera::FlyCamModeActive == false)
					{
						// the primary camera's view frustum gets updated ever so often, resetting its horizontal FOV
						// we update it here in case it has changed
						if (_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.l != TESRenderWindow::CameraFrustumBuffer.l ||
							_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.r != TESRenderWindow::CameraFrustumBuffer.r ||
							_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.b != TESRenderWindow::CameraFrustumBuffer.b ||
							_PRIMARYRENDERER->primaryCamera->m_kViewFrustum.t != TESRenderWindow::CameraFrustumBuffer.t)
						{
							SendMessage(hWnd, WM_RENDERWINDOW_UPDATEFOV, NULL, NULL);
						}
					}

					break;
				}

				break;
			case WM_SIZING:
				{
					if (TESPreviewControl::ActivePreviewControls->Count())
					{
						BGSEEUI->MsgBoxW(hWnd, 0, "Please close any dialogs with preview controls before attempting to resize the render window.");

						Return = true;
					}
				}

				break;
			case WM_LBUTTONDBLCLK:
				{
					if (*TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0 && GetAsyncKeyState(VK_MENU) == 0)
					{
						TESObjectREFR* Ref = TESRender::PickAtCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
						if (Ref)
						{
							if (Ref->GetFrozen() || (Ref->IsActive() == false && TESRenderWindow::FreezeInactiveRefs))
							{
								// ref's frozen, preempt the vanilla handler
								Return = true;
							}
						}
					}
				}

				break;
			case WM_MOUSEMOVE:
				{
					TESRenderWindow::CurrentMouseCoord.x = GET_X_LPARAM(lParam);
					TESRenderWindow::CurrentMouseCoord.y = GET_Y_LPARAM(lParam);

					TESObjectREFR* LastMouseRef = TESRenderWindow::CurrentMouseRef;
					TESRenderWindow::CurrentMouseRef = NULL;

					if (GetActiveWindow() == hWnd && GetCapture() != hWnd && *TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0)
					{
						int Enabled = Settings::RenderWindowPainter::kShowMouseRef.GetData().i;
						int ControlModified = Settings::RenderWindowPainter::kMouseRefCtrlModified.GetData().i;

						if (Enabled && (ControlModified == false || GetAsyncKeyState(VK_CONTROL)))
						{
							TESRenderWindow::CurrentMouseRef = TESRender::PickAtCoords(TESRenderWindow::CurrentMouseCoord.x,
								TESRenderWindow::CurrentMouseCoord.y);

							if (_RENDERSEL->selectionCount == 1 && _RENDERSEL->selectionList->Data == TESRenderWindow::CurrentMouseRef)
								TESRenderWindow::CurrentMouseRef = NULL;

							if (TESRenderWindow::CurrentMouseRef ||
								(LastMouseRef && TESRenderWindow::CurrentMouseRef == NULL))
							{
								TESRenderWindow::Redraw();
							}
						}
					}

					if (GetCapture() != hWnd)
					{
						HCURSOR* Icon = TESRenderWindow::CursorArrow;

						if (*TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0)
						{
							TESObjectREFR* MouseRef = TESRender::PickAtCoords(TESRenderWindow::CurrentMouseCoord.x, TESRenderWindow::CurrentMouseCoord.y);
							if (MouseRef)
							{
								if (_RENDERSEL->HasObject(MouseRef))
								{
									Icon = TESRenderWindow::CursorMove;
								}
								else
								{
									Icon = TESRenderWindow::CursorSelect;
								}
							}
						}

						if (Icon && GetCursor() != *Icon)
						{
							SetCursor(*Icon);
						}
					}
				}

				break;
			case WM_MOUSELEAVE:
			case WM_NCMOUSELEAVE:
				TESRenderWindow::CurrentMouseRef = NULL;
				TESRenderWindow::Redraw();

				break;
			case WM_LBUTTONDOWN:
				TESRenderWindow::CurrentMouseLBDragCoordDelta.x = GET_X_LPARAM(lParam);
				TESRenderWindow::CurrentMouseLBDragCoordDelta.y = GET_Y_LPARAM(lParam);

				if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL))
				{
					// place palette object, if any
					Return = true;
					ObjectPalette::CSEObjectPaletteManager::Instance.PlaceObject(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				}

				break;
			case WM_LBUTTONUP:
				TESRenderWindow::CurrentMouseLBDragCoordDelta.x -= GET_X_LPARAM(lParam);
				TESRenderWindow::CurrentMouseLBDragCoordDelta.y -= GET_Y_LPARAM(lParam);

				TESRenderWindow::CurrentMouseLBDragCoordDelta.x = abs(TESRenderWindow::CurrentMouseLBDragCoordDelta.x);
				TESRenderWindow::CurrentMouseLBDragCoordDelta.y = abs(TESRenderWindow::CurrentMouseLBDragCoordDelta.y);

				break;
			case WM_KEYUP:
				switch (wParam)
				{
				case 0x43:		// C
					{
						int SwitchEnabled = Settings::Renderer::kSwitchCAndY.GetData().i;
						if (SwitchEnabled)
						{
							if (*YKeyState)
								*UnkRotFactor = 0.0;

							*YKeyState = 0;

							Return = true;
						}
					}

					break;
				case 0x59:		// Y
					{
						int SwitchEnabled = Settings::Renderer::kSwitchCAndY.GetData().i;
						if (SwitchEnabled)
						{
							Return = true;
						}
					}

					break;
				}

				break;
			case WM_KEYDOWN:
				switch (wParam)
				{
				case VK_SHIFT:
					{
						_PRIMARYRENDERER->GetCameraPivot(&kCameraStaticPivot, 0.18);
					}

					break;
				case VK_F1:		// F1
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						TESDialog::ShowUseReportDialog(Ref);
					}

					Return = true;

					break;
				case 0x5A:		// Z
					if (*TESRenderWindow::PathGridEditFlag && GetAsyncKeyState(VK_CONTROL))
					{
						CSEPathGridUndoManager::Instance.PerformUndo();
						Return = true;
					}

					break;
				case 0x59:		// Y
					if (*TESRenderWindow::PathGridEditFlag && GetAsyncKeyState(VK_CONTROL))
					{
						CSEPathGridUndoManager::Instance.PerformRedo();
						Return = true;
					}
					else
					{
						int SwitchEnabled = Settings::Renderer::kSwitchCAndY.GetData().i;
						CSERenderWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSERenderWindowMiscData, ExtraData);
						SME_ASSERT(xData);

						if (SwitchEnabled)
						{
							xData->TunnelingKeyMessage = true;
							SendMessage(hWnd, WM_KEYDOWN, 0x43, lParam);
							xData->TunnelingKeyMessage = false;

							Return = true;
						}
					}

					break;
				case 0x43:		// C
					{
						int SwitchEnabled = Settings::Renderer::kSwitchCAndY.GetData().i;
						CSERenderWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSERenderWindowMiscData, ExtraData);
						SME_ASSERT(xData);

						if (SwitchEnabled && xData->TunnelingKeyMessage == false)
						{
							if (*YKeyState == 0)
								*UnkRotFactor = 0.0;

							*YKeyState = 1;

							Return = true;
						}
					}

					break;
				case 0x52:		// R
					if (*TESRenderWindow::PathGridEditFlag)
					{
						if (GetAsyncKeyState(VK_CONTROL))
						{
							CSEPathGridUndoManager::Instance.ResetRedoStack();

							if (TESRenderWindow::SelectedPathGridPoints->Count())
								CSEPathGridUndoManager::Instance.RecordOperation(CSEPathGridUndoManager::kOperation_DataChange, TESRenderWindow::SelectedPathGridPoints);

							for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
							{
								TESPathGridPoint* Point = Itr.Get();
								Point->UnlinkFromReference();
								Point->HideSelectionRing();
							}

							TESRenderWindow::Redraw(true);
							BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

							Return = true;
						}
						else
						{
							std::list<TESPathGridPoint*> Delinquents;

							for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
							{
								if (Itr.Get()->linkedRef)
									Delinquents.push_back(Itr.Get());

								Itr.Get()->HideSelectionRing();
							}

							for (std::list<TESPathGridPoint*>::iterator Itr = Delinquents.begin(); Itr != Delinquents.end(); Itr++)
								thisCall<void>(0x00452AE0, TESRenderWindow::SelectedPathGridPoints, *Itr);
						}
					}

					break;
				case 0x51:		// Q
					if (GetAsyncKeyState(VK_CONTROL))
					{
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_USEALTERNATEMOVEMENTSETTINGS, NULL);
						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

						Return = true;
					}
					else if (GetAsyncKeyState(VK_SHIFT) && AUXVIEWPORT->GetVisible())
					{
						if (AUXVIEWPORT->ToggleFrozenState())
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Froze auxiliary viewport camera");
						else
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Released auxiliary viewport camera");

						Return = true;
					}

					break;
				case 0x45:		// E
					if (GetAsyncKeyState(VK_SHIFT))
					{
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_COPLANARDROP, NULL);

						Return = true;
					}

					break;
				case 0x56:		// V
					if (GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_MENU))
					{
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_INVERTSELECTION, NULL);

						Return = true;
					}

					break;
				case 0x32:		// 2
					SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY, NULL);
					TESRenderWindow::Redraw();

					Return = true;
					break;
				case 0x47:		// G
					SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, TESCSMain::kToolbar_PathGridEdit, NULL);

					Return = true;
					break;
				case VK_F5:
					{
						SendMessage(hWnd, WM_RENDERWINDOW_UPDATEFOV, NULL, NULL);
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL, NULL);
					}

					break;
				case VK_OEM_3:	// ~
					{
						if (TESLODTextureGenerator::GeneratorState != TESLODTextureGenerator::kLODDiffuseMapGeneratorState_NotInUse)
							break;
						else if (GetCapture())
							break;

						TESRenderWindow::CurrentMouseRef = NULL;

						BGSEditorExtender::BGSEERenderWindowFlyCamera* xFreeCamData = BGSEE_GETWINDOWXDATA(BGSEditorExtender::BGSEERenderWindowFlyCamera, ExtraData);
						SME_ASSERT(xFreeCamData == NULL);

						xFreeCamData = new BGSEditorExtender::BGSEERenderWindowFlyCamera(new CSERenderWindowFlyCameraOperator(hWnd,
																															TESDialog::kDialogTemplate_RenderWindow));
						ExtraData->Add(xFreeCamData);

						Return = true;
					}

					break;
				case 0x41:		// A
					if (GetAsyncKeyState(VK_CONTROL))
					{
						Return = true;

						CellObjectListT Refs;
						TESRenderWindow::GetActiveCellObjects(Refs);

						for (CellObjectListT::iterator Itr = Refs.begin(); Itr != Refs.end(); ++Itr)
						{
							TESObjectREFR* Ref = *Itr;

							if (_RENDERSEL->HasObject(Ref) == false &&
								(Ref->formFlags & TESObjectREFR::kSpecialFlags_3DInvisible) == false &&
								(Ref->formFlags & TESObjectREFR::kSpecialFlags_Frozen) == false)
							{
								_RENDERSEL->AddToSelection(Ref, true);
							}
						}
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK ObjectWindowSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND FormList = GetDlgItem(hWnd, TESObjectWindow::kFormListCtrlID);
			HWND TreeList = GetDlgItem(hWnd, TESObjectWindow::kTreeViewCtrlID);

			switch (uMsg)
			{
			case WM_NOTIFY:
				{
					bool Skip = true;

					if (((NMHDR*)lParam)->code == NM_KEYDOWN && ((NMKEY*)lParam)->nVKey == VK_F5)
						Skip = false;
					else if (((NMHDR*)lParam)->code == LVN_KEYDOWN && ((NMLVKEYDOWN*)lParam)->wVKey == VK_F5)
						Skip = false;
					else if (((NMHDR*)lParam)->code == TVN_KEYDOWN && ((NMTVKEYDOWN*)lParam)->wVKey == VK_F5)
						Skip = false;

					if (Skip)
						break;

					Return = true;
					ObjectWindowImposterManager::Instance.RefreshImposters();
					SendMessage(hWnd, 0x41A, NULL, NULL);
				}
			case WM_ACTIVATE:
				ObjectWindowImposterManager::Instance.HandleObjectWindowActivating(hWnd, uMsg, wParam, lParam);
				Return = true;

				break;
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_ObjectWindow, NULL);
				Return = true;

				break;
			case 0x417:		// destroy window
			case WM_DESTROY:
				{
					CSEFilterableFormListManager::Instance.Unregister(FilterEditBox);
					ObjectWindowImposterManager::Instance.DestroyImposters();
					TESObjectWindow::PrimaryObjectWindowHandle = NULL;
				}

				break;
			case WM_INITDIALOG:
				{
					SME_ASSERT(FilterEditBox);
					CSEFilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL), FormList, hWnd);

					std::string WndTitle = "Object Window";
					if (Settings::General::kShowHallOfFameMembersInTitleBar().i != HallOfFame::kDisplayESMember_None)
					{
						HallOfFame::GetRandomESMember(WndTitle);
						WndTitle += " Object Window";
					}
					SetWindowText(hWnd, WndTitle.c_str());

					TESObjectWindow::PrimaryObjectWindowHandle = hWnd;
				}

				break;
			case WM_SIZE:
				ObjectWindowImposterManager::Instance.HandleObjectWindowSizing(hWnd, uMsg, wParam, lParam);
				Return = true;

				break;
			}

			if (Return == false && CSEFilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
			{
				HTREEITEM Selection = TreeView_GetSelection(TreeList);

				TreeView_SelectItem(TreeList, NULL);
				TreeView_SelectItem(TreeList, Selection);
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK CellViewWindowSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND WorldspaceLabel = GetDlgItem(hWnd, TESCellViewWindow::kWorldspaceLabel);
			HWND WorldspaceCombo = GetDlgItem(hWnd, TESCellViewWindow::kWorldspaceComboBox);
			HWND CellLabel = GetDlgItem(hWnd, TESCellViewWindow::kCellLabel);
			HWND CellList = GetDlgItem(hWnd, TESCellViewWindow::kCellListView);
			HWND RefList = GetDlgItem(hWnd, TESCellViewWindow::kObjectRefListView);

			HWND RefFilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND RefFilterLabel = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL);
			HWND XLabel = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_XLBL);
			HWND YLabel = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_YLBL);
			HWND XEdit = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_XEDIT);
			HWND YEdit = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_YEDIT);
			HWND GoBtn = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_GOBTN);
			HWND CellFilterEditBox = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_CELLFILTEREDIT);
			HWND CellFilterLabel = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_CELLFILTERLBL);
			int* RefListSortColumn = TESCellViewWindow::ObjectListSortColumn;

			switch (uMsg)
			{
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_CellViewWindow, NULL);
				Return = true;

				break;
			case WM_DESTROY:
				{
					CSECellViewExtraData* xData = BGSEE_GETWINDOWXDATA(CSECellViewExtraData, ExtraData);

					if (xData)
					{
						ExtraData->Remove(CSECellViewExtraData::kTypeID);
						delete xData;
					}
				}
			case 0x417:		// destroy window
				CSEFilterableFormListManager::Instance.Unregister(RefFilterEditBox);
				CSEFilterableFormListManager::Instance.Unregister(CellFilterEditBox);

				break;
			case WM_INITDIALOG:
				{
					CSECellViewExtraData* xData = BGSEE_GETWINDOWXDATA(CSECellViewExtraData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSECellViewExtraData();
						ExtraData->Add(xData);

						RECT Bounds = {0};

						SME::UIHelpers::GetClientRectInitBounds(RefFilterEditBox, hWnd, &xData->RefFilterEditBox);
						SME::UIHelpers::GetClientRectInitBounds(RefFilterLabel, hWnd, &xData->RefFilterLabel);
						SME::UIHelpers::GetClientRectInitBounds(XLabel, hWnd, &xData->XLabel);
						SME::UIHelpers::GetClientRectInitBounds(YLabel, hWnd, &xData->YLabel);
						SME::UIHelpers::GetClientRectInitBounds(XEdit, hWnd, &xData->XEdit);
						SME::UIHelpers::GetClientRectInitBounds(YEdit, hWnd, &xData->YEdit);
						SME::UIHelpers::GetClientRectInitBounds(GoBtn, hWnd, &xData->GoBtn);
						SME::UIHelpers::GetClientRectInitBounds(CellFilterEditBox, hWnd, &xData->CellFilterEditBox);
						SME::UIHelpers::GetClientRectInitBounds(CellFilterLabel, hWnd, &xData->CellFilterLabel);

						TESDialog::GetPositionFromINI("Cell View", &Bounds);
						SetWindowPos(hWnd, NULL, Bounds.left, Bounds.top, Bounds.right, Bounds.bottom, SWP_NOZORDER);
					}

					CSEFilterableFormListManager::Instance.Register(RefFilterEditBox, RefFilterLabel, RefList, hWnd);
					CSEFilterableFormListManager::Instance.Register(CellFilterEditBox, CellFilterLabel, CellList, hWnd);

					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM|LVCF_FMT;
					ColumnData.fmt = LVCFMT_CENTER;

					ColumnData.cx = 45;
					ColumnData.pszText = "Persistent";
					ColumnData.iSubItem = CSECellViewExtraData::kExtraRefListColumn_Persistent;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 45;
					ColumnData.pszText = "Initially Disabled";
					ColumnData.iSubItem = CSECellViewExtraData::kExtraRefListColumn_Disabled;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 45;
					ColumnData.pszText = "VWD";
					ColumnData.iSubItem = CSECellViewExtraData::kExtraRefListColumn_VWD;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 100;
					ColumnData.pszText = "Enable Parent";
					ColumnData.iSubItem = CSECellViewExtraData::kExtraRefListColumn_EnableParent;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 40;
					ColumnData.pszText = "Count";
					ColumnData.iSubItem = CSECellViewExtraData::kExtraRefListColumn_Count;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case LVN_GETDISPINFO:
						if (NotificationData->hwndFrom == RefList)
						{
							NMLVDISPINFO* DisplayData = (NMLVDISPINFO*)lParam;

							if ((DisplayData->item.mask & LVIF_TEXT) && DisplayData->item.lParam)
							{
								DlgProcResult = TRUE;
								Return = true;

								TESObjectREFR* Current = (TESObjectREFR*)DisplayData->item.lParam;

								switch (DisplayData->item.iSubItem)
								{
								case CSECellViewExtraData::kExtraRefListColumn_Persistent:
									sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s",
											((Current->formFlags & TESForm::kFormFlags_QuestItem) ? "Y" : ""));

									break;
								case CSECellViewExtraData::kExtraRefListColumn_Disabled:
									sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s",
											((Current->formFlags & TESForm::kFormFlags_Disabled) ? "Y" : ""));

									break;
								case CSECellViewExtraData::kExtraRefListColumn_VWD:
									sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s",
											((Current->formFlags & TESForm::kFormFlags_VisibleWhenDistant) ? "Y" : ""));

									break;
								case CSECellViewExtraData::kExtraRefListColumn_EnableParent:
									{
										BSExtraData* xData = Current->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
										if (xData)
										{
											ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
											SME_ASSERT(xParent->parent);

											if (xParent->parent->editorID.c_str() == NULL)
											{
												sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax,
													"%08X %s",
													xParent->parent->formID,
													(xParent->oppositeState ? " *" : ""));
											}
											else
											{
												sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax,
													"%s %s",
													xParent->parent->editorID.c_str(),
													(xParent->oppositeState ? " *" : ""));
											}
										}
										else
											sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "");
									}

									break;
								case CSECellViewExtraData::kExtraRefListColumn_Count:
									{
										BSExtraData* xData = Current->extraData.GetExtraDataByType(BSExtraData::kExtra_Count);
										if (xData)
										{
											ExtraCount* xCount = CS_CAST(xData, BSExtraData, ExtraCount);
											sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%d", xCount->count);
										}
										else
											sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "");
									}

									break;
								default:
									Return = false;
								}
							}
						}

						break;
					case LVN_COLUMNCLICK:
						if (NotificationData->hwndFrom == RefList)
						{
							NMLISTVIEW* ListViewData = (NMLISTVIEW*)lParam;
							if (ListViewData->iSubItem >= CSECellViewExtraData::kExtraRefListColumn_Persistent)
							{
								if (*RefListSortColumn > 0)
								{
									*RefListSortColumn = ListViewData->iSubItem;
									*RefListSortColumn *= -1;
								}
								else
									*RefListSortColumn = ListViewData->iSubItem;

								SendMessage(RefList, LVM_SORTITEMS, *RefListSortColumn, (LPARAM)CSECellViewExtraData::CustomFormListComparator);

								DlgProcResult = TRUE;
								Return = true;
							}
						}

						break;
					}
				}

				break;
			case WM_SIZE:
				if (wParam != SIZE_MINIMIZED)
				{
					RECT CurrentRect = {0};

					RECT* BaseDlgRect = (RECT*)0x00A0AA38;
					RECT* BaseCellListRect = (RECT*)0x00A0AA48;
					RECT* BaseRefListRect = (RECT*)0x00A0AA04;
					RECT* BaseWorldspaceComboRect = (RECT*)0x00A0AA24;
					RECT* BaseWorldspaceLabelRect = (RECT*)0x00A0AA14;
					RECT* BaseCellNameLabelRect = (RECT*)0x00A0A9F0;

					SetRect(&CurrentRect, 0, 0, LOWORD(lParam), HIWORD(lParam));
					int DeltaDlgWidth = (CurrentRect.right - BaseDlgRect->right) >> 1;
					HDWP DeferPosData = BeginDeferWindowPos(3);

					DeferWindowPos(DeferPosData, CellList, 0,
								0, 0,
								DeltaDlgWidth + BaseCellListRect->right, CurrentRect.bottom + BaseCellListRect->bottom - BaseDlgRect->bottom,
								SWP_NOMOVE);

					DeferWindowPos(DeferPosData, WorldspaceCombo, 0,
								0, 0,
								BaseCellListRect->right + DeltaDlgWidth - BaseWorldspaceLabelRect->right, BaseWorldspaceComboRect->bottom,
								SWP_NOMOVE);

					DeferWindowPos(DeferPosData, RefList, 0,
								DeltaDlgWidth + BaseRefListRect->left, BaseRefListRect->top,
								DeltaDlgWidth + BaseRefListRect->right, CurrentRect.bottom + BaseRefListRect->bottom - BaseDlgRect->bottom,
								NULL);

					DeferWindowPos(DeferPosData, CellLabel, 0,
								BaseCellNameLabelRect->left + DeltaDlgWidth, BaseCellNameLabelRect->top,
								DeltaDlgWidth + BaseCellNameLabelRect->right, BaseCellNameLabelRect->bottom,
								NULL);

					InvalidateRect(CellLabel, NULL, TRUE);
					InvalidateRect(WorldspaceLabel, NULL, TRUE);

					EndDeferWindowPos(DeferPosData);

					CSECellViewExtraData* xData = BGSEE_GETWINDOWXDATA(CSECellViewExtraData, ExtraData);

					if (xData)
					{
						DeferPosData = BeginDeferWindowPos(8);

						DeferWindowPos(DeferPosData, XLabel, 0,
							DeltaDlgWidth + xData->XLabel.left, xData->XLabel.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, XEdit, 0,
							DeltaDlgWidth + xData->XEdit.left, xData->XEdit.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, YLabel, 0,
							DeltaDlgWidth + xData->YLabel.left, xData->YLabel.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, YEdit, 0,
							DeltaDlgWidth + xData->YEdit.left, xData->YEdit.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, GoBtn, 0,
							DeltaDlgWidth + xData->GoBtn.left, xData->GoBtn.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, RefFilterLabel, 0,
							DeltaDlgWidth + xData->RefFilterLabel.left, xData->RefFilterLabel.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, RefFilterEditBox, 0,
							DeltaDlgWidth + xData->RefFilterEditBox.left, xData->RefFilterEditBox.top,
							DeltaDlgWidth + xData->RefFilterEditBox.right, xData->RefFilterEditBox.bottom + 2,
							NULL);

						DeferWindowPos(DeferPosData, CellFilterEditBox, 0,
									   0, 0,
									   BaseCellListRect->right + DeltaDlgWidth - xData->CellFilterLabel.right - 7, xData->CellFilterEditBox.bottom,
									   SWP_NOMOVE);

						InvalidateRect(XLabel, NULL, TRUE);
						InvalidateRect(YLabel, NULL, TRUE);
						InvalidateRect(RefFilterLabel, NULL, TRUE);

						EndDeferWindowPos(DeferPosData);
					}

					Return = true;
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_CELLVIEW_GOBTN:
					{
						char XCoord[4] = {0}, YCoord[4] = {0};
						GetWindowText(XEdit, (LPSTR)XCoord, sizeof(XCoord));
						GetWindowText(YEdit, (LPSTR)YCoord, sizeof(YCoord));

						if (strlen(XCoord) && strlen(YCoord))
						{
							Vector3 Coords((atoi(XCoord) << 12) + 2048.0, (atoi(YCoord) << 12) + 2048.0, 0);
							_TES->LoadCellIntoViewPort(&Coords, NULL);
						}
					}

					break;
				}
			}

			if (Return == false)
			{
				if (CSEFilterableFormListManager::Instance.HandleMessages(RefFilterEditBox, uMsg, wParam, lParam))
					TESCellViewWindow::RefreshObjectList();

				if (CSEFilterableFormListManager::Instance.HandleMessages(CellFilterEditBox, uMsg, wParam, lParam))
					TESCellViewWindow::RefreshCellList();
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK ResponseDlgSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND VoiceList = GetDlgItem(hWnd, ResponseEditorData::kVoiceFileListView);

			switch (uMsg)
			{
			case WM_DESTROY:
				{
					Settings::General::kFaceGenPreviewVoiceDelay.SetInt(TESDialog::GetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY));
				}

				break;
			case WM_INITDIALOG:
				{
					EnableWindow(GetDlgItem(hWnd, ResponseEditorData::kGenerateLIPFileButton), TRUE);

					CheckDlgButton(hWnd,
								IDC_CSE_RESPONSEWINDOW_FACEGENPREVIEW,
								(Settings::General::kFaceGenPreviewResponseWindow.GetData().i ? BST_CHECKED : BST_UNCHECKED));

					TESDialog::ClampDlgEditField(GetDlgItem(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY), 0.0, 5000.0, true);
					TESDialog::SetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY, Settings::General::kFaceGenPreviewVoiceDelay.GetData().i, 0);

					SetForegroundWindow(hWnd);
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;

					switch (NotificationData->idFrom)
					{
					case ResponseEditorData::kVoiceFileListView:		// voice file listview
						{
							if (NotificationData->code == LVN_ITEMACTIVATE)
							{
								if (Settings::General::kFaceGenPreviewResponseWindow.GetData().i == 0)
									break;

								int SelectedItem = ListView_GetNextItem(VoiceList, -1, LVNI_SELECTED);

								char VoicePath[0x200] = {0};
								LVITEM SelectedVoiceItem;

								SelectedVoiceItem.iItem = SelectedItem;
								SelectedVoiceItem.iSubItem = 6;
								SelectedVoiceItem.mask = LVIF_TEXT;
								SelectedVoiceItem.pszText = VoicePath;
								SelectedVoiceItem.cchTextMax = sizeof(VoicePath);
								ListView_GetItem(VoiceList, &SelectedVoiceItem);

								std::string LipPath(VoicePath);
								LipPath = LipPath.substr(0, LipPath.rfind("."));
								LipPath += ".lip";

								HWND NPCEditDlg = NULL;
								for (tList<HWND>::Iterator Itr = TESDialog::OpenDialogWindows->Begin(); Itr.End() == false && Itr.Get(); ++Itr)
								{
									HWND Current = (HWND)Itr.Get();
									TESForm* Form = TESDialog::GetDialogExtraParam(Current);
									if (Form && Form->formType == TESForm::kFormType_NPC)
									{
										NPCEditDlg = Current;
										break;
									}
								}

								if (NPCEditDlg)
								{
									Return = true;

									CSEFaceGenVoicePreviewData PreviewData = {0};
									FORMAT_STR(PreviewData.VoicePath, "%s", VoicePath);
									FORMAT_STR(PreviewData.LipPath, "%s", LipPath.c_str());
									PreviewData.DelayTime = TESDialog::GetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY);

									SendMessage(NPCEditDlg, WM_FACEGENPREVIEW_PLAYVOICE, (WPARAM)&PreviewData, NULL);
								}
							}
						}

						break;
					}
				}

				break;
			case WM_COMMAND:
				{
					int SelectedItem = ListView_GetNextItem(VoiceList, -1, LVNI_SELECTED);

					char VoicePath[0x200] = {0};
					LVITEM SelectedVoiceItem;

					SelectedVoiceItem.iItem = SelectedItem;
					SelectedVoiceItem.iSubItem = 6;
					SelectedVoiceItem.mask = LVIF_TEXT;
					SelectedVoiceItem.pszText = VoicePath;
					SelectedVoiceItem.cchTextMax = sizeof(VoicePath);

					switch (LOWORD(wParam))
					{
					case IDC_CSE_RESPONSEWINDOW_FACEGENPREVIEW:
						{
							Settings::General::kFaceGenPreviewResponseWindow.SetInt(IsDlgButtonChecked(hWnd, IDC_CSE_RESPONSEWINDOW_FACEGENPREVIEW) == BST_CHECKED);
						}

						break;
					case ResponseEditorData::kCopyExternalFileButton:					// Copy external file
						{
							if (ListView_GetItem(VoiceList, &SelectedVoiceItem) != TRUE)
							{
								Return = true;
								break;
							}

							char FilePath[MAX_PATH] = {0};
							OPENFILENAME SelectFile = {0};
							SelectFile.lStructSize = sizeof(OPENFILENAME);
							SelectFile.hwndOwner = hWnd;
							SelectFile.lpstrFilter = "MP3 Files\0*.mp3\0\0";
							SelectFile.lpstrCustomFilter = NULL;
							SelectFile.nFilterIndex = 0;
							SelectFile.lpstrFile = FilePath;
							SelectFile.nMaxFile = sizeof(FilePath);
							SelectFile.lpstrFileTitle = NULL;
							SelectFile.lpstrInitialDir = NULL;
							SelectFile.lpstrTitle = "Select an audio file to use as the current response's voice";
							SelectFile.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR;

							if (GetOpenFileName(&SelectFile))
							{
								std::string Destination(std::string(BGSEEMAIN->GetAPPPath()) + "\\" + std::string(VoicePath)), DirPath(Destination);
								DirPath = DirPath.substr(0, DirPath.rfind("\\") + 1);

								if (SHCreateDirectoryEx(NULL, DirPath.c_str(), NULL) &&
									GetLastError() != ERROR_FILE_EXISTS &&
									GetLastError() != ERROR_ALREADY_EXISTS)
								{
									BGSEECONSOLE_ERROR("Couldn't create intermediate path for the new voice file!");
								}

								if (CopyFile(FilePath, Destination.c_str(), FALSE) == FALSE)
								{
									BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't copy external file '%s' to '%s'!\n\nCheck the console for more information.",
													FilePath, Destination.c_str());

									BGSEECONSOLE_ERROR("UIManager::ResponseDlgSubclassProc - Couldn't copy external file!");
								}
								else
									BGSEECONSOLE_MESSAGE("Copied external audio file '%s' to '%s'", FilePath, Destination.c_str());

								BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
							}

							Return = true;
						}

						break;
					case 1016:					// Generate LIP
						if (ListView_GetItem(VoiceList, &SelectedVoiceItem) != TRUE)
						{
							Return = true;
							break;
						}

						std::string Path(VoicePath);
						Path = Path.substr(0, Path.rfind("."));

						HWND IdleWindow = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), BGSEEUI->GetMainWindow(), NULL, NULL);

						if (CSIOM->GenerateLIPSyncFile(Path.c_str(), (*ResponseEditorData::EditorCache)->responseLocalCopy->responseText.c_str()) == false)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't generate LIP file for the selected voice.\n\nCheck the console for more information.");
						}
						else
						{
							BGSEECONSOLE_MESSAGE("Successfully generated LIP file for the selected voice");
							BGSEEACHIEVEMENTS->Unlock(Achievements::kSoprano);
						}

						DestroyWindow(IdleWindow);

						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						Return = true;
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK LandscapeTextureUseDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_NOTIFY:
				switch (((LPNMHDR)lParam)->code)
				{
				case LVN_ITEMACTIVATE:
					NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;
					char Buffer[0x200] = {0};
					ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 0, Buffer, sizeof(Buffer));

					TESForm* Texture = TESForm::LookupByEditorID(Buffer);

					if (Texture)
					{
						*TESRenderWindow::ActiveLandscapeTexture = CS_CAST(Texture, TESForm, TESLandTexture);

						SendMessage(*TESObjectLAND::WindowHandle, 0x41A, NULL, NULL);			// select the new texture in the landscape edit dialog
						SetForegroundWindow(*TESRenderWindow::WindowHandle);

						RenderWindowPainter::RenderChannelNotifications->Queue(3, "Active landscape texture changed");
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK FilteredDialogQuestDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					if (BGSEEUI->GetSubclasser()->GetDialogTemplate(hWnd) == TESDialog::kDialogTemplate_Quest)
					{
						LVCOLUMN ColumnData = {0};
						ColumnData.mask = LVCF_WIDTH;
						HWND QuestList = GetDlgItem(hWnd, kFormList_TESFormIDListView);

						ColumnData.cx = 200;
						ListView_SetColumn(QuestList, 0, &ColumnData);

						ColumnData.cx = 42;
						ListView_SetColumn(QuestList, 2, &ColumnData);
					}
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_QUEST_EDITRESULTSCRIPT:
					if (IsWindowEnabled(GetDlgItem(hWnd, kDialogEditor_ResultScriptTextBox)))
					{
						if (BGSEEUI->ModalDialog(BGSEEMAIN->GetExtenderHandle(),
												MAKEINTRESOURCE(IDD_EDITRESULTSCRIPT),
												hWnd,
												UIManager::EditResultScriptDlgProc,
												(LPARAM)hWnd))
						{
							SendMessage(hWnd, WM_COMMAND, kDialogEditor_CompileResultScriptButton, NULL);		// compile result script
						}
					}

					Return = true;
					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK AboutDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					HINSTANCE SplashImageResource = BGSEEUI->GetDialogHotSwapper()->GetAlternateResourceInstance(TESDialog::kDialogTemplate_SplashScreen);
					if (SplashImageResource)
					{
						HANDLE Image = LoadImage(SplashImageResource,
												MAKEINTRESOURCE(100),
												IMAGE_BITMAP,
												0,
												0,
												LR_DEFAULTSIZE);

						SME_ASSERT(Image);

						HWND PictureControl = GetDlgItem(hWnd, kAboutDialog_LogoPictureControl);
						SendMessage(PictureControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)Image);
					}

					DWORD FileVersionHandle = 0;
					DWORD FileVersionSize = GetFileVersionInfoSize(BGSEEMAIN->GetDLLPath(), &FileVersionHandle);

					if (FileVersionSize)
					{
						char* Buffer = new char[FileVersionSize];
						char VersionString[0x100] = {0};
						void* VersionStringPtr = NULL;

						GetFileVersionInfo(BGSEEMAIN->GetDLLPath(), FileVersionHandle, FileVersionSize, Buffer);
						VerQueryValue(Buffer, "\\StringFileInfo\\040904b0\\ProductVersion", &VersionStringPtr, (PUINT)FileVersionHandle);
						FORMAT_STR(VersionString, "%s v%s\r\n\"%s\"", BGSEEMAIN->ExtenderGetDisplayName(),
																	VersionStringPtr, BGSEEMAIN->ExtenderGetReleaseName());

						std::string ReplacedString(VersionString);
						std::replace(ReplacedString.begin(), ReplacedString.end(), ',', '.');

						SetDlgItemText(hWnd, kAboutDialog_VersionLabel, (LPCSTR)ReplacedString.c_str());

						if (IsWarholAGenius)
						{
							Edit_SetText(GetDlgItem(hWnd, kAboutDialog_CopyrightTextBox),
								"Pain, day, sky, beauty, black, die, joy,\n love, empty, time, sun, hurt, trust, peace, dark, rage, sad, white, rain, hate,\n anger, hope, sacred, passion, life, night,\n ache, soft light");
						}

						delete [] Buffer;
					}
				}

				break;
			case WM_DESTROY:
				{
					HANDLE Image = (HANDLE)SendDlgItemMessage(hWnd, kAboutDialog_LogoPictureControl, STM_GETIMAGE, IMAGE_BITMAP, NULL);

					if (Image)
						DeleteBitmap(Image);
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK RaceDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_RACE_COPYEYES:
				case IDC_CSE_RACE_COPYHAIR:
					{
						TESRace* WorkingRace = CS_CAST(TESDialog::GetDialogExtraLocalCopy(hWnd), TESForm, TESRace);
						TESForm* Selection = (TESForm*)BGSEEUI->ModalDialog(BGSEEMAIN->GetExtenderHandle(),
																			MAKEINTRESOURCE(IDD_TESCOMBOBOX),
																			hWnd,
																			(DLGPROC)UIManager::TESComboBoxDlgProc,
																			(LPARAM)TESForm::kFormType_Race);

						if (Selection)
						{
							TESRace* SelectedRace = CS_CAST(Selection, TESForm, TESRace);

							if (WorkingRace && WorkingRace != SelectedRace)
							{
								int Count = 0;
								switch (LOWORD(wParam))
								{
								case IDC_CSE_RACE_COPYEYES:
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

										BGSEEUI->MsgBoxI(hWnd, 0, "Copied %d eye forms from race '%s'.", Count, SelectedRace->editorID.c_str());
									}

									break;
								case IDC_CSE_RACE_COPYHAIR:
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

										BGSEEUI->MsgBoxI(hWnd, 0, "Copied %d hair forms from race '%s'.", Count, SelectedRace->editorID.c_str());
									}

									break;
								}
							}
						}
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

#define ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID						0x108
#define ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID					0x109

		LRESULT CALLBACK CommonDialogExtraFittingsSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;
			CSEDialogExtraFittingsData* xData = BGSEE_GETWINDOWXDATA_QUICK(CSEDialogExtraFittingsData, ExtraData);

			switch (uMsg)
			{
			case WM_INITDIALOG:
			case WM_OBJECTWINDOWIMPOSTER_INITIALIZEXTRA:
				{
					xData = BGSEE_GETWINDOWXDATA(CSEDialogExtraFittingsData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSEDialogExtraFittingsData();
						ExtraData->Add(xData);
					}

					SetTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID, 100, NULL);
					SetTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID, 650, NULL);
				}

				break;
			case WM_DESTROY:
				{
					xData = BGSEE_GETWINDOWXDATA(CSEDialogExtraFittingsData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(CSEDialogExtraFittingsData::kTypeID);
						delete xData;

						xData = NULL;
					}

					KillTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID);
					KillTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID);
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID:
					{
						if (xData && xData->LastCursorPosWindow && xData->TrackingToolTip == false)
						{
							HWND WindowAtPoint = xData->LastCursorPosWindow;
							bool Viewable = false;
							char Buffer[0x200] = {0}, ControlText[0x200] = {0};

							if (IsWindowEnabled(WindowAtPoint))
							{
								int CtrlID = GetDlgCtrlID(WindowAtPoint);
								LONG_PTR WindowStyle = GetWindowLongPtr(WindowAtPoint, GWL_STYLE);

								GetClassName(WindowAtPoint, Buffer, sizeof(Buffer));
								GetWindowText(WindowAtPoint, (LPSTR)ControlText, sizeof(ControlText));

								if ((!_stricmp("Button", Buffer) &&
									CtrlID != kFaceGenControl_PreviewCtrl &&
									(WindowStyle & BS_AUTOCHECKBOX) == false &&
									CtrlID > 5 && strlen(ControlText) > 3) ||
									!_stricmp("ComboBox", Buffer))
								{
									Viewable = true;
								}
							}

							if (Viewable)
							{
								// valid control, show tooltip
								if (strlen(ControlText))
								{
									ZeroMemory(&xData->AssetControlToolData, sizeof(TOOLINFO));
									xData->AssetControlToolData.cbSize = sizeof(TOOLINFO);
									xData->AssetControlToolData.uFlags = TTF_TRACK;
									xData->AssetControlToolData.lpszText = ControlText;

									SendMessage(xData->AssetControlToolTip, TTM_ADDTOOL, NULL, (LPARAM)&xData->AssetControlToolData);
									SendMessage(xData->AssetControlToolTip,
												TTM_TRACKPOSITION,
												NULL,
												(LPARAM)MAKELONG(xData->LastCursorPos.x + 18, xData->LastCursorPos.y));
									SendMessage(xData->AssetControlToolTip, TTM_SETMAXTIPWIDTH, NULL, (LPARAM)355);
									SendMessage(xData->AssetControlToolTip, TTM_SETWINDOWTHEME, NULL, (LPARAM)L"Explorer");
									SendMessage(xData->AssetControlToolTip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&xData->AssetControlToolData);

									xData->TrackingToolTip = true;
									xData->LastTrackedTool = WindowAtPoint;
								}
							}
						}

						else if (xData && xData->TrackingToolTip && xData->LastCursorPosWindow != xData->LastTrackedTool)
						{
							SendMessage(xData->AssetControlToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&xData->AssetControlToolData);
							SendMessage(xData->AssetControlToolTip, TTM_DELTOOL, NULL, (LPARAM)&xData->AssetControlToolData);

							xData->TrackingToolTip = false;
						}
					}

					break;
				case ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID:
					{
						// we need to defer the looked-up window's creation a bit to keep the source window from hogging focus
						if (xData && xData->LastCursorPosWindow && xData->QuickViewTriggered)
						{
							xData->QuickViewTriggered = false;

							HWND WindowAtPoint = xData->LastCursorPosWindow;
							TESForm* Form = NULL;

							char Buffer[0x200] = {0};
							GetClassName(WindowAtPoint, Buffer, sizeof(Buffer));

							if (!_stricmp("SysListView32", Buffer))
							{
								POINT Coords = {0};
								Coords.x = xData->LastCursorPos.x;
								Coords.y = xData->LastCursorPos.y;

								ScreenToClient(WindowAtPoint, &Coords);

								LVHITTESTINFO HitTestData = {0};
								HitTestData.pt.x = Coords.x;
								HitTestData.pt.y = Coords.y;
								HitTestData.flags = LVHT_ONITEM;

								if (ListView_SubItemHitTest(WindowAtPoint, &HitTestData) != -1)
								{
									int Item = HitTestData.iItem;
									int SubItem = HitTestData.iSubItem;

									ZeroMemory(Buffer, sizeof(Buffer));
									ListView_GetItemText(WindowAtPoint, Item, SubItem, Buffer, sizeof(Buffer));
								}
							}
							else
							{
								GetWindowText(WindowAtPoint, Buffer, sizeof(Buffer));
							}

							if (strlen(Buffer) < 2)
								break;

							std::string PotentialEditorID(Buffer);
							int StatusIndicatorOffset = -1;

							if ((StatusIndicatorOffset = PotentialEditorID.find(" *")) != std::string::npos)
								PotentialEditorID.erase(StatusIndicatorOffset, 2);
							if ((StatusIndicatorOffset = PotentialEditorID.find(" D")) != std::string::npos)
								PotentialEditorID.erase(StatusIndicatorOffset, 2);

							Form = TESForm::LookupByEditorID(PotentialEditorID.c_str());

							xData->LastCursorPosWindow = NULL;

							if (Form)
							{
								switch (Form->formType)
								{
								case TESForm::kFormType_Script:
									TESDialog::ShowScriptEditorDialog(Form);
									break;
								case TESForm::kFormType_REFR:
									_TES->LoadCellIntoViewPort((CS_CAST(Form, TESForm, TESObjectREFR))->GetPosition(), CS_CAST(Form, TESForm, TESObjectREFR));
									break;
								default:
									TESDialog::ShowFormEditDialog(Form);
									break;
								}

								DlgProcResult = TRUE;
								Return = true;
							}
						}
					}

					break;
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case NM_CUSTOMDRAW:
												// valid listviews
						if (wParam == kFormList_ObjectWindowObjects ||
							wParam == kFormList_TESPackage ||
							wParam == kFormList_CellViewCells ||
							wParam == kFormList_CellViewRefs ||
							wParam == kFormList_TESFormIDListView ||
							wParam == kFormList_DialogEditorTopics ||
							wParam == kFormList_DialogEditorTopicInfos ||
							wParam == kFormList_DialogEditorAddedTopics ||
							wParam == kFormList_DialogEditorLinkedToTopics ||
							wParam == kFormList_DialogEditorLinkedFromTopics ||
							wParam == kFormList_Generic ||
							wParam == kFormList_TESContainer ||
							wParam == kFormList_TESSpellList ||
							wParam == kFormList_ActorFactions ||
							wParam == kFormList_TESLeveledList ||
							wParam == kFormList_WeatherSounds ||
							wParam == kFormList_ClimateWeatherRaceHairFindTextTopics ||
							wParam == kFormList_RaceEyes ||
							wParam == kFormList_TESReactionForm ||
							wParam == kFormList_FindTextTopicInfos ||
							wParam == kFormList_LandTextures ||
							wParam == kFormList_CrossReferences ||
							wParam == kFormList_CellUseList)
						{
							NMLVCUSTOMDRAW* DrawData = (NMLVCUSTOMDRAW*)lParam;

							switch (DrawData->nmcd.dwDrawStage)
							{
							case CDDS_PREPAINT:
								{
									SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
									DlgProcResult = CDRF_NOTIFYITEMDRAW;
									Return = true;
								}

								break;
							case CDDS_ITEMPREPAINT:
								{
									TESForm* Form = NULL;

									switch (wParam)
									{
									case kFormList_ActorFactions:
										{
											TESActorBase::FactionInfo* Data = (TESActorBase::FactionInfo*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Data);
											Form = Data->faction;
										}

										break;
									case kFormList_TESContainer:
										{
											TESContainer::ContentEntry* Entry = (TESContainer::ContentEntry*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Entry);
											Form = Entry->form;
										}

										break;
									case kFormList_TESLeveledList:
										{
											TESLeveledList::ListEntry* Entry = (TESLeveledList::ListEntry*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Entry);
											Form = Entry->form;
										}

										break;
									case kFormList_WeatherSounds:
										{
											UInt32* FormID = (UInt32*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(FormID);
											Form = TESForm::LookupByFormID(*FormID);
										}

										break;
									case kFormList_ClimateWeatherRaceHairFindTextTopics:
										{
											BGSEditorExtender::ResourceTemplateT TemplateID = BGSEEUI->GetSubclasser()->GetDialogTemplate(hWnd);

											switch (TemplateID)
											{
											case TESDialog::kDialogTemplate_Climate:
												{
													TESClimate::WeatherEntry* Entry = (TESClimate::WeatherEntry*)DrawData->nmcd.lItemlParam;
													SME_ASSERT(Entry);
													Form = Entry->weather;
												}

												break;
											case TESDialog::kDialogTemplate_Race:
												{
													Form = (TESForm*)DrawData->nmcd.lItemlParam;
												}

												break;
											case TESDialog::kDialogTemplate_FindText:
												{
													FindTextWindowData::TopicSearchResult* Data = (FindTextWindowData::TopicSearchResult*)DrawData->nmcd.lItemlParam;
													SME_ASSERT(Data);
													Form = Data->topic;
												}

												break;
											}
										}

										break;
									case kFormList_TESReactionForm:
										{
											TESReactionForm::ReactionInfo* Info = (TESReactionForm::ReactionInfo*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Info);
											Form = Info->target;
										}

										break;
									case kFormList_CellUseList:
										{
											TESCellUseList::CellUseInfo* Info = (TESCellUseList::CellUseInfo*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Info);
											Form = Info->cell;
										}

										break;
									default:
										Form = (TESForm*)DrawData->nmcd.lItemlParam;

										break;
									}

									bool ColorizeActiveFormsEnabled = Settings::Dialogs::kColorizeActiveForms.GetData().i &&
																	CSEFormEnumerationManager::Instance.GetVisibleUnmodifiedForms();

									bool ColorizeFormOverridesEnabled = Settings::Dialogs::kColorizeFormOverrides.GetData().i;

									if (Form && (ColorizeActiveFormsEnabled || ColorizeFormOverridesEnabled))
									{
										COLORREF ForeColor, BackColor;
										bool ColorOverridden = false;

										if (ColorizeFormOverridesEnabled)
										{
											ColorOverridden = true;

											switch (Form->fileList.Count())
											{
											case 0:
												ForeColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kFormOverrideLevel0ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kFormOverrideLevel0BackColor.GetData().s);

												break;
											case 1:
												ForeColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kFormOverrideLevel1ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kFormOverrideLevel1BackColor.GetData().s);

												break;
											case 2:
												ForeColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kFormOverrideLevel2ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kFormOverrideLevel2BackColor.GetData().s);

												break;
											default:
												ForeColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kFormOverrideLevel3ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kFormOverrideLevel3BackColor.GetData().s);

												break;
											}
										}

										if (Form->IsActive() && ColorizeActiveFormsEnabled)
										{
											ColorOverridden = true;

											ForeColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kActiveFormForeColor.GetData().s);
											BackColor = SME::StringHelpers::GetRGB(Settings::Dialogs::kActiveFormBackColor.GetData().s);
										}

										TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);

										if (Reference &&
											Reference->GetNiNode() &&
											(Reference->GetNiNode()->m_flags & NiAVObject::kFlag_AppCulled))
										{
											ColorOverridden = true;

											ForeColor = RGB(255, 0, 255);
											BackColor = RGB(255, 255, 255);
										}

										if (ColorOverridden)
										{
											DrawData->clrText = ForeColor;
											DrawData->clrTextBk = BackColor;

											SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NEWFONT);
											DlgProcResult = TRUE;
											Return = true;
										}
									}
								}

								break;
							}
						}

						break;
					}
				}

				break;
			case WM_MOUSEACTIVATE:
			case WM_SETCURSOR:		// the mouse move message isn't dispatched if the pointer is over a dialog control, but this one is
			case WM_MBUTTONUP:
				{
					if (uMsg == WM_MOUSEACTIVATE)
					{
						if (HIWORD(lParam) != WM_MBUTTONDOWN &&
							HIWORD(lParam) != WM_MBUTTONUP)
						{
							break;
						}
					}
					else if (uMsg == WM_SETCURSOR)
					{
						if (HIWORD(lParam) != WM_MOUSEMOVE)
						{
							break;
						}
					}

					POINT Coords = {0};

					if (uMsg == WM_MOUSEACTIVATE ||
						uMsg == WM_SETCURSOR)
					{
						GetCursorPos(&Coords);
					}
					else
					{
						Coords.x = GET_X_LPARAM(lParam);
						Coords.y = GET_Y_LPARAM(lParam);
						ClientToScreen(hWnd, &Coords);
					}

					if (xData)
					{
						xData->LastCursorPos.x = Coords.x;
						xData->LastCursorPos.y = Coords.y;
						xData->LastCursorPosWindow = WindowFromPoint(Coords);

						if (uMsg == WM_MBUTTONUP ||
							uMsg == WM_MOUSEACTIVATE)
						{
							xData->QuickViewTriggered = true;
						}
					}
				}

				break;
			}

			return DlgProcResult;
		}

		int CALLBACK SelectTopicsQuestsFormListComparator( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
		{
			int Result = 0;

			TESForm* FormA = (TESForm*)lParam1;
			TESForm* FormB = (TESForm*)lParam2;

			if (FormA && FormB)
			{
				const char* EDIDA = FormA->editorID.c_str();
				const char* EDIDB = FormB->editorID.c_str();

				if (EDIDA && EDIDB)
				{
					Result = _stricmp(EDIDA, EDIDB);
					if (lParamSort)
						Result *= -1;

					Result = CSEFormEnumerationManager::Instance.CompareActiveForms(FormA, FormB, Result);
				}
			}

			return Result;
		}

		LRESULT CALLBACK SelectTopicsQuestsSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND FormList = GetDlgItem(hWnd, kFormList_Generic);

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 360;
					ListView_SetColumn(FormList, 0, &ColumnData);

					SetWindowLongPtr(FormList, GWL_USERDATA, NULL);
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case LVN_COLUMNCLICK:
						{
							// only one column, so let's sort that
							int SortOrder = GetWindowLongPtr(FormList, GWL_USERDATA);

							SortOrder = (SortOrder == 0);
							SendMessage(FormList, LVM_SORTITEMS, SortOrder, (LPARAM)SelectTopicsQuestsFormListComparator);

							SetWindowLongPtr(FormList, GWL_USERDATA, (LONG)SortOrder);

							DlgProcResult = TRUE;
							Return = true;
						}

						break;
					}
				}

				break;
			}

			return DlgProcResult;
		}

// returns TRUE if there are changes
#define WM_TESFORMIDLISTVIEW_HASCHANGES							(WM_USER + 2006)
// lParam = NMLISTVIEW*
#define WM_TESFORMIDLISTVIEW_SAVECHANGES						(WM_USER + 2007)

		LRESULT CALLBACK TESFormIDListViewDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);

			Return = false;

			switch (uMsg)
			{
			case WM_TESFORMIDLISTVIEW_HASCHANGES:
				{
					// doesn't reliably work on TESGlobal forms
					// given their bollocks'd-up way of copying data from the dialog
					Return = true;

					if (IsWindowEnabled(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok)))
					{
						TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
						TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

						if (WorkingCopy)
						{
							LocalCopy->GetDataFromDialog(hWnd);

							if (WorkingCopy->CompareTo(LocalCopy))
							{
								DlgProcResult = TRUE;
							}
						}
					}

					SetWindowLongPtr(hWnd, DWL_MSGRESULT, DlgProcResult);
				}

				break;
			case WM_TESFORMIDLISTVIEW_SAVECHANGES:
				{
					if (IsWindowEnabled(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok)))
					{
						TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
						TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);
						NMLISTVIEW* ChangeData = (NMLISTVIEW*)lParam;

						if (WorkingCopy)
						{
							LocalCopy->GetDataFromDialog(hWnd);

							if (WorkingCopy->CompareTo(LocalCopy))
							{
								// fill in a new proxy from the working copy
								BGSEEUNDOSTACK->Record(new FormUndoStack::CSEFormUndoProxy(WorkingCopy));

								if (WorkingCopy->UpdateUsageInfo())
								{
									WorkingCopy->SetFromActiveFile(true);
									WorkingCopy->CopyFrom(LocalCopy);

									if (WorkingCopy->formType == TESForm::kFormType_EffectSetting)
										BGSEEACHIEVEMENTS->Unlock(Achievements::kMagister);

									if (ChangeData)
										SendMessage(ChangeData->hdr.hwndFrom, LVM_REDRAWITEMS, ChangeData->iItem, ChangeData->iItem);
								}
							}
						}
					}

					Return = true;
				}

				break;
			case WM_INITDIALOG:
				{
					if (BGSEEUI->GetSubclasser()->GetDialogTemplate(hWnd) != TESDialog::kDialogTemplate_Quest)
					{
						SetWindowText(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok), "Apply");
						SetWindowText(GetDlgItem(hWnd, TESDialog::kStandardButton_Cancel), "Close");
					}

					// ideally, we'd be changing the form listview's style to allow multiple selections
					// unfortunately, adding/removing the LVS_SINGLESEL style post-window creation has no effect
					// so we tuck in our tails and create replacement templates for all TESFormIDListView forms
					// PS: Dammit!

					SME_ASSERT(FilterEditBox);
					CSEFilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL),
																	GetDlgItem(hWnd, kFormList_TESFormIDListView), hWnd);
				}

				break;
			case WM_DESTROY:
				{
					CSEFilterableFormListManager::Instance.Unregister(FilterEditBox);
				}

				break;
			case WM_COMMAND:
				if (HIWORD(wParam))		// to keep EN_KILLFOCUS notifications from inadvertently calling the button handlers
					break;				// ### could cause weird behaviour later on

				if (BGSEEUI->GetSubclasser()->GetDialogTemplate(hWnd) == TESDialog::kDialogTemplate_Quest)
					break;

				switch (LOWORD(wParam))
				{
				case TESDialog::kStandardButton_Ok:			// OK/Apply button
					{
						Return = true;

						SendMessage(hWnd, WM_TESFORMIDLISTVIEW_SAVECHANGES, NULL, NULL);
					}

					break;
				case TESDialog::kStandardButton_Cancel:			// Cancel/Close button
					{
						Return = true;
						bool Cancelled = false;

						if (SendMessage(hWnd, WM_TESFORMIDLISTVIEW_HASCHANGES, NULL, NULL) == TRUE)
						{
							int MsgResult = BGSEEUI->MsgBoxW(hWnd, MB_YESNOCANCEL, "Save changes made to the active form before closing the dialog?");

							switch (MsgResult)
							{
							case IDCANCEL:
								Cancelled = true;

								break;
							case IDYES:
								SendMessage(hWnd, WM_TESFORMIDLISTVIEW_SAVECHANGES, NULL, NULL);

								break;
							}

							if (Cancelled)
								break;
						}

						DestroyWindow(hWnd);
					}

					break;
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;

					if (NotificationData->idFrom != kFormList_TESFormIDListView)
						break;		// only interested in the main listview control

					if (BGSEEUI->GetSubclasser()->GetDialogTemplate(hWnd) == TESDialog::kDialogTemplate_Quest)
						break;

					switch (NotificationData->code)
					{
					case LVN_BEGINLABELEDIT:
						{
							// prevent the modification of GMST editorIDs, they aren't meant to be modified
							DialogExtraParam* xParam = CS_CAST(TESDialog::GetDialogExtraByType(hWnd, 0), BSExtraData, DialogExtraParam);
							SME_ASSERT(xParam);

							if (xParam->formType == TESForm::kFormType_GMST)
							{
								Return = true;

								DlgProcResult = TRUE;
								SetWindowLongPtr(hWnd, DWL_MSGRESULT, DlgProcResult);
							}
						}

						break;
					case LVN_BEGINDRAG:
						{
							// override the vanilla handler to allow multiple selections
							NMLISTVIEW* Data = (NMLISTVIEW*)lParam;
							std::list<TESForm*> Selection;

							int Index = -1;
							while ((Index = ListView_GetNextItem(Data->hdr.hwndFrom, Index, LVNI_SELECTED)) != -1)
							{
								TESForm* Form = (TESForm*)TESListView::GetItemData(Data->hdr.hwndFrom, Index);
								SME_ASSERT(Form);

								Selection.push_back(Form);
							}

							if (Selection.size())
							{
								*TESDialog::TESFormIDListViewDragDropInProgress = 1;

								_RENDERSEL->ClearSelection(true);
								TESObjectWindow::SetSplitterEnabled(*TESObjectWindow::SplitterHandle, false);

								for (std::list<TESForm*>::iterator Itr = Selection.begin(); Itr != Selection.end(); ++Itr)
									_RENDERSEL->AddToSelection(*Itr);

								SetCursor(LoadCursor(*TESCSMain::Instance, (LPCSTR)0xB8));
								SetCapture(hWnd);

								Return = true;
							}
						}

						break;
					case LVN_ITEMCHANGED:
						{
							NMLISTVIEW* ChangeData = (NMLISTVIEW*)lParam;

							if ((ChangeData->uChanged & LVIF_STATE) &&
								(ChangeData->uOldState & LVIS_FOCUSED) &&
								(ChangeData->uNewState & LVIS_FOCUSED) == false)
							{
								// before the new listview item is selected

								if (SendMessage(hWnd, WM_TESFORMIDLISTVIEW_HASCHANGES, NULL, NULL) == TRUE)
								{
									int MsgResult = BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "Save changes made to the active form?");

									switch (MsgResult)
									{
									case IDYES:
										SendMessage(hWnd, WM_TESFORMIDLISTVIEW_SAVECHANGES, NULL, lParam);

										break;
									}
								}

								Return = true;
								SetWindowLongPtr(hWnd, DWL_MSGRESULT, DlgProcResult);
							}
							else if ((ChangeData->uChanged & LVIF_STATE) &&
									 (ChangeData->uOldState & LVIS_FOCUSED) == false &&
									 (ChangeData->uNewState & LVIS_FOCUSED) == false)
							{
								// after the new listview item is selected

								int NewIndex = ChangeData->iItem;
								HWND ListView = ChangeData->hdr.hwndFrom;
								DialogExtraParam* xParam = CS_CAST(TESDialog::GetDialogExtraByType(hWnd, BSExtraData::kDialogExtra_Param), BSExtraData, DialogExtraParam);
								DialogExtraLocalCopy* xLocalCopy = CS_CAST(TESDialog::GetDialogExtraByType(hWnd, BSExtraData::kDialogExtra_Param), BSExtraData, DialogExtraLocalCopy);
								TESForm* NewSelection = NULL;

								if (xParam && xLocalCopy && NewIndex != -1 && (NewSelection = (TESForm*)TESListView::GetItemData(ListView, NewIndex)))
								{
									xParam->form = NewSelection;
									xLocalCopy->localCopy->CopyFrom(NewSelection);
									xLocalCopy->localCopy->SetFromActiveFile(true);
									xLocalCopy->localCopy->SetDataInDialog(hWnd);

									char WindowTitle[0x100] = {0};
									if (NewSelection->editorID.Size())
									{
										FORMAT_STR(WindowTitle, "%s : %s", NewSelection->GetTypeIDString(), NewSelection->GetEditorID());
									}
									else
									{
										FORMAT_STR(WindowTitle, "%s : EMPTY", NewSelection->GetTypeIDString());
									}

									SetWindowText(hWnd, WindowTitle);
									if (IsWindowEnabled(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok)) == FALSE)
									{
										EnableWindow(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok), TRUE);
										InvalidateRect(hWnd, NULL, TRUE);
									}

									SetFocus(ListView);

									Return = true;
									SetWindowLongPtr(hWnd, DWL_MSGRESULT, DlgProcResult);
								}
							}
						}

						break;
					}
				}

				break;
			}

			if (Return == false && CSEFilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
			{
				TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
				if (LocalCopy)
				{
					TESFormIDListView* Item = CS_CAST(LocalCopy, TESForm, TESFormIDListView);
					Item->RefreshFormList(GetDlgItem(hWnd, kFormList_TESFormIDListView));
				}
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK LandscapeEditDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					if (*TESRenderWindow::ActiveCell == NULL)		// immediately close the dialog if you haven't got any cell loaded
						SendMessage(hWnd, WM_COMMAND, 2, NULL);			// otherwise, the editor will crash as soon as the render window acquires input focus
					else
						SendDlgItemMessage(hWnd, kFormList_LandTextures, LVM_SORTITEMS, 0, (LPARAM)TESDialog::LandscapeTextureSortComparator);
				}

				break;
			case 0x41A:		// update active landscape texture
				{
					Return = true;

					HWND TexList = GetDlgItem(hWnd, kFormList_LandTextures);
					int Selection = TESListView::GetItemByData(TexList, *TESRenderWindow::ActiveLandscapeTexture);
					if (Selection != -1)
					{
						TESListView::SetSelectedItem(TexList, Selection);
						ListView_EnsureVisible(TexList, Selection, FALSE);
					}
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK AIPackagesDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND FormList = GetDlgItem(hWnd, kFormList_TESPackage);

			switch (uMsg)
			{
			case WM_DESTROY:
				CSEFilterableFormListManager::Instance.Unregister(FilterEditBox);

				break;
			case WM_INITDIALOG:
				{
					HWND PackageListView = GetDlgItem(hWnd, kFormList_TESPackage);
					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 175;
					ListView_SetColumn(PackageListView, 0, &ColumnData);

					CSEFilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL), FormList, hWnd);
				}

				break;
			case WM_COMMAND:
				if (LOWORD(wParam) == TESDialog::kStandardButton_Cancel)		// prevents the dialog from closing itself on renaming an AI package
				{
					if (HIWORD(wParam) == EN_KILLFOCUS || HIWORD(wParam) == EN_SETFOCUS)
					{
						Return = true;
						DlgProcResult = TRUE;
					}
				}

				break;
			}

			if (CSEFilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
			{
				SendMessage(hWnd, 0x41A, 0, 0);
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK AIFormDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					HWND PackageListView = GetDlgItem(hWnd, kFormList_TESPackage);

					TESPackage::InitializeListViewColumns(PackageListView);	// do this just once, not every time the listview is refreshed

					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 150;
					ListView_SetColumn(PackageListView, 0, &ColumnData);
				}

				break;
			}

			return DlgProcResult;
		}

#define IDT_FACEGENPREVIEW_VOICEPLAYBACK			0x6FF
#define IDT_FACEGENPREVIEW_PREVIEWUPDATE			0x7FF

		LRESULT CALLBACK FaceGenParamListSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_MOUSEWHEEL:
				{
					if (GetAsyncKeyState(VK_RBUTTON))
					{
						// RMB is held down, forward the message to the trackbar and then consume it
						Return = true;

						HWND Parent = NULL;
						if ((Parent = GetParent(hWnd)))
						{
							HWND Trackbar = GetDlgItem(Parent, kFaceGenControl_AdvancedTrackbar);
							if (Trackbar)
							{
								SendMessage(Trackbar, uMsg, wParam, lParam);
							}
						}
					}
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK FaceGenDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_TIMER:
				{
					switch (wParam)
					{
					case IDT_FACEGENPREVIEW_VOICEPLAYBACK:
						{
							Return = true;

							CSEFaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(CSEFaceGenWindowData, ExtraData);
							SME_ASSERT(xData);

							TESSound::PlaySoundFile(xData->VoicePlaybackFilePath.c_str());
							KillTimer(hWnd, IDT_FACEGENPREVIEW_VOICEPLAYBACK);
						}

						break;
					case IDT_FACEGENPREVIEW_PREVIEWUPDATE:
						{
							Return = true;

							CSEFaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(CSEFaceGenWindowData, ExtraData);
							SME_ASSERT(xData);

							// re-enable updates now that most, if not all, of the init is done
							xData->AllowPreviewUpdates = true;
							KillTimer(hWnd, IDT_FACEGENPREVIEW_PREVIEWUPDATE);
						}

						break;
					}
				}

				break;
			case WM_FACEGENPREVIEW_PLAYVOICE:
				{
					CSEFaceGenVoicePreviewData* Data = (CSEFaceGenVoicePreviewData*)wParam;
					SME_ASSERT(Data);

					const char* VoicePath = Data->VoicePath;
					const char* LipPath = Data->LipPath;

					if (LipPath)
					{
						std::string RelativeLipPath(_FILEFINDER->GetRelativePath(LipPath, "Data\\Sound\\Voice\\"));
						std::string RelativeVoicePath = "";
						if (strlen(VoicePath))
							RelativeVoicePath = std::string(_FILEFINDER->GetRelativePath(VoicePath, "Data\\Sound\\Voice\\"));

						if (_FILEFINDER->FindFile(RelativeLipPath.c_str()) == FileFinder::kFileStatus_NotFound)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't find lip file at '%s'.", RelativeLipPath.c_str());
							break;
						}
						else if (RelativeVoicePath != "" && _FILEFINDER->FindFile(RelativeVoicePath.c_str()) == FileFinder::kFileStatus_NotFound)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't find voice file at '%s'.", RelativeVoicePath.c_str());
							break;
						}

						Setting* FaceTestSoundName = INISettingCollection::Instance->LookupByName("sFaceTestSoundName:General");
						SME_ASSERT(FaceTestSoundName);

						FaceTestSoundName->SetStringValue(RelativeLipPath.c_str());

						SetForegroundWindow(hWnd);

						// make sure the head preview is selected
						if (IsDlgButtonChecked(hWnd, TESNPC::kHeadPeviewCheckBox) != BST_CHECKED)
							SendMessage(hWnd, WM_COMMAND, TESNPC::kHeadPeviewCheckBox, NULL);

						SetActiveWindow(GetDlgItem(hWnd, kFaceGenControl_PreviewCtrl));
						SendDlgItemMessage(hWnd, kFaceGenControl_PreviewCtrl, WM_KEYDOWN, 0x4C, NULL);		// L key

						// delay voice file playback to account for synchronization
						if (RelativeVoicePath != "")
						{
							CSEFaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(CSEFaceGenWindowData, ExtraData);
							SME_ASSERT(xData);

							if (Data->DelayTime < 8 || Data->DelayTime > 5000)
								Data->DelayTime = 10;

							xData->VoicePlaybackFilePath = RelativeVoicePath;
							SetTimer(hWnd, IDT_FACEGENPREVIEW_VOICEPLAYBACK, Data->DelayTime, NULL);
						}
					}
				}

				break;
			case WM_DESTROY:
				{
					CSEFaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(CSEFaceGenWindowData, ExtraData);

					if (xData)
					{
						ExtraData->Remove(CSEFaceGenWindowData::kTypeID);
						delete xData;
					}

					DragAcceptFiles(hWnd, FALSE);
					Settings::General::kFaceGenPreviewVoiceDelay.SetInt(TESDialog::GetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY));
				}

				break;
			case WM_INITDIALOG:
				{
					CSEFaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(CSEFaceGenWindowData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSEFaceGenWindowData();
						ExtraData->Add(xData);
					}

					DragAcceptFiles(hWnd, TRUE);
					TESDialog::ClampDlgEditField(GetDlgItem(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY), 0.0, 5000.0, true);
					TESDialog::SetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY, Settings::General::kFaceGenPreviewVoiceDelay.GetData().i, 0);
				}

				break;
			case WM_DROPFILES:
				{
					HDROP DropData = (HDROP)wParam;
					char FilePathVoice[MAX_PATH] = {0}, FilePathLip[MAX_PATH] = {0}, Buffer[MAX_PATH] = {0};
					UInt32 FileCount = DragQueryFile(DropData, 0xFFFFFFFF, FilePathLip, sizeof(FilePathLip));

					if (FileCount == 1 || FileCount == 2)
					{
						for (int i = 0; i < FileCount; i++)
						{
							if (DragQueryFile(DropData, i, Buffer, sizeof(Buffer)))
							{
								if (_FILEFINDER->GetRelativePath(Buffer, "Data\\Sound\\Voice\\") && strrchr(Buffer, '.'))
								{
									char* Extension = strrchr(Buffer, '.') + 1;
									if (!_stricmp(Extension, "lip"))
										memcpy(FilePathLip, Buffer, sizeof(Buffer));
									else if (!_stricmp(Extension, "wav") || !_stricmp(Extension, "mp3"))
										memcpy(FilePathVoice, Buffer, sizeof(Buffer));
									else
										BGSEEUI->MsgBoxE(hWnd, 0, "Invalid extension '%s' - Must be one of the following: mp3, wav, lip.", Extension);
								}
								else
								{
									BGSEEUI->MsgBoxE(hWnd, 0, "Path '%s' is invalid - Must be inside the Data\\Sound\\Voice directory.", Buffer);
								}
							}
							else
							{
								BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't retrieve dropped file path at index %d.", i);
							}
						}

						if (strlen(FilePathLip))
						{
							CSEFaceGenVoicePreviewData PreviewData = {0};
							FORMAT_STR(PreviewData.VoicePath, "%s", FilePathVoice);
							FORMAT_STR(PreviewData.LipPath, "%s", FilePathLip);
							PreviewData.DelayTime = TESDialog::GetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY);

							SendMessage(hWnd, WM_FACEGENPREVIEW_PLAYVOICE, (WPARAM)&PreviewData, NULL);
						}
					}
					else if (FileCount)
					{
						BGSEEUI->MsgBoxW(hWnd, 0, "No more than two files may be dropped into the preview control.");
					}

					DragFinish(DropData);
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;

					switch (NotificationData->idFrom)
					{
					case TESTabControl::kTabControl:		// tab control, same for both the NPC and Race dialogs
						{
							if (NotificationData->code == TCN_SELCHANGE)
							{
								CSEFaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(CSEFaceGenWindowData, ExtraData);

								// consume the original notification and disable preview control updates
								// otherwise, the morph values will get reset upon subwindow init
								if (xData && xData->TunnelingTabSelectMessage == false)
								{
									Return = true;

									xData->TunnelingTabSelectMessage = true;
									xData->AllowPreviewUpdates = false;

									SendMessage(hWnd, uMsg, wParam, lParam);

									xData->TunnelingTabSelectMessage = false;

									// don't allow updates immediately, use a cool-down timer
									static const UInt32 kPreviewUpdateCoolDownPeriod = 1000;		// in ms

									SetTimer(hWnd, IDT_FACEGENPREVIEW_PREVIEWUPDATE, kPreviewUpdateCoolDownPeriod, NULL);

									HWND AdvancedParamsList = GetDlgItem(hWnd, kFaceGenControl_AdvancedParamsListView);
									if (AdvancedParamsList)
									{
										char Buffer[0x200] = {0};
										GetClassName(AdvancedParamsList, Buffer, sizeof(Buffer));

										if (!_stricmp("SysListView32", Buffer))
										{
											// right, subclass it to forward mouse wheel messages to the trackbar
											BGSEEUI->GetSubclasser()->RegisterRegularWindowSubclass(AdvancedParamsList, FaceGenParamListSubClassProc);
										}
									}
								}
							}
						}

						break;
					}
				}

				break;
			case WM_COMMAND:
				switch (HIWORD(wParam))
				{
				case EN_CHANGE:
					{
						// the method used here is actually rather buggy
						// the 0x41A message updates the NPC/Race form before regenerating the face model
						// this causes the form's compare method to return true, leading to false changes
						// the issue is only noticeable in the race edit dialog as it's a FormIDListView dlg
						// we HACK around this by preventing updates for a short time after the current tab's been switched
						// during this time, updates to the edit controls' text will be ignored by this handler
						CSEFaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(CSEFaceGenWindowData, ExtraData);
						if (xData == NULL || xData->AllowPreviewUpdates == false)
							break;

						if (LOWORD(wParam) == kFaceGenControl_AgeEditCtrl ||
							LOWORD(wParam) == kFaceGenControl_ComplexionEditCtrl ||
							LOWORD(wParam) == kFaceGenControl_HairLengthEditCtrl ||
							LOWORD(wParam) == kFaceGenControl_AdvancedEditCtrl)
						{
							// refresh the preview control
							SendMessage(hWnd, 0x41A, NULL, NULL);
						}
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK TESFormEditDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					CSETESFormEditData* xData = BGSEE_GETWINDOWXDATA(CSETESFormEditData, ExtraData);
					TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

					if (xData == NULL)
					{
						xData = new CSETESFormEditData();
						ExtraData->Add(xData);

						if (WorkingCopy)		// can be NULL when creating new forms
						{
							xData->FillBuffer(WorkingCopy);
						}
					}

					if (WorkingCopy)
					{
						std::string Desc = std::string(WorkingCopy->GetTypeIDString()) + " [" + std::string(WorkingCopy->GetEditorID()) + "]";
						std::string WndTitle = Desc;

						if (Settings::General::kShowHallOfFameMembersInTitleBar().i == HallOfFame::kDisplayESMember_ObjectPreviewEdit)
						{
							HallOfFame::GetRandomESMember(WndTitle);
							WndTitle += " " + Desc;
						}
						SetWindowText(hWnd, WndTitle.c_str());
					}
				}

				break;
			case WM_DESTROY:
				{
					CSETESFormEditData* xData = BGSEE_GETWINDOWXDATA(CSETESFormEditData, ExtraData);
					if (xData)
					{
						// at this point, the working copy is already modified (if there were changes and the user confirmed them)
						TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

						if (WorkingCopy && xData->HasChanges(WorkingCopy))
						{
							// create a proxy from the pre-filled buffer
							BGSEEUNDOSTACK->Record(new FormUndoStack::CSEFormUndoProxy(WorkingCopy, xData->Buffer));
						}

						ExtraData->Remove(CSETESFormEditData::kTypeID);
						delete xData;
					}

					ObjectWindowImposterManager::Instance.RefreshImposters();
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK MagicItemFormDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_NOTIFY:
				{
					HWND EffectItemListView = GetDlgItem(hWnd, EffectItemList::kEffectItemListView);
					NMHDR* NotificationData = (NMHDR*)lParam;

					if (NotificationData->hwndFrom != EffectItemListView)
						break;

					switch (NotificationData->code)
					{
					case LVN_COLUMNCLICK:
						{
							Return = true;				// no sorting!
						}

						break;
					case LVN_KEYDOWN:
						if (GetAsyncKeyState(VK_CONTROL))
						{
							Return = true;

							NMLVKEYDOWN* KeyData = (NMLVKEYDOWN*)lParam;
							TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);

							if (LocalCopy && (KeyData->wVKey == VK_UP || KeyData->wVKey == VK_DOWN))
							{
								EffectItemList* EffectItems = CS_CAST(LocalCopy, TESForm, EffectItemList);
								EffectItem* Selection = (EffectItem*)TESListView::GetSelectedItemData(EffectItemListView);

								if (EffectItems && Selection)
								{
									EffectItemList::EffectItemListT::_Node* CurrentNode = NULL;
									EffectItemList::EffectItemListT::_Node* PreviousNode = NULL;
									EffectItemList::EffectItemListT::_Node* NextNode = NULL;

									for (EffectItemList::EffectItemListT::_Node* Itr = EffectItems->effects.Head();
																				Itr; PreviousNode = Itr, Itr = Itr->Next())
									{
										CurrentNode = Itr;
										NextNode = Itr->Next();

										if (Itr->Item() == Selection)
											break;
									}

									bool SelectionMoved = true, Up = false;
									switch (KeyData->wVKey)
									{
									case VK_UP:
										Up = true;

										if (PreviousNode == NULL)
											SelectionMoved = false;
										else
										{
											EffectItem* Buffer = CurrentNode->item;
											CurrentNode->item = PreviousNode->item;
											PreviousNode->item = Buffer;
										}

										break;
									case VK_DOWN:
										if (NextNode == NULL)
											SelectionMoved = false;
										else
										{
											EffectItem* Buffer = NextNode->item;
											NextNode->item = CurrentNode->item;
											CurrentNode->item = Buffer;
										}

										break;
									}

									if (SelectionMoved)
									{
										EffectItems->PopulateListView(EffectItemListView);
										TESListView::SetSelectedItem(EffectItemListView,
																	TESListView::GetItemByData(EffectItemListView, Selection) + (Up ? 1 : -1));
									}
								}
							}
						}
					}
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK LeveledItemFormDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case TESDialog::kStandardButton_Ok:			// OK button
					{
						// validate list
						TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
						// use a temp copy as the local copy is used to determine if changes were made
						TESForm* TempCopy = TESForm::CreateTemporaryCopy(LocalCopy);
						TempCopy->GetDataFromDialog(hWnd);

						std::string ValidationOutput = "";
						if ((CS_CAST(TempCopy, TESForm, TESLeveledList))->CheckForCircularPaths(ValidationOutput) == false)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "The leveled list contents are invalid!\n\nA circular link was found at:\n%s", ValidationOutput.c_str());
							Return = true;
						}

						TempCopy->DeleteInstance();
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK TESObjectCELLDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												   bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND FormList = GetDlgItem(hWnd, kFormList_TESFormIDListView);

			switch (uMsg)
			{
			case WM_COMMAND:
				if (LOWORD(wParam) == TESCellViewWindow::kWorldspaceComboBox && HIWORD(wParam) == 1)
				{
					Return = true;

					if ((HWND)lParam == GetDlgItem(hWnd, TESCellViewWindow::kWorldspaceComboBox))
					{
						// we didn't send this message, so clear the filter string
						// this is done to prevent the dialog's controls from being disabled if the active filter string doesn't match any forms in the new worldspace
						CSEFilterableFormListManager::Instance.SetEnabledState(FilterEditBox, false);
						SetWindowText(FilterEditBox, "");
						BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, wParam, lParam);
						CSEFilterableFormListManager::Instance.SetEnabledState(FilterEditBox, true);
					}
					else if (lParam == NULL)
					{
						// filter refresh
						// prevent filter controls from being disabled
						BGSEEUI->GetSubclasser()->TunnelDialogMessage(hWnd, uMsg, wParam, lParam);
						EnableWindow(FilterEditBox, TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL), TRUE);
					}
				}

				break;
			case WM_DESTROY:
				CSEFilterableFormListManager::Instance.Unregister(FilterEditBox);

				break;
			case WM_INITDIALOG:
				{
					SME_ASSERT(FilterEditBox);
					CSEFilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL), FormList, hWnd);
				}

				break;
			}

			if (CSEFilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
			{
				// pass NULL as lParam to indicate a filter refresh
				SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(TESCellViewWindow::kWorldspaceComboBox, 1), NULL);
			}

			return DlgProcResult;
		}

		BOOL CALLBACK AssetSelectorDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_GETMINMAXINFO:
				{
					MINMAXINFO* SizeInfo = (MINMAXINFO*)lParam;
					SizeInfo->ptMaxTrackSize.x = SizeInfo->ptMinTrackSize.x = 189;
					SizeInfo->ptMaxTrackSize.y = SizeInfo->ptMinTrackSize.y = 255;

					break;
				}
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_ASSETSELECTOR_FILEBROWSER:
					EndDialog(hWnd, Hooks::e_FileBrowser);

					return TRUE;
				case IDC_ASSETSELECTOR_ARCHIVEBROWSER:
					EndDialog(hWnd, Hooks::e_BSABrowser);

					return TRUE;
				case IDC_ASSETSELECTOR_PATHEDITOR:
					EndDialog(hWnd, Hooks::e_EditPath);

					return TRUE;
				case IDC_ASSETSELECTOR_PATHCOPIER:
					EndDialog(hWnd, Hooks::e_CopyPath);

					return TRUE;
				case IDC_ASSETSELECTOR_CLEARPATH:
					EndDialog(hWnd, Hooks::e_ClearPath);

					return TRUE;
				case IDC_ASSETSELECTOR_ASSETEXTRACTOR:
					EndDialog(hWnd, Hooks::e_ExtractPath);

					return TRUE;
				case IDC_ASSETSELECTOR_OPENASSET:
					EndDialog(hWnd, Hooks::e_OpenPath);

					return TRUE;
				case IDC_CSE_CANCEL:
					EndDialog(hWnd, Hooks::e_Close);

					return TRUE;
				}

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK TextEditDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				{
					InitDialogMessageParamT<UInt32>* InitParam = (InitDialogMessageParamT<UInt32>*)GetWindowLongPtr(hWnd, GWL_USERDATA);

					switch (LOWORD(wParam))
					{
					case IDC_CSE_OK:
						GetDlgItemText(hWnd, IDC_TEXTEDIT_TEXT, InitParam->Buffer, sizeof(InitParam->Buffer));
						EndDialog(hWnd, 1);

						return TRUE;
					case IDC_CSE_CANCEL:
						EndDialog(hWnd, NULL);

						return TRUE;
					}
				}

				break;
			case WM_INITDIALOG:
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);
				SetDlgItemText(hWnd, IDC_TEXTEDIT_TEXT, ((InitDialogMessageParamT<UInt32>*)lParam)->Buffer);

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK TESFileSaveDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_TESFILESAVE_SAVEESP:
					EndDialog(hWnd, 0);

					return TRUE;
				case IDC_TESFILESAVE_SAVEESM:
					EndDialog(hWnd, 1);

					return TRUE;
				}

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK TESComboBoxDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			HWND ComboBox = GetDlgItem(hWnd, IDC_TESCOMBOBOX_FORMLIST);

			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_OK:
					{
						TESForm* SelectedForm = (TESForm*)TESComboBox::GetSelectedItemData(ComboBox);
						EndDialog(hWnd, (INT_PTR)SelectedForm);

						return TRUE;
					}
				case IDC_CSE_CANCEL:
					EndDialog(hWnd, 0);

					return TRUE;
				}
				break;
			case WM_INITDIALOG:
				TESComboBox::PopulateWithForms(ComboBox, lParam, true, false);

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK CopyPathDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			static bool s_DraggingMouse = false;

			switch (uMsg)
			{
			case WM_LBUTTONDOWN:
				if (!s_DraggingMouse)
				{
					s_DraggingMouse = true;
					Edit_SetText(GetDlgItem(hWnd, IDC_COPYPATH_ASSETPATH), "Dragging...");
					SetCapture(hWnd);
				}

				break;
			case WM_LBUTTONUP:
				if (s_DraggingMouse)
				{
					POINT Location = {0};
					Location.x = GET_X_LPARAM(lParam);
					Location.y = GET_Y_LPARAM(lParam);
					ClientToScreen(hWnd, &Location);

					HWND Window = WindowFromPoint(Location);
					if (Window)
					{
						char Buffer[0x200] = {0};
						GetWindowText(Window, Buffer, sizeof(Buffer));
						Edit_SetText(GetDlgItem(hWnd, IDC_COPYPATH_ASSETPATH), Buffer);
					}
					else
						Edit_SetText(GetDlgItem(hWnd, IDC_COPYPATH_ASSETPATH), NULL);

					ReleaseCapture();
					s_DraggingMouse = false;
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_OK:
					{
						char Buffer[0x200] = {0};
						GetDlgItemText(hWnd, IDC_COPYPATH_ASSETPATH, Buffer, sizeof(Buffer));
						InitDialogMessageParamT<UInt32>* InitParam = (InitDialogMessageParamT<UInt32>*)GetWindowLongPtr(hWnd, GWL_USERDATA);

						switch (InitParam->ExtraData)
						{
						case Hooks::e_SPT:
							FORMAT_STR(InitParam->Buffer, "\\%s", Buffer);

							break;
						case Hooks::e_KF:
							{
								std::string STLBuffer(Buffer);
								int Offset = STLBuffer.find("IdleAnims\\");

								if (Offset != -1)
									STLBuffer = STLBuffer.substr(Offset + 9);

								FORMAT_STR(InitParam->Buffer, "%s", STLBuffer.c_str());
							}

							break;
						default:
							FORMAT_STR(InitParam->Buffer, "%s", Buffer);

							break;
						}

						EndDialog(hWnd, 1);

						return TRUE;
					}
				case IDC_CSE_CANCEL:
					EndDialog(hWnd, 0);

					return TRUE;
				}

				break;
			case WM_INITDIALOG:
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);

				break;
			}

			return FALSE;
		}

		LRESULT CALLBACK CreateGlobalScriptDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_OK:
					{
						char QuestID[0x200] = {0};
						char ScriptID[0x200] = {0};
						char Delay[8] = {0};
						char Buffer[0x200] = {0};

						GetDlgItemText(hWnd, IDC_GLOBALSCRIPT_QUESTID, QuestID, sizeof(QuestID));
						GetDlgItemText(hWnd, IDC_GLOBALSCRIPT_SCRIPTID, ScriptID, sizeof(ScriptID));
						GetDlgItemText(hWnd, IDC_GLOBALSCRIPT_DELAY, Delay, sizeof(Delay));

						TESForm* Form = NULL;
						TESQuest* Quest = NULL;
						Script* QuestScript = NULL;

						Form = TESForm::LookupByEditorID(QuestID);

						if (Form)
						{
							if (Form->formType == TESForm::kFormType_Quest)
							{
								if (BGSEEUI->MsgBoxW(hWnd, MB_YESNO,
													"Quest '%s' already exists. Do you want to replace its script with a newly created one?", QuestID) != IDYES)
								{
									return TRUE;
								}
							}
							else
							{
								BGSEEUI->MsgBoxE(hWnd, 0, "EditorID '%s' is already in use.", QuestID);

								return TRUE;
							}

							Quest = CS_CAST(Form, TESForm, TESQuest);
						}
						else
						{
							Quest = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Quest), TESForm, TESQuest);
							Quest->SetFromActiveFile(true);
							Quest->SetEditorID(QuestID);

							_DATAHANDLER->quests.AddAt(Quest, eListEnd);
						}

						if (strlen(ScriptID) < 1)
						{
							FORMAT_STR(ScriptID, "%sScript", QuestID);
						}

						Form = TESForm::LookupByEditorID(ScriptID);

						if (Form)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "EditorID '%s' is already in use.", ScriptID);

							return TRUE;
						}
						else
						{
							QuestScript = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Script), TESForm, Script);
							QuestScript->info.type = Script::kScriptType_Quest;
							QuestScript->SetFromActiveFile(true);

							if (strlen(Delay))
								FORMAT_STR(Buffer, "scn %s\n\nfloat fQuestDelayTime\n\nBegin GameMode\n\tset fQuestDelayTime to %s\n\nend", ScriptID, Delay);
							else
								FORMAT_STR(Buffer, "scn %s\n\nBegin GameMode\n\nEnd", ScriptID);

							QuestScript->SetText(Buffer);
							QuestScript->SetEditorID(ScriptID);

							_DATAHANDLER->scripts.AddAt(QuestScript, eListEnd);
							_DATAHANDLER->SortScripts();
						}

						Quest->script = QuestScript;

						Quest->LinkForm();
						QuestScript->LinkForm();

						Quest->UpdateUsageInfo();
						QuestScript->UpdateUsageInfo();
						QuestScript->AddCrossReference(Quest);

						TESDialog::ShowScriptEditorDialog(TESForm::LookupByEditorID(ScriptID));
						DestroyWindow(hWnd);

						return TRUE;
					}
				case IDC_CSE_CANCEL:
					DestroyWindow(hWnd);

					return TRUE;
				}

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK BindScriptDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			HWND EditorIDBox = GetDlgItem(hWnd, IDC_BINDSCRIPT_NEWFORMEDITORID);
			HWND RefIDBox = GetDlgItem(hWnd, IDC_BINDSCRIPT_NEWREFEDITORID);
			HWND ExistFormList = GetDlgItem(hWnd, IDC_BINDSCRIPT_EXISTINGFORMLIST);
			HWND SelParentCellBtn = GetDlgItem(hWnd, IDC_BINDSCRIPT_SELECTPARENTCELL);

			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_BINDSCRIPT_SELECTPARENTCELL:
					{
						TESForm* Selection = (TESForm*)BGSEEUI->ModalDialog(BGSEEMAIN->GetExtenderHandle(),
																		MAKEINTRESOURCE(IDD_TESCOMBOBOX),
																		hWnd,
																		(DLGPROC)TESComboBoxDlgProc,
																		(LPARAM)TESForm::kFormType_Cell);

						if (Selection)
						{
							char Buffer[0x200] = {0};
							FORMAT_STR(Buffer, "%s (%08X)", Selection->editorID.c_str(), Selection->formID);

							SetWindowText(SelParentCellBtn, (LPCSTR)Buffer);
							SetWindowLongPtr(SelParentCellBtn, GWL_USERDATA, (LONG_PTR)Selection);
						}
						break;
					}
				case IDC_CSE_OK:
					{
						if (IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_BINDEXISTFORM))
						{
							TESForm* SelectedForm = (TESForm*)TESComboBox::GetSelectedItemData(ExistFormList);
							if (SelectedForm)
							{
								EndDialog(hWnd, (INT_PTR)SelectedForm);
								return TRUE;
							}
							else
								BGSEEUI->MsgBoxE(hWnd, 0, "Invalid existing form selected");
						}
						else
						{
							char BaseEditorID[0x200] = {0};
							char RefEditorID[0x200] = {0};
							char Buffer[0x200] = {0};

							Edit_GetText(EditorIDBox, BaseEditorID, 0x200);
							if (TESForm::LookupByEditorID(BaseEditorID))
							{
								BGSEEUI->MsgBoxE(hWnd, 0, "EditorID '%s' is already in use.", BaseEditorID);
							}
							else
							{
								if (IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_QUESTFORM))
								{
									bool StartGameEnabledFlag = IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_QUESTSTARTGAMEENABLED);
									bool RepeatedStagesFlag = IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_QUESTREPEATEDSTAGES);

									TESQuest* Quest = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Quest), TESForm, TESQuest);
									Quest->SetFromActiveFile(true);
									Quest->SetEditorID(BaseEditorID);
									Quest->SetStartGameEnabledFlag(StartGameEnabledFlag);
									Quest->SetAllowedRepeatedStagesFlag(RepeatedStagesFlag);
									_DATAHANDLER->quests.AddAt(Quest, eListEnd);

									TESDialog::ResetFormListControls();
									EndDialog(hWnd, (INT_PTR)Quest);

									return TRUE;
								}
								else
								{
									if (IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_OBJECTTOKEN))
									{
										bool QuestItem = IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_OBJECTTOKENQUESTITEM);

										TESObjectCLOT* Token = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Clothing), TESForm, TESObjectCLOT);
										Token->SetFromActiveFile(true);
										Token->SetEditorID(BaseEditorID);
										_DATAHANDLER->AddTESObject(Token);

										SME::MiscGunk::ToggleFlag(&Token->bipedModelFlags, TESBipedModelForm::kBipedModelFlags_NotPlayable, true);
										SME::MiscGunk::ToggleFlag(&Token->formFlags, TESForm::kFormFlags_QuestItem, QuestItem);

										TESDialog::ResetFormListControls();
										EndDialog(hWnd, (INT_PTR)Token);

										return TRUE;
									}
									else
									{
										Edit_GetText(RefIDBox, RefEditorID, 0x200);
										if (TESForm::LookupByEditorID(RefEditorID))
										{
											BGSEEUI->MsgBoxE(hWnd, 0, "EditorID '%s' is already in use.", BaseEditorID);
										}
										else
										{
											bool InitiallyDisabled = IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_OBJECTREFERENCEDISABLED);
											TESObjectCELL* ParentCell = CS_CAST(GetWindowLongPtr(SelParentCellBtn, GWL_USERDATA), TESForm, TESObjectCELL);

											if (!ParentCell || ParentCell->GetIsInterior() == 0)
											{
												BGSEEUI->MsgBoxE(hWnd, 0, "Invalid/exterior cell selected as parent.");
											}
											else
											{
												TESObjectACTI* Activator =  CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Activator), TESForm, TESObjectACTI);
												Activator->SetFromActiveFile(true);
												Activator->SetEditorID(BaseEditorID);
												_DATAHANDLER->AddTESObject(Activator);

												static Vector3 ZeroVector(0.0, 0.0, 0.0);

												TESObjectREFR* Ref = _DATAHANDLER->PlaceObjectRef(Activator,
																								&ZeroVector,
																								&ZeroVector,
																								CS_CAST(ParentCell, TESForm, TESObjectCELL),
																								NULL,
																								NULL);

												SME::MiscGunk::ToggleFlag(&Ref->formFlags, TESForm::kFormFlags_Disabled, InitiallyDisabled);
												SME::MiscGunk::ToggleFlag(&Ref->formFlags, TESForm::kFormFlags_QuestItem, true);

												Ref->SetEditorID(RefEditorID);

												TESDialog::ResetFormListControls();
												EndDialog(hWnd, (INT_PTR)Activator);

												return TRUE;
											}
										}
									}
								}
							}
						}

						return FALSE;
					}
				case IDC_CSE_CANCEL:
					EndDialog(hWnd, 0);

					return TRUE;
				}
				break;
			case WM_INITDIALOG:
				Edit_SetText(EditorIDBox, "Base Form EditorID");
				Edit_SetText(RefIDBox, "Ref EditorID");

				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Activator, true, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Apparatus, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Armor, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Book, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Clothing, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Container, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Door, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Ingredient, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Light, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Misc, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Furniture, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Weapon, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Ammo, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_NPC, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Creature, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_SoulGem, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Key, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_AlchemyItem, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_SigilStone, false, false);
				TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Quest, false, false);

				CheckDlgButton(hWnd, IDC_BINDSCRIPT_BINDEXISTFORM, BST_CHECKED);
				CheckDlgButton(hWnd, IDC_BINDSCRIPT_QUESTFORM, BST_CHECKED);
				CheckDlgButton(hWnd, IDC_BINDSCRIPT_OBJECTTOKEN, BST_CHECKED);

				SetWindowLongPtr(SelParentCellBtn, GWL_USERDATA, (LONG_PTR)0);

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK EditResultScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_CANCEL:
					EndDialog(hWnd, NULL);

					return TRUE;
				case IDC_EDITRESULTSCRIPT_COMPILE:
				case IDC_EDITRESULTSCRIPT_SAVE:
					char Buffer[0x1000] = {0};
					HWND Parent = (HWND)GetWindowLongPtr(hWnd, GWL_USERDATA);

					GetDlgItemText(hWnd, IDC_EDITRESULTSCRIPT_SCRIPTTEXT, Buffer, sizeof(Buffer));
					SetDlgItemText(Parent, kDialogEditor_ResultScriptTextBox, (LPSTR)Buffer);

					if (LOWORD(wParam) == IDC_EDITRESULTSCRIPT_COMPILE)
						EndDialog(hWnd, 1);
					else
						EndDialog(hWnd, NULL);

					return TRUE;
				}

				break;
			case WM_INITDIALOG:
				PARAFORMAT FormattingData = {0};

				FormattingData.cbSize = sizeof(PARAFORMAT);
				FormattingData.dwMask = PFM_TABSTOPS;
				FormattingData.cTabCount = 32;

				for (int i = 0, j = 300; i < sizeof(FormattingData.rgxTabs); i++, j += 300)
					FormattingData.rgxTabs[i] = j;

				SendDlgItemMessage(hWnd, IDC_EDITRESULTSCRIPT_SCRIPTTEXT, EM_SETPARAFORMAT, NULL, (LPARAM)&FormattingData);
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);

				HWND ResultScriptEditBox = GetDlgItem((HWND)lParam, kDialogEditor_ResultScriptTextBox);
				char Buffer[0x1000] = {0};

				GetWindowText(ResultScriptEditBox, Buffer, sizeof(Buffer));
				SetDlgItemText(hWnd, IDC_EDITRESULTSCRIPT_SCRIPTTEXT, (LPSTR)Buffer);

				break;
			}

			return FALSE;
		}

		void Initialize( void )
		{
			// FormEdit subclasses
			{
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LandTexture, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Enchantment, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Spell, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Sound, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Activator, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Apparatus, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Armor, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Book, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Clothing, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Container, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Door, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Ingredient, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Light, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_MiscItem, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Static, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Grass, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Tree, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Flora, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Furniture, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Weapon, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Ammo, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_NPC, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Creature, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledCreature, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SoulGem, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Potion, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Subspace, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SigilStone, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledItem, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Package, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CombatStyle, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LoadingScreen, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledSpell, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AnimObject, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Water, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_EffectShader, TESFormEditDlgSubClassProc);
			}

			// FormIDListView subclasses
			{
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Faction, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Race, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Class, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Skill, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_EffectSetting, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_GameSetting, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Globals, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Birthsign, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Climate, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Worldspace, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Hair, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Eyes, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Weather, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Quest, TESFormIDListViewDlgSubClassProc);
			}

			// Generic extra fittings subclasses
			{
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CellEdit, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Data, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SearchReplace, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LandscapeEdit, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FindText, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_IdleAnimations, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AIPackages, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FilteredDialog, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Weapon, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Armor, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Clothing, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_MiscItem, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Static, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Reference, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Apparatus, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Book, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Container, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Activator, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AIForm, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Light, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Potion, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Enchantment, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledCreature, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Sound, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Door, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledItem, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LandTexture, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SoulGem, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Ammo, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Spell, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Flora, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Tree, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CombatStyle, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Water, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_NPC, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Creature, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Grass, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Furniture, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LoadingScreen, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Ingredient, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledSpell, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AnimObject, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Subspace, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_EffectShader, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SigilStone, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Faction, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Race, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Class, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Skill, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_EffectSetting, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_GameSetting, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Globals, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Birthsign, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Climate, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Worldspace, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Hair, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Quest, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Eyes, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Weather, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectTopic, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectQuests, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_UseReport, CommonDialogExtraFittingsSubClassProc);
			}

			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FindText, FindTextDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Data, DataDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ResponseEditor, ResponseDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_TextureUse, LandscapeTextureUseDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Quest, FilteredDialogQuestDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FilteredDialog, FilteredDialogQuestDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_About, AboutDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Race, RaceDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectTopic, SelectTopicsQuestsSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectQuests, SelectTopicsQuestsSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LandscapeEdit, LandscapeEditDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AIPackages, AIPackagesDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AIForm, AIFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_NPC, FaceGenDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Race, FaceGenDlgSubClassProc);

			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Ingredient, MagicItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SigilStone, MagicItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Enchantment, MagicItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Potion, MagicItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Spell, MagicItemFormDlgSubClassProc);

			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledItem, LeveledItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledCreature, LeveledItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledSpell, LeveledItemFormDlgSubClassProc);

			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CellEdit, TESObjectCELLDlgSubClassProc);

			BGSEEUI->GetWindowHandleCollection(BGSEditorExtender::BGSEEUIManager::kHandleCollection_DragDropableWindows)->Add(
																								CLIWrapper::Interfaces::TAG->GetFormDropWindowHandle());

			if (Settings::Dialogs::kShowEditDialogsInTaskbar.GetData().i)
			{
				BGSEditorExtender::BGSEEWindowStyler::StyleData RegularAppWindow = {0};
				RegularAppWindow.Extended = WS_EX_APPWINDOW;
				RegularAppWindow.ExtendedOp = BGSEditorExtender::BGSEEWindowStyler::StyleData::kOperation_OR;

				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_CellEdit, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LandscapeEdit, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_IdleAnimations, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_AIPackages, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_FilteredDialog, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Weapon, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Armor, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Clothing, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_MiscItem, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Static, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Reference, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Apparatus, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Book, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Container, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Activator, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_AIForm, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Light, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Potion, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Enchantment, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LeveledCreature, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Sound, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Door, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LeveledItem, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LandTexture, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_SoulGem, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Ammo, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Spell, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Flora, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Tree, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_CombatStyle, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Water, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_NPC, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Creature, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Grass, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Furniture, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LoadingScreen, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Ingredient, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LeveledSpell, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_AnimObject, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Subspace, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_EffectShader, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_SigilStone, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Faction, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Race, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Class, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Skill, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_EffectSetting, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_GameSetting, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Globals, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Birthsign, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Climate, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Worldspace, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Hair, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Quest, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Eyes, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Weather, RegularAppWindow);
			}

			SendMessage(*TESCSMain::WindowHandle, WM_MAINWINDOW_INITEXTRADATA, NULL, NULL);
			SendMessage(*TESRenderWindow::WindowHandle, WM_RENDERWINDOW_UPDATEFOV, NULL, NULL);

			PreviewWindowImposterManager::Instance.SetEnabled(Settings::Dialogs::kMultiplePreviewWindows().i == 1);
		}
	}
}