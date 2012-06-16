#pragma once
#include <BGSEEUIManager.h>

class Subwindow;

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		enum
		{
			kStartupPlugin_LoadPlugin = 0,
			kStartupPlugin_PluginName,

			kStartupPlugin__MAX
		};
		extern const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kStartupPluginINISettings[kStartupPlugin__MAX];
		BGSEditorExtender::BGSEEINIManagerSettingFactory*								GetStartupPlugin(void);

		enum
		{
			kPlugins_PreventTimeStampChanges = 0,
			kPlugins_SaveLoadedESPsAsMasters,

			kPlugins__MAX
		};
		extern const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kPluginsINISettings[kPlugins__MAX];
		BGSEditorExtender::BGSEEINIManagerSettingFactory*								GetPlugins(void);
	}

	namespace UIManager
	{
		template <typename T>
		struct InitDialogMessageParamT
		{
			char						Buffer[0x400];
			T							ExtraData;
		};

		class CSEFilterableFormListManager
		{
		public:
			class FilterableWindowData
			{
				static LRESULT CALLBACK		FormListSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

				HWND					ParentWindow;
				HWND					FilterEditBox;
				HWND					FormListView;
				WNDPROC					FormListWndProc;

				std::string				FilterString;
				int						TimerPeriod;
				int						TimeCounter;
				bool					ObjRefList;
			public:
				FilterableWindowData(HWND Parent, HWND EditBox, HWND FormList, bool RefList, int TimerPeriod);
				~FilterableWindowData();

				bool					HandleMessages(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam);		// returns true on timeout
			};

			static CSEFilterableFormListManager				Instance;
		private:
			typedef std::map<HWND, FilterableWindowData*>	FilterDataMapT;

			FilterDataMapT				ActiveWindows;
		public:
			CSEFilterableFormListManager();
			~CSEFilterableFormListManager();

			bool						Register(HWND Window, HWND FilterEdit, HWND FormList, bool ObjRefList = false, int TimePeriod = 500);
			bool						Unregister(HWND Window);

			bool						HandleMessages(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam);
		};

		class CSEFormEnumerationManager
		{
			bool						VisibilityDeletedForms;
			bool						VisibilityUnmodifiedForms;
		public:
			CSEFormEnumerationManager();
			~CSEFormEnumerationManager();

			bool						GetVisibleDeletedForms(void) const;
			bool						GetVisibleUnmodifiedForms(void) const;

			bool						ToggleVisibilityDeletedForms(void);
			bool						ToggleVisibilityUnmodifiedForms(void);

			bool						GetShouldEnumerate(TESForm* Form);
			void						ResetVisibility(void);

			static CSEFormEnumerationManager				Instance;
		};

		class CSEWindowInvalidationManager
		{
			typedef std::map<HWND, UInt32>		InvalidationMapT;

			InvalidationMapT					ActiveInvalidatedWindows;

			void								Invalidate(HWND Window, bool State);
		public:
			CSEWindowInvalidationManager();
			~CSEWindowInvalidationManager();

			void								Push(HWND Window);
			void								Pop(HWND Window);

			static CSEWindowInvalidationManager				Instance;
		};

		struct CSECellViewExtraData
		{
			RECT	FilterEditBox;		// init bounds of the new controls
			RECT	FilterLabel;
			RECT	XLabel;
			RECT	YLabel;
			RECT	XEdit;
			RECT	YEdit;
			RECT	GoBtn;
		};

		struct CSEDialogQuickViewData
		{
			POINT	Cursor;
			HWND	Window;
		};

		struct CSEMainWindowMiscExtraData
		{
			Subwindow*			ToolbarExtras;

			CSEMainWindowMiscExtraData();
			~CSEMainWindowMiscExtraData();
		};

		LRESULT CALLBACK		FindTextDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);
		LRESULT CALLBACK		DataDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);

		LRESULT CALLBACK		MainWindowMenuInitSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);
		LRESULT CALLBACK		MainWindowMenuSelectSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);
		LRESULT CALLBACK		MainWindowMiscSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);
		LRESULT CALLBACK		MainWindowToolbarSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);

		LRESULT CALLBACK		RenderWindowMenuInitSelectSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);
		LRESULT CALLBACK		RenderWindowMiscSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);
		LRESULT CALLBACK		ObjectWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);
		LRESULT CALLBACK		CellViewWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);

		LRESULT CALLBACK		ResponseDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);
		LRESULT CALLBACK		LandscapeTextureUseDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);
		LRESULT CALLBACK		QuestDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);

		LRESULT CALLBACK		CommonDialogQuickViewSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData);

		BOOL CALLBACK			AssetSelectorDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			TextEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			TESFileSaveDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			TESComboBoxDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			CopyPathDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK		CreateGlobalScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			BindScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			EditResultScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void					Initialize(void);
	}
}

// custom control IDs, as baked into dialog templates
#define IDC_CSE_DATA_QUICKLOAD                  9900
#define IDC_CSE_DATA_SETSTARTUPPLUGIN           9906

#define IDC_CSE_RACE_COPYHAIR                   9915
#define IDC_CSE_RACE_COPYEYES                   9916

#define IDC_CSEFILTERABLEFORMLIST_FILTERLBL		9926
#define IDC_CSEFILTERABLEFORMLIST_FILTEREDIT	9927

#define IDC_CSE_QUEST_EDITRESULTSCRIPT			9928

#define IDC_CSE_CELLVIEW_XLBL					9929
#define IDC_CSE_CELLVIEW_YLBL					9930
#define IDC_CSE_CELLVIEW_XEDIT					9931
#define IDC_CSE_CELLVIEW_YEDIT					9932
#define IDC_CSE_CELLVIEW_GOBTN					9933

// custom popup menu item IDs
#define IDC_CSE_POPUP_SETFORMID                 9907
#define IDC_CSE_POPUP_MARKUNMODIFIED            9908
#define IDC_CSE_POPUP_JUMPTOUSEINFOLIST         9909
#define IDC_CSE_POPUP_UNDELETE                  9910
#define IDC_CSE_POPUP_EDITBASEFORM              9913
#define IDC_CSE_POPUP_TOGGLEVISIBILITY          9917
#define IDC_CSE_POPUP_TOGGLECHILDRENVISIBILITY  9918
#define IDC_CSE_POPUP_ADDTOTAG                  9919
#define IDC_CSE_POPUP_SHOWOVERRIDES             9925
#define IDC_CSE_POPUP_PREVIEW		            9926