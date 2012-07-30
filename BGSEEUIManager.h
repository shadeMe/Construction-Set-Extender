#pragma once
#include "BGSEEMain.h"
#include "BGSEEWorkspaceManager.h"

namespace BGSEditorExtender
{
	class BGSEEUIManager;

	class BGSEEWindowHandleCollection
	{
		typedef std::list<HWND>					HandleCollectionT;
		HandleCollectionT						HandleList;

		HandleCollectionT::iterator				Find(HWND Handle);
	public:
		BGSEEWindowHandleCollection();
		~BGSEEWindowHandleCollection();

		bool									Add(HWND Handle);
		bool									Remove(HWND Handle);
		bool									GetExists(HWND Handle);
		void									Clear(void);

		void									SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	};

	typedef UInt32 ResourceTemplateT;

	class BGSEEWindowSubclasser
	{
		friend class BGSEEUIManager;
	public:
		typedef LRESULT									(CALLBACK* SubclassProc)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																				bool& Return, LPARAM& InstanceUserData);
		typedef std::map<SubclassProc, LPARAM>			SubclassProcMapT;

		struct DialogSubclassData
		{
			DLGPROC								Original;
			BGSEEWindowHandleCollection			ActiveHandles;				// open windows using the parent templateID
			SubclassProcMapT					Subclasses;

			DialogSubclassData();

			INT_PTR								ProcessSubclasses(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);
		};

		struct DialogSubclassUserData
		{
			BGSEEWindowSubclasser*				Instance;
			DialogSubclassData*					Data;
			LPARAM								InitParam;
			LPARAM								ExtraData;
			ResourceTemplateT					TemplateID;					// the template ID of the dialog resource
			bool								Initialized;				// set after the WM_INITDIALOG message is processed
		};

		struct WindowSubclassData
		{
			WNDPROC								Original;
			SubclassProcMapT					Subclasses;

			WindowSubclassData();

			LRESULT								ProcessSubclasses(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);
		};

		struct WindowSubclassUserData
		{
			BGSEEWindowSubclasser*				Instance;
			WindowSubclassData*					Data;
			LONG_PTR							OriginalUserData;			// ping-pong b'ween this and our userdata
		};
	private:
		typedef std::map<ResourceTemplateT, DialogSubclassData>		DialogSubclassMapT;
		typedef std::map<HWND, WindowSubclassData>					WindowSubclassMapT;

		static LRESULT CALLBACK					MainWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK					DialogSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK					RegularWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		HWND									EditorMainWindow;
		WNDPROC									EditorMainWindowProc;
		SubclassProcMapT						MainWindowSubclasses;
		DialogSubclassMapT						DialogSubclasses;
		WindowSubclassMapT						RegularWindowSubclasses;

												// returns true if callbacks are registered for the template
		bool									GetShouldSubclassDialog(ResourceTemplateT TemplateID,
																		LPARAM InitParam,
																		DLGPROC OriginalProc,
																		DLGPROC& OutSubclassProc,
																		DialogSubclassUserData** OutSubclassUserData);
		void									PreSubclassMainWindow(HWND MainWindow);
	public:
		BGSEEWindowSubclasser();
		~BGSEEWindowSubclasser();

		bool									RegisterMainWindowSubclass(SubclassProc Proc, LPARAM UserData = NULL);
		bool									UnregisterMainWindowSubclass(SubclassProc Proc);

		bool									RegisterDialogSubclass(ResourceTemplateT TemplateID, SubclassProc Proc, LPARAM UserData = NULL);
		bool									UnregisterDialogSubclass(ResourceTemplateT TemplateID, SubclassProc Proc);

		bool									RegisterRegularWindowSubclass(HWND Handle, SubclassProc Proc, LPARAM UserData = NULL);
		bool									UnregisterRegularWindowSubclass(HWND Handle, SubclassProc Proc);

		bool									GetHasDialogSubclass(ResourceTemplateT TemplateID);
	};

	class BGSEEResourceTemplateHotSwapper
	{
		friend class BGSEEUIManager;
	protected:
		typedef std::map<ResourceTemplateT, HINSTANCE>		TemplateResourceInstanceMapT;

		BGSEEResourceLocation					SourceDepot;
		TemplateResourceInstanceMapT			TemplateMap;

		void									PopulateTemplateMap(void);
		void									ReleaseTemplateMap(void);
	public:
		BGSEEResourceTemplateHotSwapper(std::string SourcePath);
		virtual ~BGSEEResourceTemplateHotSwapper() = 0;

		virtual HINSTANCE						GetAlternateResourceInstance(ResourceTemplateT TemplateID);		// returns NULL if there isn't one
	};

	class BGSEEDialogTemplateHotSwapper : public BGSEEResourceTemplateHotSwapper
	{
		static const std::string				kDefaultLoc;
	public:
		BGSEEDialogTemplateHotSwapper();
		virtual ~BGSEEDialogTemplateHotSwapper();
	};

	class BGSEEMenuTemplateHotSwapper : public BGSEEResourceTemplateHotSwapper
	{
	public:
		BGSEEMenuTemplateHotSwapper();
		virtual ~BGSEEMenuTemplateHotSwapper();

		bool									RegisterTemplateReplacer(ResourceTemplateT TemplateID, HINSTANCE Replacer);
		bool									UnregisterTemplateReplacer(ResourceTemplateT TemplateID);
	};

	class BGSEEWindowStyler
	{
		friend class BGSEEUIManager;
	public:
		struct StyleData
		{
			LONG		Regular;
			UInt8		RegularOp;
			LONG		Extended;
			UInt8		ExtendedOp;

			enum
			{
				kOperation_None		= 0,
				kOperation_OR,
				kOperation_AND,
				kOperation_Replace
			};
		};
	private:
		typedef std::map<ResourceTemplateT, StyleData>		TemplateStyleMapT;

		TemplateStyleMapT						StyleListings;

		bool									StyleWindow(HWND Window, ResourceTemplateT Template);
	public:
		BGSEEWindowStyler();
		~BGSEEWindowStyler();

		bool									RegisterStyle(ResourceTemplateT TemplateID, StyleData& Data);
		bool									UnregisterStyle(ResourceTemplateT TemplateID);
	};

	class BGSEEUIManager
	{
	public:
		static SME::UIHelpers::CSnapWindow		WindowEdgeSnapper;

		enum
		{
			kHandleCollection_MainWindowChildren	= 0,
			kHandleCollection_DragDropableWindows,

			kHandleCollection__MAX,
		};
	private:
		static BGSEEUIManager*					Singleton;

		struct IATPatchData
		{
			const char*							DLL;
			const char*							Import;

			void*								Location;
			void*								OriginalFunction;
			void*								CallbackFunction;
			bool								Replaced;

			IATPatchData();

			void								Replace(void);
			void								Reset(void);
		};

		typedef HWND							(CALLBACK* _CallbackCreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
		typedef HMENU							(CALLBACK* _CallbackLoadMenuA)(HINSTANCE hInstance, LPCSTR lpMenuName);
		typedef HWND							(CALLBACK* _CallbackCreateDialogParamA)(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
		typedef INT_PTR							(CALLBACK* _CallbackDialogBoxParamA)(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);

		static HWND CALLBACK					CallbackCreateWindowExA(DWORD dwExStyle,
																	LPCSTR lpClassName,
																	LPCSTR lpWindowName,
																	DWORD dwStyle,
																	int X, int Y,
																	int nWidth, int nHeight,
																	HWND hWndParent,
																	HMENU hMenu,
																	HINSTANCE hInstance,
																	LPVOID lpParam);

		static HMENU CALLBACK					CallbackLoadMenuA(HINSTANCE hInstance, LPCSTR lpMenuName);

		static HWND CALLBACK					CallbackCreateDialogParamA(HINSTANCE hInstance,
																		LPCSTR lpTemplateName,
																		HWND hWndParent,
																		DLGPROC lpDialogFunc,
																		LPARAM dwInitParam);

		static INT_PTR CALLBACK					CallbackDialogBoxParamA(HINSTANCE hInstance,
																		LPCSTR lpTemplateName,
																		HWND hWndParent,
																		DLGPROC lpDialogFunc,
																		LPARAM dwInitParam);

		static BOOL CALLBACK					EnumThreadWindowsCallback(HWND hwnd, LPARAM lParam);

		enum
		{
			kIATPatch_CreateWindowEx		= 0,		// one-time hook to grab the editor's UI stuffings
			kIATPatch_LoadMenu,
			kIATPatch_CreateDialogParam,
			kIATPatch_DialogBoxParam,

			kIATPatch__MAX
		};

		BGSEEUIManager();

		DWORD									OwnerThreadID;
		IATPatchData							PatchDepot[kIATPatch__MAX];
		std::string								EditorWindowClassName;
		HWNDGetter*								EditorWindowHandle;
		HINSTANCEGetter*						EditorResourceInstance;
		HMENU									EditorMainMenuReplacement;
		BGSEEWindowSubclasser*					Subclasser;
		BGSEEDialogTemplateHotSwapper*			DialogHotSwapper;
		BGSEEMenuTemplateHotSwapper*			MenuHotSwapper;
		BGSEEWindowStyler*						WindowStyler;
		BGSEEWindowHandleCollection				HandleCollections[kHandleCollection__MAX];

		bool									Initialized;

		void									PatchIAT(UInt8 PatchType, void* Callback);		// CALLBACK call convention
	public:
		virtual ~BGSEEUIManager();

		static BGSEEUIManager*					GetSingleton();
		bool									Initialize(const char* MainWindowClassName, HMENU MainMenuHandle = NULL);

		int										MsgBoxI(HWND Parent, UINT Flags, const char* Format, ...);
		int										MsgBoxI(UINT Flags, const char* Format, ...);
		int										MsgBoxI(const char* Format, ...);

		int										MsgBoxW(HWND Parent, UINT Flags, const char* Format, ...);
		int										MsgBoxW(UINT Flags, const char* Format, ...);
		int										MsgBoxW(const char* Format, ...);

		int										MsgBoxE(HWND Parent, UINT Flags, const char* Format, ...);
		int										MsgBoxE(UINT Flags, const char* Format, ...);
		int										MsgBoxE(const char* Format, ...);

		HWND									ModelessDialog(HINSTANCE hInstance,
															LPSTR lpTemplateName,
															HWND hWndParent,
															DLGPROC lpDialogFunc,
															LPARAM dwInitParam = NULL,
															bool Override = false);
		INT_PTR									ModalDialog(HINSTANCE hInstance,
															LPSTR lpTemplateName,
															HWND hWndParent,
															DLGPROC lpDialogFunc,
															LPARAM dwInitParam = NULL,
															bool Override = false);

		HWND									GetMainWindow(void) const;
		BGSEEWindowHandleCollection*			GetWindowHandleCollection(UInt8 ID);
		BGSEEWindowSubclasser*					GetSubclasser(void);
		BGSEEDialogTemplateHotSwapper*			GetDialogHotSwapper(void);
		BGSEEMenuTemplateHotSwapper*			GetMenuHotSwapper(void);
		BGSEEWindowStyler*						GetWindowStyler(void);
	};
#define BGSEEUI									BGSEditorExtender::BGSEEUIManager::GetSingleton()

	class BGSEEGenericModelessDialog
	{
		static LRESULT CALLBACK			DefaultDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	protected:
		typedef LRESULT (CALLBACK*		MessageCallback)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);

		static const BGSEEINIManagerSettingFactory::SettingData		kDefaultINISettings[5];
		enum
		{
			kDefaultINISetting_Top = 0,
			kDefaultINISetting_Left,
			kDefaultINISetting_Right,
			kDefaultINISetting_Bottom,
			kDefaultINISetting_Visible,
		};

		struct DlgUserData
		{
			BGSEEGenericModelessDialog*		Instance;
			LPARAM							UserData;
			bool							Initialized;
		};

		HWND							DialogHandle;
		HWND							ParentHandle;
		HMENU							ContextMenuHandle;				// all menu items must be a part of a sub-menu at index 0
		HMENU							ContextMenuParentHandle;		// handle of the base menu
		HINSTANCE						ResourceInstance;
		ResourceTemplateT				DialogTemplateID;
		ResourceTemplateT				DialogContextMenuID;
		MessageCallback					CallbackDlgProc;
		bool							Visible;
		bool							Topmost;
		float							AspectRatio;
		bool							LockAspectRatio;

		void							INILoadUIState(BGSEEINIManagerGetterFunctor* Getter, const char* Section);
		void							INISaveUIState(BGSEEINIManagerSetterFunctor* Setter, const char* Section);

		bool							SetVisibility(bool State);
		bool							SetTopmost(bool State);

		virtual void					Create(LPARAM InitParam, bool Hide, bool OverrideCreation = false);

		BGSEEGenericModelessDialog();
	public:
		BGSEEGenericModelessDialog(HWND Parent, HINSTANCE Resource, ResourceTemplateT DialogTemplate, ResourceTemplateT ContextMenuTemplate, MessageCallback CallbackProc, float AspectRatio = 0.0f);
		virtual ~BGSEEGenericModelessDialog();

		bool							ToggleVisibility(void);		// returns the new state
		bool							ToggleTopmost(void);

		bool							GetVisible(void) const;
		bool							GetTopmost(void) const;
		bool							GetInitialized(void) const;
	};
}