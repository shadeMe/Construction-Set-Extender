#include "BGSEEUIManager.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace BGSEditorExtender
{
	BGSEEUIManager*						BGSEEUIManager::Singleton = NULL;
	SME::UIHelpers::CSnapWindow			BGSEEUIManager::WindowEdgeSnapper;

	BGSEEUIManager::WindowHandleCollection::HandleCollectionT::iterator BGSEEUIManager::WindowHandleCollection::Find( HWND Handle )
	{
		for (HandleCollectionT::iterator Itr = HandleList.begin(); Itr != HandleList.end(); Itr++)
		{
			if (*Itr == Handle)
				return Itr;
		}

		return HandleList.end();
	}

	bool BGSEEUIManager::WindowHandleCollection::Add( HWND Handle )
	{
		if (GetExists(Handle) == false)
		{
			HandleList.push_back(Handle);
			return true;
		}
		else
			return false;
	}

	bool BGSEEUIManager::WindowHandleCollection::Remove( HWND Handle )
	{
		HandleCollectionT::iterator Match = Find(Handle);
		if (Match != HandleList.end())
		{
			HandleList.erase(Match);
			return true;
		}
		else
			return false;
	}

	bool BGSEEUIManager::WindowHandleCollection::GetExists( HWND Handle )
	{
		return Find(Handle) != HandleList.end();
	}

	void BGSEEUIManager::WindowHandleCollection::Clear( void )
	{
		HandleList.clear();
	}

	BGSEEUIManager::BGSEEUIManager() :
		HandleCollections()
	{
		Initialized = false;
	}

	BGSEEUIManager::~BGSEEUIManager()
	{
		Singleton = NULL;

		for (int i = 0; i < kHandleCollection__MAX; i++)
			HandleCollections[i].Clear();

		SAFEDELETE(EditorWindowHandle);
	}

	BGSEEUIManager* BGSEEUIManager::GetSingleton()
	{
		if (Singleton == NULL)
			Singleton = new BGSEEUIManager();

		return Singleton;
	}

	bool BGSEEUIManager::Initialize( HWND MainWindowHandle )
	{
		if (Initialized)
			return false;

		SME_ASSERT(MainWindowHandle);
		Initialized = true;

		SAFEDELETE(EditorWindowHandle);
		EditorWindowHandle = new HWNDGetter(MainWindowHandle);

		return Initialized;
	}

	BGSEEUIManager::WindowHandleCollection* BGSEEUIManager::GetWindowHandleCollection( UInt8 ID ) const
	{
		SME_ASSERT(ID < kHandleCollection__MAX);

		return const_cast<WindowHandleCollection*>(&HandleCollections[ID]);
	}

	HWND BGSEEUIManager::GetMainWindow( void ) const
	{
		SME_ASSERT(EditorWindowHandle);
		return EditorWindowHandle->operator()();
	}

	int BGSEEUIManager::MsgBoxI( HWND Parent, UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		return MessageBox(Parent, Buffer, BGSEEMAIN->ExtenderGetShortName(), Flags|MB_ICONINFORMATION);
	}

	int BGSEEUIManager::MsgBoxI( UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		return MessageBox(GetMainWindow(), Buffer, BGSEEMAIN->ExtenderGetShortName(), Flags|MB_ICONINFORMATION);
	}

	int BGSEEUIManager::MsgBoxI( const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		return MessageBox(GetMainWindow(), Buffer, BGSEEMAIN->ExtenderGetShortName(), MB_OK|MB_ICONINFORMATION);
	}

	int BGSEEUIManager::MsgBoxW( HWND Parent, UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		return MessageBox(Parent, Buffer, BGSEEMAIN->ExtenderGetShortName(), Flags|MB_ICONWARNING);
	}

	int BGSEEUIManager::MsgBoxW( UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		return MessageBox(GetMainWindow(), Buffer, BGSEEMAIN->ExtenderGetShortName(), Flags|MB_ICONWARNING);
	}

	int BGSEEUIManager::MsgBoxW( const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		return MessageBox(GetMainWindow(), Buffer, BGSEEMAIN->ExtenderGetShortName(), MB_OK|MB_ICONWARNING);
	}

	int BGSEEUIManager::MsgBoxE( HWND Parent, UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		return MessageBox(Parent, Buffer, BGSEEMAIN->ExtenderGetShortName(), Flags|MB_ICONERROR);
	}

	int BGSEEUIManager::MsgBoxE( UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		return MessageBox(GetMainWindow(), Buffer, BGSEEMAIN->ExtenderGetShortName(), Flags|MB_ICONERROR);
	}

	int BGSEEUIManager::MsgBoxE( const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		return MessageBox(GetMainWindow(), Buffer, BGSEEMAIN->ExtenderGetShortName(), MB_OK|MB_ICONERROR);
	}

	const BGSEEINIManagerSettingFactory::SettingData		BGSEEGenericModelessDialog::kDefaultINISettings[5] =
	{
		{ "Top",		"150",		"Dialog Rect Top" },
		{ "Left",		"150",		"Dialog Rect Left" },
		{ "Right",		"150",		"Dialog Rect Right" },
		{ "Bottom",		"150",		"Dialog Rect Bottom" },
		{ "Visible",	"1",		"Dialog Visibility State" }
	};

	LRESULT CALLBACK BGSEEGenericModelessDialog::DefaultDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		bool SkipCallback = false;
		bool SkipCallbackResult = false;
		bool SkipDefaultProc = false;
		LRESULT DlgProcResult = FALSE;
		DlgUserData* UserData = (DlgUserData*)GetWindowLong(hWnd, GWL_USERDATA);
		BGSEEGenericModelessDialog* Instance = UserData->Instance;

		switch (uMsg)
		{
		case WM_INITMENUPOPUP:
			if (wParam == (WPARAM)Instance->ContextMenuHandle)
			{
				if (Instance->GetTopmost())
					CheckMenuItem(Instance->ContextMenuHandle, IDC_BGSEE_GENERICMODELESSDLG_CONTEXTMENU_ALWAYSONTOP, MF_CHECKED|MF_BYPOSITION);
				else
					CheckMenuItem(Instance->ContextMenuHandle, IDC_BGSEE_GENERICMODELESSDLG_CONTEXTMENU_ALWAYSONTOP, MF_UNCHECKED|MF_BYPOSITION);

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

				if (PtInRect((LPRECT) &Rect, Point))
				{
					ClientToScreen(hWnd, (LPPOINT)&Point);
					TrackPopupMenu(Instance->ContextMenuHandle, TPM_LEFTALIGN|TPM_LEFTBUTTON, Point.x, Point.y, 0, hWnd, NULL);
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
			DlgProcResult = BGSEEUIManager::WindowEdgeSnapper.OnSnapMoving(hWnd, uMsg, wParam, lParam);
			SkipDefaultProc = true;
			break;
		case WM_ENTERSIZEMOVE:
			DlgProcResult = BGSEEUIManager::WindowEdgeSnapper.OnSnapEnterSizeMove(hWnd, uMsg, wParam, lParam);
			SkipDefaultProc = true;
			break;
		case WM_INITDIALOG:
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);
			UserData = (DlgUserData*)lParam;
			Instance = UserData->Instance;
			break;
		case WM_DESTROY:
			Instance->CallbackDlgProc(hWnd, uMsg, wParam, lParam, SkipCallback);
			SkipCallback = true;
			delete UserData;
			break;
		}

		bool CallbackReturn = false;
		if (SkipCallback == false)
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

	void BGSEEGenericModelessDialog::INILoadUIState( BGSEEINIManagerGetterFunctor* Getter, const char* Section )
	{
		int Top = atoi(Getter->operator()(kDefaultINISettings[kDefaultINISetting_Top].Key, Section)),
			Left = atoi(Getter->operator()(kDefaultINISettings[kDefaultINISetting_Left].Key, Section)),
			Right = atoi(Getter->operator()(kDefaultINISettings[kDefaultINISetting_Right].Key, Section)),
			Bottom = atoi(Getter->operator()(kDefaultINISettings[kDefaultINISetting_Bottom].Key, Section));

		SetWindowPos(DialogHandle, HWND_NOTOPMOST, Left, Top, Right, Bottom, NULL);
		SetVisibility(atoi(Getter->operator()(kDefaultINISettings[kDefaultINISetting_Visible].Key, Section)));
	}

	void BGSEEGenericModelessDialog::INISaveUIState( BGSEEINIManagerSetterFunctor* Setter, const char* Section )
	{
		tagRECT WindowRect;
		char Buffer[20] = {0};
		GetWindowRect(DialogHandle, &WindowRect);

		_itoa_s(WindowRect.top, Buffer, sizeof(Buffer), 10);
		Setter->operator()(kDefaultINISettings[kDefaultINISetting_Top].Key, Section, Buffer);
		_itoa_s(WindowRect.left, Buffer, sizeof(Buffer), 10);
		Setter->operator()(kDefaultINISettings[kDefaultINISetting_Left].Key, Section, Buffer);
		_itoa_s(WindowRect.right - WindowRect.left, Buffer, sizeof(Buffer), 10);
		Setter->operator()(kDefaultINISettings[kDefaultINISetting_Right].Key, Section, Buffer);
		_itoa_s(WindowRect.bottom - WindowRect.top, Buffer, sizeof(Buffer), 10);
		Setter->operator()(kDefaultINISettings[kDefaultINISetting_Bottom].Key, Section, Buffer);

		Setter->operator()(kDefaultINISettings[kDefaultINISetting_Visible].Key, Section, ((Visible)?"1":"0"));
	}

	bool BGSEEGenericModelessDialog::SetVisibility( bool State )
	{
		if (State)
			ShowWindow(DialogHandle, SW_SHOWNA);
		else
			ShowWindow(DialogHandle, SW_SHOWNA);

		Visible = State;
		return Visible;
	}

	bool BGSEEGenericModelessDialog::SetTopmost( bool State )
	{
		if (State)
			SetWindowPos(DialogHandle, HWND_TOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
		else
			SetWindowPos(DialogHandle, HWND_NOTOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);

		Topmost = State;
		return Topmost;
	}

	BGSEEGenericModelessDialog::BGSEEGenericModelessDialog( HWND Parent,
															HINSTANCE Resource,
															UInt32 DialogTemplate, UInt32 ContextMenuTemplate,
															MessageCallback CallbackProc,
															float AspectRatio /*= 0.0f*/ )
	{
		SME_ASSERT(Parent && Resource && DialogTemplate && ContextMenuTemplate && CallbackProc);

		DialogHandle = NULL;
		ContextMenuHandle = NULL;
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

	BGSEEGenericModelessDialog::BGSEEGenericModelessDialog()
	{
		DialogHandle = NULL;
		ContextMenuHandle = NULL;
		ParentHandle = NULL;
		ResourceInstance = NULL;
		DialogTemplateID = NULL;
		DialogContextMenuID = NULL;
		CallbackDlgProc = NULL;
		Visible = false;
		Topmost = false;
		AspectRatio = 0.0f;
		LockAspectRatio = false;
	}

	BGSEEGenericModelessDialog::~BGSEEGenericModelessDialog()
	{
		if (DialogHandle == NULL || ContextMenuHandle == NULL)
			return;

		BGSEEUI->GetWindowHandleCollection(BGSEEUIManager::kHandleCollection_MainWindowChildren)->Remove(DialogHandle);

		DestroyMenu(ContextMenuHandle);
		DestroyWindow(DialogHandle);
	}

	void BGSEEGenericModelessDialog::Create( LPARAM InitParam, bool Hide )
	{
		if (DialogHandle || ContextMenuHandle || CallbackDlgProc == NULL)
			return;

		DlgUserData* UserData = new DlgUserData();
		UserData->Instance = const_cast<BGSEEGenericModelessDialog*>(this);
		UserData->UserData = InitParam;
		DialogHandle = BGSEEUI->ModelessDialog(ResourceInstance, MAKEINTRESOURCE(DialogTemplateID), ParentHandle, (DLGPROC)DefaultDlgProc, (LPARAM)UserData);
		ContextMenuHandle = LoadMenu(ResourceInstance, (LPSTR)DialogContextMenuID);
		SME_ASSERT(DialogHandle && ContextMenuHandle);

		BGSEEUI->GetWindowHandleCollection(BGSEEUIManager::kHandleCollection_MainWindowChildren)->Add(DialogHandle);
		SetVisibility((!Hide));
	}

	bool BGSEEGenericModelessDialog::ToggleVisibility( void )
	{
		return SetVisibility((Visible == false));
	}

	bool BGSEEGenericModelessDialog::ToggleTopmost( void )
	{
		return SetTopmost((Topmost == false));
	}

	bool BGSEEGenericModelessDialog::GetVisible( void ) const
	{
		return Visible;
	}

	bool BGSEEGenericModelessDialog::GetTopmost( void ) const
	{
		return Topmost;
	}

	bool BGSEEGenericModelessDialog::GetInitialized( void ) const
	{
		return (DialogHandle != NULL);
	}
}