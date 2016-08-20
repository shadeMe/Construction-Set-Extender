#include "UIManager.h"
#include "Main.h"
#include "Construction Set Extender_Resource.h"
#include "MiscWindowOverrides.h"
#include "MainWindowOverrides.h"
#include "DialogImposterManager.h"
#include "Achievements.h"
#include "HallOfFame.h"
#include "FormUndoStack.h"
#include "[Common]\CLIWrapper.h"


namespace cse
{
	namespace uiManager
	{
#define ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID				0x99

		FilterableFormListManager													FilterableFormListManager::Instance;
		FilterableFormListManager::FilterableWindowData::WindowTimerMapT			FilterableFormListManager::FilterableWindowData::FilterTimerTable;
		FilterableFormListManager::FilterableWindowData::FormListFilterDataMapT		FilterableFormListManager::FilterableWindowData::FormListDataTable;

		FilterableFormListManager::FilterableWindowData::FilterableWindowData( HWND Parent, HWND EditBox, HWND FormList, HWND Label, int TimerPeriod, SecondaryFilterT UserFilter ) :
			ParentWindow(Parent),
			FilterEditBox(EditBox),
			FormListView(FormList),
			FormListWndProc(nullptr),
			FilterLabel(Label),
			FilterString(""),
			TimerPeriod(TimerPeriod),
			TimeCounter(-1),
			SecondFilter(UserFilter),
			Enabled(true)
		{
			SME_ASSERT(ParentWindow && FilterEditBox && FormListView);

			HookFormList();
			CreateTimer();
			Flags = kFlags_SearchEditorID | kFlags_SearchDescription | kFlags_SearchName;
		}

		FilterableFormListManager::FilterableWindowData::~FilterableWindowData()
		{
			UnhookFormList();
			DestroyTimer();
		}

		LRESULT CALLBACK FilterableFormListManager::FilterableWindowData::FormListSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
						else if (UserData->SecondFilter && UserData->SecondFilter(Form) == false)
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

		bool FilterableFormListManager::FilterableWindowData::HandleMessages( UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == FilterEditBox)
					TimeCounter = 0;

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
							FilterString = "";

						TimeCounter = -1;
						return true;
					}
					else if (TimeCounter != -1)
						TimeCounter += TimerPeriod;

					break;
				}

				break;
			}

			return false;
		}

		bool FilterableFormListManager::FilterableWindowData::FilterForm(TESForm* Form)
		{
			SME_ASSERT(Form);

			// fall through if no filters are active
			if (HasEditorID() == false && HasName() == false && HasDescription() == false && HasFormID() == false)
				return true;
			else if (FilterString.length() == 0)
				return true;

			std::string EditorID, FullName, Description, FormIDStr;
			TESFullName* FullNameCmpt = nullptr;
			TESDescription* DescriptionCmpt = nullptr;

			if (Form->IsReference())
			{
				TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
				if (Reference->baseForm)
				{
					FullNameCmpt = CS_CAST(Reference->baseForm, TESForm, TESFullName);
					DescriptionCmpt = CS_CAST(Reference->baseForm, TESForm, TESDescription);
				}
			}

			if (Form->IsReference() == false && FullNameCmpt == nullptr)
				FullNameCmpt = CS_CAST(Form, TESForm, TESFullName);

			if (Form->IsReference() == false && DescriptionCmpt == nullptr)
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
					BGSEECONSOLE_MESSAGE("An error occurred while matching the regular expression filter string. Exception - %s", e.what());
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

			achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_FormListFilter);
			return CanAdd;
		}

		void FilterableFormListManager::FilterableWindowData::HandlePopupMenu(HWND Parent, int X, int Y)
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

			InsertMenu(Popup, -1, MF_BYPOSITION | MF_SEPARATOR, NULL, nullptr);

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

			switch (TrackPopupMenu(Popup, TPM_RETURNCMD, X, Y, NULL, Parent, nullptr))
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

		void FilterableFormListManager::FilterableWindowData::SetEnabledState(bool State)
		{
			Enabled = State;
		}

		bool FilterableFormListManager::FilterableWindowData::operator==(HWND FilterEditBox)
		{
			return this->FilterEditBox == FilterEditBox;
		}

		void FilterableFormListManager::FilterableWindowData::CreateTimer(void) const
		{
			if (FilterTimerTable.count(ParentWindow) == 0)
			{
				FilterTimerTable[ParentWindow] = 1;
				SetTimer(ParentWindow, ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID, TimerPeriod, nullptr);
			}
			else
				FilterTimerTable[ParentWindow]++;
		}

		void FilterableFormListManager::FilterableWindowData::DestroyTimer(void) const
		{
			SME_ASSERT(FilterTimerTable.count(ParentWindow));

			if (--FilterTimerTable[ParentWindow] == 0)
			{
				KillTimer(ParentWindow, ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID);
				FilterTimerTable.erase(ParentWindow);
			}
		}

		void FilterableFormListManager::FilterableWindowData::HookFormList(void)
		{
			FormListWndProc = (WNDPROC)SetWindowLongPtr(FormListView, GWL_WNDPROC, (LONG)FormListSubclassProc);
			FormListDataTable[FormListView] = this;
		}

		void FilterableFormListManager::FilterableWindowData::UnhookFormList(void)
		{
			SetWindowLongPtr(FormListView, GWL_WNDPROC, (LONG)FormListWndProc);
			FormListDataTable.erase(FormListView);
		}

		FilterableFormListManager::FilterableFormListManager() :
			ActiveFilters()
		{
			;//
		}

		FilterableFormListManager::~FilterableFormListManager()
		{
			for (auto Itr : ActiveFilters)
				delete Itr;

			ActiveFilters.clear();
		}

		bool FilterableFormListManager::Register(HWND FilterEdit, HWND FilterLabel, HWND FormList, HWND ParentWindow, int TimePeriod /*= 250*/, SecondaryFilterT UserFilter /*= NULL*/ )
		{
			SME_ASSERT(ParentWindow && FormList);
			SME_ASSERT(FilterEdit && FilterLabel);

			if (Lookup(FilterEdit) == nullptr)
			{
				ActiveFilters.push_back(new FilterableWindowData(ParentWindow, FilterEdit, FormList, FilterLabel, TimePeriod, UserFilter));
				return true;
			}

			return false;
		}

		void FilterableFormListManager::Unregister(HWND FilterEdit)
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

		bool FilterableFormListManager::HandleMessages(HWND FilterEdit, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			SME_ASSERT(FilterEdit);

			FilterableWindowData* Data = Lookup(FilterEdit);
			if (Data)
				return Data->HandleMessages(uMsg, wParam, lParam);

			return false;
		}

		void FilterableFormListManager::SetEnabledState(HWND FilterEdit, bool State)
		{
			SME_ASSERT(FilterEdit);

			FilterableWindowData* Data = Lookup(FilterEdit);
			if (Data)
				Data->SetEnabledState(State);
		}

		FilterableFormListManager::FilterableWindowData* FilterableFormListManager::Lookup(HWND FilterEdit)
		{
			for (auto Itr : ActiveFilters)
			{
				if (*Itr == FilterEdit)
					return Itr;
			}

			return nullptr;
		}

		FormEnumerationManager		FormEnumerationManager::Instance;

		FormEnumerationManager::FormEnumerationManager() :
			VisibilityDeletedForms(true),
			VisibilityUnmodifiedForms(true)
		{
			;//
		}

		FormEnumerationManager::~FormEnumerationManager()
		{
			;//
		}

		bool FormEnumerationManager::GetVisibleDeletedForms( void ) const
		{
			return VisibilityDeletedForms == true;
		}

		bool FormEnumerationManager::GetVisibleUnmodifiedForms( void ) const
		{
			return VisibilityUnmodifiedForms == true;
		}

		bool FormEnumerationManager::ToggleVisibilityUnmodifiedForms( void )
		{
			VisibilityUnmodifiedForms = (VisibilityUnmodifiedForms == false);
			TESDialog::ResetFormListControls();

			return VisibilityUnmodifiedForms;
		}

		bool FormEnumerationManager::ToggleVisibilityDeletedForms( void )
		{
			VisibilityDeletedForms = (VisibilityDeletedForms == false);
			TESDialog::ResetFormListControls();

			return VisibilityDeletedForms;
		}

		bool FormEnumerationManager::GetShouldEnumerate( TESForm* Form )
		{
			if (Form && (Form->formFlags & TESForm::kFormFlags_FromActiveFile) == 0 && GetVisibleUnmodifiedForms() == false)
				return false;
			else if (Form && (Form->formFlags & TESForm::kFormFlags_Deleted) && GetVisibleDeletedForms() == false)
				return false;
			else
				return true;
		}

		void FormEnumerationManager::ResetVisibility( void )
		{
			VisibilityDeletedForms = true;
			VisibilityUnmodifiedForms = true;
		}

		int FormEnumerationManager::CompareActiveForms( TESForm* FormA, TESForm* FormB, int OriginalResult )
		{
			int Result = OriginalResult;
			bool Enabled = settings::dialogs::kSortFormListsByActiveForm.GetData().i;

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

		DialogExtraFittingsData::DialogExtraFittingsData() :
			bgsee::WindowExtraData(kTypeID)
		{
			LastCursorPos.x = LastCursorPos.y = 0;
			LastCursorPosWindow = nullptr;
			QuickViewTriggered = false;

			AssetControlToolTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, nullptr,
												TTS_ALWAYSTIP|TTS_NOPREFIX,
												CW_USEDEFAULT, CW_USEDEFAULT,
												CW_USEDEFAULT, CW_USEDEFAULT,
												nullptr, nullptr, nullptr, nullptr);
			ZeroMemory(&AssetControlToolData, sizeof(AssetControlToolData));
			AssetControlToolData.cbSize = sizeof(AssetControlToolData);
			LastTrackedTool = nullptr;
			TrackingToolTip = false;
		}

		DialogExtraFittingsData::~DialogExtraFittingsData()
		{
			if (AssetControlToolTip)
				DestroyWindow(AssetControlToolTip);
		}

		TESFormEditData::TESFormEditData() :
			bgsee::WindowExtraData(kTypeID)
		{
			Buffer = nullptr;
		}

		TESFormEditData::~TESFormEditData()
		{
			if (Buffer)
				Buffer->DeleteInstance();
		}

		void TESFormEditData::FillBuffer( TESForm* Parent )
		{
			SME_ASSERT(Buffer == nullptr);

			Buffer = TESForm::CreateTemporaryCopy(Parent);
		}

		bool TESFormEditData::HasChanges( TESForm* Parent )
		{
			SME_ASSERT(Buffer && Buffer->formType == Parent->formType);

			return Buffer->CompareTo(Parent);
		}


		TESFormIDListViewData::TESFormIDListViewData() :
			bgsee::WindowExtraData(kTypeID)
		{
			DisableDragHandling = false;
		}

		TESFormIDListViewData::~TESFormIDListViewData()
		{
			;//
		}


#define ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID						0x108
#define ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID					0x109

		LRESULT CALLBACK CommonDialogExtraFittingsSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, bgsee::WindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;
			DialogExtraFittingsData* xData = BGSEE_GETWINDOWXDATA_QUICK(DialogExtraFittingsData, ExtraData);

			switch (uMsg)
			{
			case WM_INITDIALOG:
			case WM_OBJECTWINDOWIMPOSTER_INITIALIZEXTRA:
				{
					xData = BGSEE_GETWINDOWXDATA(DialogExtraFittingsData, ExtraData);
					if (xData == nullptr)
					{
						xData = new DialogExtraFittingsData();
						ExtraData->Add(xData);
					}

					SetTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID, 100, nullptr);
					SetTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID, 650, nullptr);
				}

				break;
			case WM_DESTROY:
				{
					xData = BGSEE_GETWINDOWXDATA(DialogExtraFittingsData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(DialogExtraFittingsData::kTypeID);
						delete xData;

						xData = nullptr;
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
							TESForm* Form = nullptr;

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
								GetWindowText(WindowAtPoint, Buffer, sizeof(Buffer));

							if (strlen(Buffer) < 2)
								break;

							std::string PotentialEditorID(Buffer);
							int StatusIndicatorOffset = -1;

							if ((StatusIndicatorOffset = PotentialEditorID.find(" *")) != std::string::npos)
								PotentialEditorID.erase(StatusIndicatorOffset, 2);
							if ((StatusIndicatorOffset = PotentialEditorID.find(" D")) != std::string::npos)
								PotentialEditorID.erase(StatusIndicatorOffset, 2);

							Form = TESForm::LookupByEditorID(PotentialEditorID.c_str());
							xData->LastCursorPosWindow = nullptr;

							if (Form)
							{
								achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_QuickLookup);
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
									TESForm* Form = nullptr;

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
											bgsee::ResourceTemplateT TemplateID = BGSEEUI->GetSubclasser()->GetDialogTemplate(hWnd);

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

									bool ColorizeActiveFormsEnabled = settings::dialogs::kColorizeActiveForms.GetData().i &&
																	FormEnumerationManager::Instance.GetVisibleUnmodifiedForms();

									bool ColorizeFormOverridesEnabled = settings::dialogs::kColorizeFormOverrides.GetData().i;

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
												ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel0ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel0BackColor.GetData().s);

												break;
											case 1:
												ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel1ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel1BackColor.GetData().s);

												break;
											case 2:
												ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel2ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel2BackColor.GetData().s);

												break;
											default:
												ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel3ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel3BackColor.GetData().s);

												break;
											}
										}

										if (Form->IsActive() && ColorizeActiveFormsEnabled)
										{
											ColorOverridden = true;

											ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kActiveFormForeColor.GetData().s);
											BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kActiveFormBackColor.GetData().s);
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
							break;
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


// returns TRUE if there are changes
#define WM_TESFORMIDLISTVIEW_HASCHANGES							(WM_USER + 2006)
// lParam = NMLISTVIEW*
#define WM_TESFORMIDLISTVIEW_SAVECHANGES						(WM_USER + 2007)
#define ID_TESFORMIDLISTVIEW_DRAGTIMER							(WM_USER + 2008)

		LRESULT CALLBACK TESFormIDListViewDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, bgsee::WindowExtraDataCollection* ExtraData )
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
								DlgProcResult = TRUE;
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
								BGSEEUNDOSTACK->Record(new formUndoStack::FormUndoProxy(WorkingCopy));

								if (WorkingCopy->UpdateUsageInfo())
								{
									WorkingCopy->SetFromActiveFile(true);
									WorkingCopy->CopyFrom(LocalCopy);

									if (WorkingCopy->formType == TESForm::kFormType_EffectSetting)
										BGSEEACHIEVEMENTS->Unlock(achievements::kMagister);

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
					// so we tuck in our tail and create replacement templates for all TESFormIDListView forms
					// PS: Dammit!

					SME_ASSERT(FilterEditBox);
					FilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL),
																	GetDlgItem(hWnd, kFormList_TESFormIDListView), hWnd);

					TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
					if (xData == nullptr)
					{
						xData = new TESFormIDListViewData();
						ExtraData->Add(xData);
					}
				}

				break;
			case WM_DESTROY:
				{
					FilterableFormListManager::Instance.Unregister(FilterEditBox);
					TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(TESFormIDListViewData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case ID_TESFORMIDLISTVIEW_DRAGTIMER:
					{
						// reenable dragging after a single tick
						KillTimer(hWnd, ID_TESFORMIDLISTVIEW_DRAGTIMER);
						TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
						SME_ASSERT(xData);
						xData->DisableDragHandling = false;
					}

					break;
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
							DialogExtraParam* xParam = CS_CAST(TESDialog::GetDialogExtraByType(hWnd, BSExtraData::kDialogExtra_Param), BSExtraData, DialogExtraParam);
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
							TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
							SME_ASSERT(xData);
							if (xData->DisableDragHandling == false)
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
							else
								Return = true;
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

								// we need to disable the listview begin drag handler for a bit to prevent it from getting triggered after the selection
								TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
								SME_ASSERT(xData);
								xData->DisableDragHandling = true;
								SetTimer(hWnd, ID_TESFORMIDLISTVIEW_DRAGTIMER, 1000, nullptr);

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
								TESForm* NewSelection = nullptr;

								if (xParam && xLocalCopy && NewIndex != -1 && (NewSelection = (TESForm*)TESListView::GetItemData(ListView, NewIndex)))
								{
									xParam->form = NewSelection;
									xLocalCopy->localCopy->CopyFrom(NewSelection);
									xLocalCopy->localCopy->SetFromActiveFile(true);
									xLocalCopy->localCopy->SetDataInDialog(hWnd);

									char WindowTitle[0x100] = {0};
									if (NewSelection->editorID.Size())
										FORMAT_STR(WindowTitle, "%s : %s", NewSelection->GetTypeIDString(), NewSelection->GetEditorID());
									else
										FORMAT_STR(WindowTitle, "%s : EMPTY", NewSelection->GetTypeIDString());

									SetWindowText(hWnd, WindowTitle);
									if (IsWindowEnabled(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok)) == FALSE)
									{
										EnableWindow(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok), TRUE);
										InvalidateRect(hWnd, nullptr, TRUE);
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

			if (Return == false && FilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
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


		LRESULT CALLBACK TESFormEditDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, bgsee::WindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					TESFormEditData* xData = BGSEE_GETWINDOWXDATA(TESFormEditData, ExtraData);
					TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

					if (xData == nullptr)
					{
						xData = new TESFormEditData();
						ExtraData->Add(xData);

						if (WorkingCopy)		// can be NULL when creating new forms
							xData->FillBuffer(WorkingCopy);
					}

					if (WorkingCopy)
					{
						std::string Desc(WorkingCopy->GetTypeIDString());
						if (WorkingCopy->GetEditorID())
							Desc += " [" + std::string(WorkingCopy->GetEditorID()) + "]";

						std::string WndTitle = Desc;

						if (settings::general::kShowHallOfFameMembersInTitleBar().i == hallOfFame::kDisplayESMember_ObjectPreviewEdit)
						{
							hallOfFame::GetRandomESMember(WndTitle);
							WndTitle += " " + Desc;
						}
						SetWindowText(hWnd, WndTitle.c_str());
					}
				}

				break;
			case WM_DESTROY:
				{
					TESFormEditData* xData = BGSEE_GETWINDOWXDATA(TESFormEditData, ExtraData);
					if (xData)
					{
						// at this point, the working copy is already modified (if there were changes and the user confirmed them)
						TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

						if (WorkingCopy && xData->HasChanges(WorkingCopy))
						{
							// create a proxy from the pre-filled buffer
							BGSEEUNDOSTACK->Record(new formUndoStack::FormUndoProxy(WorkingCopy, xData->Buffer));
						}

						ExtraData->Remove(TESFormEditData::kTypeID);
						delete xData;
					}

					ObjectWindowImposterManager::Instance.RefreshImposters();
				}

				break;
			}

			return DlgProcResult;
		}


		LRESULT CALLBACK WindowPosDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												  bool& Return, bgsee::WindowExtraDataCollection* ExtraData)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_WINDOWPOS_GETCLASSNAME:
				{
					Return = true;
					DlgProcResult = TRUE;

					bgsee::ResourceTemplateT Template = BGSEEUI->GetSubclasser()->GetDialogTemplate(hWnd);
					std::string* OutClassName = (std::string*)wParam;
					OutClassName->clear();

					switch (Template)
					{
					// TESBoundObject/FormEdit Dialogs
					case TESDialog::kDialogTemplate_Weapon:
					case TESDialog::kDialogTemplate_Armor:
					case TESDialog::kDialogTemplate_Clothing:
					case TESDialog::kDialogTemplate_MiscItem:
					case TESDialog::kDialogTemplate_Static:
					case TESDialog::kDialogTemplate_Reference:
					case TESDialog::kDialogTemplate_Apparatus:
					case TESDialog::kDialogTemplate_Book:
					case TESDialog::kDialogTemplate_Container:
					case TESDialog::kDialogTemplate_Activator:
					case TESDialog::kDialogTemplate_AIForm:
					case TESDialog::kDialogTemplate_Light:
					case TESDialog::kDialogTemplate_Potion:
					case TESDialog::kDialogTemplate_Enchantment:
					case TESDialog::kDialogTemplate_LeveledCreature:
					case TESDialog::kDialogTemplate_Sound:
					case TESDialog::kDialogTemplate_Door:
					case TESDialog::kDialogTemplate_LeveledItem:
					case TESDialog::kDialogTemplate_LandTexture:
					case TESDialog::kDialogTemplate_SoulGem:
					case TESDialog::kDialogTemplate_Ammo:
					case TESDialog::kDialogTemplate_Spell:
					case TESDialog::kDialogTemplate_Flora:
					case TESDialog::kDialogTemplate_Tree:
					case TESDialog::kDialogTemplate_CombatStyle:
					case TESDialog::kDialogTemplate_Water:
					case TESDialog::kDialogTemplate_NPC:
					case TESDialog::kDialogTemplate_Creature:
					case TESDialog::kDialogTemplate_Grass:
					case TESDialog::kDialogTemplate_Furniture:
					case TESDialog::kDialogTemplate_LoadingScreen:
					case TESDialog::kDialogTemplate_Ingredient:
					case TESDialog::kDialogTemplate_LeveledSpell:
					case TESDialog::kDialogTemplate_AnimObject:
					case TESDialog::kDialogTemplate_Subspace:
					case TESDialog::kDialogTemplate_EffectShader:
					case TESDialog::kDialogTemplate_SigilStone:
					// TESFormIDListView Dialogs
					case TESDialog::kDialogTemplate_Faction:
					case TESDialog::kDialogTemplate_Race:
					case TESDialog::kDialogTemplate_Class:
					case TESDialog::kDialogTemplate_Skill:
					case TESDialog::kDialogTemplate_EffectSetting:
					case TESDialog::kDialogTemplate_GameSetting:
					case TESDialog::kDialogTemplate_Globals:
					case TESDialog::kDialogTemplate_Birthsign:
					case TESDialog::kDialogTemplate_Climate:
					case TESDialog::kDialogTemplate_Worldspace:
					case TESDialog::kDialogTemplate_Hair:
					case TESDialog::kDialogTemplate_Eyes:
					case TESDialog::kDialogTemplate_Weather:
						{
							DialogExtraParam* xParam = CS_CAST(TESDialog::GetDialogExtraByType(hWnd, BSExtraData::kDialogExtra_Param),
															   BSExtraData, DialogExtraParam);
							if (xParam)
								*OutClassName = TESForm::GetFormTypeIDLongName(xParam->formType);
						}
						break;
					// Misc Dialogs
					case TESDialog::kDialogTemplate_CellEdit:
						*OutClassName = "Cell Edit";
						break;
					case TESDialog::kDialogTemplate_SearchReplace:
						*OutClassName = "Search Replace";
						break;
					case TESDialog::kDialogTemplate_LandscapeEdit:
						*OutClassName = "Landscape Edit";
						break;
					case TESDialog::kDialogTemplate_FindText:
						*OutClassName = "Find Text";
						break;
					case TESDialog::kDialogTemplate_RegionEditor:
						*OutClassName = "Region Editor";
						break;
					case TESDialog::kDialogTemplate_HeightMapEditor:
						*OutClassName = "Height Map Editor";
						break;
					case TESDialog::kDialogTemplate_IdleAnimations:
						*OutClassName = "Idle Anims";
						break;
					case TESDialog::kDialogTemplate_AIPackages:
						*OutClassName = "AI Packages";
						break;
					case TESDialog::kDialogTemplate_TextureUse:
						*OutClassName = "Texture Use";
						break;
					case TESDialog::kDialogTemplate_Package:
						*OutClassName = "Package";
						break;
					case TESDialog::kDialogTemplate_ChooseReference:
						*OutClassName = "Choose Ref";
						break;
					default:
						*OutClassName = "";
						break;
					}
				}

				break;
			case WM_INITDIALOG:
				if (settings::dialogs::kPreserveEditorDialogLocations().i)
				{
					std::string ClassName = "";
					SendMessage(hWnd, WM_WINDOWPOS_GETCLASSNAME, (WPARAM)&ClassName, NULL);

					if (ClassName.length())
					{
						RECT Bounds = { 0 };
						if (TESDialog::ReadBoundsFromINI(ClassName.c_str(), &Bounds))
							SetWindowPos(hWnd, HWND_TOP, Bounds.left, Bounds.top, 0, 0, SWP_NOSIZE);
					}
				}

				break;
			case WM_DESTROY:
				if (settings::dialogs::kPreserveEditorDialogLocations().i)
				{
					std::string ClassName = "";
					SendMessage(hWnd, WM_WINDOWPOS_GETCLASSNAME, (WPARAM)&ClassName, NULL);

					if (ClassName.length())
						TESDialog::WriteBoundsToINI(hWnd, ClassName.c_str());
				}

				break;
			}

			return DlgProcResult;
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

			// window pos saver subclass
			{
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LandTexture, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Enchantment, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Spell, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Sound, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Activator, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Apparatus, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Armor, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Book, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Clothing, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Container, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Door, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Ingredient, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Light, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_MiscItem, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Static, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Grass, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Tree, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Flora, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Furniture, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Weapon, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Ammo, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_NPC, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Creature, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledCreature, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SoulGem, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Potion, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Subspace, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SigilStone, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledItem, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Package, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CombatStyle, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LoadingScreen, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledSpell, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AnimObject, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Water, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_EffectShader, WindowPosDlgSubClassProc);

				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Faction, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Race, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Class, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Skill, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_EffectSetting, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_GameSetting, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Globals, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Birthsign, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Climate, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Worldspace, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Hair, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Eyes, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Weather, WindowPosDlgSubClassProc);

				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CellEdit, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SearchReplace, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LandscapeEdit, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FindText, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RegionEditor, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_HeightMapEditor, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_IdleAnimations, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AIPackages, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_TextureUse, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Package, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ChooseReference, WindowPosDlgSubClassProc);
			}

			InitializeMiscWindowOverrides();

			BGSEEUI->GetWindowHandleCollection(bgsee::UIManager::kHandleCollection_DragDropableWindows)->Add(
																								cliWrapper::interfaces::TAG->GetFormDropWindowHandle());

			if (settings::dialogs::kShowEditDialogsInTaskbar.GetData().i)
			{
				bgsee::WindowStyler::StyleData RegularAppWindow = {0};
				RegularAppWindow.Extended = WS_EX_APPWINDOW;
				RegularAppWindow.ExtendedOp = bgsee::WindowStyler::StyleData::kOperation_OR;

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


			PreviewWindowImposterManager::Instance.SetEnabled(settings::dialogs::kMultiplePreviewWindows().i == 1);

			// thanks to UAC and its rather lovely UIPI component on Vista+,
			// Win32 drag-drop operations don't work when the editor is running with elevated privileges
			// we fix this by add the corresponding messages to the UIPI filter (using the process-wide filter to make things easier for me)
			ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
			ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
			ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);

			SendMessage(*TESCSMain::WindowHandle, WM_MAINWINDOW_INITEXTRADATA, NULL, NULL);
		}
	}
}