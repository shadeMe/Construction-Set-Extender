#include "ObjectWindowOverrides.h"
#include "DialogImposterManager.h"
#include "HallOfFame.h"

namespace cse
{
	namespace uiManager
	{
		LRESULT CALLBACK ObjectWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												  bool& Return, bgsee::WindowExtraDataCollection* ExtraData)
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
					SendMessage(hWnd, TESDialog::kWindowMessage_Refresh, NULL, NULL);
				}
			case WM_ACTIVATE:
				ObjectWindowImposterManager::Instance.HandleObjectWindowActivating(hWnd, uMsg, wParam, lParam);
				Return = true;

				break;
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_ObjectWindow, NULL);
				Return = true;

				break;
			case TESDialog::kWindowMessage_Destroy:
			case WM_DESTROY:
				{
					FilterableFormListManager::Instance.Unregister(FilterEditBox);
					ObjectWindowImposterManager::Instance.DestroyImposters();
					TESObjectWindow::PrimaryObjectWindowHandle = NULL;
				}

				break;
			case WM_INITDIALOG:
				{
					SME_ASSERT(FilterEditBox);
					FilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL), FormList, hWnd);

					std::string WndTitle = "Object Window";
					if (settings::general::kShowHallOfFameMembersInTitleBar().i != hallOfFame::kDisplayESMember_None)
					{
						hallOfFame::GetRandomESMember(WndTitle);
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

			if (Return == false && FilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
			{
				HTREEITEM Selection = TreeView_GetSelection(TreeList);

				TreeView_SelectItem(TreeList, NULL);
				TreeView_SelectItem(TreeList, Selection);
			}

			return DlgProcResult;
		}

		void InitializeObjectWindowOverrides()
		{
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ObjectWindow, uiManager::ObjectWindowSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ObjectWindow, uiManager::CommonDialogExtraFittingsSubClassProc);

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