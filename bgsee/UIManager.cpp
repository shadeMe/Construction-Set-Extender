#include "UIManager.h"
#include "Console.h"
#include "WorkspaceManager.h"

namespace bgsee
{
	void ResourceTemplateHotSwapper::PopulateTemplateMap( void )
	{
		for (IDirectoryIterator Itr(SourceDepot().c_str(), "*.dll"); !Itr.Done(); Itr.Next())
		{
			std::string FileName = Itr.Get()->cFileName;
			std::string FullPath = SourceDepot() + "\\" + FileName;

			HINSTANCE Module = (HINSTANCE)LoadLibraryEx(FullPath.c_str(), nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE|LOAD_LIBRARY_AS_DATAFILE);
			if (Module == nullptr)
			{
				BGSEECONSOLE_ERROR("BGSEEResourceTemplateHotSwapper::PopulateTemplateMap - Failed to load resource library '%s'!", FullPath.c_str());
				continue;
			}

			int Index = FileName.rfind(".");
			ResourceTemplateOrdinalT TemplateID = 0;
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
			if (FreeLibrary(Itr->second) == FALSE)
				BGSEECONSOLE_ERROR("BGSEEResourceTemplateHotSwapper::ReleaseTemplateMap - Failed to release resource library for template %d", Itr->first);
		}

		TemplateMap.clear();
	}

	HINSTANCE ResourceTemplateHotSwapper::GetAlternateResourceInstance( ResourceTemplateOrdinalT TemplateID )
	{
		TemplateResourceInstanceMapT::iterator Match = TemplateMap.find(TemplateID);

		if (Match != TemplateMap.end())
			return Match->second;
		else
			return nullptr;
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

	bool MenuTemplateHotSwapper::RegisterTemplateReplacer( ResourceTemplateOrdinalT TemplateID, HINSTANCE Replacer )
	{
		if (TemplateMap.count(TemplateID))
			return false;
		else
			TemplateMap[TemplateID] = Replacer;

		return true;
	}

	bool MenuTemplateHotSwapper::UnregisterTemplateReplacer( ResourceTemplateOrdinalT TemplateID )
	{
		if (TemplateMap.count(TemplateID))
		{
			TemplateMap.erase(TemplateMap.find(TemplateID));
			return true;
		}
		else
			return false;
	}

	bool WindowStyler::StyleWindow( HWND Window, ResourceTemplateOrdinalT Template )
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
			InvalidateRect(Window, nullptr, TRUE);
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

	bool WindowStyler::RegisterStyle( ResourceTemplateOrdinalT TemplateID, StyleData& Data )
	{
		if (StyleListings.count(TemplateID))
			return false;
		else
			StyleListings[TemplateID] = Data;

		return true;
	}

	bool WindowStyler::UnregisterStyle( ResourceTemplateOrdinalT TemplateID )
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
			Invalidate(Itr->first, false, true);

		ActiveInvalidatedWindows.clear();
	}

	void WindowInvalidationManager::Push( HWND Window )
	{
		SME_ASSERT(Window);

		if (ActiveInvalidatedWindows.count(Window) == 0)
		{
			ActiveInvalidatedWindows.insert(std::make_pair(Window, 1));
			Invalidate(Window, true, false);
		}
		else
			ActiveInvalidatedWindows[Window] += 1;
	}

	void WindowInvalidationManager::Pop( HWND Window, bool SuppressRedraw )
	{
		SME_ASSERT(Window);
		SME_ASSERT(ActiveInvalidatedWindows.count(Window));

		UInt32 RefCount = ActiveInvalidatedWindows[Window];
		SME_ASSERT(RefCount);

		if (RefCount == 1)
		{
			Invalidate(Window, false, SuppressRedraw);
			ActiveInvalidatedWindows.erase(Window);
		}
		else
			ActiveInvalidatedWindows[Window] -= 1;
	}

	void WindowInvalidationManager::Invalidate( HWND Window, bool State, bool SuppressRedraw )
	{
		SME_ASSERT(Window);

		if (State)
		{
			SendMessage(Window, WM_SETREDRAW, FALSE, NULL);
		}
		else
		{
			SendMessage(Window, WM_SETREDRAW, TRUE, NULL);
			if (!SuppressRedraw)
				Redraw(Window);
		}
	}

	void WindowInvalidationManager::Redraw(HWND Window)
	{
		RedrawWindow(Window, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
	}

	UIManager*						UIManager::Singleton = nullptr;
	SME::UIHelpers::CSnapWindow		UIManager::WindowEdgeSnapper;



	HWND UIManager::CallbackCreateWindowExA( DWORD dwExStyle,
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
		SME_ASSERT(Initialized);

		bool EditorWindow = false;
		if (!_stricmp(lpClassName, EditorWindowClassName.c_str()))
		{
			EditorWindow = true;

			if (EditorMainMenuReplacement)
				hMenu = EditorMainMenuReplacement;
		}

		HWND Result = ((_CallbackCreateWindowExA)(PatchDepot[kIATPatch_CreateWindowEx].OriginalFunction))
						(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

		if (EditorWindow)
		{
			EditorWindowHandle = Result;
			EditorResourceInstance = hInstance;
			PatchDepot[kIATPatch_CreateWindowEx].Reset();

			BGSEEDAEMON->ExecuteInitCallbacks(Daemon::kInitCallback_PostMainWindowInit);
		}

		return Result;
	}

	HMENU UIManager::CallbackLoadMenuA( HINSTANCE hInstance, LPCSTR lpMenuName )
	{
		SME_ASSERT(MenuHotSwapper);

		if (hInstance != EditorResourceInstance)
			return ((_CallbackLoadMenuA)(PatchDepot[kIATPatch_LoadMenu].OriginalFunction))(hInstance, lpMenuName);

		HINSTANCE Alternate = MenuHotSwapper->GetAlternateResourceInstance((UInt32)lpMenuName);
		if (Alternate)
			hInstance = Alternate;

		return ((_CallbackLoadMenuA)(PatchDepot[kIATPatch_LoadMenu].OriginalFunction))(hInstance, lpMenuName);
	}

	HWND UIManager::CallbackCreateDialogParamA( HINSTANCE hInstance,
												LPCSTR lpTemplateName,
												HWND hWndParent,
												DLGPROC lpDialogFunc,
												LPARAM dwInitParam )
	{
		DialogCreationData CreationData;
		CreationData.Modal = false;
		CreationData.InstantiationTemplate = reinterpret_cast<ResourceTemplateOrdinalT>(lpTemplateName);
		CreationData.CreationDlgProc = lpDialogFunc;
		CreationData.CreationUserData = dwInitParam;

		return CallbackCreateDialogParamA(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam, CreationData);
	}

	HWND UIManager::CallbackCreateDialogParamA(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam, const DialogCreationData& CreationData)
	{
		SME_ASSERT(DialogHotSwapper && Styler);

		HINSTANCE Alternate = DialogHotSwapper->GetAlternateResourceInstance((UInt32)lpTemplateName);
		if (Alternate)
			hInstance = Alternate;

		HWND Result = ((_CallbackCreateDialogParamA)(PatchDepot[kIATPatch_CreateDialogParam].OriginalFunction))
			(hInstance, lpTemplateName, hWndParent, lpDialogFunc, reinterpret_cast<LPARAM>(&CreationData));

		if (Result)
			Styler->StyleWindow(Result, (UInt32)lpTemplateName);

		return Result;
	}

	INT_PTR UIManager::CallbackDialogBoxParamA(HINSTANCE hInstance,
														LPCSTR lpTemplateName,
														HWND hWndParent,
														DLGPROC lpDialogFunc,
														LPARAM dwInitParam )
	{
		DialogCreationData CreationData;
		CreationData.Modal = true;
		CreationData.InstantiationTemplate = reinterpret_cast<ResourceTemplateOrdinalT>(lpTemplateName);
		CreationData.CreationDlgProc = lpDialogFunc;
		CreationData.CreationUserData = dwInitParam;

		return CallbackDialogBoxParamA(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam, CreationData);
	}

	INT_PTR UIManager::CallbackDialogBoxParamA(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam, const DialogCreationData& CreationData)
	{
		SME_ASSERT(DialogHotSwapper);

		HINSTANCE Alternate = DialogHotSwapper->GetAlternateResourceInstance((ResourceTemplateOrdinalT)lpTemplateName);
		if (Alternate)
			hInstance = Alternate;

		return ((_CallbackDialogBoxParamA)(PatchDepot[kIATPatch_DialogBoxParam].OriginalFunction))
			(hInstance, lpTemplateName, hWndParent, lpDialogFunc, reinterpret_cast<LPARAM>(&CreationData));
	}

	BOOL CALLBACK UIManager::EnumThreadWindowsCallback(HWND hwnd, LPARAM lParam)
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
		const char* ImportName = nullptr;

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

		util::IATPatchData* Patch = &PatchDepot[PatchType];
		Patch->ImportModule = DLLName;
		Patch->ImportName = ImportName;
		Patch->Location = 0;
		Patch->CallbackFunction = reinterpret_cast<uintptr_t>(Callback);

		auto PatchSuccessful = Patch->Initialize();
		SME_ASSERT(PatchSuccessful);

		Patch->Replace();
	}

	UIManager::UIManager(const char* MainWindowClassName, HMENU MainMenuHandle) :
		OwnerThreadID(0),
		ThunkCreateWindowExA(this, &UIManager::CallbackCreateWindowExA),
		ThunkLoadMenuA(this, &UIManager::CallbackLoadMenuA),
		ThunkCreateDialogParamA(this, &UIManager::CallbackCreateDialogParamA),
		ThunkDialogParamA(this, &UIManager::CallbackDialogBoxParamA),
		EditorWindowClassName(MainWindowClassName),
		EditorWindowHandle(nullptr),
		EditorResourceInstance(nullptr),
		EditorMainMenuReplacement(MainMenuHandle),
		Subclasser(nullptr),
		DialogHotSwapper(nullptr),
		MenuHotSwapper(nullptr),
		Styler(nullptr),
		InvalidationManager(nullptr),
		Initialized(false)
	{
		SME_ASSERT(Singleton == nullptr);
		Singleton = this;
		OwnerThreadID = GetCurrentThreadId();

		EditorWindowClassName = MainWindowClassName;
		EditorMainMenuReplacement = MainMenuHandle;
		Subclasser = new WindowSubclasser();
		DialogHotSwapper = new DialogTemplateHotSwapper();
		MenuHotSwapper = new MenuTemplateHotSwapper();
		Styler = new WindowStyler();
		ColorThemer = new WindowColorThemer(Subclasser);
		InvalidationManager = new WindowInvalidationManager();

		PatchIAT(kIATPatch_CreateWindowEx, ThunkCreateWindowExA());
		PatchIAT(kIATPatch_CreateDialogParam, ThunkCreateDialogParamA());
		PatchIAT(kIATPatch_DialogBoxParam, ThunkDialogParamA());
		PatchIAT(kIATPatch_LoadMenu, ThunkLoadMenuA());

		Initialized = true;
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
		SAFEDELETE(Styler);
		SAFEDELETE(ColorThemer);
		SAFEDELETE(InvalidationManager);

		Initialized = false;

		Singleton = nullptr;
	}

	UIManager* UIManager::Get()
	{
		return Singleton;
	}

	bool UIManager::Initialize( const char* MainWindowClassName, HMENU MainMenuHandle )
	{
		if (Singleton)
			return false;

		UIManager* Buffer = new UIManager(MainWindowClassName, MainMenuHandle);
		return Buffer->Initialized;
	}

	void UIManager::Deinitialize()
	{
		SME_ASSERT(Singleton);
		delete Singleton;
	}

	WindowHandleCollection* UIManager::GetWindowHandleCollection(UInt8 ID)
	{
		SME_ASSERT(ID < kHandleCollection__MAX);
		return &HandleCollections[ID];
	}

	HWND UIManager::GetMainWindow() const
	{
		SME_ASSERT(EditorWindowHandle);
		return EditorWindowHandle;
	}

	int UIManager::MsgBoxI( HWND Parent, UINT Flags, const char* Format, ... )
	{
		char Buffer[0x1000] = {0};
		va_list Args;
		va_start(Args, Format);
		vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
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
		vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
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
		vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
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
		vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
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
		vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
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
		vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
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
		vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
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
		vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
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
		vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
		va_end(Args);

		char Title[0x100] = {0};
		FORMAT_STR(Title, "Error - %s", BGSEEMAIN->ExtenderGetDisplayName());

		return MessageBox(GetMainWindow(), Buffer, Title, MB_OK|MB_ICONERROR);
	}

	HWND UIManager::ModelessDialog( HINSTANCE hInstance, LPSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam /*= NULL*/, bool Override )
	{
		SME_ASSERT(Initialized);

		DialogCreationData CreationData;
		CreationData.Modal = false;
		CreationData.SkipSubclassing = Override == false;
		CreationData.InstantiationTemplate = reinterpret_cast<ResourceTemplateOrdinalT>(lpTemplateName);
		CreationData.CreationDlgProc = lpDialogFunc;
		CreationData.CreationUserData = dwInitParam;

		return CallbackCreateDialogParamA(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam, CreationData);
	}

	INT_PTR UIManager::ModalDialog( HINSTANCE hInstance, LPSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam /*= NULL*/, bool Override )
	{
		SME_ASSERT(Initialized);
		EnumThreadWindows(OwnerThreadID, EnumThreadWindowsCallback, 0);

		INT_PTR Result = NULL;

		DialogCreationData CreationData;
		CreationData.Modal = true;
		CreationData.SkipSubclassing = Override == false;
		CreationData.InstantiationTemplate = reinterpret_cast<ResourceTemplateOrdinalT>(lpTemplateName);
		CreationData.CreationDlgProc = lpDialogFunc;
		CreationData.CreationUserData = dwInitParam;

		Result = CallbackDialogBoxParamA(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam, CreationData);

		EnumThreadWindows(OwnerThreadID, EnumThreadWindowsCallback, 1);

		return Result;
	}

	WindowSubclasser* UIManager::GetSubclasser()
	{
		return Subclasser;
	}

	DialogTemplateHotSwapper* UIManager::GetDialogHotSwapper()
	{
		return DialogHotSwapper;
	}

	MenuTemplateHotSwapper* UIManager::GetMenuHotSwapper()
	{
		return MenuHotSwapper;
	}

	WindowStyler* UIManager::GetWindowStyler()
	{
		return Styler;
	}

	bgsee::WindowColorThemer* UIManager::GetColorThemer()
	{
		return ColorThemer;
	}

	WindowInvalidationManager* UIManager::GetInvalidationManager()
	{
		return InvalidationManager;
	}
}