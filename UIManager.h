#pragma once

namespace cse
{
	namespace uiManager
	{
		template <typename T>
		struct InitDialogMessageParamT
		{
			char	Buffer[0x400];
			T		ExtraData;
		};

		class FilterableFormListManager
		{
		public:
			typedef std::function<bool(TESForm*)>			SecondaryFilterT;			// returns true if the form is to be added
		private:
			class FilterableWindowData
			{
				static LRESULT CALLBACK			FormListSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																	bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);

				typedef std::map<HWND, UInt32>	WindowTimerMapT;
				static WindowTimerMapT			FilterTimerTable;

				typedef std::map<HWND, FilterableWindowData*>	FormListFilterDataMapT;
				static FormListFilterDataMapT	FormListDataTable;

				HWND					ParentWindow;
				HWND					FilterEditBox;
				HWND					FormListView;
				WNDPROC					FormListWndProc;
				HWND					FilterLabel;

				std::string				FilterString;
				int						TimerPeriod;
				int						TimeCounter;
				UInt8					Flags;
				SecondaryFilterT		SecondFilter;
				bool					Enabled;

				enum
				{
					kFlags_RegEx				= 1 << 0,
					kFlags_SearchEditorID		= 1 << 1,
					kFlags_SearchName			= 1 << 2,
					kFlags_SearchDescription	= 1 << 3,
					kFlags_SearchFormID			= 1 << 4,
				};

				bool					HasRegEx(void) const { return Flags & kFlags_RegEx; }
				bool					HasEditorID(void) const { return Flags & kFlags_SearchEditorID; }
				bool					HasName(void) const { return Flags & kFlags_SearchName; }
				bool					HasDescription(void) const { return Flags & kFlags_SearchDescription; }
				bool					HasFormID(void) const { return Flags & kFlags_SearchFormID; }

				bool					FilterForm(TESForm* Form);		// returns true if the form matches the active filter
				void					HandlePopupMenu(HWND Parent, int X, int Y);

				void					CreateTimer(void) const;
				void					DestroyTimer(void) const;

				void					HookFormList(void);
				void					UnhookFormList(void);
			public:
				FilterableWindowData(HWND Parent, HWND EditBox, HWND FormList, HWND Label, int TimerPeriod, SecondaryFilterT UserFilter = nullptr);
				~FilterableWindowData();

				bool					HandleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);		// returns true on timeout
				void					SetEnabledState(bool State);

				bool					operator==(HWND FilterEditBox);
			};

			typedef std::vector<FilterableWindowData*>	FilterDataArrayT;

			FilterDataArrayT				ActiveFilters;

			FilterableWindowData*		Lookup(HWND FilterEdit);
		public:
			FilterableFormListManager();
			~FilterableFormListManager();

			bool						Register(HWND FilterEdit, HWND FilterLabel, HWND FormList, HWND ParentWindow, int TimePeriod = 250, SecondaryFilterT UserFilter = nullptr);
			void						Unregister(HWND FilterEdit);

			bool						HandleMessages(HWND FilterEdit, UINT uMsg, WPARAM wParam, LPARAM lParam);		// returns true to request a refresh of the form list
			void						SetEnabledState(HWND FilterEdit, bool State);

			static FilterableFormListManager				Instance;
		};

		class FormEnumerationManager
		{
			bool						VisibilityDeletedForms;
			bool						VisibilityUnmodifiedForms;
		public:
			FormEnumerationManager();
			~FormEnumerationManager();

			bool						GetVisibleDeletedForms(void) const;
			bool						GetVisibleUnmodifiedForms(void) const;

			bool						ToggleVisibilityDeletedForms(void);
			bool						ToggleVisibilityUnmodifiedForms(void);

			bool						GetShouldEnumerate(TESForm* Form);
			void						ResetVisibility(void);

			int							CompareActiveForms(TESForm* FormA, TESForm* FormB, int OriginalResult);

			static FormEnumerationManager				Instance;
		};

		void Initialize(void);
	}
}


#define IDC_CSEFILTERABLEFORMLIST_FILTERLBL		9926
#define IDC_CSEFILTERABLEFORMLIST_FILTEREDIT	9927

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
#define IDC_CSE_POPUP_EXPORTFACETEXTURES		9927
#define IDC_CSE_POPUP_GLOBALCOPY				9928
#define IDC_CSE_POPUP_REPLACEBASEFORM			9929
#define IDC_CSE_POPUP_GLOBALPASTE				9930
#define IDC_CSE_POPUP_SPAWNNEWOBJECTWINDOW		9931
