#include "UIManager.h"
#include "Console.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace bgsee
{
#define WM_SUBCLASSER_RELEASE		((WM_USER) + 0x100)

	WindowHandleCollection::HandleCollectionT::iterator WindowHandleCollection::Find( HWND Handle )
	{
		for (HandleCollectionT::iterator Itr = HandleList.begin(); Itr != HandleList.end(); Itr++)
		{
			if (*Itr == Handle)
				return Itr;
		}

		return HandleList.end();
	}

	bool WindowHandleCollection::Add( HWND Handle )
	{
		if (GetExists(Handle) == false)
		{
			HandleList.push_back(Handle);
			return true;
		}
		else
			return false;
	}

	bool WindowHandleCollection::Remove( HWND Handle )
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

	bool WindowHandleCollection::GetExists( HWND Handle )
	{
		return Find(Handle) != HandleList.end();
	}

	void WindowHandleCollection::Clear( void )
	{
		HandleList.clear();
	}

	WindowHandleCollection::WindowHandleCollection() :
		HandleList()
	{
		;//
	}

	WindowHandleCollection::~WindowHandleCollection()
	{
		Clear();
	}

	void WindowHandleCollection::SendMessage( UINT Msg, WPARAM wParam, LPARAM lParam )
	{
		// need to operate on a buffer as the handle list can be modified inside a subclass callback
		HandleCollectionT Buffer(HandleList);

		for (HandleCollectionT::iterator Itr = Buffer.begin(); Itr != Buffer.end(); Itr++)
			::SendMessage(*Itr, Msg, wParam, lParam);
	}

	int				WindowExtraData::GIC = 0;

	WindowExtraData::WindowExtraData(WindowExtraDataIDT Type) :
		TypeID(Type)
	{
		GIC++;
	}

	WindowExtraData::~WindowExtraData()
	{
		GIC--;
		SME_ASSERT(GIC >= 0);
	}

	const WindowExtraDataIDT WindowExtraData::GetTypeID(void) const
	{
		return TypeID;
	}

	WindowExtraDataCollection::WindowExtraDataCollection() :
		DataStore()
	{
		;//
	}

	WindowExtraDataCollection::~WindowExtraDataCollection()
	{
		DataStore.clear();
	}

	bool WindowExtraDataCollection::Add( WindowExtraData* Data )
	{
		SME_ASSERT(Data);

		if (Lookup(Data->GetTypeID()))
			return false;
		else
			DataStore.insert(std::make_pair(Data->GetTypeID(), Data));

		return true;
	}

	bool WindowExtraDataCollection::Remove( WindowExtraDataIDT ID )
	{
		if (Lookup(ID) == NULL)
			return false;
		else
			DataStore.erase(ID);

		return true;
	}

	WindowExtraData* WindowExtraDataCollection::Lookup( WindowExtraDataIDT ID )
	{
		for (ExtraDataMapT::iterator Itr = DataStore.begin(); Itr != DataStore.end(); Itr++)
		{
			if (Itr->first == ID)
				return Itr->second;
		}

		return NULL;
	}

	WindowSubclasser::DialogSubclassData::DialogSubclassData() :
		Original(NULL),
		ActiveHandles(),
		Subclasses()
	{
		;//
	}

	INT_PTR WindowSubclasser::DialogSubclassData::ProcessSubclasses( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return )
	{
		INT_PTR Result = FALSE;
		bool ReturnMark = Return;

		// work on a buffer as the subclass list can change inside a callback
		// since the number of subclasses for any dialog isn't gonna be very large at any given time, the performance hit should be negligible
		SubclassProcArrayT SubclassBuffer(Subclasses);

		for (SubclassProcArrayT::iterator Itr = SubclassBuffer.begin(); Itr != SubclassBuffer.end(); Itr++)
		{
			DialogSubclassUserData* UserData = (DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
			if (UserData == NULL)
			{
				// the dialog was destroyed inside a callback
				break;
			}

			INT_PTR CurrentResult = (INT_PTR)(*Itr)(hWnd, uMsg, wParam, lParam, ReturnMark, &UserData->ExtraData);

			if (ReturnMark && Return == false)
			{
				Result = CurrentResult;
				Return = true;
			}
		}

		return Result;
	}

	WindowSubclasser::WindowSubclassData::WindowSubclassData() :
		Original(NULL),
		Subclasses(),
		UserData(NULL)
	{
		;//
	}

	LRESULT WindowSubclasser::WindowSubclassData::ProcessSubclasses( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return )
	{
		LRESULT Result = FALSE;
		bool ReturnMark = Return;

		// work on a buffer, same as before
		SubclassProcArrayT SubclassBuffer(Subclasses);

		for (SubclassProcArrayT::iterator Itr = SubclassBuffer.begin(); Itr != SubclassBuffer.end(); Itr++)
		{
			WindowSubclassUserData* UserData = (WindowSubclassUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
			if (UserData == NULL)
				break;

			LRESULT CurrentResult = (*Itr)(hWnd, uMsg, wParam, lParam, ReturnMark, &UserData->ExtraData);

			if (ReturnMark && Return == false)
			{
				Result = CurrentResult;
				Return = true;
			}
		}

		return Result;
	}

	WindowSubclasser::DialogSubclassUserData::DialogSubclassUserData() :
		Instance(NULL),
		Data(NULL),
		InitParam(NULL),
		ExtraData(),
		TemplateID(NULL),
		Initialized(false)
	{
		;//
	}

	WindowSubclasser::WindowSubclassUserData::WindowSubclassUserData() :
		Instance(NULL),
		Data(NULL),
		OriginalUserData(NULL),
		ExtraData()
	{
		;//
	}

	INT_PTR CALLBACK WindowSubclasser::DialogSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		bool CallbackReturn = false;
		INT_PTR DlgProcResult = FALSE;
		DialogSubclassUserData* UserData = (DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);

		switch (uMsg)
		{
		case WM_INITDIALOG:
			{
				if (lParam)
				{
					SetWindowLongPtr(hWnd, DWL_USER, (LONG_PTR)lParam);
					UserData = (DialogSubclassUserData*)lParam;

					if (UserData->Initialized == false)
					{
						bool AddResult = UserData->Data->ActiveHandles.Add(hWnd);
						SME_ASSERT(AddResult);
					}

					UserData->Initialized = true;
				}

				// send the pre-initdialog message first
				UserData->Data->ProcessSubclasses(hWnd, WM_SUBCLASSER_PREDIALOGINIT, wParam, UserData->InitParam, CallbackReturn);

				// make sure the dialog's not destroyed
				UserData = (DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
				SME_ASSERT(UserData);

				// send the initdialog message to the original handler
				DlgProcResult = UserData->Data->Original(hWnd, uMsg, wParam, UserData->InitParam);

				// re-check the userdata as the dialog can get destroyed inside the original WM_INITDIALOG callback
				UserData = (DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
				if (UserData)
				{
					UserData->Data->ProcessSubclasses(hWnd, uMsg, wParam, UserData->InitParam, CallbackReturn);
				}

				return DlgProcResult;
			}

			break;
		case WM_SUBCLASSER_RELEASE:
		case WM_DESTROY:
			{
				UserData->Data->ProcessSubclasses(hWnd, WM_DESTROY, wParam, lParam, CallbackReturn);
				if (uMsg != WM_SUBCLASSER_RELEASE)
					DlgProcResult = UserData->Data->Original(hWnd, uMsg, wParam, lParam);

				bool RemoveResult = UserData->Data->ActiveHandles.Remove(hWnd);
				SME_ASSERT(RemoveResult);

				SetWindowLongPtr(hWnd, DWL_USER, NULL);
				SetWindowLongPtr(hWnd, GWL_WNDPROC, (LONG_PTR)UserData->Data->Original);

				delete UserData;
				return DlgProcResult;
			}

			break;
		}

		if (UserData && UserData->Initialized)
		{
			INT_PTR CallbackResult = UserData->Data->ProcessSubclasses(hWnd, uMsg, wParam, lParam, CallbackReturn);

			if (CallbackReturn)
				return CallbackResult;
		}

		if (UserData)
			return UserData->Data->Original(hWnd, uMsg, wParam, lParam);
		else
			return DlgProcResult;
	}

	LRESULT CALLBACK WindowSubclasser::RegularWindowSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		// directly accessing the subclasser singleton as we'll lose userdata b'ween CallWindowProc calls
		WindowSubclassUserData* UserData = BGSEEUI->GetSubclasser()->RegularWindowSubclasses[hWnd].UserData;

		bool CallbackReturn = false;
		LRESULT CallbackResult = FALSE;

		switch (uMsg)
		{
		case WM_SUBCLASSER_RELEASE:
		case WM_DESTROY:
			{
				UserData->Data->ProcessSubclasses(hWnd, WM_DESTROY, wParam, lParam, CallbackReturn);
				SetWindowLongPtr(hWnd, GWL_USERDATA, UserData->OriginalUserData);

				if (uMsg == WM_DESTROY)
				{
					CallbackResult = CallWindowProc(UserData->Data->Original, hWnd, uMsg, wParam, lParam);
				}

				SetWindowLongPtr(hWnd, GWL_WNDPROC, (LONG_PTR)UserData->Data->Original);

				delete UserData;
				return CallbackResult;
			}

			break;
		}

		CallbackResult = UserData->Data->ProcessSubclasses(hWnd, uMsg, wParam, lParam, CallbackReturn);
		if (CallbackReturn == false)
		{
			if (UserData->OriginalUserData)
				SetWindowLongPtr(hWnd, GWL_USERDATA, UserData->OriginalUserData);

			CallbackResult = CallWindowProc(UserData->Data->Original, hWnd, uMsg, wParam, lParam);

			if (UserData->OriginalUserData)
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)UserData);
		}

		return CallbackResult;
	}

	bool WindowSubclasser::GetShouldSubclassDialog( ResourceTemplateT TemplateID,
												LPARAM InitParam,
												DLGPROC OriginalProc,
												DLGPROC& OutSubclassProc,
												DialogSubclassUserData** OutSubclassUserData )
	{
		SME_ASSERT(OutSubclassUserData);

		DialogSubclassMapT::iterator Match = DialogSubclasses.find(TemplateID);
		if (Match != DialogSubclasses.end())
		{
			OutSubclassProc = DialogSubclassProc;
			(*OutSubclassUserData) = new DialogSubclassUserData();
			(*OutSubclassUserData)->Instance = this;
			(*OutSubclassUserData)->Data = &Match->second;
			(*OutSubclassUserData)->Data->Original = OriginalProc;
			(*OutSubclassUserData)->InitParam = InitParam;
			(*OutSubclassUserData)->TemplateID = TemplateID;

			return true;
		}
		else
			return false;
	}

	void WindowSubclasser::HandleMainWindowInit( HWND MainWindow )
	{
		EditorMainWindow = MainWindow;

		SME_ASSERT(EditorMainWindow);
	}

	WindowSubclasser::WindowSubclasser() :
		EditorMainWindow(NULL),
		DialogSubclasses(),
		RegularWindowSubclasses()
	{
		;//
	}

	WindowSubclasser::~WindowSubclasser()
	{
		for (DialogSubclassMapT::iterator Itr = DialogSubclasses.begin(); Itr != DialogSubclasses.end(); Itr++)
			Itr->second.ActiveHandles.SendMessage(WM_SUBCLASSER_RELEASE, NULL, NULL);

		DialogSubclasses.clear();

		for (WindowSubclassMapT::iterator Itr = RegularWindowSubclasses.begin(); Itr != RegularWindowSubclasses.end(); Itr++)
			SendMessage(Itr->first, WM_SUBCLASSER_RELEASE, NULL, NULL);

		RegularWindowSubclasses.clear();

		bool Leakage = false;
		if (WindowExtraData::GIC)
		{
			BGSEECONSOLE_MESSAGE("BGSEEWindowSubclasser::D'tor - Session leaked %d instances of BGSEEWindowExtraData!", WindowExtraData::GIC);
			Leakage = true;
		}

		if (Leakage)
			SHOW_LEAKAGE_MESSAGE("BGSEEWindowSubclasser");
	}

	bool WindowSubclasser::RegisterMainWindowSubclass( SubclassProc Proc )
	{
		return RegisterRegularWindowSubclass(EditorMainWindow, Proc);
	}

	bool WindowSubclasser::UnregisterMainWindowSubclass( SubclassProc Proc )
	{
		return UnregisterRegularWindowSubclass(EditorMainWindow, Proc);
	}

	bool WindowSubclasser::RegisterDialogSubclass( ResourceTemplateT TemplateID, SubclassProc Proc )
	{
		DialogSubclassMapT::iterator Match = DialogSubclasses.find(TemplateID);
		if (Match != DialogSubclasses.end())
		{
			for (SubclassProcArrayT::iterator Itr = Match->second.Subclasses.begin(); Itr != Match->second.Subclasses.end(); Itr++)
			{
				if (*Itr == Proc)
					return false;
			}

			Match->second.Subclasses.push_back(Proc);
		}
		else
		{
			DialogSubclasses.insert(std::make_pair(TemplateID, DialogSubclassData()));
			DialogSubclasses[TemplateID].Subclasses.push_back(Proc);
		}

		return true;
	}

	bool WindowSubclasser::UnregisterDialogSubclass( ResourceTemplateT TemplateID, SubclassProc Proc )
	{
		DialogSubclassMapT::iterator Match = DialogSubclasses.find(TemplateID);
		if (Match != DialogSubclasses.end())
		{
			for (SubclassProcArrayT::iterator Itr = Match->second.Subclasses.begin(); Itr != Match->second.Subclasses.end(); Itr++)
			{
				if (*Itr == Proc)
				{
					Match->second.Subclasses.erase(Itr);
					return true;
				}
			}
		}

		return false;
	}

	bool WindowSubclasser::GetHasDialogSubclass( ResourceTemplateT TemplateID ) const
	{
		DialogSubclassMapT::const_iterator Match = DialogSubclasses.find(TemplateID);
		if (Match != DialogSubclasses.end())
			return true;
		else
			return false;
	}

	bool WindowSubclasser::RegisterRegularWindowSubclass( HWND Handle, SubclassProc Proc )
	{
		WindowSubclassMapT::iterator Match = RegularWindowSubclasses.find(Handle);
		if (Match != RegularWindowSubclasses.end())
		{
			for (SubclassProcArrayT::iterator Itr = Match->second.Subclasses.begin(); Itr != Match->second.Subclasses.end(); Itr++)
			{
				if (*Itr == Proc)
					return false;
			}

			Match->second.Subclasses.push_back(Proc);
		}
		else
		{
			RegularWindowSubclasses.insert(std::make_pair(Handle, WindowSubclassData()));
			RegularWindowSubclasses[Handle].Subclasses.push_back(Proc);

			WindowSubclassUserData* UserData = new WindowSubclassUserData();
			UserData->Instance = this;
			UserData->Data = &RegularWindowSubclasses[Handle];
			UserData->OriginalUserData = GetWindowLongPtr(Handle, GWL_USERDATA);

			SetWindowLongPtr(Handle, GWL_USERDATA, (LONG_PTR)UserData);
			RegularWindowSubclasses[Handle].Original = (WNDPROC)SetWindowLongPtr(Handle, GWL_WNDPROC, (LONG_PTR)RegularWindowSubclassProc);
			RegularWindowSubclasses[Handle].UserData = UserData;
		}

		return true;
	}

	bool WindowSubclasser::UnregisterRegularWindowSubclass( HWND Handle, SubclassProc Proc )
	{
		bool Result = false;

		WindowSubclassMapT::iterator Match = RegularWindowSubclasses.find(Handle);
		if (Match != RegularWindowSubclasses.end())
		{
			for (SubclassProcArrayT::iterator Itr = Match->second.Subclasses.begin(); Itr != Match->second.Subclasses.end(); Itr++)
			{
				if (*Itr == Proc)
				{
					Match->second.Subclasses.erase(Itr);
					Result = true;
					break;
				}
			}

			if (Match->second.Subclasses.size() == 0)		// remove the subclass and reset the wndproc
			{
				SendMessage(Match->first, WM_SUBCLASSER_RELEASE, NULL, NULL);
				RegularWindowSubclasses.erase(Match);
			}
		}

		return Result;
	}

	ResourceTemplateT WindowSubclasser::GetDialogTemplate(HWND SubclassedDialog) const
	{
		DialogSubclassUserData* UserData = (DialogSubclassUserData*)GetWindowLongPtr(SubclassedDialog, DWL_USER);
		SME_ASSERT(UserData);

		return UserData->TemplateID;
	}

	INT_PTR WindowSubclasser::TunnelDialogMessage(HWND SubclassedDialog, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DialogSubclassUserData* UserData = (DialogSubclassUserData*)GetWindowLongPtr(SubclassedDialog, DWL_USER);
		SME_ASSERT(UserData);

		return UserData->Data->Original(SubclassedDialog, uMsg, wParam, lParam);
	}

	void ResourceTemplateHotSwapper::PopulateTemplateMap( void )
	{
		for (IDirectoryIterator Itr(SourceDepot().c_str(), "*.dll"); !Itr.Done(); Itr.Next())
		{
			std::string FileName = Itr.Get()->cFileName;
			std::string FullPath = SourceDepot() + "\\" + FileName;

			HINSTANCE Module = (HINSTANCE)LoadLibraryEx(FullPath.c_str(), NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE|LOAD_LIBRARY_AS_DATAFILE);
			if (Module == NULL)
			{
				BGSEECONSOLE_ERROR("BGSEEResourceTemplateHotSwapper::PopulateTemplateMap - Failed to load resource library '%s'!", FullPath.c_str());
				continue;
			}

			int Index = FileName.rfind(".");
			ResourceTemplateT TemplateID = 0;
			SME_ASSERT(Index != -1);

			TemplateID = atoi((FileName.substr(0, Index)).c_str());
			SME_ASSERT(TemplateMap.count(TemplateID) == 0);

			TemplateMap[TemplateID] = Module;
		}
	}

	void ResourceTemplateHotSwapper::ReleaseTemplateMap( void )
	{
		for (TemplateResourceInstanceMapT::iterator Itr = TemplateMap.begin(); Itr != TemplateMap.end(); Itr++)
		{
			if (FreeLibrary(Itr->second) == NULL)
				BGSEECONSOLE_ERROR("BGSEEResourceTemplateHotSwapper::ReleaseTemplateMap - Failed to release resource library for template %d", Itr->first);
		}

		TemplateMap.clear();
	}

	HINSTANCE ResourceTemplateHotSwapper::GetAlternateResourceInstance( ResourceTemplateT TemplateID )
	{
		TemplateResourceInstanceMapT::iterator Match = TemplateMap.find(TemplateID);

		if (Match != TemplateMap.end())
			return Match->second;
		else
			return NULL;
	}

	ResourceTemplateHotSwapper::ResourceTemplateHotSwapper( std::string SourcePath ) :
		SourceDepot(SourcePath),
		TemplateMap()
	{
		;//
	}

	ResourceTemplateHotSwapper::~ResourceTemplateHotSwapper()
	{
		;//
	}

	const std::string					DialogTemplateHotSwapper::kDefaultLoc = "Dialog Templates";

	DialogTemplateHotSwapper::DialogTemplateHotSwapper() :
		ResourceTemplateHotSwapper(kDefaultLoc)
	{
		PopulateTemplateMap();
	}

	DialogTemplateHotSwapper::~DialogTemplateHotSwapper()
	{
		ReleaseTemplateMap();
	}

	MenuTemplateHotSwapper::MenuTemplateHotSwapper() :
		ResourceTemplateHotSwapper("")
	{
		;//
	}

	MenuTemplateHotSwapper::~MenuTemplateHotSwapper()
	{
		;//
	}

	bool MenuTemplateHotSwapper::RegisterTemplateReplacer( ResourceTemplateT TemplateID, HINSTANCE Replacer )
	{
		if (TemplateMap.count(TemplateID))
			return false;
		else
			TemplateMap[TemplateID] = Replacer;

		return true;
	}

	bool MenuTemplateHotSwapper::UnregisterTemplateReplacer( ResourceTemplateT TemplateID )
	{
		if (TemplateMap.count(TemplateID))
		{
			TemplateMap.erase(TemplateMap.find(TemplateID));
			return true;
		}
		else
			return false;
	}

	bool WindowStyler::StyleWindow( HWND Window, ResourceTemplateT Template )
	{
		TemplateStyleMapT::iterator Match = StyleListings.find(Template);

		if (Match != StyleListings.end())
		{
			StyleWindow(Window, Match->second);
			return true;
		}

		return false;
	}

	void WindowStyler::StyleWindow(HWND Window, StyleData& Data)
	{
		LONG_PTR WindowRegular = GetWindowLongPtr(Window, GWL_STYLE);
		LONG_PTR WindowExtended = GetWindowLongPtr(Window, GWL_EXSTYLE);

		switch (Data.RegularOp)
		{
		case StyleData::kOperation_OR:
			WindowRegular |= Data.Regular;
			break;
		case StyleData::kOperation_AND:
			WindowRegular &= ~Data.Regular;
			break;
		case StyleData::kOperation_Replace:
			WindowRegular = Data.Regular;
			break;
		}

		switch (Data.ExtendedOp)
		{
		case StyleData::kOperation_OR:
			WindowExtended |= Data.Extended;
			break;
		case StyleData::kOperation_AND:
			WindowExtended &= ~Data.Extended;
			break;
		case StyleData::kOperation_Replace:
			WindowExtended = Data.Extended;
			break;
		}

		bool PerformOperation = Data.RegularOp != StyleData::kOperation_None || Data.ExtendedOp != StyleData::kOperation_None;

		if (PerformOperation)
		{
			ShowWindow(Window, SW_HIDE);
		}

		if (Data.RegularOp != StyleData::kOperation_None)
			SetWindowLongPtr(Window, GWL_STYLE, (LONG_PTR)WindowRegular);

		if (Data.ExtendedOp != StyleData::kOperation_None)
			SetWindowLongPtr(Window, GWL_EXSTYLE, (LONG_PTR)WindowExtended);

		if (PerformOperation)
		{
			SetWindowPos(Window, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_DRAWFRAME | SWP_SHOWWINDOW);
			InvalidateRect(Window, NULL, TRUE);
		}
	}

	WindowStyler::WindowStyler() :
		StyleListings()
	{
		;//
	}

	WindowStyler::~WindowStyler()
	{
		StyleListings.clear();
	}

	bool WindowStyler::RegisterStyle( ResourceTemplateT TemplateID, StyleData& Data )
	{
		if (StyleListings.count(TemplateID))
			return false;
		else
			StyleListings[TemplateID] = Data;

		return true;
	}

	bool WindowStyler::UnregisterStyle( ResourceTemplateT TemplateID )
	{
		if (StyleListings.count(TemplateID))
		{
			StyleListings.erase(StyleListings.find(TemplateID));
			return true;
		}
		else
			return false;
	}

	WindowInvalidationManager::WindowInvalidationManager() :
		ActiveInvalidatedWindows()
	{
		;//
	}

	WindowInvalidationManager::~WindowInvalidationManager()
	{
		for (InvalidationMapT::iterator Itr = ActiveInvalidatedWindows.begin(); Itr != ActiveInvalidatedWindows.end(); Itr++)
			Invalidate(Itr->first, false);

		ActiveInvalidatedWindows.clear();
	}

	void WindowInvalidationManager::Push( HWND Window )
	{
		SME_ASSERT(Window);

		if (ActiveInvalidatedWindows.count(Window) == 0)
		{
			ActiveInvalidatedWindows.insert(std::make_pair(Window, 1));
			Invalidate(Window, true);
		}
		else
			ActiveInvalidatedWindows[Window] += 1;
	}

	void WindowInvalidationManager::Pop( HWND Window )
	{
		SME_ASSERT(Window);
		SME_ASSERT(ActiveInvalidatedWindows.count(Window));

		UInt32 RefCount = ActiveInvalidatedWindows[Window];
		SME_ASSERT(RefCount);

		if (RefCount == 1)
		{
			Invalidate(Window, false);
			ActiveInvalidatedWindows.erase(Window);
		}
		else
			ActiveInvalidatedWindows[Window] -= 1;
	}

	void WindowInvalidationManager::Invalidate( HWND Window, bool State )
	{
		SME_ASSERT(Window);

		if (State)
		{
			SendMessage(Window, WM_SETREDRAW, FALSE, NULL);
		}
		else
		{
			SendMessage(Window, WM_SETREDRAW, TRUE, NULL);
			Redraw(Window);
		}
	}

	void WindowInvalidationManager::Redraw(HWND Window)
	{
		RedrawWindow(Window, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}

	UIManager*						UIManager::Singleton = NULL;
	SME::UIHelpers::CSnapWindow			UIManager::WindowEdgeSnapper;

	UIManager::IATPatchData::IATPatchData() :
		DLL(NULL),
		Import(NULL),
		Location(NULL),
		OriginalFunction(NULL),
		CallbackFunction(NULL),
		Replaced(false)
	{
		;//
	}

	void UIManager::IATPatchData::Replace( void )
	{
		if (Replaced)
			return;

		SME::MemoryHandler::SafeWrite32((UInt32)Location, (UInt32)CallbackFunction);
		Replaced = true;
	}

	void UIManager::IATPatchData::Reset( void )
	{
		if (Replaced == false)
			return;

		SME::MemoryHandler::SafeWrite32((UInt32)Location, (UInt32)OriginalFunction);
		Replaced = true;
	}

	HWND CALLBACK UIManager::CallbackCreateWindowExA( DWORD dwExStyle,
														LPCSTR lpClassName,
														LPCSTR lpWindowName,
														DWORD dwStyle,
														int X, int Y,
														int nWidth, int nHeight,
														HWND hWndParent,
														HMENU hMenu,
														HINSTANCE hInstance,
														LPVOID lpParam )
	{
		SME_ASSERT(BGSEEUI->Initialized);

		bool EditorWindow = false;
		if (!_stricmp(lpClassName, BGSEEUI->EditorWindowClassName.c_str()))
		{
			EditorWindow = true;

			if (BGSEEUI->EditorMainMenuReplacement)
				hMenu = BGSEEUI->EditorMainMenuReplacement;
		}

		HWND Result = ((_CallbackCreateWindowExA)(BGSEEUI->PatchDepot[kIATPatch_CreateWindowEx].OriginalFunction))
						(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

		if (EditorWindow)
		{
			BGSEEUI->EditorWindowHandle = new HWNDGetter(Result);
			BGSEEUI->EditorResourceInstance = new HINSTANCEGetter(hInstance);
			BGSEEUI->Subclasser->HandleMainWindowInit(Result);
			BGSEEUI->PatchDepot[kIATPatch_CreateWindowEx].Reset();

			BGSEEMAIN->GetDaemon()->ExecuteInitCallbacks(Daemon::kInitCallback_PostMainWindowInit);

			SendMessage(Result, WM_INITDIALOG, NULL, NULL);
		}

		return Result;
	}

	HMENU CALLBACK UIManager::CallbackLoadMenuA( HINSTANCE hInstance, LPCSTR lpMenuName )
	{
		SME_ASSERT(BGSEEUI->MenuHotSwapper);

		if (hInstance != BGSEEUI->EditorResourceInstance->operator()())
			return ((_CallbackLoadMenuA)(BGSEEUI->PatchDepot[kIATPatch_LoadMenu].OriginalFunction))(hInstance, lpMenuName);

		HINSTANCE Alternate = BGSEEUI->MenuHotSwapper->GetAlternateResourceInstance((UInt32)lpMenuName);
		if (Alternate)
			hInstance = Alternate;

		return ((_CallbackLoadMenuA)(BGSEEUI->PatchDepot[kIATPatch_LoadMenu].OriginalFunction))(hInstance, lpMenuName);
	}

	HWND CALLBACK UIManager::CallbackCreateDialogParamA( HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam )
	{
		SME_ASSERT(BGSEEUI->Subclasser && BGSEEUI->DialogHotSwapper);

		DLGPROC Replacement = NULL;
		WindowSubclasser::DialogSubclassUserData* UserData = NULL;
		HINSTANCE Alternate = BGSEEUI->DialogHotSwapper->GetAlternateResourceInstance((UInt32)lpTemplateName);

		if (Alternate)
			hInstance = Alternate;

		if (BGSEEUI->Subclasser->GetShouldSubclassDialog((UInt32)lpTemplateName, dwInitParam, lpDialogFunc, Replacement, &UserData))
		{
			lpDialogFunc = Replacement;
			dwInitParam = (LPARAM)UserData;
		}

		HWND Result = ((_CallbackCreateDialogParamA)(BGSEEUI->PatchDepot[kIATPatch_CreateDialogParam].OriginalFunction))
												(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);

		if (Result)
		{
			BGSEEUI->Styler->StyleWindow(Result, (UInt32)lpTemplateName);
		}

		return Result;
	}

	INT_PTR CALLBACK UIManager::CallbackDialogBoxParamA( HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam )
	{
		SME_ASSERT(BGSEEUI->Subclasser && BGSEEUI->DialogHotSwapper);
		SME_ASSERT(hInstance == BGSEEUI->EditorResourceInstance->operator()());

		DLGPROC Replacement = NULL;
		WindowSubclasser::DialogSubclassUserData* UserData = NULL;
		HINSTANCE Alternate = BGSEEUI->DialogHotSwapper->GetAlternateResourceInstance((ResourceTemplateT)lpTemplateName);

		if (Alternate)
			hInstance = Alternate;

		if (BGSEEUI->Subclasser->GetShouldSubclassDialog((ResourceTemplateT)lpTemplateName, dwInitParam, lpDialogFunc, Replacement, &UserData))
		{
			lpDialogFunc = Replacement;
			dwInitParam = (LPARAM)UserData;
		}

		return ((_CallbackDialogBoxParamA)(BGSEEUI->PatchDepot[kIATPatch_DialogBoxParam].OriginalFunction))
										(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	}

	BOOL CALLBACK UIManager::EnumThreadWindowsCallback( HWND hwnd, LPARAM lParam )
	{
		if (lParam)
			EnableWindow(hwnd, TRUE);
		else
			EnableWindow(hwnd, FALSE);

		return TRUE;
	}

	void UIManager::PatchIAT( UInt8 PatchType, void* Callback )
	{
		const char* DLLName = "USER32.DLL";
		const char* ImportName = NULL;

		SME_ASSERT(PatchType < kIATPatch__MAX);

		switch (PatchType)
		{
		case kIATPatch_LoadMenu:
			ImportName = "LoadMenuA";
			break;
		case kIATPatch_CreateDialogParam:
			ImportName = "CreateDialogParamA";
			break;
		case kIATPatch_DialogBoxParam:
			ImportName = "DialogBoxParamA";
			break;
		case kIATPatch_CreateWindowEx:
			ImportName = "CreateWindowExA";
			break;
		}

		IATPatchData* Patch = &PatchDepot[PatchType];
		Patch->DLL = DLLName;
		Patch->Import = ImportName;
		Patch->Location = NULL;

		UInt8* Base = (UInt8*)GetModuleHandle(NULL);
		IMAGE_DOS_HEADER* DOSHeader = (IMAGE_DOS_HEADER*)Base;
		IMAGE_NT_HEADERS* NTHeader = (IMAGE_NT_HEADERS*)(Base + DOSHeader->e_lfanew);

		IMAGE_IMPORT_DESCRIPTOR* IAT = (IMAGE_IMPORT_DESCRIPTOR*)(Base + NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		for(; IAT->Characteristics && Patch->Location == NULL; ++IAT)
		{
			if(!_stricmp(DLLName, (const char*)(Base + IAT->Name)))
			{
				IMAGE_THUNK_DATA* ThunkData = (IMAGE_THUNK_DATA*)(Base + IAT->OriginalFirstThunk);
				UInt32* DLLIAT = (UInt32*)(Base + IAT->FirstThunk);

				for(; ThunkData->u1.Ordinal; ++ThunkData, ++DLLIAT)
				{
					if(!IMAGE_SNAP_BY_ORDINAL(ThunkData->u1.Ordinal))
					{
						IMAGE_IMPORT_BY_NAME* ImportInfo = (IMAGE_IMPORT_BY_NAME*)(Base + ThunkData->u1.AddressOfData);

						if(!_stricmp((char *)ImportInfo->Name, ImportName))
						{
							Patch->Location = DLLIAT;
							break;
						}
					}
				}
			}
		}

		SME_ASSERT(Patch->Location);

		Patch->OriginalFunction = *((void**)Patch->Location);
		Patch->CallbackFunction = Callback;
		Patch->Replaced = false;

		Patch->Replace();
	}

	UIManager::UIManager() :
		OwnerThreadID(0),
		EditorWindowClassName(""),
		EditorWindowHandle(NULL),
		EditorResourceInstance(NULL),
		EditorMainMenuReplacement(NULL),
		Subclasser(NULL),
		DialogHotSwapper(NULL),
		MenuHotSwapper(NULL),
		Styler(NULL),
		InvalidationManager(NULL),
		Initialized(false)
	{
		OwnerThreadID = GetCurrentThreadId();
	}

	UIManager::~UIManager()
	{
		for (int i = 0; i < kHandleCollection__MAX; i++)
			HandleCollections[i].Clear();

		for (int i = 0; i < kIATPatch__MAX; i++)
			PatchDepot[i].Reset();

		SAFEDELETE(Subclasser);
		SAFEDELETE(DialogHotSwapper);
		SAFEDELETE(MenuHotSwapper);
		SAFEDELETE(EditorWindowHandle);
		SAFEDELETE(EditorResourceInstance);
		SAFEDELETE(Styler);
		SAFEDELETE(InvalidationManager);

		Initialized = false;

		Singleton = NULL;
	}

	UIManager* UIManager::GetSingleton()
	{
		if (Singleton == NULL)
			Singleton = new UIManager();

		return Singleton;
	}

	bool UIManager::Initialize( const char* MainWindowClassName, HMENU MainMenuHandle )
	{
		if (Initialized)
			return false;

		Initialized = true;

		EditorWindowClassName = MainWindowClassName;
		EditorMainMenuReplacement = MainMenuHandle;
		Subclasser = new WindowSubclasser();
		DialogHotSwapper = new DialogTemplateHotSwapper();
		MenuHotSwapper = new MenuTemplateHotSwapper();
		Styler = new WindowStyler();
		InvalidationManager = new WindowInvalidationManager();

		PatchIAT(kIATPatch_CreateWindowEx, CallbackCreateWindowExA);
		PatchIAT(kIATPatch_CreateDialogParam, CallbackCreateDialogParamA);
		PatchIAT(kIATPatch_DialogBoxParam, CallbackDialogBoxParamA);
		PatchIAT(kIATPatch_LoadMenu, CallbackLoadMenuA);

		return Initialized;
	}

	WindowHandleCollection* UIManager::GetWindowHandleCollection( UInt8 ID )
	{
		SME_ASSERT(ID < kHandleCollection__MAX);
		return &HandleCollections[ID];
	}

	HWND UIManager::GetMainWindow( void ) const
	{
		SME_ASSERT(EditorWindowHandle);
		return EditorWindowHandle->operator()();
	}

	int UIManager::MsgBoxI( HWND Parent, UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Information - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(Parent, Buffer, Title, Flags|MB_ICONINFORMATION);
	}

	int UIManager::MsgBoxI( UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Information - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(GetMainWindow(), Buffer, Title, Flags|MB_ICONINFORMATION);
	}

	int UIManager::MsgBoxI( const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Information - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(GetMainWindow(), Buffer, Title, MB_OK|MB_ICONINFORMATION);
	}

	int UIManager::MsgBoxW( HWND Parent, UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Warning - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(Parent, Buffer, Title, Flags|MB_ICONWARNING);
	}

	int UIManager::MsgBoxW( UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Warning - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(GetMainWindow(), Buffer, Title, Flags|MB_ICONWARNING);
	}

	int UIManager::MsgBoxW( const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Warning - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(GetMainWindow(), Buffer, Title, MB_OK|MB_ICONWARNING);
	}

	int UIManager::MsgBoxE( HWND Parent, UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Error - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(Parent, Buffer, Title, Flags|MB_ICONERROR);
	}

	int UIManager::MsgBoxE( UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Error - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(GetMainWindow(), Buffer, Title, Flags|MB_ICONERROR);
	}

	int UIManager::MsgBoxE( const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Error - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(GetMainWindow(), Buffer, Title, MB_OK|MB_ICONERROR);
	}

	HWND UIManager::ModelessDialog( HINSTANCE hInstance, LPSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam /*= NULL*/, bool Override )
	{
		SME_ASSERT(Initialized);

		if (Override)
			return CallbackCreateDialogParamA(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
		else
		{
			return ((_CallbackCreateDialogParamA)(PatchDepot[kIATPatch_CreateDialogParam].OriginalFunction))
												(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
		}
	}

	INT_PTR UIManager::ModalDialog( HINSTANCE hInstance, LPSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam /*= NULL*/, bool Override )
	{
		SME_ASSERT(Initialized);

		EnumThreadWindows(OwnerThreadID, EnumThreadWindowsCallback, 0);

		INT_PTR Result = NULL;

		if (Override)
			Result = CallbackDialogBoxParamA(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
		else
		{
			Result = ((_CallbackDialogBoxParamA)(PatchDepot[kIATPatch_DialogBoxParam].OriginalFunction))
												(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
		}

		EnumThreadWindows(OwnerThreadID, EnumThreadWindowsCallback, 1);

		return Result;
	}

	WindowSubclasser* UIManager::GetSubclasser( void )
	{
		SME_ASSERT(Subclasser);
		return Subclasser;
	}

	DialogTemplateHotSwapper* UIManager::GetDialogHotSwapper( void )
	{
		SME_ASSERT(DialogHotSwapper);
		return DialogHotSwapper;
	}

	MenuTemplateHotSwapper* UIManager::GetMenuHotSwapper( void )
	{
		SME_ASSERT(MenuHotSwapper);
		return MenuHotSwapper;
	}

	WindowStyler* UIManager::GetWindowStyler( void )
	{
		SME_ASSERT(Initialized);

		return Styler;
	}

	WindowInvalidationManager* UIManager::GetInvalidationManager( void )
	{
		SME_ASSERT(Initialized);

		return InvalidationManager;
	}

	LRESULT CALLBACK GenericModelessDialog::DefaultDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		bool SkipCallback = false;
		bool SkipCallbackResult = false;
		bool SkipDefaultProc = false;
		LRESULT DlgProcResult = FALSE;
		DlgUserData* UserData = (DlgUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		GenericModelessDialog* Instance = NULL;

		if (UserData)
			 Instance = UserData->Instance;

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

			UserData = NULL;
			SetWindowLongPtr(hWnd, GWL_USERDATA, NULL);

			break;
		}

		bool CallbackReturn = false;
		if (SkipCallback == false && UserData && UserData->Initialized)
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
															ResourceTemplateT DialogTemplate, ResourceTemplateT ContextMenuTemplate,
															MessageCallback CallbackProc,
															float AspectRatio /*= 0.0f*/ )
	{
		SME_ASSERT(Parent && Resource && DialogTemplate && ContextMenuTemplate && CallbackProc);

		DialogHandle = NULL;
		ContextMenuHandle = NULL;
		ContextMenuParentHandle = NULL;
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
		DialogHandle = NULL;
		ContextMenuHandle = NULL;
		ContextMenuParentHandle = NULL;
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

	GenericModelessDialog::~GenericModelessDialog()
	{
		if (DialogHandle == NULL || ContextMenuParentHandle == NULL)
			return;

		BGSEEUI->GetWindowHandleCollection(UIManager::kHandleCollection_MainWindowChildren)->Remove(DialogHandle);

		DestroyMenu(ContextMenuParentHandle);
		DestroyWindow(DialogHandle);
	}

	void GenericModelessDialog::Create( LPARAM InitParam, bool Hide, bool OverrideCreation )
	{
		if (DialogHandle || ContextMenuParentHandle || CallbackDlgProc == NULL)
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

	bool GenericModelessDialog::GetVisible( void ) const
	{
		return Visible;
	}

	bool GenericModelessDialog::GetTopmost( void ) const
	{
		return Topmost;
	}

	bool GenericModelessDialog::GetInitialized( void ) const
	{
		return (DialogHandle != NULL);
	}
}