#include "GenericModelessDialog.h"
#include "UIManager.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace bgsee
{
	LRESULT CALLBACK GenericModelessDialog::DefaultDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		bool SkipCallback = false;
		bool SkipCallbackResult = false;
		bool SkipDefaultProc = false;
		LRESULT DlgProcResult = FALSE;
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		GenericModelessDialog* Instance = UserData ? UserData->Instance : nullptr;

		if (UserData == nullptr && uMsg != WM_INITDIALOG)
			return DlgProcResult;

		switch (uMsg)
		{
		case WM_CLOSE:
			Instance->ToggleVisibility();
			SkipCallback = true;

			break;
		case WM_INITMENUPOPUP:
			if (wParam == (WPARAM)Instance->ContextMenuHandle)
			{
				if (Instance->GetTopmost())
					CheckMenuItem(Instance->ContextMenuHandle, IDC_BGSEE_GENERICMODELESSDLG_CONTEXTMENU_ALWAYSONTOP, MF_CHECKED);
				else
					CheckMenuItem(Instance->ContextMenuHandle, IDC_BGSEE_GENERICMODELESSDLG_CONTEXTMENU_ALWAYSONTOP, MF_UNCHECKED);

				DlgProcResult = FALSE;
				SkipCallbackResult = true;
			}

			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_BGSEE_GENERICMODELESSDLG_CONTEXTMENU_HIDE:
				Instance->ToggleVisibility();
				SkipCallback = true;

				break;
			case IDC_BGSEE_GENERICMODELESSDLG_CONTEXTMENU_ALWAYSONTOP:
				Instance->ToggleTopmost();
				SkipCallback = true;

				break;
			}

			break;
		case WM_CONTEXTMENU:
			{
				RECT Rect;
				POINT Point;

				GetClientRect(hWnd, &Rect);
				Point.x = GET_X_LPARAM(lParam);
				Point.y = GET_Y_LPARAM(lParam);
				ScreenToClient(hWnd, &Point);

				if (PtInRect((LPRECT) &Rect, Point))
				{
					ClientToScreen(hWnd, (LPPOINT)&Point);
					TrackPopupMenu(Instance->ContextMenuHandle, TPM_LEFTALIGN|TPM_LEFTBUTTON, Point.x, Point.y, 0, hWnd, nullptr);
				}
			}

			break;
		case WM_SIZING:
			{
				if (Instance->LockAspectRatio)
				{
					RECT* NewBounds = (RECT*)lParam;
					tagRECT CurrentBounds;
					GetClientRect(hWnd, &CurrentBounds);

					switch (wParam)
					{
					case WMSZ_LEFT:
					case WMSZ_RIGHT:
						{
							//Adjust height and vertical position
							int TargetHeight = (int)((CurrentBounds.right - CurrentBounds.left) / Instance->AspectRatio);
							int OriginalHeight = NewBounds->bottom - NewBounds->top;
							int DiffHeight = OriginalHeight - TargetHeight;

							NewBounds->top += (DiffHeight / 2);
							NewBounds->bottom = NewBounds->top + TargetHeight;

							break;
						}
					case WMSZ_TOP:
					case WMSZ_BOTTOM:
						{
							//Adjust width and horizontal position
							int TargetWidth = (int)((CurrentBounds.bottom - CurrentBounds.top) * Instance->AspectRatio);
							int OriginalWidth = NewBounds->right - NewBounds->left;
							int DiffWidth = OriginalWidth - TargetWidth;

							NewBounds->left += (DiffWidth / 2);
							NewBounds->right = NewBounds->left + TargetWidth;

							break;
						}
					case WMSZ_RIGHT + WMSZ_BOTTOM:
						//Lower-right corner
						NewBounds->bottom = NewBounds->top + (int)((CurrentBounds.right - CurrentBounds.left) / Instance->AspectRatio);

						break;
					case WMSZ_LEFT + WMSZ_BOTTOM:
						//Lower-left corner
						NewBounds->bottom = NewBounds->top + (int)((CurrentBounds.right - CurrentBounds.left)  / Instance->AspectRatio);

						break;
					case WMSZ_LEFT + WMSZ_TOP:
						//Upper-left corner
						NewBounds->left = NewBounds->right - (int)((CurrentBounds.bottom - CurrentBounds.top) * Instance->AspectRatio);

						break;
					case WMSZ_RIGHT + WMSZ_TOP:
						//Upper-right corner
						NewBounds->right = NewBounds->left + (int)((CurrentBounds.bottom - CurrentBounds.top) * Instance->AspectRatio);

						break;
					}
				}

				break;
			}
		case WM_MOVING:
			DlgProcResult = UIManager::WindowEdgeSnapper.OnSnapMoving(hWnd, uMsg, wParam, lParam);
			SkipDefaultProc = true;

			break;
		case WM_ENTERSIZEMOVE:
			DlgProcResult = UIManager::WindowEdgeSnapper.OnSnapEnterSizeMove(hWnd, uMsg, wParam, lParam);
			SkipDefaultProc = true;

			break;
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);
			UserData = (DlgUserData*)lParam;
			Instance = UserData->Instance;
			UserData->Initialized = true;

			break;
		case WM_DESTROY:
			Instance->CallbackDlgProc(hWnd, uMsg, wParam, lParam, SkipCallback);
			SkipCallback = true;
			delete UserData;

			UserData = nullptr;
			SetWindowLongPtr(hWnd, GWL_USERDATA, NULL);

			break;
		}

		bool CallbackReturn = false;
		if (SkipCallback == false && UserData && UserData->Initialized && Instance->CallbackDlgProc)
		{
			LRESULT CallbackResult = Instance->CallbackDlgProc(hWnd, uMsg, wParam, lParam, CallbackReturn);
			if (SkipDefaultProc == false && SkipCallbackResult == false && CallbackReturn)
				return CallbackResult;
		}

		if (SkipDefaultProc)
			return DlgProcResult;
		else
			return FALSE;
	}

	void GenericModelessDialog::INILoadUIState( INISetting* Top, INISetting* Left, INISetting* Right, INISetting* Bottom, INISetting* Visible )
	{
		SME_ASSERT(Top && Top->GetType() == INISetting::kType_Integer);
		SME_ASSERT(Left && Left->GetType() == INISetting::kType_Integer);
		SME_ASSERT(Right && Right->GetType() == INISetting::kType_Integer);
		SME_ASSERT(Bottom && Bottom->GetType() == INISetting::kType_Integer);
		SME_ASSERT(Visible && Visible->GetType() == INISetting::kType_Integer);

		SetWindowPos(DialogHandle,
					HWND_NOTOPMOST,
					Left->GetData().i,
					Top->GetData().i,
					Right->GetData().i,
					Bottom->GetData().i,
					SWP_SHOWWINDOW|SWP_FRAMECHANGED);

		SetVisibility(Visible->GetData().i);
	}

	void GenericModelessDialog::INISaveUIState( INISetting* Top, INISetting* Left, INISetting* Right, INISetting* Bottom, INISetting* Visible )
	{
		SME_ASSERT(Top && Top->GetType() == INISetting::kType_Integer);
		SME_ASSERT(Left && Left->GetType() == INISetting::kType_Integer);
		SME_ASSERT(Right && Right->GetType() == INISetting::kType_Integer);
		SME_ASSERT(Bottom && Bottom->GetType() == INISetting::kType_Integer);
		SME_ASSERT(Visible && Visible->GetType() == INISetting::kType_Integer);

		RECT WindowRect;
		char Buffer[20] = {0};
		GetWindowRect(DialogHandle, &WindowRect);

		Top->SetInt(WindowRect.top);
		Left->SetInt(WindowRect.left);
		Right->SetInt(WindowRect.right - WindowRect.left);
		Bottom->SetInt(WindowRect.bottom - WindowRect.top);
		Visible->SetInt(this->Visible);
	}

	bool GenericModelessDialog::SetVisibility( bool State )
	{
		if (State)
			ShowWindow(DialogHandle, SW_SHOWNA);
		else
			ShowWindow(DialogHandle, SW_HIDE);

		Visible = State;
		return Visible;
	}

	bool GenericModelessDialog::SetTopmost( bool State )
	{
		if (State)
			SetWindowPos(DialogHandle, HWND_TOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
		else
			SetWindowPos(DialogHandle, HWND_NOTOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);

		Topmost = State;
		return Topmost;
	}

	GenericModelessDialog::GenericModelessDialog( HWND Parent,
															HINSTANCE Resource,
															ResourceTemplateOrdinalT DialogTemplate, ResourceTemplateOrdinalT ContextMenuTemplate,
															MessageCallback CallbackProc,
															float AspectRatio /*= 0.0f*/ )
	{
		SME_ASSERT(Parent && Resource && DialogTemplate && ContextMenuTemplate && CallbackProc);

		DialogHandle = nullptr;
		ContextMenuHandle = nullptr;
		ContextMenuParentHandle = nullptr;
		ParentHandle = Parent;
		ResourceInstance = Resource;
		DialogTemplateID = DialogTemplate;
		DialogContextMenuID = ContextMenuTemplate;
		CallbackDlgProc = CallbackProc;
		Visible = false;
		Topmost = false;
		this->AspectRatio = AspectRatio;

		if (AspectRatio == 0.0f)
			LockAspectRatio = false;
		else
			LockAspectRatio = true;
	}

	GenericModelessDialog::GenericModelessDialog()
	{
		DialogHandle = nullptr;
		ContextMenuHandle = nullptr;
		ContextMenuParentHandle = nullptr;
		ParentHandle = nullptr;
		ResourceInstance = nullptr;
		DialogTemplateID = 0;
		DialogContextMenuID = 0;
		CallbackDlgProc = nullptr;
		Visible = false;
		Topmost = false;
		AspectRatio = 0.0f;
		LockAspectRatio = false;
	}

	GenericModelessDialog::~GenericModelessDialog()
	{
		if (DialogHandle == nullptr || ContextMenuParentHandle == nullptr)
			return;

		if (BGSEEUI)
			BGSEEUI->GetWindowHandleCollection(UIManager::kHandleCollection_MainWindowChildren)->Remove(DialogHandle);

		DestroyMenu(ContextMenuParentHandle);
		DestroyWindow(DialogHandle);
	}

	void GenericModelessDialog::Create( LPARAM InitParam, bool Hide, bool OverrideCreation )
	{
		if (DialogHandle || ContextMenuParentHandle || CallbackDlgProc == nullptr)
			return;

		DlgUserData* UserData = new DlgUserData();
		UserData->Instance = this;
		UserData->ExtraData = InitParam;
		DialogHandle = BGSEEUI->ModelessDialog(ResourceInstance,
											MAKEINTRESOURCE(DialogTemplateID),
											ParentHandle,
											(DLGPROC)DefaultDlgProc,
											(LPARAM)UserData,
											OverrideCreation);
		ContextMenuParentHandle = LoadMenu(ResourceInstance, (LPSTR)DialogContextMenuID);
		ContextMenuHandle = GetSubMenu(ContextMenuParentHandle, 0);

		SME_ASSERT(DialogHandle && ContextMenuParentHandle && ContextMenuHandle);

		BGSEEUI->GetWindowHandleCollection(UIManager::kHandleCollection_MainWindowChildren)->Add(DialogHandle);
		SetVisibility(Hide == false);
	}

	bool GenericModelessDialog::ToggleVisibility( void )
	{
		return SetVisibility((Visible == false));
	}

	bool GenericModelessDialog::ToggleTopmost( void )
	{
		return SetTopmost((Topmost == false));
	}

	bool GenericModelessDialog::IsVisible( void ) const
	{
		return Visible;
	}

	bool GenericModelessDialog::GetTopmost( void ) const
	{
		return Topmost;
	}

	bool GenericModelessDialog::GetInitialized( void ) const
	{
		return (DialogHandle != nullptr);
	}
}

