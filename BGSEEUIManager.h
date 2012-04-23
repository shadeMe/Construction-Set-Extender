#pragma once
#include "BGSEEMain.h"

namespace BGSEditorExtender
{
	class BGSEEINIManagerGetterFunctor;
	class BGSEEINIManagerSetterFunctor;

	class BGSEEUIManager
	{
		static BGSEEUIManager*					Singleton;
	public:
		static SME::UIHelpers::CSnapWindow		WindowEdgeSnapper;

		class WindowHandleCollection
		{
			typedef std::list<HWND>				HandleCollectionT;
			HandleCollectionT					HandleList;

			HandleCollectionT::iterator			Find(HWND Handle);
		public:
			bool								Add(HWND Handle);
			bool								Remove(HWND Handle);
			bool								GetExists(HWND Handle);
			void								Clear(void);
		};

		enum
		{
			kHandleCollection_MainWindowChildren = 0x0,
			kHandleCollection__MAX,
		};
	protected:
		BGSEEUIManager();

		HWNDGetter*								EditorWindowHandle;
		WindowHandleCollection					HandleCollections[kHandleCollection__MAX];

		bool									Initialized;

	public:
		virtual ~BGSEEUIManager();

		static BGSEEUIManager*					GetSingleton();
		bool									Initialize(HWND MainWindowHandle);

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
																LPARAM dwInitParam = NULL);
		INT_PTR									ModalDialog(HINSTANCE hInstance,
															LPSTR lpTemplateName,
															HWND hWndParent,
															DLGPROC lpDialogFunc,
															LPARAM dwInitParam = NULL);

		HWND									GetMainWindow(void) const;
		WindowHandleCollection*					GetWindowHandleCollection(UInt8 ID) const;
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
		};

		HWND							DialogHandle;
		HWND							ParentHandle;
		HMENU							ContextMenuHandle;
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