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

	class BGSEEWindowSubclasser
	{
		friend class BGSEEUIManager;
	public:
		typedef LRESULT									(CALLBACK* SubclassProc)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																				bool& Return, LPARAM& InstanceUserData);
		typedef std::map<SubclassProc, LPARAM>			SubclassProcMapT;

		struct SubclassData
		{
			DLGPROC								Original;
			BGSEEWindowHandleCollection			ActiveHandles;				// open windows using the parent templateID
			SubclassProcMapT					Subclasses;

			SubclassData();

			INT_PTR								ProcessSubclasses(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);
		};

		struct SubclassUserData
		{
			BGSEEWindowSubclasser*				Instance;
			SubclassData*						Data;
			LPARAM								InitParam;
			LPARAM								ExtraData;
			bool								Initialized;
		};
	private:
		typedef std::map<UInt32, SubclassData>	DialogSubclassMapT;			// key = templateID

		static LRESULT CALLBACK					MainWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK					DialogSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		HWND									EditorMainWindow;
		WNDPROC									EditorMainWindowProc;
		SubclassProcMapT						MainWindowSubclasses;
		DialogSubclassMapT						DialogSubclasses;

												// returns true if callbacks are registered for the template
		bool									GetShouldSubclass(UInt32 TemplateID,
																LPARAM InitParam,
																DLGPROC OriginalProc,
																DLGPROC& OutSubclassProc,
																SubclassUserData** OutSubclassUserData);
		void									PreSubclassMainWindow(HWND MainWindow);
	public:
		BGSEEWindowSubclasser();
		~BGSEEWindowSubclasser();

		bool									RegisterMainWindowSubclass(SubclassProc Proc, LPARAM UserData = NULL);
		bool									UnregisterMainWindowSubclass(SubclassProc Proc);

		bool									RegisterDialogSubclass(UInt32 TemplateID, SubclassProc Proc, LPARAM UserData = NULL);
		bool									UnregisterDialogSubclass(UInt32 TemplateID, SubclassProc Proc);

		bool									GetHasDialogSubclass(UInt32 TemplateID);
	};

	class BGSEEResourceTemplateHotSwapper
	{
		friend class BGSEEUIManager;
	protected:
		typedef std::map<UInt32, HINSTANCE>		TemplateResourceInstanceMapT;		// key = templateID

		BGSEEResourceLocation					SourceDepot;
		TemplateResourceInstanceMapT			TemplateMap;

		void									PopulateTemplateMap(void);
		void									ReleaseTemplateMap(void);
	public:
		BGSEEResourceTemplateHotSwapper(std::string SourcePath);
		virtual ~BGSEEResourceTemplateHotSwapper() = 0;

		virtual HINSTANCE						GetAlternateResourceInstance(UInt32 TemplateID);		// returns NULL if there isn't one
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

		bool									RegisterTemplateReplacer(UInt32 TemplateID, HINSTANCE Replacer);
		bool									UnregisterTemplateReplacer(UInt32 TemplateID);
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
		typedef std::map<UInt32, StyleData>		TemplateStyleMapT;

		TemplateStyleMapT						StyleListings;

		bool									StyleWindow(HWND Window, UInt32 Template);
	public:
		BGSEEWindowStyler();
		~BGSEEWindowStyler();

		bool									RegisterStyle(UInt32 TemplateID, StyleData& Data);
		bool									UnregisterStyle(UInt32 TemplateID);
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
		UInt32							DialogTemplateID;
		UInt32							DialogContextMenuID;
		MessageCallback					CallbackDlgProc;
		bool							Visible;
		bool							Topmost;
		float							AspectRatio;
		bool							LockAspectRatio;

		void							INILoadUIState(BGSEEINIManagerGetterFunctor* Getter, const char* Section);
		void							INISaveUIState(BGSEEINIManagerSetterFunctor* Setter, const char* Section);

		bool							SetVisibility(bool State);
		bool							SetTopmost(bool State);

		virtual void					Create(LPARAM InitParam, bool Hide);

		BGSEEGenericModelessDialog();
	public:
		BGSEEGenericModelessDialog(HWND Parent, HINSTANCE Resource, UInt32 DialogTemplate, UInt32 ContextMenuTemplate, MessageCallback CallbackProc, float AspectRatio = 0.0f);
		virtual ~BGSEEGenericModelessDialog();

		bool							ToggleVisibility(void);		// returns the new state
		bool							ToggleTopmost(void);

		bool							GetVisible(void) const;
		bool							GetTopmost(void) const;
		bool							GetInitialized(void) const;
	};
}