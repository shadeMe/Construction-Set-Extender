#include "UIManager.h"
#include "Main.h"
#include "Construction Set Extender_Resource.h"
#include "DialogImposterManager.h"
#include "MiscWindowOverrides.h"
#include "MainWindowOverrides.h"
#include "Achievements.h"
#include "HallOfFame.h"
#include "[Common]\CLIWrapper.h"


namespace cse
{
	namespace uiManager
	{
		FilterableFormListManager FilterableFormListManager::Instance;

		FilterableFormListManager::FilterableWindowData::FilterableWindowData( InitParams Params ) :
			ThunkFormListSubclassProc(this, &FilterableWindowData::FormListSubclassProc),
			ThunkFilterEditBoxSubclassProc(this, &FilterableWindowData::FilterEditBoxSubclassProc),
			Params(Params),
			FilterString(""),
			FilterRegEx(""),
			TimeCounter(-1),
			Enabled(true)
		{
			SME_ASSERT(Params.ParentWindow);
			SME_ASSERT(Params.FilterEditBox);
			SME_ASSERT(Params.FilterLabel);
			SME_ASSERT(Params.FormListView);

			auto TimerResult = SetTimer(Params.ParentWindow, reinterpret_cast<UINT_PTR>(this), 500, nullptr);
			SME_ASSERT(TimerResult != 0);

			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(Params.FormListView, ThunkFormListSubclassProc());
			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(Params.FilterEditBox, ThunkFilterEditBoxSubclassProc());

			Flags = kFlags_SearchEditorID | kFlags_SearchDescription | kFlags_SearchName;

			EnumerateSearchableColumns();
		}

		FilterableFormListManager::FilterableWindowData::~FilterableWindowData()
		{
			SetWindowText(Params.FilterEditBox, "");

			BGSEEUI->GetSubclasser()->DeregisterSubclassForWindow(Params.FormListView, ThunkFormListSubclassProc());
			BGSEEUI->GetSubclasser()->DeregisterSubclassForWindow(Params.FilterEditBox, ThunkFilterEditBoxSubclassProc());
			KillTimer(Params.ParentWindow, reinterpret_cast<UINT_PTR>(this));
		}

		LRESULT FilterableFormListManager::FilterableWindowData::FormListSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																					bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT CallbackResult = FALSE;

			switch (uMsg)
			{
			case LVM_INSERTCOLUMN:
			case LVM_DELETECOLUMN:
				// Tunnel the message and reenumerate searchable columns. The currently implementation is rather wasteful,
				// but we don't expect column modification to happen often enough to matter.
				SubclassParams->Out.MarkMessageAsHandled = true;

				CallbackResult = SubclassParams->In.Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);
				EnumerateSearchableColumns();

				break;
			case LVM_INSERTITEM:
				LVITEM* Item = (LVITEM*)lParam;

				if (Enabled && Item->lParam)
				{
					TESForm* Form = (TESForm*)Item->lParam;
					if (Form)
					{
						if (!FilterForm(Form) || (Params.CustomFilterPredicate && !Params.CustomFilterPredicate(Form)))
						{
							SubclassParams->Out.MarkMessageAsHandled = true;
							CallbackResult = -1;
						}
					}
				}

				break;
			}

			return CallbackResult;
		}

		LRESULT FilterableFormListManager::FilterableWindowData::FilterEditBoxSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																							bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT CallbackResult = FALSE;

			switch (uMsg)
			{
			case WM_KEYDOWN:
			case WM_KEYUP:
				TimeCounter = GetTickCount64();
				break;
			case WM_RBUTTONUP:
				if (!GetAsyncKeyState(VK_CONTROL))
					break;

				SubclassParams->Out.MarkMessageAsHandled = true;

				POINT CursorLoc { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				ClientToScreen(hWnd, &CursorLoc);
				HandlePopupMenu(Params.ParentWindow, CursorLoc.x, CursorLoc.y);

				break;
			}

			return CallbackResult;
		}

		bool FilterableFormListManager::FilterableWindowData::HandleMessages( UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_RBUTTONUP:
				{
					POINT CursorLoc = { 0 };
					GetCursorPos(&CursorLoc);

					HWND WindowFromPoint = SME::UIHelpers::WinSpy::WindowFromPointEx(CursorLoc, FALSE);
					if (WindowFromPoint == Params.FilterLabel)
						HandlePopupMenu(Params.ParentWindow, CursorLoc.x, CursorLoc.y);
				}

				break;
			case WM_TIMER:
				if (wParam != reinterpret_cast<WPARAM>(this))
					break;
				else if (TimeCounter == -1)
					break;

				auto Elapsed = GetTickCount64() - TimeCounter;
				if (Elapsed < Params.InputTimeoutThreshold)
					break;

				TimeCounter = -1;

				char Buffer[0x200] = {0};
				GetWindowText(Params.FilterEditBox, Buffer, sizeof(Buffer));

				if (HasRegEx() && strcmp(Buffer, FilterString.c_str()) == 0)
					break;
				else if (!HasRegEx() && _stricmp(Buffer, FilterString.c_str()) == 0)
					break;

				FilterString = Buffer;
				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_FormListFilter);

				if (!HasRegEx())
					SME::StringHelpers::MakeLower(FilterString);
				else try {
					FilterRegEx = FilterString;
				} catch (...) {
					FilterString.clear();
					FilterRegEx = "";

					return false;
				}

				return true;
			}

			return false;
		}

		void FilterableFormListManager::FilterableWindowData::EnumerateSearchableColumns()
		{
			static const std::unordered_set<std::string> IgnoredColumnsNames {
				"Editor ID", "Form ID",
				"EditorID", "FormID",
				"Name", "Description"
			};

			if (Params.ColumnTextCallback == nullptr)
				return;

			SearchableColumns.clear();

			char Buffer[0x100];
			auto ColumnCount = TESListView::GetColumnCount(Params.FormListView);
			for (int i = 0; i < ColumnCount; ++i)
			{
				LVCOLUMN ColData;
				ColData.mask = LVCF_TEXT;
				ColData.pszText = Buffer;
				ColData.cchTextMax = sizeof(Buffer);

				ListView_GetColumn(Params.FormListView, i, &ColData);

				if (IgnoredColumnsNames.find(ColData.pszText) != IgnoredColumnsNames.cend())
					continue;

				SearchableColumns.emplace_back(i, SearchEnabledColumns.find(ColData.pszText) != SearchEnabledColumns.cend(), ColData.pszText);
			}
		}

		bool FilterableFormListManager::FilterableWindowData::FilterForm(TESForm* Form)
		{
			SME_ASSERT(Form);

			if (FilterString.empty())
				return true;

			auto MatchesFilter = [&](std::string& String) -> bool {
				if (FilterString.empty())
					return true;
				else if (String.empty())
					return false;

				if (HasRegEx())
				{
					try
					{
						std::smatch Results;
						if (std::regex_search(String, Results, FilterRegEx))
							return true;
					} catch (...) {
						// Fail fast if bad regex.
						FilterString.clear();
						FilterRegEx = "";

						return true;
					}
				}
				else
				{
					SME::StringHelpers::MakeLower(String);
					if (String.find(FilterString) != std::string::npos)
						return true;
				}

				return false;
			};

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


			if (HasEditorID())
			{
				std::string EditorId;

				if (Form->editorID.c_str())
					EditorId = Form->GetEditorID();
				else if (Form->IsReference())
				{
					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (Reference->baseForm && Reference->baseForm->GetEditorID())
						EditorId = Reference->baseForm->GetEditorID();
				}

				if (MatchesFilter(EditorId))
					return true;
			}

			if (HasName() && FullNameCmpt && FullNameCmpt->name.c_str())
			{
				if (MatchesFilter(std::string(FullNameCmpt->name.c_str())))
					return true;
			}

			if (HasDescription() && DescriptionCmpt && DescriptionCmpt->description.c_str())
			{
				if (MatchesFilter(std::string(DescriptionCmpt->description.c_str())))
					return true;
			}

			if (HasFormID())
			{
				char Buffer[50] = { 0 };
				FORMAT_STR(Buffer, "%08x", Form->formID);

				if (MatchesFilter(std::string(Buffer)))
					return true;
			}

			for (const auto& Itr : SearchableColumns)
			{
				if (!Itr.Search)
					continue;

				auto ColumnText = Params.ColumnTextCallback(Params.FormListView, Form, Itr.Index);
				if (MatchesFilter(ColumnText))
					return true;
			}

			return false;
		}

#define IDC_CSEFILTERABLEFORMLIST_REGEX				9009
#define IDC_CSEFILTERABLEFORMLIST_EDITORID			9010
#define IDC_CSEFILTERABLEFORMLIST_NAME				9011
#define IDC_CSEFILTERABLEFORMLIST_DESCRIPTION		9012
#define IDC_CSEFILTERABLEFORMLIST_FORMID			9013
#define IDC_CSEFILTERABLEFORMLIST_COLUMN_OFFSET		9500


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

			if (Params.ColumnTextCallback)
			{
				InsertMenu(Popup, -1, MF_BYPOSITION | MF_SEPARATOR, NULL, nullptr);

				int ColumnItemId = IDC_CSEFILTERABLEFORMLIST_COLUMN_OFFSET;
				for (const auto& Column : SearchableColumns)
				{
					char Buffer[0x100];
					_snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "Search %s", Column.Name.c_str());

					Item.fState = MFS_ENABLED;
					Item.dwTypeData = Buffer;
					Item.cch = strlen((const char*)Item.dwTypeData);
					if (Column.Search)
						Item.fState |= MFS_CHECKED;
					Item.wID = ColumnItemId;
					InsertMenuItem(Popup, -1, TRUE, &Item);

					++ColumnItemId;
				}
			}

			auto MenuSelection = TrackPopupMenu(Popup, TPM_RETURNCMD, X, Y, NULL, Parent, nullptr);
			switch (MenuSelection)
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
			default:
				if (MenuSelection < IDC_CSEFILTERABLEFORMLIST_COLUMN_OFFSET || MenuSelection >= IDC_CSEFILTERABLEFORMLIST_COLUMN_OFFSET + SearchableColumns.size())
					break;

				auto ColumnIdx = MenuSelection - IDC_CSEFILTERABLEFORMLIST_COLUMN_OFFSET;
				auto& ColumnData = SearchableColumns[ColumnIdx];

				ColumnData.Search = ColumnData.Search == false;
				if (ColumnData.Search)
					SearchEnabledColumns.emplace(ColumnData.Name);
				else
					SearchEnabledColumns.erase(ColumnData.Name);
			}

			if (MenuSelection != 0)
			{
				TimeCounter = 0;
				FilterString.clear();
				FilterRegEx = "";
			}

			DestroyMenu(Popup);
		}

		void FilterableFormListManager::FilterableWindowData::SetEnabled(bool State)
		{
			Enabled = State;
		}

		bool FilterableFormListManager::FilterableWindowData::HasFilter() const
		{
			return !FilterString.empty();
		}

		void FilterableFormListManager::FilterableWindowData::ResetFilter()
		{
			FilterString.clear();
			FilterRegEx = "";
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

		bool FilterableFormListManager::Register(InitParams Params)
		{
			if (Params.FilterEditBox == NULL)
				return false;

			if (LookupByFilterEdit(Params.FilterEditBox) == nullptr)
			{
				ActiveFilters.emplace_back(new FilterableWindowData(Params));
				return true;
			}

			return false;
		}

		void FilterableFormListManager::Unregister(HWND FilterEdit)
		{
			DEBUG_ASSERT(FilterEdit);

			for (auto Itr = ActiveFilters.begin(); Itr != ActiveFilters.end(); Itr++)
			{
				if ((*Itr)->Params.FilterEditBox == FilterEdit)
				{
					delete *Itr;
					ActiveFilters.erase(Itr);
					return;
				}
			}
		}

		bool FilterableFormListManager::HandleMessages(HWND FilterEdit, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			//DEBUG_ASSERT(FilterEdit);

			FilterableWindowData* Data = LookupByFilterEdit(FilterEdit);
			if (Data)
				return Data->HandleMessages(uMsg, wParam, lParam);

			return false;
		}

		void FilterableFormListManager::SetEnabled(HWND FilterEdit, bool State)
		{
			DEBUG_ASSERT(FilterEdit);

			FilterableWindowData* Data = LookupByFilterEdit(FilterEdit);
			if (Data)
				Data->SetEnabled(State);
		}

		bool FilterableFormListManager::HasActiveFilter(HWND FilterEdit) const
		{
			DEBUG_ASSERT(FilterEdit);

			auto FilterData = LookupByFilterEdit(FilterEdit);
			if (FilterData == nullptr)
				return false;

			return FilterData->HasFilter();
		}

		void FilterableFormListManager::ResetFilter(HWND FilterEdit)
		{
			DEBUG_ASSERT(FilterEdit);

			auto FilterData = LookupByFilterEdit(FilterEdit);
			if (FilterData == nullptr)
				return;

			return FilterData->ResetFilter();
		}

		FilterableFormListManager::FilterableWindowData* FilterableFormListManager::LookupByFilterEdit(HWND FilterEdit) const
		{
			for (auto Itr : ActiveFilters)
			{
				if (Itr->Params.FilterEditBox == FilterEdit)
					return Itr;
			}

			return nullptr;
		}

		FilterableFormListManager::FilterableWindowData* FilterableFormListManager::LookupByFormList(HWND FormList) const
		{
			for (auto Itr : ActiveFilters)
			{
				if (Itr->Params.FormListView == FormList)
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


		DeferredComboBoxController DeferredComboBoxController::Instance;


		DeferredComboBoxController::Message::Message(UINT uMsg /*= WM_NULL*/, WPARAM wParam /*= NULL*/, LPARAM lParam /*= NULL*/)
			: uMsg(uMsg), wParam(wParam), lParam(lParam)
		{
			if (uMsg == CustomMessageAddItem)
				StringPayload = reinterpret_cast<const char*>(wParam);
			else if (uMsg == CB_ADDSTRING || uMsg == CB_INSERTSTRING)
				StringPayload = reinterpret_cast<const char*>(lParam);
		}

		LRESULT DeferredComboBoxController::ComboBoxSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT Result = NULL;

			switch (uMsg)
			{
			case WM_CREATE:
				RegisterComboBox(hWnd);

				break;
			case WM_DESTROY:
				DeregisterComboBox(hWnd);

				break;
			default:
				switch (uMsg)
				{
				case CustomMessageAddItem:
					SubclassParams->Out.MarkMessageAsHandled = true;
					QueueMessage(hWnd, uMsg, wParam, lParam);
					break;
				case CB_ADDSTRING:
					SubclassParams->Out.MarkMessageAsHandled = true;
					Result = AddStringMarkerResult;
					QueueMessage(hWnd, uMsg, wParam, lParam);
					break;
				case CB_SETITEMDATA:
					if (wParam != AddStringMarkerResult)
					{
						// attempting to set the data of an item whose insertion wasn't tracked by us
						// so, flush the queued messages and break early
						FlushQueuedMessages(hWnd, SubclassParams->In.Subclasser);
						break;
					}

					SubclassParams->Out.MarkMessageAsHandled = true;
					QueueMessage(hWnd, uMsg, wParam, lParam);
					break;
				default:
					if (uMsg < CB_GETEDITSEL || uMsg >= CB_MSGMAX)
					{
						// not a combo box message, return early
						break;
					}

					// all other messages will trigger the flushing of queued messages
					// then, pass through the message to the default handler
					FlushQueuedMessages(hWnd, SubclassParams->In.Subclasser);
				}
			}

			return Result;
		}

		void DeferredComboBoxController::RegisterComboBox(HWND hWnd)
		{
			const auto Existing = ActiveComboBoxes.find(hWnd);
			SME_ASSERT(Existing == ActiveComboBoxes.cend());

			ActiveComboBoxes.emplace(std::make_pair(hWnd, TrackedData()));
		}

		void DeferredComboBoxController::DeregisterComboBox(HWND hWnd)
		{
			const auto Existing = ActiveComboBoxes.find(hWnd);
			SME_ASSERT(Existing != ActiveComboBoxes.cend());

			ActiveComboBoxes.erase(hWnd);
		}

		void DeferredComboBoxController::FlushQueuedMessages(HWND hWnd, bgsee::WindowSubclasser* Subclasser)
		{
			const auto Data = ActiveComboBoxes.find(hWnd);
			SME_ASSERT(Data != ActiveComboBoxes.cend());

			if (Data->second.PendingMessages.empty())
				return;


			std::vector<std::pair<std::string, LPARAM>> ItemsToInsert;
			UINT PreviousMsg = WM_NULL;
			for (const auto& Msg : Data->second.PendingMessages)
			{
				switch (Msg.uMsg)
				{
				case CustomMessageAddItem:
					ItemsToInsert.emplace_back(Msg.StringPayload, Msg.lParam);
					break;
				case CB_ADDSTRING:
					ItemsToInsert.emplace_back(Msg.StringPayload, NULL);
					break;
				case CB_SETITEMDATA:
					SME_ASSERT(Msg.wParam == AddStringMarkerResult);
					SME_ASSERT(PreviousMsg == CB_ADDSTRING);

					ItemsToInsert.back().second = Msg.lParam;
					break;
				}

				PreviousMsg = Msg.uMsg;
			}

			Data->second.PendingMessages.clear();

			bool SortedStyle = GetWindowLongPtr(hWnd, GWL_STYLE) & CBS_SORT;
			if (SortedStyle)
			{
				std::sort(ItemsToInsert.begin(), ItemsToInsert.end(), [](const auto& a, const auto& b) -> bool {
					return _stricmp(a.first.c_str(), b.first.c_str()) > 0;
				});
			}

			Subclasser->TunnelMessageToOrgWndProc(hWnd, CB_INITSTORAGE,
				ItemsToInsert.size(), Data->second.TotalStringLength * sizeof(char),
				true);
			SuspendComboBoxUpdates(hWnd, Subclasser, true);
			{
				for (const auto& Itr : ItemsToInsert)
				{
					auto Index = Subclasser->TunnelMessageToOrgWndProc(hWnd, CB_ADDSTRING, NULL,
						reinterpret_cast<LPARAM>(Itr.first.c_str()), true);

					if (Index != CB_ERR && Index != CB_ERRSPACE && Itr.second)
						Subclasser->TunnelMessageToOrgWndProc(hWnd, CB_SETITEMDATA, Index, Itr.second, true);
				}

				auto DC = GetDC(hWnd);
				if (DC)
				{
					SIZE Extents;

					// lazy text width calculation
					if (GetTextExtentPoint32A(DC, std::string(Data->second.LongestStringLength, 'a').c_str(),
						Data->second.LongestStringLength, &Extents))
					{
						auto CurrentWidth = Subclasser->TunnelMessageToOrgWndProc(hWnd, CB_GETDROPPEDWIDTH, NULL, NULL, true);
						if (CurrentWidth < Extents.cx)
							Subclasser->TunnelMessageToOrgWndProc(hWnd, CB_SETDROPPEDWIDTH, Extents.cx, NULL, true);
					}
					ReleaseDC(hWnd, DC);
				}
			}
			SuspendComboBoxUpdates(hWnd, Subclasser, false);
		}

		void DeferredComboBoxController::QueueMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			const auto Data = ActiveComboBoxes.find(hWnd);
			SME_ASSERT(Data != ActiveComboBoxes.cend());

			Data->second.PendingMessages.emplace_back(uMsg, wParam, lParam);
			const auto& NewMsg = Data->second.PendingMessages.back();

			Data->second.TotalStringLength += NewMsg.StringPayload.size();
			if (NewMsg.StringPayload.size() > Data->second.LongestStringLength)
				Data->second.LongestStringLength = NewMsg.StringPayload.size();
		}

		void DeferredComboBoxController::SuspendComboBoxUpdates(HWND hWnd, bgsee::WindowSubclasser* Subclasser, bool Suspend) const
		{
			COMBOBOXINFO Info;
			Info.cbSize = sizeof(COMBOBOXINFO);

			if (!Subclasser->TunnelMessageToOrgWndProc(hWnd, CB_GETCOMBOBOXINFO, NULL, reinterpret_cast<LPARAM>(&Info), true))
				return;

			if (!Suspend)
			{
				SendMessage(Info.hwndList, WM_SETREDRAW, TRUE, 0);
				Subclasser->TunnelMessageToOrgWndProc(hWnd, CB_SETMINVISIBLE, 30, 0, true);
				Subclasser->TunnelMessageToOrgWndProc(hWnd, WM_SETREDRAW, TRUE, 0, true);
			}
			else
			{
				Subclasser->TunnelMessageToOrgWndProc(hWnd, WM_SETREDRAW, FALSE, 0, true);	// Prevent repainting until finished
				Subclasser->TunnelMessageToOrgWndProc(hWnd, CB_SETMINVISIBLE, 1, 0, true);	// Possible optimization for older libraries (source: MSDN forums)
				SendMessage(Info.hwndList, WM_SETREDRAW, FALSE, 0);
			}
		}

		DeferredComboBoxController::DeferredComboBoxController() :
			ThunkComboBoxSubclassProc(this, &DeferredComboBoxController::ComboBoxSubclassProc)
		{
			Initialized = false;
		}


		DeferredComboBoxController::~DeferredComboBoxController()
		{
			;//
		}

		void DeferredComboBoxController::Initialize()
		{
			SME_ASSERT(Initialized == false);

			auto ComboBoxSubclassPredicate = [](HWND hWnd) -> bool {
				char ClassName[100];
				GetClassName(hWnd, ClassName, ARRAYSIZE(ClassName));

				bool IsComboBox = strcmp(ClassName, "ComboBox") == 0;
				return IsComboBox;
			};

			BGSEEUI->GetSubclasser()->RegisterGlobalSubclass(ThunkComboBoxSubclassProc(),
															bgsee::WindowSubclassProcCollection::kPriority_Default,
															ComboBoxSubclassPredicate);
			Initialized = true;
		}


		void Initialize( void )
		{
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

			SendMessage(*TESCSMain::WindowHandle, WM_MAINWINDOW_INIT_EXTRADATA, NULL, NULL);
		}

	}
}