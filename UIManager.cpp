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

		LRESULT CALLBACK FilterableFormListManager::FilterableWindowData::FormListSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																								bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser*)
		{
			SME_ASSERT(FormListDataTable.find(hWnd) != FormListDataTable.end());
			FilterableWindowData* UserData = FormListDataTable[hWnd];
			LRESULT CallbackResult = FALSE;

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
						if (UserData->FilterForm(Form) == false || (UserData->SecondFilter && UserData->SecondFilter(Form) == false))
						{
							Return = true;
							CallbackResult = -1;
						}
					}
				}

				break;
			}

			return CallbackResult;
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
			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(FormListView, FormListSubclassProc);
			FormListDataTable[FormListView] = this;
		}

		void FilterableFormListManager::FilterableWindowData::UnhookFormList(void)
		{
			BGSEEUI->GetSubclasser()->DeregisterSubclassForWindow(FormListView, FormListSubclassProc);
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