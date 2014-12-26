#include "CSEObjectWindowManager.h"
#include "CSEUIManager.h"
#include "CSEHallOfFame.h"

namespace ConstructionSetExtender
{
	ObjectWindowManager					ObjectWindowManager::Instance;

	INT_PTR CALLBACK ObjectWindowManager::ObjectWindowImposterDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

				TreeView_SelectItem(TreeList, NULL);
				TreeView_SelectItem(TreeList, Selection);

				DlgProcResult = TRUE;
				break;
			}
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
			}
		case 0x412:
			TODO("force a full refresh on new object creation/ ObjectWindow::ProcessForm");
			// full refreshes are handled by the primary window exclusively
			// but we can't forward the message, as we can potentially be in a nested call inside the imposter dlg proc, i.e., the caches may be dirty
			// so we need to defer the refresh until the outermost call returns
			SME_ASSERT(TESObjectWindow::PrimaryObjectWindowHandle);

			SendMessage(TESObjectWindow::PrimaryObjectWindowHandle, WM_OBJECTWINDOWIMPOSTER_FULLREFRESH, NULL, NULL);
			DlgProcResult = TRUE;
			break;
		case WM_ACTIVATE:
			ObjectWindowManager::Instance.HandleObjectWindowActivating(hWnd, uMsg, wParam, lParam);

			DlgProcResult = TRUE;
			break;
		case WM_CLOSE:
			// destroy window
			ObjectWindowManager::Instance.DisposeImposter(hWnd);

			DlgProcResult = TRUE;
			break;
		case WM_DESTROY:
			{
				UIManager::CSEFilterableFormListManager::Instance.Unregister(hWnd);

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
				UIManager::CSEFilterableFormListManager::Instance.Register(hWnd, FilterEditBox, FormList);

				// reproduce the relevant bits of the org wnd proc's code
				CacheOperator CacheBackup(hWnd);
				std::string WndTitle;

				TESObjectWindow::PerformLimitedInit(hWnd);
				SendMessage(hWnd, WM_OBJECTWINDOWIMPOSTER_REFRESHTREEVIEW, NULL, NULL);

				HallOfFame::GetRandomESMember(WndTitle);
				WndTitle += " Object Window";
				SetWindowText(hWnd, WndTitle.c_str());

				BGSEEUI->GetSubclasser()->RegisterRegularWindowSubclass(hWnd, UIManager::CommonDialogExtraFittingsSubClassProc);
				SendMessage(hWnd, WM_OBJECTWINDOWIMPOSTER_INITIALIZEXTRA, NULL, NULL);
				SetWindowLongPtr(FormList, GWL_USERDATA, 1);

				DlgProcResult = TRUE;
				break;
			}
		case WM_SIZE:
			ObjectWindowManager::Instance.HandleObjectWindowSizing(hWnd, uMsg, wParam, lParam);

			DlgProcResult = TRUE;
			break;
		default:
			if (UIManager::CSEFilterableFormListManager::Instance.HandleMessages(hWnd, uMsg, wParam, lParam))
			{
				SendMessage(hWnd, WM_OBJECTWINDOWIMPOSTER_REFRESHFORMLIST, NULL, NULL);
				DlgProcResult = TRUE;
			}

			break;
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

	ObjectWindowManager::CacheOperator::CacheOperator(HWND Imposter) :
		ParentData(NULL)
	{
		SME_ASSERT(Imposter);
		ParentData = ObjectWindowManager::Instance.GetImposterData(Imposter);
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

	ObjectWindowManager::CacheOperator::~CacheOperator()
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

	void ObjectWindowManager::DisposeImposter(HWND Imposter)
	{
		if (ImposterRegistry.count(Imposter))
		{
			ImposterData* Data = ImposterRegistry[Imposter];

			DestroyWindow(Imposter);
			delete Data;
			ImposterRegistry.erase(Imposter);
		}
	}

	ObjectWindowManager::ImposterData* ObjectWindowManager::GetImposterData(HWND Imposter) const
	{
		SME_ASSERT(Imposter);

		if (ImposterRegistry.count(Imposter))
			return ImposterRegistry.at(Imposter);
		else
			return NULL;
	}

	ObjectWindowManager::ObjectWindowManager() :
		ImposterRegistry()
	{
		;//
	}

	ObjectWindowManager::~ObjectWindowManager()
	{
		DestroyImposters();
	}

	void ObjectWindowManager::SpawnImposter(void)
	{
		if (*TESObjectWindow::Initialized == 0)
			return;

		ImposterData* Data = new ImposterData();
		HINSTANCE ReplacementTemplate = BGSEEUI->GetDialogHotSwapper()->GetAlternateResourceInstance(TESDialog::kDialogTemplate_ObjectWindow);
		SME_ASSERT(ReplacementTemplate);

		HWND Imposter = BGSEEUI->ModelessDialog(ReplacementTemplate,
												(LPSTR)TESDialog::kDialogTemplate_ObjectWindow,
												*TESCSMain::WindowHandle,
												ObjectWindowImposterDlgProc,
												(LPARAM)Data);
		SME_ASSERT(Imposter);

		if (Settings::Dialogs::kShowMainWindowsInTaskbar.GetData().i)
		{
			BGSEditorExtender::BGSEEWindowStyler::StyleData RegularAppWindow = { 0 };
			RegularAppWindow.Extended = WS_EX_APPWINDOW;
			RegularAppWindow.ExtendedOp = BGSEditorExtender::BGSEEWindowStyler::StyleData::kOperation_OR;

			BGSEEUI->GetWindowStyler()->StyleWindow(Imposter, RegularAppWindow);
		}

		ImposterRegistry[Imposter] = Data;

		// init needs to be deferred until after the WM_INITDIALOG message has been handled
		SendMessage(Imposter, WM_OBJECTWINDOWIMPOSTER_INITIALIZE, NULL, NULL);
	}

	void ObjectWindowManager::RefreshImposters(void) const
	{
		for each (auto Itr  in ImposterRegistry)
		{
			SendMessage(Itr.first, WM_OBJECTWINDOWIMPOSTER_REFRESHTREEVIEW, NULL, NULL);
			SendMessage(Itr.first, WM_OBJECTWINDOWIMPOSTER_REFRESHFORMLIST, NULL, NULL);
		}
	}

	void ObjectWindowManager::DestroyImposters(void)
	{
		for each (auto Itr  in ImposterRegistry)
		{
			DestroyWindow(Itr.first);
			delete Itr.second;
		}

		ImposterRegistry.clear();
	}

	void ObjectWindowManager::HandleObjectWindowSizing(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
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

	void ObjectWindowManager::HandleObjectWindowActivating(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
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
}