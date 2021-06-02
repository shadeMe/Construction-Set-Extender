#include "ObjectWindowOverrides.h"
#include "MiscWindowOverrides.h"
#include "DialogImposterManager.h"
#include "HallOfFame.h"

namespace cse
{
	namespace uiManager
	{
		void GetRelativeCoords(HWND hWnd, RECT* Rect)
		{
			GetWindowRect(hWnd, Rect);
			MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), reinterpret_cast<LPPOINT>(Rect), 2);
		};

		LRESULT CALLBACK ObjectWindowSplitterSubclassOverrideProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			auto State = reinterpret_cast<TESObjectWindow::SplitterData*>(GetWindowLongPtr(hWnd, GWL_USERDATA));

			if (State == nullptr)
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			else if (!State->enabled)
				return FALSE;

			switch (uMsg)
			{
			case WM_LBUTTONDOWN:
			{
				if (State->mouseOver && !State->performingDrag)
				{
					State->performingDrag = 1;
					GetCursorPos(&State->dragOrigin);
				}

				break;
			}
			case WM_LBUTTONUP:
			{
				if (State->mouseOver && State->performingDrag)
				{
					State->performingDrag = 0;

					POINT CursorPos;
					GetCursorPos(&CursorPos);
					auto WindowUnder = WindowFromPoint(CursorPos);
					if (WindowUnder != hWnd)
						ReleaseCapture();
				}

				break;
			}
			case WM_MOUSEMOVE:
			{
				if (!State->mouseOver)
					break;

				if (!State->performingDrag)
				{
					POINT CursorPos;
					GetCursorPos(&CursorPos);

					auto WindowUnder = WindowFromPoint(CursorPos);
					if (WindowUnder == State->parent)
					{
						ScreenToClient(State->parent, &CursorPos);
						WindowUnder = ChildWindowFromPoint(State->parent, CursorPos);
					}

					if (WindowUnder != hWnd)
						ReleaseCapture();

					break;
				}

				RECT SplitterBounds, TreeBounds, ListViewBounds, FormFilterLabelBounds, FormFilterEditBounds, TagSearchEditBounds;
				RECT TreeBoundsScreen, ListViewBoundsScreen;

				auto FormFilterLabel = GetDlgItem(State->parent, IDC_CSEFILTERABLEFORMLIST_FILTERLBL);
				auto FormFilterEdit = GetDlgItem(State->parent, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
				auto TagSearchEdit = GetDlgItem(State->parent, IDC_OBJECTWINDOW_TAG_SEARCH_EDIT);

				GetRelativeCoords(hWnd, &SplitterBounds);
				GetRelativeCoords(State->treeView, &TreeBounds);
				GetRelativeCoords(State->listView, &ListViewBounds);
				GetRelativeCoords(FormFilterLabel, &FormFilterLabelBounds);
				GetRelativeCoords(FormFilterEdit, &FormFilterEditBounds);
				GetRelativeCoords(TagSearchEdit, &TagSearchEditBounds);

				GetWindowRect(State->treeView, &TreeBoundsScreen);
				GetWindowRect(State->listView, &ListViewBoundsScreen);

				static constexpr auto EdgeSafeAreaThreshold = 125;

				POINT CursorPos;
				GetCursorPos(&CursorPos);

				if (CursorPos.x > ListViewBoundsScreen.right - EdgeSafeAreaThreshold)
					CursorPos.x = ListViewBoundsScreen.right - EdgeSafeAreaThreshold;
				else if (CursorPos.x < TreeBoundsScreen.left + EdgeSafeAreaThreshold)
					CursorPos.x = TreeBoundsScreen.left + EdgeSafeAreaThreshold;

				auto Delta = CursorPos.x - State->dragOrigin.x;
				State->dragOrigin.x = CursorPos.x;
				State->dragOrigin.y = CursorPos.y;

				if (Delta == 0)
					break;

				auto DeferPosData = BeginDeferWindowPos(6);

				if (DeferPosData)
				{
					DeferPosData = DeferWindowPos(DeferPosData, hWnd, HWND_BOTTOM,
						Delta + SplitterBounds.left,
						SplitterBounds.top,
						0,
						0,
						SWP_NOSIZE);
				}

				if (DeferPosData)
				{
					DeferPosData = DeferWindowPos(DeferPosData, State->treeView, HWND_TOP,
						TreeBounds.left,
						TreeBounds.top,
						Delta + TreeBounds.right - TreeBounds.left,
						TreeBounds.bottom - TreeBounds.top,
						0);
				}

				if (DeferPosData)
				{
					DeferPosData = DeferWindowPos(DeferPosData, TagSearchEdit, HWND_TOP,
						TagSearchEditBounds.left,
						TagSearchEditBounds.top,
						Delta + TagSearchEditBounds.right - TagSearchEditBounds.left,
						TagSearchEditBounds.bottom - TagSearchEditBounds.top,
						0);
				}

				if (DeferPosData)
				{
					DeferPosData = DeferWindowPos(DeferPosData, FormFilterLabel, HWND_TOP,
						Delta + FormFilterLabelBounds.left,
						FormFilterLabelBounds.top,
						0,
						0,
						SWP_NOSIZE);
				}

				if (DeferPosData)
				{
					DeferPosData = DeferWindowPos(DeferPosData, FormFilterEdit, HWND_TOP,
						Delta + FormFilterEditBounds.left,
						FormFilterEditBounds.top,
						-Delta + FormFilterEditBounds.right - FormFilterEditBounds.left,
						FormFilterEditBounds.bottom - FormFilterEditBounds.top,
						0);
				}

				if (DeferPosData)
				{
					DeferPosData = DeferWindowPos(DeferPosData, State->listView, HWND_TOP,
						Delta + ListViewBounds.left,
						ListViewBounds.top,
						-Delta + ListViewBounds.right - ListViewBounds.left,
						ListViewBounds.bottom - ListViewBounds.top,
						0);
				}

				if (DeferPosData)
					EndDeferWindowPos(DeferPosData);

				break;
			}
			case WM_NCHITTEST:
			{
				if (State->mouseOver)
					break;

				State->mouseOver = 1;
				SetCapture(hWnd);
				SetCursor(LoadCursor(0, IDC_SIZEWE));

				break;
			}
			case WM_CAPTURECHANGED:
			{
				State->performingDrag = 0;
				State->mouseOver = 0;
				SetCursor(LoadCursor(0, IDC_ARROW));

				break;
			}
			case WM_DESTROY:
			{
				FormHeap_Free(State);
				SetWindowLongPtr(hWnd, GWL_USERDATA, 0);
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
			}

			return CallWindowProc(State->orgWndProc, hWnd, uMsg, wParam, lParam);
		}

		ObjectWindowExtraState::ObjectWindowExtraState(HWND ObjectWindow)
			: WindowExtraData(kTypeID)
		{
			RECT Bounds;
			GetClientRect(ObjectWindow, &Bounds);
			LastDialogDimensions.x = Bounds.right - Bounds.left;
			LastDialogDimensions.y = Bounds.bottom - Bounds.top;
		}

		void ObjectWindowCommonHandlers::OnInit(HWND hWnd, bgsee::WindowExtraDataCollection* ExtraData)
		{
			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			if (xData == nullptr)
			{
				xData = new ObjectWindowExtraState(hWnd);
				ExtraData->Add(xData);
			}

			RECT Bounds = { 0 };
			if (TESDialog::ReadBoundsFromINI("Object Window", &Bounds))
				SetWindowPos(hWnd, HWND_TOP, Bounds.left, Bounds.top, Bounds.right, Bounds.bottom, NULL);

			FilterableFormListManager::Instance.Register(GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT),
				GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL),
				GetDlgItem(hWnd, TESObjectWindow::kFormListCtrlID),
				hWnd);

			std::string WndTitle = "Object Window";
			if (settings::general::kShowHallOfFameMembersInTitleBar().i != hallOfFame::kDisplayESMember_None)
			{
				hallOfFame::GetRandomESMember(WndTitle);
				WndTitle += " Object Window";
			}
			SetWindowText(hWnd, WndTitle.c_str());
		}

		void ObjectWindowCommonHandlers::OnDestroy(HWND hWnd, UINT uMsg, bgsee::WindowExtraDataCollection* ExtraData)
		{
			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			if (xData == nullptr)
				return;

			FilterableFormListManager::Instance.Unregister(GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT));

			// only remove extra data on true destruction
			if (uMsg == WM_DESTROY)
			{
				ExtraData->Remove(ObjectWindowExtraState::kTypeID);
				delete xData;
			}
		}

		void ObjectWindowCommonHandlers::OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData)
		{
			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			if (wParam == SIZE_MINIMIZED)
				return;
			else if (!xData)
				return;

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND FilterLabel = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL);
			HWND FormList = GetDlgItem(hWnd, TESObjectWindow::kFormListCtrlID);
			HWND TreeList = GetDlgItem(hWnd, TESObjectWindow::kTreeViewCtrlID);
			HWND Splitter = GetDlgItem(hWnd, TESObjectWindow::kSplitterCtrlID);
			auto TagSearchEdit = GetDlgItem(hWnd, IDC_OBJECTWINDOW_TAG_SEARCH_EDIT);
			auto TagSearchLabel = GetDlgItem(hWnd, IDC_OBJECTWINDOW_TAG_SEARCH_LABEL);

			RECT RectFormList, RectTreeView, RectSplitter, RectFilterEdit, RectTagSearchEdit, RectTagSearchLabel;

			GetRelativeCoords(TreeList, &RectTreeView);
			GetRelativeCoords(Splitter, &RectSplitter);
			GetRelativeCoords(FormList, &RectFormList);
			GetRelativeCoords(TagSearchEdit, &RectTagSearchEdit);
			GetRelativeCoords(TagSearchLabel, &RectTagSearchLabel);
			GetRelativeCoords(FilterEditBox, &RectFilterEdit);

			auto NewWidth = LOWORD(lParam);
			auto NewHeight = HIWORD(lParam);
			POINT SizeDelta { NewWidth - xData->LastDialogDimensions.x, NewHeight - xData->LastDialogDimensions.y };
			auto DeferPosData = BeginDeferWindowPos(6);

			if (SizeDelta.x == 0 && SizeDelta.y == 0)
				return;

			if (DeferPosData)
			{
				DeferPosData = DeferWindowPos(DeferPosData, TreeList, HWND_TOP,
					0,
					0,
					RectTreeView.right - RectTreeView.left,
					SizeDelta.y + RectTreeView.bottom - RectTreeView.top,
					SWP_NOMOVE);
			}

			if (DeferPosData)
			{
				DeferPosData = DeferWindowPos(DeferPosData, Splitter, HWND_TOP,
					0,
					0,
					RectSplitter.right - RectSplitter.left,
					SizeDelta.y + RectSplitter.bottom - RectSplitter.top,
					SWP_NOMOVE);
			}

			if (DeferPosData)
			{
				DeferPosData = DeferWindowPos(DeferPosData, TagSearchLabel, HWND_TOP,
					RectTagSearchLabel.left,
					SizeDelta.y + RectTagSearchLabel.top,
					0,
					0,
					SWP_NOSIZE);
			}

			if (DeferPosData)
			{
				DeferPosData = DeferWindowPos(DeferPosData, TagSearchEdit, HWND_TOP,
					RectTagSearchEdit.left,
					SizeDelta.y + RectTagSearchEdit.top,
					0,
					0,
					SWP_NOSIZE);
			}

			if (DeferPosData)
			{
				DeferPosData = DeferWindowPos(DeferPosData, FilterEditBox, HWND_TOP,
					0,
					0,
					SizeDelta.x + RectFilterEdit.right - RectFilterEdit.left,
					RectFilterEdit.bottom - RectFilterEdit.top,
					SWP_NOMOVE);
			}

			if (DeferPosData)
			{
				DeferPosData = DeferWindowPos(DeferPosData, FormList, HWND_TOP,
					0,
					0,
					SizeDelta.x + RectFormList.right - RectFormList.left,
					SizeDelta.y + RectFormList.bottom - RectFormList.top,
					SWP_NOMOVE);
			}

			if (DeferPosData)
				EndDeferWindowPos(DeferPosData);

			xData->LastDialogDimensions.x = NewWidth;
			xData->LastDialogDimensions.y = NewHeight;
		}

		void ObjectWindowCommonHandlers::OnWindowPosChanging(HWND hWnd, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData)
		{
			static constexpr auto kMinWidth = 400;
			static constexpr auto kMinHeight = 400;

			auto WindowPosParams = reinterpret_cast<WINDOWPOS*>(lParam);
			if (WindowPosParams->cx < kMinWidth)
				WindowPosParams->cx = kMinWidth;

			if (WindowPosParams->cy < kMinHeight)
				WindowPosParams->cy = kMinHeight;
		}

		void ObjectWindowCommonHandlers::OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam)
		{
			if (LOWORD(wParam) == WA_INACTIVE)
				return;

			HWND Deactivated = reinterpret_cast<HWND>(lParam);
			SME_ASSERT(Deactivated != hWnd);

			if (Deactivated == TESObjectWindow::PrimaryObjectWindowHandle)
			{
				// redraw the window to propagate potential form list changes when switching from another object window
				BGSEEUI->GetInvalidationManager()->Redraw(hWnd);
			}
		}

		void ObjectWindowCommonHandlers::HandleFormListFilter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData)
		{
			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			if (xData == nullptr)
				return;

			if (FilterableFormListManager::Instance.HandleMessages(GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT),
																   uMsg, wParam, lParam))
			{
				// ### TODO Refresh form list
			}
		}

		LRESULT CALLBACK ObjectWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												  bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT DlgProcResult = FALSE;

			if (ObjectWindowImposterManager::Instance.IsImposter(hWnd))
			{
				// the imposter needs to handle its own messages exclusively
				return DlgProcResult;
			}

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND FormList = GetDlgItem(hWnd, TESObjectWindow::kFormListCtrlID);
			HWND TreeList = GetDlgItem(hWnd, TESObjectWindow::kTreeViewCtrlID);

			switch (uMsg)
			{
			case TESObjectWindow::kWindowMessage_Reload:
				SubclassParams->Out.MarkMessageAsHandled = true;

				// the window subclasser's windows hook has an out-sized performance penalty for
				// insertions in tree view controls (probably also has to do with the way it's used in this dialog)
				// so we have to temporarily suspend the hook when this code is executing
				BGSEEUI->GetInvalidationManager()->Push(TreeList);
				SubclassParams->In.Subclasser->SuspendHooks();
				{
					DlgProcResult = SubclassParams->In.Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);
				}
				SubclassParams->In.Subclasser->ResumeHooks();
				BGSEEUI->GetInvalidationManager()->Pop(TreeList);
				BGSEEUI->GetInvalidationManager()->Redraw(TreeList);

				break;
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

					SubclassParams->Out.MarkMessageAsHandled = true;
					ObjectWindowImposterManager::Instance.RefreshImposters();
					SendMessage(hWnd, TESDialog::kWindowMessage_Refresh, NULL, NULL);
				}
			case WM_ACTIVATE:
				ObjectWindowCommonHandlers::OnActivate(hWnd, wParam, lParam);
				SubclassParams->Out.MarkMessageAsHandled = true;

				break;
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_ObjectWindow, NULL);
				SubclassParams->Out.MarkMessageAsHandled = true;

				break;
			case TESDialog::kWindowMessage_Destroy:
			case WM_DESTROY:
				ObjectWindowCommonHandlers::OnDestroy(hWnd, uMsg, SubclassParams->In.ExtraData);

				ObjectWindowImposterManager::Instance.DestroyImposters();
				TESObjectWindow::PrimaryObjectWindowHandle = nullptr;

				break;
			case WM_INITDIALOG:
				ObjectWindowCommonHandlers::OnInit(hWnd, SubclassParams->In.ExtraData);
				TESObjectWindow::PrimaryObjectWindowHandle = hWnd;

				break;
			case WM_WINDOWPOSCHANGING:
				ObjectWindowCommonHandlers::OnWindowPosChanging(hWnd, wParam, lParam, SubclassParams->In.ExtraData);

				break;
			case WM_SIZE:
				ObjectWindowCommonHandlers::OnSize(hWnd, wParam, lParam, SubclassParams->In.ExtraData);
				SubclassParams->Out.MarkMessageAsHandled = true;

				break;
			}

			if (!SubclassParams->Out.MarkMessageAsHandled)
				ObjectWindowCommonHandlers::HandleFormListFilter(hWnd, uMsg, wParam, lParam, SubclassParams->In.ExtraData);

			return DlgProcResult;
		}

		void InitializeObjectWindowOverrides()
		{
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_ObjectWindow,
				uiManager::ObjectWindowSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_ObjectWindow,
				uiManager::CommonDialogExtraFittingsSubClassProc);

			if (settings::dialogs::kShowMainWindowsInTaskbar.GetData().i)
			{
				bgsee::WindowStyler::StyleData RegularAppWindow = { 0 };
				RegularAppWindow.Extended = WS_EX_APPWINDOW;
				RegularAppWindow.ExtendedOp = bgsee::WindowStyler::StyleData::kOperation_OR;

				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_ObjectWindow, RegularAppWindow);
			}
		}
	}
}