#include "ObjectWindowOverrides.h"
#include "MiscWindowOverrides.h"
#include "DialogImposterManager.h"
#include "HallOfFame.h"

namespace cse
{
	namespace uiManager
	{
		void ObjectWindowExtraState::SplitterState::UpdateCursor() const
		{
			auto NewCursor = LoadCursor(NULL, IDC_ARROW);
			if (MouseOver)
			{
				if (ID == kVerticalSplitter)
					NewCursor = LoadCursor(NULL, IDC_SIZENS);
				else
					NewCursor = LoadCursor(NULL, IDC_SIZEWE);
			}

			SetCursor(NewCursor);
		}

		LRESULT ObjectWindowExtraState::SplitterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT ProcResult = FALSE;
			auto SplitterData = reinterpret_cast<SplitterState*>(GetWindowLongPtr(hWnd, GWL_USERDATA));
			if (SplitterData == nullptr)
				return ProcResult;
			else if (!SplitterData->Enabled)
				return ProcResult;

			switch (uMsg)
			{
			case WM_NCHITTEST:
			{
				if (SplitterData->MouseOver)
					break;

				Return = true;
				ProcResult = TRUE;

				SplitterData->MouseOver = true;
				SplitterData->UpdateCursor();
				SetCapture(hWnd);
				SetActiveSplitter(SplitterData->ID, true);

				break;
			}
			case WM_LBUTTONDOWN:
			{
				if (!SplitterData->MouseOver)
					break;
				else if (SplitterData->Dragging)
					break;

				Return = true;
				ProcResult = TRUE;

				SplitterData->Dragging = true;
				GetCursorPos(&SplitterData->DragOrigin);

				break;
			}
			case WM_LBUTTONUP:
			{
				if (!SplitterData->MouseOver)
					break;
				else if (!SplitterData->Dragging)
					break;

				Return = true;
				ProcResult = TRUE;

				if (GetCapture() != hWnd)
				{
					// something went wrong somewhere, reset state and continue
					SplitterData->Dragging = false;
					SplitterData->MouseOver = false;
					SplitterData->UpdateCursor();
					SetActiveSplitter(SplitterData->ID, false);

					break;
				}

				SplitterData->Dragging = false;
				SplitterData->MouseOver = IsMouseOverSplitter(hWnd);
				SplitterData->UpdateCursor();
				if (!SplitterData->MouseOver)
				{
					ReleaseCapture();
					SetActiveSplitter(SplitterData->ID, false);
				}

				break;
			}
			case WM_MOUSEMOVE:
			{
				Return = true;
				ProcResult = TRUE;

				if (!SplitterData->Dragging)
				{
					SplitterData->MouseOver = IsMouseOverSplitter(hWnd);
					SplitterData->UpdateCursor();
					if (!SplitterData->MouseOver)
					{
						ReleaseCapture();
						SetActiveSplitter(SplitterData->ID, false);
					}

					break;
				}

				POINT CurrentCursorPos;
				GetCursorPos(&CurrentCursorPos);

				POINT Delta { CurrentCursorPos.x - SplitterData->DragOrigin.x, CurrentCursorPos.y - SplitterData->DragOrigin.y };
				SplitterData->DragOrigin.x = CurrentCursorPos.x;
				SplitterData->DragOrigin.y = CurrentCursorPos.y;

				HWND CurrentHandle = NULL;
				RECT* CurrentRect = nullptr;

				switch (SplitterData->ID)
				{
				case kVerticalSplitter:
				{
					auto NewSplitterYPos = CurrentRects.at(ObjectWindowExtraState::kVerticalSplitter).top + Delta.y;

					if (NewSplitterYPos <= kMinVerticalSplitterHeight || NewSplitterYPos >= (CurrentDialogSize.y - kMinVerticalSplitterHeight))
						break;

					// form tree view - new height
					CurrentHandle = Handles.at(ObjectWindowExtraState::kForm_TreeView);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kForm_TreeView);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top + Delta.y,
							TRUE);

					// form splitter - new height
					CurrentHandle = Handles.at(ObjectWindowExtraState::kForm_Splitter);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kForm_Splitter);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top + Delta.y,
							TRUE);

					// form list view - new height
					CurrentHandle = Handles.at(ObjectWindowExtraState::kForm_ListView);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kForm_ListView);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top + Delta.y,
							TRUE);

					// vertical splitter - new y pos
					CurrentHandle = Handles.at(ObjectWindowExtraState::kVerticalSplitter);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kVerticalSplitter);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top + Delta.y,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top,
							TRUE);

					// tag filter label - new y pos
					CurrentHandle = Handles.at(ObjectWindowExtraState::kTag_FilterLabel);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kTag_FilterLabel);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top + Delta.y,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top,
							TRUE);

					// tag filter edit - new y pos
					CurrentHandle = Handles.at(ObjectWindowExtraState::kTag_FilterEdit);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kTag_FilterEdit);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top + Delta.y,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top,
							TRUE);

					// tag tree view - new y pos, new height
					CurrentHandle = Handles.at(ObjectWindowExtraState::kTag_TreeView);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kTag_TreeView);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top + Delta.y,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top + (-Delta.y),
							TRUE);

					// tag splitter - new y pos, new height
					CurrentHandle = Handles.at(ObjectWindowExtraState::kTag_Splitter);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kTag_Splitter);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top + Delta.y,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top + (-Delta.y),
							TRUE);

					// tag list view - new y pos, new height
					CurrentHandle = Handles.at(ObjectWindowExtraState::kTag_ListView);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kTag_ListView);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top + Delta.y,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top + (-Delta.y),
							TRUE);

					break;
				}
				case kForm_Splitter:
				{
					auto NewSplitterXPos = CurrentRects.at(ObjectWindowExtraState::kForm_Splitter).left + Delta.x;

					if (NewSplitterXPos <= kMinHorizontalSplitterWidth || NewSplitterXPos >= (CurrentDialogSize.x - kMinHorizontalSplitterWidth ))
						break;

					// form filter edit - new width
					CurrentHandle = Handles.at(ObjectWindowExtraState::kForm_FilterEdit);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kForm_FilterEdit);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top,
							CurrentRect->right - CurrentRect->left + Delta.x,
							CurrentRect->bottom - CurrentRect->top,
							TRUE);

					// form tree view - new width
					CurrentHandle = Handles.at(ObjectWindowExtraState::kForm_TreeView);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kForm_TreeView);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top,
							CurrentRect->right - CurrentRect->left + Delta.x,
							CurrentRect->bottom - CurrentRect->top,
							TRUE);

					// form splitter - new x pos
					CurrentHandle = Handles.at(ObjectWindowExtraState::kForm_Splitter);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kForm_Splitter);

					MoveWindow(CurrentHandle,
							CurrentRect->left + Delta.x,
							CurrentRect->top,
							CurrentRect->right - CurrentRect->left,
							CurrentRect->bottom - CurrentRect->top,
							TRUE);

					// form list view - new x pos, new width
					CurrentHandle = Handles.at(ObjectWindowExtraState::kForm_ListView);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kForm_ListView);

					MoveWindow(CurrentHandle,
							CurrentRect->left + Delta.x,
							CurrentRect->top,
							CurrentRect->right - CurrentRect->left + (-Delta.x),
							CurrentRect->bottom - CurrentRect->top,
							TRUE);

					break;
				}
				case kTag_Splitter:
				{
					auto NewSplitterXPos = CurrentRects.at(ObjectWindowExtraState::kTag_Splitter).left + Delta.x;

					if (NewSplitterXPos <= kMinHorizontalSplitterWidth || NewSplitterXPos >= (CurrentDialogSize.x - kMinHorizontalSplitterWidth ))
						break;


					// tag filter edit - new width
					CurrentHandle = Handles.at(ObjectWindowExtraState::kTag_FilterEdit);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kTag_FilterEdit);

					MoveWindow(CurrentHandle,
							CurrentRect->left,
							CurrentRect->top,
							CurrentRect->right - CurrentRect->left + Delta.x,
							CurrentRect->bottom - CurrentRect->top,
							TRUE);

					// tag tree view - new width
					CurrentHandle = Handles.at(ObjectWindowExtraState::kTag_TreeView);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kTag_TreeView);

					MoveWindow(CurrentHandle,
						CurrentRect->left,
						CurrentRect->top,
						CurrentRect->right - CurrentRect->left + Delta.x,
						CurrentRect->bottom - CurrentRect->top,
						TRUE);

					// tag splitter - new x pos
					CurrentHandle = Handles.at(ObjectWindowExtraState::kTag_Splitter);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kTag_Splitter);

					MoveWindow(CurrentHandle,
						CurrentRect->left + Delta.x,
						CurrentRect->top,
						CurrentRect->right - CurrentRect->left,
						CurrentRect->bottom - CurrentRect->top,
						TRUE);

					// tag list view - new x pos, new width
					CurrentHandle = Handles.at(ObjectWindowExtraState::kTag_ListView);
					CurrentRect = &CurrentRects.at(ObjectWindowExtraState::kTag_ListView);

					MoveWindow(CurrentHandle,
						CurrentRect->left + Delta.x,
						CurrentRect->top,
						CurrentRect->right - CurrentRect->left + (-Delta.x),
						CurrentRect->bottom - CurrentRect->top,
						TRUE);

					break;
				}
				}

				UpdateRects();

				break;
			}
			}

			return ProcResult;
		}

		bool ObjectWindowExtraState::IsMouseOverSplitter(HWND hWnd) const
		{
			POINT CursorLoc;
			GetCursorPos(&CursorLoc);

			auto WindowUnderCursor = WindowFromPoint(CursorLoc);
			if (WindowUnderCursor == hWnd)
				return true;

			if (WindowUnderCursor == Parent)
			{
				ScreenToClient(Parent, &CursorLoc);
				WindowUnderCursor = ChildWindowFromPoint(Parent, CursorLoc);
				return WindowUnderCursor == hWnd;
			}

			return false;
		}

		void ObjectWindowExtraState::SetActiveSplitter(ControlID Splitter, bool ActiveState)
		{
			switch (Splitter)
			{
			case kForm_Splitter:
				TagViewSplitterState.Enabled = ActiveState == false;
				VerticalSplitterState.Enabled = ActiveState == false;
				break;
			case kTag_Splitter:
				FormViewSplitterState.Enabled = ActiveState == false;
				VerticalSplitterState.Enabled = ActiveState == false;
				break;
			case kVerticalSplitter:
				FormViewSplitterState.Enabled = ActiveState == false;
				TagViewSplitterState.Enabled = ActiveState == false;
				break;
			default:
				SME_ASSERT(!"Invalid splitter control ID");
			}
		}

		ObjectWindowExtraState::ObjectWindowExtraState(HWND ObjectWindow)
			: bgsee::WindowExtraData(kTypeID), Parent(ObjectWindow), ThunkSplitterSubclassProc(this, &ObjectWindowExtraState::SplitterSubclassProc)
		{
			Handles.emplace(kForm_FilterLabel, GetDlgItem(Parent, kForm_FilterLabel));
			Handles.emplace(kForm_FilterEdit, GetDlgItem(Parent, kForm_FilterEdit));
			Handles.emplace(kForm_ListView, GetDlgItem(Parent, kForm_ListView));
			Handles.emplace(kForm_TreeView, GetDlgItem(Parent, kForm_TreeView));
			Handles.emplace(kForm_Splitter, GetDlgItem(Parent, kForm_Splitter));
			Handles.emplace(kTag_FilterLabel, GetDlgItem(Parent, kTag_FilterLabel));
			Handles.emplace(kTag_FilterEdit, GetDlgItem(Parent, kTag_FilterEdit));
			Handles.emplace(kTag_ListView, GetDlgItem(Parent, kTag_ListView));
			Handles.emplace(kTag_TreeView, GetDlgItem(Parent, kTag_TreeView));
			Handles.emplace(kTag_Splitter, GetDlgItem(Parent, kTag_Splitter));
			Handles.emplace(kVerticalSplitter, GetDlgItem(Parent, kVerticalSplitter));

			for (const auto& Itr : Handles)
				SME_ASSERT(Itr.second != NULL && "Invalid handle for object window control");

			UpdateRects();
			UpdateDialogSize();

			FormViewSplitterState.ID = kForm_Splitter;
			TagViewSplitterState.ID = kTag_Splitter;
			VerticalSplitterState.ID = kVerticalSplitter;

			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(Handles.at(kForm_Splitter), ThunkSplitterSubclassProc());
			SetWindowLongPtr(Handles.at(kForm_Splitter), GWL_USERDATA, reinterpret_cast<LONG_PTR>(&FormViewSplitterState));

			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(Handles.at(kTag_Splitter), ThunkSplitterSubclassProc());
			SetWindowLongPtr(Handles.at(kTag_Splitter), GWL_USERDATA, reinterpret_cast<LONG_PTR>(&TagViewSplitterState));

			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(Handles.at(kVerticalSplitter), ThunkSplitterSubclassProc());
			SetWindowLongPtr(Handles.at(kVerticalSplitter), GWL_USERDATA, reinterpret_cast<LONG_PTR>(&VerticalSplitterState));
		}

		ObjectWindowExtraState::~ObjectWindowExtraState()
		{
			BGSEEUI->GetSubclasser()->DeregisterSubclassForWindow(Handles.at(kForm_Splitter), ThunkSplitterSubclassProc());
			SetWindowLongPtr(Handles.at(kForm_Splitter), GWL_USERDATA, NULL);

			BGSEEUI->GetSubclasser()->DeregisterSubclassForWindow(Handles.at(kTag_Splitter), ThunkSplitterSubclassProc());
			SetWindowLongPtr(Handles.at(kTag_Splitter), GWL_USERDATA, NULL);

			BGSEEUI->GetSubclasser()->DeregisterSubclassForWindow(Handles.at(kVerticalSplitter), ThunkSplitterSubclassProc());
			SetWindowLongPtr(Handles.at(kVerticalSplitter), GWL_USERDATA, NULL);
		}

		void ObjectWindowExtraState::UpdateRects()
		{
			auto GetRelativeCoords = [](HWND hWnd, RECT* Rect) -> void {
				GetWindowRect(hWnd, Rect);
				MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), reinterpret_cast<LPPOINT>(Rect), 2);
			};

			GetRelativeCoords(Handles.at(kForm_FilterLabel), &CurrentRects[kForm_FilterLabel]);
			GetRelativeCoords(Handles.at(kForm_FilterEdit), &CurrentRects[kForm_FilterEdit]);
			GetRelativeCoords(Handles.at(kForm_ListView), &CurrentRects[kForm_ListView]);
			GetRelativeCoords(Handles.at(kForm_TreeView), &CurrentRects[kForm_TreeView]);
			GetRelativeCoords(Handles.at(kForm_Splitter), &CurrentRects[kForm_Splitter]);
			GetRelativeCoords(Handles.at(kTag_FilterLabel), &CurrentRects[kTag_FilterLabel]);
			GetRelativeCoords(Handles.at(kTag_FilterEdit), &CurrentRects[kTag_FilterEdit]);
			GetRelativeCoords(Handles.at(kTag_ListView), &CurrentRects[kTag_ListView]);
			GetRelativeCoords(Handles.at(kTag_TreeView), &CurrentRects[kTag_TreeView]);
			GetRelativeCoords(Handles.at(kTag_Splitter), &CurrentRects[kTag_Splitter]);
			GetRelativeCoords(Handles.at(kVerticalSplitter), &CurrentRects[kVerticalSplitter]);
		}

		void ObjectWindowExtraState::UpdateDialogSize(POINT* Delta /*= nullptr*/)
		{
			RECT ClientRect;
			GetClientRect(Parent, &ClientRect);

			if (Delta)
			{
				Delta->x = ClientRect.right - CurrentDialogSize.x;
				Delta->y = ClientRect.bottom - CurrentDialogSize.y;
			}

			CurrentDialogSize.x = ClientRect.right;
			CurrentDialogSize.y = ClientRect.bottom;
		}


		RECT ObjectWindowExtraState::GetRelativeBounds(ControlID Control) const
		{
			RECT Rect;

			auto Handle = Handles.at(Control);
			GetWindowRect(Handle, &Rect);
			MapWindowPoints(HWND_DESKTOP, Parent, reinterpret_cast<LPPOINT>(&Rect), 2);

			return Rect;
		}

		bool ObjectWindowExtraState::OnWindowPosChanging(WINDOWPOS* PosParams) const
		{
			static constexpr auto kMinWidth = 400;
			static constexpr auto kMinHeight = 400;

			if (PosParams->cx < kMinWidth)
				PosParams->cx = kMinWidth;

			if (PosParams->cy < kMinHeight)
				PosParams->cy = kMinHeight;


			// ### HACK! doesn't work consistently in both axes
			POINT Delta = { PosParams->cx - CurrentDialogSize.x, PosParams->cy - CurrentDialogSize.y };
			auto NewSplitterYPos = CurrentRects.at(ObjectWindowExtraState::kVerticalSplitter).top + Delta.y;

			if (NewSplitterYPos <= kMinVerticalSplitterHeight || NewSplitterYPos >= (PosParams->cy - kMinVerticalSplitterHeight))
				return false;

			//auto NewSplitterXPos = CurrentRects.at(ObjectWindowExtraState::kForm_Splitter).left + Delta.x;
			//if (NewSplitterXPos <= kMinHorizontalSplitterWidth || NewSplitterXPos >= (PosParams->cx - kMinHorizontalSplitterWidth))
			//	return false;

			//NewSplitterXPos = CurrentRects.at(ObjectWindowExtraState::kTag_Splitter).left + Delta.x;
			//if (NewSplitterXPos <= kMinHorizontalSplitterWidth || NewSplitterXPos >= (PosParams->cx - kMinHorizontalSplitterWidth))
			//	return false;

			return true;
		}

		void ObjectWindowStateManager::OnInit(HWND hWnd, bgsee::WindowExtraDataCollection* ExtraData)
		{
			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			SME_ASSERT(xData == nullptr);

			xData = new ObjectWindowExtraState(hWnd);
			ExtraData->Add(xData);

			RECT Bounds = { 0 };
			if (TESDialog::ReadBoundsFromINI("Object Window", &Bounds))
				SetWindowPos(hWnd, HWND_TOP, Bounds.left, Bounds.top, Bounds.right - Bounds.left, Bounds.bottom - Bounds.top, NULL);

			FilterableFormListManager::Instance.Register(xData->Handles.at(ObjectWindowExtraState::kForm_FilterEdit),
														xData->Handles.at(ObjectWindowExtraState::kForm_FilterLabel),
														xData->Handles.at(ObjectWindowExtraState::kForm_ListView),
														hWnd);

			FilterableFormListManager::Instance.Register(xData->Handles.at(ObjectWindowExtraState::kTag_FilterEdit),
														xData->Handles.at(ObjectWindowExtraState::kTag_FilterLabel),
														xData->Handles.at(ObjectWindowExtraState::kTag_ListView),
														hWnd);

			std::string WndTitle = "Object Window";
			if (settings::general::kShowHallOfFameMembersInTitleBar().i != hallOfFame::kDisplayESMember_None)
			{
				hallOfFame::GetRandomESMember(WndTitle);
				WndTitle += " Object Window";
			}
			SetWindowText(hWnd, WndTitle.c_str());
		}

		void ObjectWindowStateManager::OnDestroy(HWND hWnd, bgsee::WindowExtraDataCollection* ExtraData)
		{
			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			if (xData == nullptr)
				return;

			FilterableFormListManager::Instance.Unregister(xData->Handles.at(ObjectWindowExtraState::kForm_FilterEdit));
			FilterableFormListManager::Instance.Unregister(xData->Handles.at(ObjectWindowExtraState::kTag_FilterEdit));

			ExtraData->Remove(ObjectWindowExtraState::kTypeID);
			delete xData;
		}

		void ObjectWindowStateManager::OnWindowPosChanging(HWND hWnd, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData)
		{
			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			if (xData == nullptr)
				return;

			auto WindowPosParams = reinterpret_cast<WINDOWPOS*>(lParam);
			if (!xData->OnWindowPosChanging(WindowPosParams))
				WindowPosParams->flags |= SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOMOVE;
		}

		void ObjectWindowStateManager::OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData)
		{
			if (wParam == SIZE_MINIMIZED)
				return;

			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			if (xData == nullptr)
				return;

			POINT NewSize { LOWORD(lParam), HIWORD(lParam) };
			POINT SizeDelta;
			xData->UpdateDialogSize(&SizeDelta);

			if (SizeDelta.x == 0 && SizeDelta.y ==0)
				return;

			HWND CurrentHandle = NULL;
			RECT* CurrentRect = nullptr;

			// form tree view - same width, new height
			CurrentHandle = xData->Handles.at(ObjectWindowExtraState::kForm_TreeView);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowExtraState::kForm_TreeView);

			MoveWindow(CurrentHandle,
					CurrentRect->left,
					CurrentRect->top,
					CurrentRect->right - CurrentRect->left,
					CurrentRect->bottom - CurrentRect->top + SizeDelta.y,
					TRUE);

			// form splitter - same width, new height
			CurrentHandle = xData->Handles.at(ObjectWindowExtraState::kForm_Splitter);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowExtraState::kForm_Splitter);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top + SizeDelta.y,
				TRUE);

			// form list view - new width, new height
			CurrentHandle = xData->Handles.at(ObjectWindowExtraState::kForm_ListView);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowExtraState::kForm_ListView);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top,
				CurrentRect->right - CurrentRect->left + SizeDelta.x,
				CurrentRect->bottom - CurrentRect->top + SizeDelta.y,
				TRUE);

			// vertical splitter - new vertical pos, same horizontal pos, new width, same height
			CurrentHandle = xData->Handles.at(ObjectWindowExtraState::kVerticalSplitter);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowExtraState::kVerticalSplitter);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left + SizeDelta.x,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag filter label - new vertical pos, same horizontal pos, same width/height
			CurrentHandle = xData->Handles.at(ObjectWindowExtraState::kTag_FilterLabel);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowExtraState::kTag_FilterLabel);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag filter edit - new vertical pos, same horizontal pos, same width/height
			CurrentHandle = xData->Handles.at(ObjectWindowExtraState::kTag_FilterEdit);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowExtraState::kTag_FilterEdit);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag tree view - new vertical pos, same horizontal pos, same width/height
			CurrentHandle = xData->Handles.at(ObjectWindowExtraState::kTag_TreeView);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowExtraState::kTag_TreeView);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag splitter - new vertical pos, same horizontal pos, same width/height
			CurrentHandle = xData->Handles.at(ObjectWindowExtraState::kTag_Splitter);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowExtraState::kTag_Splitter);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag list view - new vertical pos, same horizontal pos, new width, same height
			CurrentHandle = xData->Handles.at(ObjectWindowExtraState::kTag_ListView);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowExtraState::kTag_ListView);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left + SizeDelta.x,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			xData->UpdateRects();
			BGSEEUI->GetInvalidationManager()->Redraw(hWnd);
		}

		void ObjectWindowStateManager::OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam)
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

		void ObjectWindowStateManager::HandleFilters(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData)
		{
			auto RefreshFormList = [](HWND TreeView) -> void {
				auto Selection = TreeView_GetSelection(TreeView);

				TreeView_SelectItem(TreeView, nullptr);
				TreeView_SelectItem(TreeView, Selection);
			};

			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			if (xData == nullptr)
				return;

			if (FilterableFormListManager::Instance.HandleMessages(xData->Handles.at(ObjectWindowExtraState::kForm_FilterEdit),
																uMsg, wParam, lParam))
				RefreshFormList(xData->Handles.at(ObjectWindowExtraState::kForm_TreeView));

			if (FilterableFormListManager::Instance.HandleMessages(xData->Handles.at(ObjectWindowExtraState::kTag_FilterEdit),
																uMsg, wParam, lParam))
				RefreshFormList(xData->Handles.at(ObjectWindowExtraState::kTag_TreeView));
		}


		LRESULT CALLBACK ObjectWindowPrimarySubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												  bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			if (ObjectWindowImposterManager::Instance.IsImposter(hWnd))
			{
				// the imposter needs to handle its own messages exclusively
				return DlgProcResult;
			}

			switch (uMsg)
			{
			case TESObjectWindow::kWindowMessage_Reload:
			{
				auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
				if (xData == nullptr)
					break;

				// the window subclasser's windows hook has an out-sized performance penalty for
				// insertions in tree view controls (probably also has to do with the way it's used in this dialog)
				// so we have to temporarily suspend the hook when this code is executing
				Return = true;

				BGSEEUI->GetInvalidationManager()->Push(xData->Handles.at(ObjectWindowExtraState::kForm_TreeView));
				Subclasser->SuspendHooks();
				{
					DlgProcResult = Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);
				}
				Subclasser->ResumeHooks();
				BGSEEUI->GetInvalidationManager()->Pop(xData->Handles.at(ObjectWindowExtraState::kForm_TreeView));

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

				Return = true;
				ObjectWindowImposterManager::Instance.RefreshImposters();
				SendMessage(hWnd, TESDialog::kWindowMessage_Refresh, NULL, NULL);

				break;
			}
			case WM_ACTIVATE:
				Return = true;
				ObjectWindowStateManager::OnActivate(hWnd, wParam, lParam);

				break;
			case WM_CLOSE:
				Return = true;
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_ObjectWindow, NULL);

				break;
			case TESDialog::kWindowMessage_Destroy:
			case WM_DESTROY:
				ObjectWindowStateManager::OnDestroy(hWnd, ExtraData);

				ObjectWindowImposterManager::Instance.DestroyImposters();
				TESObjectWindow::PrimaryObjectWindowHandle = nullptr;

				break;
			case WM_INITDIALOG:
				ObjectWindowStateManager::OnInit(hWnd, ExtraData);
				TESObjectWindow::PrimaryObjectWindowHandle = hWnd;

				break;
			case WM_WINDOWPOSCHANGING:
				ObjectWindowStateManager::OnWindowPosChanging(hWnd, wParam, lParam, ExtraData);

				break;
			case WM_SIZE:
				Return = true;
				ObjectWindowStateManager::OnSize(hWnd, wParam, lParam, ExtraData);

				break;
			}

			if (!Return)
				ObjectWindowStateManager::HandleFilters(hWnd, uMsg, wParam, lParam, ExtraData);

			return DlgProcResult;
		}

		void InitializeObjectWindowOverrides()
		{
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_ObjectWindow,
				uiManager::ObjectWindowPrimarySubclassProc);
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