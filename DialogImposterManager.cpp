#include "DialogImposterManager.h"
#include "UIManager.h"
#include "HallOfFame.h"

namespace cse
{
	ObjectWindowImposterManager					ObjectWindowImposterManager::Instance;

#define ID_OBJECTWIDOWIMPOSTER_COLUMNRESIZETIMERID			0x656

	INT_PTR CALLBACK ObjectWindowImposterManager::ImposterDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
		HWND FormList = GetDlgItem(hWnd, TESObjectWindow::kFormListCtrlID);
		HWND TreeList = GetDlgItem(hWnd, TESObjectWindow::kTreeViewCtrlID);

		INT_PTR DlgProcResult = FALSE;

		switch (uMsg)
		{
		case WM_OBJECTWINDOWIMPOSTER_REFRESHTREEVIEW:
			{
				CacheOperator CacheBackup(hWnd);
				TESObjectWindow::UpdateTreeChildren(hWnd);

				DlgProcResult = TRUE;
				break;
			}
		case WM_OBJECTWINDOWIMPOSTER_REFRESHFORMLIST:
			{
				HTREEITEM Selection = TreeView_GetSelection(TreeList);

				TreeView_SelectItem(TreeList, nullptr);
				TreeView_SelectItem(TreeList, Selection);

				DlgProcResult = TRUE;
				break;
			}
		case WM_TIMER:
			switch (wParam)
			{
			case ID_OBJECTWIDOWIMPOSTER_COLUMNRESIZETIMERID:
				for (int i = 0;; i++)
				{
					// skip the formID column
					if (i == 1)
						continue;

					if (ListView_SetColumnWidth(FormList, i, LVSCW_AUTOSIZE_USEHEADER) == FALSE)
						break;
				}

				KillTimer(hWnd, ID_OBJECTWIDOWIMPOSTER_COLUMNRESIZETIMERID);
				break;
			}

			break;
		case WM_NOTIFY:
			{
				NMHDR* NotifyData = (NMHDR*)lParam;
				switch (NotifyData->code)
				{
				case LVN_KEYDOWN:
				case TVN_KEYDOWN:
					if (((NMLVKEYDOWN*)lParam)->wVKey == VK_F5)
					{
						SendMessage(hWnd, WM_OBJECTWINDOWIMPOSTER_REFRESHFORMLIST, NULL, NULL);
						DlgProcResult = TRUE;
					}

					break;
				case TVN_SELCHANGED:
					{
						// (hack)workaround for the non-sorting form list
						SetTimer(hWnd, ID_OBJECTWIDOWIMPOSTER_COLUMNRESIZETIMERID, 500, nullptr);
						break;
					}
				}

				break;
			}
		case WM_ACTIVATE:
			ObjectWindowImposterManager::Instance.HandleObjectWindowActivating(hWnd, uMsg, wParam, lParam);

			DlgProcResult = TRUE;
			break;
		case WM_CLOSE:
			// destroy window
			ObjectWindowImposterManager::Instance.DisposeImposter(hWnd);

			DlgProcResult = TRUE;
			break;
		case WM_DESTROY:
			{
				uiManager::FilterableFormListManager::Instance.Unregister(FilterEditBox);

				CacheOperator CacheBackup(hWnd);
				TESObjectWindow::PerformLimitedDeinit(hWnd);

				DlgProcResult = TRUE;
				break;
			}
		case WM_INITDIALOG:
			{
				// save the imposter's initialized state
				SetWindowLongPtr(FormList, GWL_USERDATA, NULL);

				RECT Bounds = { 0 };
				GetWindowRect(TESObjectWindow::PrimaryObjectWindowHandle, &Bounds);
				SetWindowPos(hWnd, HWND_NOTOPMOST, Bounds.left, Bounds.top + 25, Bounds.right - Bounds.left, Bounds.bottom - Bounds.top, SWP_SHOWWINDOW);

				DlgProcResult = TRUE;
				break;
			}
		case WM_OBJECTWINDOWIMPOSTER_INITIALIZE:
			{
				SME_ASSERT(FilterEditBox);
				uiManager::FilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL), FormList, hWnd);

				// reproduce the relevant bits of the org wnd proc's code
				CacheOperator CacheBackup(hWnd);
				std::string WndTitle = "Object Window";

				TESObjectWindow::PerformLimitedInit(hWnd);
				SendMessage(hWnd, WM_OBJECTWINDOWIMPOSTER_REFRESHTREEVIEW, NULL, NULL);

				if (settings::general::kShowHallOfFameMembersInTitleBar().i != hallOfFame::kDisplayESMember_None)
				{
					hallOfFame::GetRandomESMember(WndTitle);
					WndTitle += " Object Window";
				}
				SetWindowText(hWnd, WndTitle.c_str());

				BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(hWnd, uiManager::CommonDialogExtraFittingsSubClassProc);
				SendMessage(hWnd, WM_OBJECTWINDOWIMPOSTER_INITIALIZEXTRA, NULL, NULL);
				SetWindowLongPtr(FormList, GWL_USERDATA, 1);

				TreeView_SelectItem(TreeList, TreeView_GetFirstVisible(TreeList));
				SendMessage(hWnd, WM_OBJECTWINDOWIMPOSTER_REFRESHFORMLIST, NULL, NULL);

				DlgProcResult = TRUE;
				break;
			}
		case WM_SIZE:
			ObjectWindowImposterManager::Instance.HandleObjectWindowSizing(hWnd, uMsg, wParam, lParam);

			DlgProcResult = TRUE;
			break;
		}

		if (DlgProcResult == FALSE && GetWindowLongPtr(FormList, GWL_USERDATA) != NULL)
		{
			if (uiManager::FilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
			{
				SendMessage(hWnd, WM_OBJECTWINDOWIMPOSTER_REFRESHFORMLIST, NULL, NULL);
				DlgProcResult = TRUE;
			}
		}

		if (DlgProcResult == FALSE && GetWindowLongPtr(FormList, GWL_USERDATA) != NULL)
		{
			// pass on the message to the original wnd proc
			// we update the caches to point to the active imposter's controls for a single message instance
			CacheOperator CacheBackup(hWnd);
			DlgProcResult = TESObjectWindow::DialogProc(hWnd, uMsg, wParam, lParam);
		}

		return DlgProcResult;
	}

	ObjectWindowImposterManager::ImposterData::ImposterData()
	{
		TreeSelection = 0;
		for (int i = 0; i < TESObjectWindow::TreeEntryInfo::kTreeEntryCount; i++)
			SortColumns[i] = 1;
	}

	ObjectWindowImposterManager::CacheOperator::CacheOperator(HWND Imposter) :
		ParentData(nullptr)
	{
		SME_ASSERT(Imposter);
		ParentData = ObjectWindowImposterManager::Instance.GetImposterData(Imposter);
		SME_ASSERT(ParentData);

		MainWindow = *TESObjectWindow::WindowHandle;
		FormList = *TESObjectWindow::FormListHandle;
		TreeView = *TESObjectWindow::TreeViewHandle;
		Splitter = *TESObjectWindow::SplitterHandle;
		TreeSelection = *TESObjectWindow::CurrentTreeNode;
		memcpy(SortColumns, TESObjectWindow::SortColumnArray, sizeof(SortColumns));

		*TESObjectWindow::WindowHandle = Imposter;
		*TESObjectWindow::FormListHandle = GetDlgItem(Imposter, TESObjectWindow::kFormListCtrlID);
		*TESObjectWindow::TreeViewHandle = GetDlgItem(Imposter, TESObjectWindow::kTreeViewCtrlID);
		*TESObjectWindow::SplitterHandle = GetDlgItem(Imposter, TESObjectWindow::kSplitterCtrlID);
		*TESObjectWindow::CurrentTreeNode = ParentData->TreeSelection;
		memcpy(TESObjectWindow::SortColumnArray, ParentData->SortColumns, sizeof(ParentData->SortColumns));
	}

	ObjectWindowImposterManager::CacheOperator::~CacheOperator()
	{
		*TESObjectWindow::WindowHandle = MainWindow;
		*TESObjectWindow::FormListHandle = FormList;
		*TESObjectWindow::TreeViewHandle = TreeView;
		*TESObjectWindow::SplitterHandle = Splitter;

		// copy the current selection and sort columns into the imposter data before resetting them
		ParentData->TreeSelection = *TESObjectWindow::CurrentTreeNode;
		memcpy(ParentData->SortColumns, TESObjectWindow::SortColumnArray, sizeof(ParentData->SortColumns));

		*TESObjectWindow::CurrentTreeNode = TreeSelection;
		memcpy(TESObjectWindow::SortColumnArray, SortColumns, sizeof(SortColumns));
	}

	void ObjectWindowImposterManager::DisposeImposter(HWND Imposter)
	{
		if (ImposterRegistry.count(Imposter))
		{
			ImposterData* Data = ImposterRegistry[Imposter];

			DestroyWindow(Imposter);
			delete Data;
			ImposterRegistry.erase(Imposter);
		}
	}

	ObjectWindowImposterManager::ImposterData* ObjectWindowImposterManager::GetImposterData(HWND Imposter) const
	{
		SME_ASSERT(Imposter);

		if (ImposterRegistry.count(Imposter))
			return ImposterRegistry.at(Imposter);
		else
			return nullptr;
	}

	ObjectWindowImposterManager::ObjectWindowImposterManager() :
		ImposterRegistry()
	{
		;//
	}

	ObjectWindowImposterManager::~ObjectWindowImposterManager()
	{
		DestroyImposters();
	}

	void ObjectWindowImposterManager::SpawnImposter(void)
	{
		if (*TESObjectWindow::Initialized == 0)
			return;

		ImposterData* Data = new ImposterData();
		HINSTANCE ReplacementTemplate = BGSEEUI->GetDialogHotSwapper()->GetAlternateResourceInstance(TESDialog::kDialogTemplate_ObjectWindow);
		SME_ASSERT(ReplacementTemplate);

		HWND Imposter = BGSEEUI->ModelessDialog(ReplacementTemplate,
												(LPSTR)TESDialog::kDialogTemplate_ObjectWindow,
												*TESCSMain::WindowHandle,
												ImposterDlgProc,
												(LPARAM)Data);
		SME_ASSERT(Imposter);

		if (settings::dialogs::kShowMainWindowsInTaskbar.GetData().i)
		{
			bgsee::WindowStyler::StyleData RegularAppWindow = { 0 };
			RegularAppWindow.Extended = WS_EX_APPWINDOW;
			RegularAppWindow.ExtendedOp = bgsee::WindowStyler::StyleData::kOperation_OR;

			BGSEEUI->GetWindowStyler()->StyleWindow(Imposter, RegularAppWindow);
		}

		ImposterRegistry[Imposter] = Data;

		// init needs to be deferred until after the WM_INITDIALOG message has been handled
		SendMessage(Imposter, WM_OBJECTWINDOWIMPOSTER_INITIALIZE, NULL, NULL);
	}

	void ObjectWindowImposterManager::RefreshImposters(void) const
	{
		for (const auto& Itr : ImposterRegistry)
			SendMessage(Itr.first, WM_OBJECTWINDOWIMPOSTER_REFRESHFORMLIST, NULL, NULL);
	}

	void ObjectWindowImposterManager::DestroyImposters(void)
	{
		for (auto& Itr : ImposterRegistry)
		{
			DestroyWindow(Itr.first);
			delete Itr.second;
		}

		ImposterRegistry.clear();
	}

	bool ObjectWindowImposterManager::IsImposter(HWND hWnd) const
	{
		return ImposterRegistry.find(hWnd) != ImposterRegistry.end();
	}

	void ObjectWindowImposterManager::HandleObjectWindowSizing(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
		HWND FilterLabel = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL);
		HWND FormList = GetDlgItem(hWnd, TESObjectWindow::kFormListCtrlID);
		HWND TreeList = GetDlgItem(hWnd, TESObjectWindow::kTreeViewCtrlID);
		HWND Splitter = GetDlgItem(hWnd, TESObjectWindow::kSplitterCtrlID);

		if (wParam != SIZE_MINIMIZED)
		{
			RECT RectFormList = { 0 }, RectTreeView = { 0 }, RectSplitter = { 0 }, RectFilterLbl = { 0 }, RectFilterEdit = { 0 };
			POINT Point = { 0 };

			GetWindowRect(TreeList, &RectTreeView);
			GetWindowRect(Splitter, &RectSplitter);
			GetWindowRect(FormList, &RectFormList);
			GetWindowRect(FilterLabel, &RectFilterLbl);
			GetWindowRect(FilterEditBox, &RectFilterEdit);

			Point.x = RectFilterLbl.left;
			Point.y = RectFilterLbl.bottom + 3;
			ScreenToClient(hWnd, &Point);
			MoveWindow(TreeList, Point.x, Point.y, RectTreeView.right - RectTreeView.left, (unsigned int)(lParam >> 16) - Point.y - 6, 1);

			Point.x = RectFormList.left;
			Point.y = RectFilterEdit.top;
			ScreenToClient(hWnd, &Point);
			MoveWindow(FormList, Point.x, Point.y, (unsigned __int16)lParam - Point.x - 6, (unsigned int)(lParam >> 16) - Point.y - 6, 1);

			Point.x = RectSplitter.left;
			ScreenToClient(hWnd, &Point);
			MoveWindow(Splitter, Point.x, 0, RectSplitter.right - RectSplitter.left, (unsigned int)lParam >> 16, 1);
		}
	}

	void ObjectWindowImposterManager::HandleObjectWindowActivating(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		if (LOWORD(wParam) != WA_INACTIVE)
		{
			HWND Deactivated = (HWND)lParam;
			if (Deactivated && hWnd != Deactivated)
			{
				if (Deactivated == TESObjectWindow::PrimaryObjectWindowHandle || ImposterRegistry.count(Deactivated))
				{
					// redraw the window to propagate potential form list changes when switching from another object window
					BGSEEUI->GetInvalidationManager()->Redraw(hWnd);
				}
			}
		}
	}

	PreviewWindowImposterManager		PreviewWindowImposterManager::Instance;

	INT_PTR CALLBACK PreviewWindowImposterManager::ImposterDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		HWND AnimList = GetDlgItem(hWnd, TESPreviewWindow::kAnimListCtrlID);

		INT_PTR DlgProcResult = FALSE;

		switch (uMsg)
		{
		case WM_SIZE:
			{
				ImposterData* Data = PreviewWindowImposterManager::Instance.GetImposterData(hWnd);
				SME_ASSERT(Data);

				RECT NewBounds = { 0 };
				HWND PreviewCtrl = GetDlgItem(hWnd, TESRenderControl::kPreviewOutputCtrlID);

				GetClientRect(hWnd, &NewBounds);
				int DeltaX = NewBounds.right - Data->Bounds.right;
				int DeltaY = NewBounds.bottom - Data->Bounds.bottom;
				memcpy(&Data->Bounds, &NewBounds, sizeof(NewBounds));

				GetWindowRect(AnimList, &NewBounds);
				SetWindowPos(AnimList, HWND_NOTOPMOST, NewBounds.left, NewBounds.top, NewBounds.right - NewBounds.left, NewBounds.bottom - NewBounds.top + DeltaY, SWP_NOZORDER|SWP_NOMOVE);

				GetWindowRect(PreviewCtrl, &NewBounds);
				SetWindowPos(PreviewCtrl, HWND_NOTOPMOST, NewBounds.left, NewBounds.top, NewBounds.right - NewBounds.left + DeltaX, NewBounds.bottom - NewBounds.top + DeltaY, SWP_NOZORDER|SWP_NOMOVE);

				Data->Renderer->HandleResize();

				DlgProcResult = TRUE;
				break;
			};
		case WM_CLOSE:
			// destroy window
			PreviewWindowImposterManager::Instance.DisposeImposter(hWnd);

			DlgProcResult = TRUE;
			break;
		case WM_INITDIALOG:
			{
				ImposterData* Data = (ImposterData*)lParam;
				SME_ASSERT(Data);

				BaseExtraList* ExtraList = TESDialog::CreateDialogExtraDataList(hWnd);
				SME_ASSERT(ExtraList);

				Data->DialogExtraList = ExtraList;
				Data->InitTickCount = GetTickCount();
				Data->PreviewRef = (TESObjectREFR*)TESForm::CreateInstance(TESForm::kFormType_REFR);
				Data->PreviewRef->MarkAsTemporary();
				Data->PreviewGround = (TESObjectSTAT*)TESForm::CreateInstance(TESForm::kFormType_Static);
				Data->PreviewGround->MarkAsTemporary();

				TESRenderControl::Parameters Params = { 0 };
				Params.previewOutputCtrlID = TESRenderControl::kPreviewOutputCtrlID;
				Params.renderTargetWidth = Params.renderTargetHeight = 1024.f;
				Data->Renderer = TESPreviewControl::CreatePreviewControl(hWnd, &Params);

				SetTimer(hWnd, 1, 5, nullptr);		// preview control refresh timer
				ShowWindow(hWnd, SW_SHOW);

				GetClientRect(hWnd, &Data->Bounds);

				DlgProcResult = TRUE;
				break;
			}
		case WM_PREVIEWWINDOWIMPOSTER_INITIALIZE:
			{
				TESBoundObject* Object = (TESBoundObject*)lParam;

				CacheOperator CacheBackup(hWnd);
				TESPreviewWindow::SetSourceObject(Object);

				char Buffer[0x100] = { 0 };
				FORMAT_STR(Buffer, "Preview Window - '%s' %08X", Object->GetEditorID(), Object->formID);
				std::string WndTitle = Buffer;
				if (settings::general::kShowHallOfFameMembersInTitleBar().i != hallOfFame::kDisplayESMember_None)
				{
					hallOfFame::GetRandomESMember(WndTitle);
					WndTitle += " " + std::string(Buffer);
				}
				SetWindowText(hWnd, WndTitle.c_str());

				int X = INISettingCollection::Instance->LookupByName("iPreviewX:General")->value.i;
				int Y = INISettingCollection::Instance->LookupByName("iPreviewY:General")->value.i;
				int W = INISettingCollection::Instance->LookupByName("iPreviewW:General")->value.i;
				int H = INISettingCollection::Instance->LookupByName("iPreviewH:General")->value.i;
				MoveWindow(hWnd, X, Y, W, H, TRUE);

				SetWindowLongPtr(AnimList, GWL_USERDATA, 1);

				DlgProcResult = TRUE;
				break;
			}
		case WM_DESTROY:
			{
				RECT Bounds = { 0 };
				GetWindowRect(hWnd, &Bounds);

				INISettingCollection::Instance->LookupByName("iPreviewX:General")->value.i = Bounds.left;
				INISettingCollection::Instance->LookupByName("iPreviewY:General")->value.i = Bounds.top;
				INISettingCollection::Instance->LookupByName("iPreviewW:General")->value.i = Bounds.right - Bounds.left;
				INISettingCollection::Instance->LookupByName("iPreviewH:General")->value.i = Bounds.bottom - Bounds.top;

				KillTimer(hWnd, 1);

				DlgProcResult = TRUE;
				break;
			}
		}

		if (DlgProcResult == FALSE && GetWindowLongPtr(AnimList, GWL_USERDATA) != NULL)
		{
			CacheOperator CacheBackup(hWnd);
			DlgProcResult = TESPreviewWindow::DialogProc(hWnd, uMsg, wParam, lParam);
		}

		return DlgProcResult;
	}

	PreviewWindowImposterManager::ImposterData::ImposterData()
	{
		InitTickCount = 0;
		PreviewRef = nullptr;
		PreviewGround = nullptr;
		Renderer = nullptr;
		PreviewSource = nullptr;
		ZeroMemory(&Bounds, sizeof(Bounds));
		DialogExtraList = nullptr;
	}

	PreviewWindowImposterManager::ImposterData::~ImposterData()
	{
		if (PreviewRef)
			PreviewRef->DeleteInstance();

		if (PreviewGround)
			PreviewGround->DeleteInstance();

		DialogExtraList->Dtor();
	}

	PreviewWindowImposterManager::CacheOperator::CacheOperator(HWND Imposter) :
		ParentData(nullptr)
	{
		SME_ASSERT(Imposter);
		ParentData = PreviewWindowImposterManager::Instance.GetImposterData(Imposter);
		SME_ASSERT(ParentData);

		MainWindow = *TESPreviewWindow::WindowHandle;
		AnimationList = *TESPreviewWindow::AnimationListHandle;
		InitTicks = *TESPreviewWindow::InitialTickCount;
		PreviewRef = *TESPreviewWindow::PreviewRef;
		PreviewGround = *TESPreviewWindow::PreviewStatic;
		Renderer = *TESPreviewWindow::PreviewControl;

		*TESPreviewWindow::WindowHandle = Imposter;
		*TESPreviewWindow::AnimationListHandle = GetDlgItem(Imposter, TESPreviewWindow::kAnimListCtrlID);
		*TESPreviewWindow::InitialTickCount = ParentData->InitTickCount;
		*TESPreviewWindow::PreviewRef = ParentData->PreviewRef;
		*TESPreviewWindow::PreviewStatic = ParentData->PreviewGround;
		*TESPreviewWindow::PreviewControl = ParentData->Renderer;
	}

	PreviewWindowImposterManager::CacheOperator::~CacheOperator()
	{
		*TESPreviewWindow::WindowHandle = MainWindow;
		*TESPreviewWindow::AnimationListHandle = AnimationList;
		*TESPreviewWindow::InitialTickCount = InitTicks;
		*TESPreviewWindow::PreviewRef = PreviewRef;
		*TESPreviewWindow::PreviewStatic = PreviewGround;
		*TESPreviewWindow::PreviewControl = Renderer;
	}

	void PreviewWindowImposterManager::DisposeImposter(HWND Imposter)
	{
		if (ImposterRegistry.count(Imposter))
		{
			ImposterData* Data = ImposterRegistry[Imposter];

			DestroyWindow(Imposter);
			delete Data;
			ImposterRegistry.erase(Imposter);
		}
	}

	PreviewWindowImposterManager::ImposterData* PreviewWindowImposterManager::GetImposterData(HWND Imposter) const
	{
		SME_ASSERT(Imposter);

		if (ImposterRegistry.count(Imposter))
			return ImposterRegistry.at(Imposter);
		else
			return nullptr;
	}

	PreviewWindowImposterManager::PreviewWindowImposterManager() :
		ImposterRegistry(),
		Enabled(false)
	{
		;//
	}

	PreviewWindowImposterManager::~PreviewWindowImposterManager()
	{
		DestroyImposters();
	}

	void PreviewWindowImposterManager::SpawnImposter(TESBoundObject* Object)
	{
		if (Enabled && Object)
		{
			ImposterData* Data = new ImposterData();
			HINSTANCE ReplacementTemplate = BGSEEUI->GetDialogHotSwapper()->GetAlternateResourceInstance(TESDialog::kDialogTemplate_PreviewWindow);
			SME_ASSERT(ReplacementTemplate);

			Data->PreviewSource = Object;
			HWND Imposter = BGSEEUI->ModelessDialog(ReplacementTemplate,
													(LPSTR)TESDialog::kDialogTemplate_PreviewWindow,
													*TESCSMain::WindowHandle,
													ImposterDlgProc,
													(LPARAM)Data);
			SME_ASSERT(Imposter);

			if (settings::dialogs::kShowMainWindowsInTaskbar.GetData().i)
			{
				bgsee::WindowStyler::StyleData RegularAppWindow = { 0 };
				RegularAppWindow.Extended = WS_EX_APPWINDOW;
				RegularAppWindow.ExtendedOp = bgsee::WindowStyler::StyleData::kOperation_OR;

				BGSEEUI->GetWindowStyler()->StyleWindow(Imposter, RegularAppWindow);
			}

			ImposterRegistry[Imposter] = Data;
			SendMessage(Imposter, WM_PREVIEWWINDOWIMPOSTER_INITIALIZE, NULL, (LPARAM)Object);
		}
	}

	void PreviewWindowImposterManager::DestroyImposters(void)
	{
		for (auto& Itr : ImposterRegistry)
		{
			DestroyWindow(Itr.first);
			delete Itr.second;
		}

		ImposterRegistry.clear();
	}

	bool PreviewWindowImposterManager::IsImposter(HWND hWnd) const
	{
		return ImposterRegistry.find(hWnd) != ImposterRegistry.end();
	}

	bool PreviewWindowImposterManager::GetEnabled(void) const
	{
		return Enabled;
	}

	void PreviewWindowImposterManager::SetEnabled(bool State)
	{
		Enabled = State;
	}
}