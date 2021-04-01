#include "ObjectWindowOverrides.h"
#include "MiscWindowOverrides.h"
#include "DialogImposterManager.h"
#include "HallOfFame.h"

namespace cse
{
	namespace uiManager
	{
		void GetRelativeBounds(HWND hWnd, RECT* Rect)
		{
			GetWindowRect(hWnd, Rect);
			MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), reinterpret_cast<LPPOINT>(Rect), 2);
		}

		SplitterState::SplitterState(ObjectWindowExtraState* WindowState, ObjectWindowControlID ID)
			: WindowState(WindowState), Enabled(true), MouseOver(false), Dragging(false), DragOrigin{ 0 }, ID(ID)
		{
			SplitterHandle = WindowState->Handles.at(ID);
			SplitterBounds = &WindowState->CurrentRects.at(ID);
		}

		bool SplitterState::IsMouseOver() const
		{
			POINT CursorLoc;
			GetCursorPos(&CursorLoc);

			auto WindowUnderCursor = WindowFromPoint(CursorLoc);
			if (WindowUnderCursor == SplitterHandle)
				return true;

			if (WindowUnderCursor == WindowState->Parent)
			{
				ScreenToClient(WindowState->Parent, &CursorLoc);
				WindowUnderCursor = ChildWindowFromPoint(WindowState->Parent, CursorLoc);
				return WindowUnderCursor == SplitterHandle;
			}

			return false;
		}

		void SplitterState::UpdateCursor() const
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

		HorizontalSplitter::HorizontalSplitter(ObjectWindowExtraState* WindowState, ObjectWindowControlID ID)
			: SplitterState(WindowState, ID)
		{
			SME_ASSERT(ID == ObjectWindowControlID::kForm_Splitter || ID == ObjectWindowControlID::kTag_Splitter);

			if (ID == ObjectWindowControlID::kForm_Splitter)
			{
				FilterLabel = ObjectWindowControlID::kForm_FilterLabel;
				FilterEdit = ObjectWindowControlID::kForm_FilterEdit;
				TreeView = ObjectWindowControlID::kForm_TreeView;
				ListView = ObjectWindowControlID::kForm_ListView;
			}
			else
			{
				FilterLabel = ObjectWindowControlID::kTag_FilterLabel;
				FilterEdit = ObjectWindowControlID::kTag_FilterEdit;
				TreeView = ObjectWindowControlID::kTag_TreeView;
				ListView = ObjectWindowControlID::kTag_ListView;
			}
		}

		void HorizontalSplitter::Move(LONG Delta)
		{
			auto CurrentSplitterPos = SplitterBounds->left;
			auto NewSplitterPos = CurrentSplitterPos + Delta;

			const auto SafeZoneOffset = 75;
			// do nothing if the new splitter position is outside the safe-zone
			if ((NewSplitterPos <= SafeZoneOffset && Delta < 0) || (NewSplitterPos >= (WindowState->CurrentDialogSize.x - SafeZoneOffset) && Delta > 0))
				return;

			// update the splitter's position first
			{
				RECT NewBounds{
					SplitterBounds->left + Delta,
					SplitterBounds->top,
					SplitterBounds->right - SplitterBounds->left,
					SplitterBounds->bottom - SplitterBounds->top,
				};

				MoveWindow(SplitterHandle,
					NewBounds.left,
					NewBounds.top,
					NewBounds.right,
					NewBounds.bottom,
					TRUE);

				GetRelativeBounds(SplitterHandle, SplitterBounds);
			}

			HWND CurrentHandle = NULL;
			RECT* CurrentRect = nullptr;

			// form filter edit - new width
			{
				CurrentHandle = WindowState->Handles.at(FilterEdit);
				CurrentRect = &WindowState->CurrentRects.at(FilterEdit);

				RECT NewBounds{
					CurrentRect->left,
					CurrentRect->top,
					CurrentRect->right - CurrentRect->left + Delta,
					CurrentRect->bottom - CurrentRect->top,
				};

				MoveWindow(CurrentHandle,
					NewBounds.left,
					NewBounds.top,
					NewBounds.right,
					NewBounds.bottom,
					TRUE);

				GetRelativeBounds(CurrentHandle, CurrentRect);
			}

			// form tree view - new width
			{
				CurrentHandle = WindowState->Handles.at(TreeView);
				CurrentRect = &WindowState->CurrentRects.at(TreeView);

				RECT NewBounds{
					CurrentRect->left,
					CurrentRect->top,
					CurrentRect->right - CurrentRect->left + Delta,
					CurrentRect->bottom - CurrentRect->top,
				};

				MoveWindow(CurrentHandle,
					NewBounds.left,
					NewBounds.top,
					NewBounds.right,
					NewBounds.bottom,
					TRUE);

				GetRelativeBounds(CurrentHandle, CurrentRect);
			}

			// form list view - new x pos, new width
			{
				CurrentHandle = WindowState->Handles.at(ListView);
				CurrentRect = &WindowState->CurrentRects.at(ListView);

				RECT NewBounds{
					CurrentRect->left + Delta,
					CurrentRect->top,
					CurrentRect->right - CurrentRect->left + (-Delta),
					CurrentRect->bottom - CurrentRect->top,
				};

				MoveWindow(CurrentHandle,
					NewBounds.left,
					NewBounds.top,
					NewBounds.right,
					NewBounds.bottom,
					TRUE);

				GetRelativeBounds(CurrentHandle, CurrentRect);
			}
		}

		LPCSTR HorizontalSplitter::GetSizingCursor() const
		{
			return IDC_SIZEWE;
		}

		void VerticalSplitter::ResizeControl(ObjectWindowControlID Control, LONG Delta) const
		{
			BGSEECONSOLE_MESSAGE("Vert splitter resizing control: %d | Delta: %d", Control, Delta);

			HWND CurrentHandle = NULL;
			RECT* CurrentRect = nullptr;
			RECT NewBounds { 0 };

			switch (Control)
			{
			case ObjectWindowControlID::kForm_TreeView:
				// form tree view - new height
				CurrentHandle = WindowState->Handles.at(ObjectWindowControlID::kForm_TreeView);
				CurrentRect = &WindowState->CurrentRects.at(ObjectWindowControlID::kForm_TreeView);

				NewBounds.left = CurrentRect->left;
				NewBounds.top = CurrentRect->top;
				NewBounds.right = CurrentRect->right - CurrentRect->left;
				NewBounds.bottom = CurrentRect->bottom - CurrentRect->top + Delta;

				break;
			case ObjectWindowControlID::kForm_ListView:
				// form list view - new height
				CurrentHandle = WindowState->Handles.at(ObjectWindowControlID::kForm_ListView);
				CurrentRect = &WindowState->CurrentRects.at(ObjectWindowControlID::kForm_ListView);

				NewBounds.left = CurrentRect->left;
				NewBounds.top = CurrentRect->top;
				NewBounds.right = CurrentRect->right - CurrentRect->left;
				NewBounds.bottom = CurrentRect->bottom - CurrentRect->top + Delta;

				break;
			case ObjectWindowControlID::kForm_Splitter:
				// form splitter - new height
				CurrentHandle = WindowState->Handles.at(ObjectWindowControlID::kForm_Splitter);
				CurrentRect = &WindowState->CurrentRects.at(ObjectWindowControlID::kForm_Splitter);

				NewBounds.left = CurrentRect->left;
				NewBounds.top = CurrentRect->top;
				NewBounds.right = CurrentRect->right - CurrentRect->left;
				NewBounds.bottom = CurrentRect->bottom - CurrentRect->top + Delta;

				break;
			case ObjectWindowControlID::kTag_FilterLabel:
				// tag filter label - new y pos

				CurrentHandle = WindowState->Handles.at(ObjectWindowControlID::kTag_FilterLabel);
				CurrentRect = &WindowState->CurrentRects.at(ObjectWindowControlID::kTag_FilterLabel);

				NewBounds.left = CurrentRect->left;
				NewBounds.top = CurrentRect->top + Delta;
				NewBounds.right = CurrentRect->right - CurrentRect->left;
				NewBounds.bottom = CurrentRect->bottom - CurrentRect->top;

				break;
			case ObjectWindowControlID::kTag_FilterEdit:
				// tag filter edit - new y pos
				CurrentHandle = WindowState->Handles.at(ObjectWindowControlID::kTag_FilterEdit);
				CurrentRect = &WindowState->CurrentRects.at(ObjectWindowControlID::kTag_FilterEdit);

				NewBounds.left = CurrentRect->left;
				NewBounds.top = CurrentRect->top + Delta;
				NewBounds.right = CurrentRect->right - CurrentRect->left;
				NewBounds.bottom = CurrentRect->bottom - CurrentRect->top;

				break;
			case ObjectWindowControlID::kTag_ListView:
				// tag list view - new y pos, new height

				CurrentHandle = WindowState->Handles.at(ObjectWindowControlID::kTag_ListView);
				CurrentRect = &WindowState->CurrentRects.at(ObjectWindowControlID::kTag_ListView);

				NewBounds.left = CurrentRect->left;
				NewBounds.top = CurrentRect->top + Delta;
				NewBounds.right = CurrentRect->right - CurrentRect->left;
				NewBounds.bottom = CurrentRect->bottom - CurrentRect->top + (-Delta);

				break;
			case ObjectWindowControlID::kTag_TreeView:
				// tag tree view - new y pos, new height

				CurrentHandle = WindowState->Handles.at(ObjectWindowControlID::kTag_TreeView);
				CurrentRect = &WindowState->CurrentRects.at(ObjectWindowControlID::kTag_TreeView);

				NewBounds.left = CurrentRect->left;
				NewBounds.top = CurrentRect->top + Delta;
				NewBounds.right = CurrentRect->right - CurrentRect->left;
				NewBounds.bottom = CurrentRect->bottom - CurrentRect->top + (-Delta);

				break;
			case ObjectWindowControlID::kTag_Splitter:
				// tag splitter - new y pos, new height

				CurrentHandle = WindowState->Handles.at(ObjectWindowControlID::kTag_Splitter);
				CurrentRect = &WindowState->CurrentRects.at(ObjectWindowControlID::kTag_Splitter);

				NewBounds.left = CurrentRect->left;
				NewBounds.top = CurrentRect->top + Delta;
				NewBounds.right = CurrentRect->right - CurrentRect->left;
				NewBounds.bottom = CurrentRect->bottom - CurrentRect->top + (-Delta);

				break;
			default:
				// no other controls need to be handled here
				return;
			}

			MoveWindow(CurrentHandle,
				NewBounds.left,
				NewBounds.top,
				NewBounds.right,
				NewBounds.bottom,
				TRUE);

			GetRelativeBounds(CurrentHandle, CurrentRect);
		}

		VerticalSplitter::VerticalSplitter(ObjectWindowExtraState* WindowState)
			: SplitterState(WindowState, ObjectWindowControlID::kVerticalSplitter), TopHidden(false), BottomHidden(false)
		{
			;//
		}

		void VerticalSplitter::Move(LONG Delta)
		{
			const auto SafeZoneOffset = 7;

			//if (SplitterBounds->top + Delta <= SafeZoneOffset && Delta < 0)
			//	Delta = SafeZoneOffset - SplitterBounds->top;
			//else if (SplitterBounds->bottom + Delta >= (WindowState->CurrentDialogSize.y - SafeZoneOffset) && Delta > 0)
			//	Delta = (WindowState->CurrentDialogSize.y - SafeZoneOffset) - SplitterBounds->bottom;

			//if (Delta == 0)
			//	return;

			// do nothing if the new splitter position is outside the safe-zone
			if ((SplitterBounds->top + Delta <= SafeZoneOffset && Delta < 0) || (SplitterBounds->bottom + Delta >= (WindowState->CurrentDialogSize.y - SafeZoneOffset) && Delta > 0))
				return;


			HWND CurrentHandle = NULL;
			RECT* CurrentRect = nullptr;

			enum Mode
			{
				Resize,
				HideTop,
				HideBottom,
			};

			// hide the top or bottom panel if the new splitter position is outside of the "visible zone"
			auto Action = Mode::Resize;
			if (SplitterBounds->top + Delta <= kHidePanelMinOffset)
				Action = Mode::HideTop;
			else if (SplitterBounds->bottom + Delta >= WindowState->CurrentDialogSize.y - kHidePanelMaxOffset)
				Action = Mode::HideBottom;

			auto SetHidden = [](HWND hWnd, bool Hide) -> void {
				if (Hide)
				{
					EnableWindow(hWnd, FALSE);
					ShowWindow(hWnd, SW_HIDE);
				}
				else if (!Hide)
				{
					EnableWindow(hWnd, TRUE);
					ShowWindow(hWnd, SW_SHOWNA);
				}
			};

			const auto TopControls = {
				ObjectWindowControlID::kForm_FilterLabel,
				ObjectWindowControlID::kForm_FilterEdit,
				ObjectWindowControlID::kForm_ListView,
				ObjectWindowControlID::kForm_TreeView,
				ObjectWindowControlID::kForm_Splitter,
			};

			const auto BottomControls = {
				ObjectWindowControlID::kTag_FilterLabel,
				ObjectWindowControlID::kTag_FilterEdit,
				ObjectWindowControlID::kTag_ListView,
				ObjectWindowControlID::kTag_TreeView,
				ObjectWindowControlID::kTag_Splitter,
			};

			// before performing the new action, reset any previously hidden controls
			switch (Action)
			{
			case Mode::HideTop:
			case Mode::HideBottom:
			{
				auto Toggle = Action == Mode::HideTop ? &TopHidden : &BottomHidden;
				auto OtherToggle = Action == Mode::HideTop ? &BottomHidden : &TopHidden;

				if (*OtherToggle)
				{
					*OtherToggle = false;

					auto NewSplitterPos = Delta < 0 ? SplitterBounds->top + Delta : SplitterBounds->bottom + Delta;
					auto NewDelta = NewSplitterPos - (Action == Mode::HideTop ? kHidePanelMaxOffset : kHidePanelMinOffset);
					for (const auto& Itr : Action == Mode::HideTop ? BottomControls : TopControls)
					{
						SetHidden(WindowState->Handles.at(Itr), false);
						ResizeControl(Itr, NewDelta);
					}
				}

				*Toggle = true;

				for (const auto& Itr : Action == Mode::HideTop ? TopControls : BottomControls)
					SetHidden(WindowState->Handles.at(Itr), true);

				for (const auto& Itr : Action == Mode::HideTop ? BottomControls : TopControls)
					ResizeControl(Itr, Delta);

				break;
			}
			case Mode::Resize:
			{
				if (TopHidden || BottomHidden)
				{
					SME_ASSERT(TopHidden != BottomHidden);

					for (const auto& Itr : TopHidden ? TopControls : BottomControls)
						SetHidden(WindowState->Handles.at(Itr), false);
				}

				if (!TopHidden)
				{
					for (const auto& Itr : TopControls)
						ResizeControl(Itr, Delta);
				}

				if (!BottomHidden)
				{
					for (const auto& Itr : BottomControls)
						ResizeControl(Itr, Delta);
				}


				if (TopHidden)
					TopHidden = false;
				else
					BottomHidden = false;

				break;
			}
			}

			// update the splitter's position
			{
				RECT NewBounds{
					SplitterBounds->left,
					SplitterBounds->top + Delta,
					SplitterBounds->right - SplitterBounds->left,
					SplitterBounds->bottom - SplitterBounds->top,
				};

				MoveWindow(SplitterHandle,
					NewBounds.left,
					NewBounds.top,
					NewBounds.right,
					NewBounds.bottom,
					TRUE);

				GetRelativeBounds(SplitterHandle, SplitterBounds);
			}
		}

		LPCSTR VerticalSplitter::GetSizingCursor() const
		{
			return IDC_SIZENS;
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
				SplitterData->MouseOver = SplitterData->IsMouseOver();
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
					SplitterData->MouseOver = SplitterData->IsMouseOver();
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

				//constexpr auto kMaxDeltaAbsolute = 5;
				//if (Delta.x < -kMaxDeltaAbsolute)
				//	Delta.x = -kMaxDeltaAbsolute;
				//else if (Delta.x > kMaxDeltaAbsolute)
				//	Delta.x = kMaxDeltaAbsolute;

				//if (Delta.y < -kMaxDeltaAbsolute)
				//	Delta.y = -kMaxDeltaAbsolute;
				//else if (Delta.y > kMaxDeltaAbsolute)
				//	Delta.y = kMaxDeltaAbsolute;

				const auto SplitterDelta = SplitterData->ID == kVerticalSplitter ? Delta.y : Delta.x;
				if (SplitterDelta != 0)
					SplitterData->Move(SplitterDelta);

				break;
			}
			}

			return ProcResult;
		}

		void ObjectWindowExtraState::SetActiveSplitter(ObjectWindowControlID Splitter, bool ActiveState)
		{
			switch (Splitter)
			{
			case kForm_Splitter:
				TagViewSplitter->Enabled = ActiveState == false;
				VerticalSplitter->Enabled = ActiveState == false;
				break;
			case kTag_Splitter:
				FormViewSplitter->Enabled = ActiveState == false;
				VerticalSplitter->Enabled = ActiveState == false;
				break;
			case kVerticalSplitter:
				FormViewSplitter->Enabled = ActiveState == false;
				TagViewSplitter->Enabled = ActiveState == false;
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

			FormViewSplitter.reset(new HorizontalSplitter(this, kForm_Splitter));
			TagViewSplitter.reset(new HorizontalSplitter(this, kTag_Splitter));
			VerticalSplitter.reset(new uiManager::VerticalSplitter(this));

			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(Handles.at(kForm_Splitter), ThunkSplitterSubclassProc());
			SetWindowLongPtr(Handles.at(kForm_Splitter), GWL_USERDATA, reinterpret_cast<LONG_PTR>(FormViewSplitter.get()));

			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(Handles.at(kTag_Splitter), ThunkSplitterSubclassProc());
			SetWindowLongPtr(Handles.at(kTag_Splitter), GWL_USERDATA, reinterpret_cast<LONG_PTR>(TagViewSplitter.get()));

			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(Handles.at(kVerticalSplitter), ThunkSplitterSubclassProc());
			SetWindowLongPtr(Handles.at(kVerticalSplitter), GWL_USERDATA, reinterpret_cast<LONG_PTR>(VerticalSplitter.get()));
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

		void ObjectWindowExtraState::UpdateRect(ObjectWindowControlID Control)
		{
			GetRelativeBounds(Handles.at(Control), &CurrentRects[Control]);
		}

		void ObjectWindowExtraState::UpdateRects()
		{
			auto GetRelativeCoords = [](HWND hWnd, RECT* Rect) -> void {
				GetWindowRect(hWnd, Rect);
				MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), reinterpret_cast<LPPOINT>(Rect), 2);
			};

			UpdateRect(kForm_FilterLabel);
			UpdateRect(kForm_FilterEdit);
			UpdateRect(kForm_ListView);
			UpdateRect(kForm_TreeView);
			UpdateRect(kForm_Splitter);
			UpdateRect(kTag_FilterLabel);
			UpdateRect(kTag_FilterEdit);
			UpdateRect(kTag_ListView);
			UpdateRect(kTag_TreeView);
			UpdateRect(kTag_Splitter);
			UpdateRect(kVerticalSplitter);
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

		bool ObjectWindowExtraState::OnWindowPosChanging(WINDOWPOS* PosParams) const
		{
			static constexpr auto kMinWidth = 400;
			static constexpr auto kMinHeight = 400;

			if (PosParams->cx < kMinWidth)
				PosParams->cx = kMinWidth;

			if (PosParams->cy < kMinHeight)
				PosParams->cy = kMinHeight;

			return true;
		}

		void ObjectWindowStateManager::OnInit(HWND hWnd, bgsee::WindowExtraDataCollection* ExtraData)
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

			FilterableFormListManager::Instance.Register(xData->Handles.at(ObjectWindowControlID::kForm_FilterEdit),
														xData->Handles.at(ObjectWindowControlID::kForm_FilterLabel),
														xData->Handles.at(ObjectWindowControlID::kForm_ListView),
														hWnd);

			FilterableFormListManager::Instance.Register(xData->Handles.at(ObjectWindowControlID::kTag_FilterEdit),
														xData->Handles.at(ObjectWindowControlID::kTag_FilterLabel),
														xData->Handles.at(ObjectWindowControlID::kTag_ListView),
														hWnd);

			std::string WndTitle = "Object Window";
			if (settings::general::kShowHallOfFameMembersInTitleBar().i != hallOfFame::kDisplayESMember_None)
			{
				hallOfFame::GetRandomESMember(WndTitle);
				WndTitle += " Object Window";
			}
			SetWindowText(hWnd, WndTitle.c_str());
		}

		void ObjectWindowStateManager::OnDestroy(HWND hWnd, UINT uMsg, bgsee::WindowExtraDataCollection* ExtraData)
		{
			auto xData = BGSEE_GETWINDOWXDATA(ObjectWindowExtraState, ExtraData);
			if (xData == nullptr)
				return;

			FilterableFormListManager::Instance.Unregister(xData->Handles.at(ObjectWindowControlID::kForm_FilterEdit));
			FilterableFormListManager::Instance.Unregister(xData->Handles.at(ObjectWindowControlID::kTag_FilterEdit));

			// only remove extra data on true destruction
			if (uMsg == WM_DESTROY)
			{
				ExtraData->Remove(ObjectWindowExtraState::kTypeID);
				delete xData;
			}
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
			CurrentHandle = xData->Handles.at(ObjectWindowControlID::kForm_TreeView);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowControlID::kForm_TreeView);

			MoveWindow(CurrentHandle,
					CurrentRect->left,
					CurrentRect->top,
					CurrentRect->right - CurrentRect->left,
					CurrentRect->bottom - CurrentRect->top + SizeDelta.y,
					TRUE);

			// form splitter - same width, new height
			CurrentHandle = xData->Handles.at(ObjectWindowControlID::kForm_Splitter);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowControlID::kForm_Splitter);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top + SizeDelta.y,
				TRUE);

			// form list view - new width, new height
			CurrentHandle = xData->Handles.at(ObjectWindowControlID::kForm_ListView);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowControlID::kForm_ListView);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top,
				CurrentRect->right - CurrentRect->left + SizeDelta.x,
				CurrentRect->bottom - CurrentRect->top + SizeDelta.y,
				TRUE);

			// vertical splitter - new vertical pos, same horizontal pos, new width, same height
			CurrentHandle = xData->Handles.at(ObjectWindowControlID::kVerticalSplitter);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowControlID::kVerticalSplitter);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left + SizeDelta.x,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag filter label - new vertical pos, same horizontal pos, same width/height
			CurrentHandle = xData->Handles.at(ObjectWindowControlID::kTag_FilterLabel);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowControlID::kTag_FilterLabel);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag filter edit - new vertical pos, same horizontal pos, same width/height
			CurrentHandle = xData->Handles.at(ObjectWindowControlID::kTag_FilterEdit);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowControlID::kTag_FilterEdit);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag tree view - new vertical pos, same horizontal pos, same width/height
			CurrentHandle = xData->Handles.at(ObjectWindowControlID::kTag_TreeView);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowControlID::kTag_TreeView);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag splitter - new vertical pos, same horizontal pos, same width/height
			CurrentHandle = xData->Handles.at(ObjectWindowControlID::kTag_Splitter);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowControlID::kTag_Splitter);

			MoveWindow(CurrentHandle,
				CurrentRect->left,
				CurrentRect->top + SizeDelta.y,
				CurrentRect->right - CurrentRect->left,
				CurrentRect->bottom - CurrentRect->top,
				TRUE);

			// tag list view - new vertical pos, same horizontal pos, new width, same height
			CurrentHandle = xData->Handles.at(ObjectWindowControlID::kTag_ListView);
			CurrentRect = &xData->CurrentRects.at(ObjectWindowControlID::kTag_ListView);

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

			if (FilterableFormListManager::Instance.HandleMessages(xData->Handles.at(ObjectWindowControlID::kForm_FilterEdit),
																uMsg, wParam, lParam))
				RefreshFormList(xData->Handles.at(ObjectWindowControlID::kForm_TreeView));

			if (FilterableFormListManager::Instance.HandleMessages(xData->Handles.at(ObjectWindowControlID::kTag_FilterEdit),
																uMsg, wParam, lParam))
				RefreshFormList(xData->Handles.at(ObjectWindowControlID::kTag_TreeView));
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

				BGSEEUI->GetInvalidationManager()->Push(xData->Handles.at(ObjectWindowControlID::kForm_TreeView));
				Subclasser->SuspendHooks();
				{
					DlgProcResult = Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);
				}
				Subclasser->ResumeHooks();
				BGSEEUI->GetInvalidationManager()->Pop(xData->Handles.at(ObjectWindowControlID::kForm_TreeView));

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
				ObjectWindowStateManager::OnDestroy(hWnd, uMsg, ExtraData);

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