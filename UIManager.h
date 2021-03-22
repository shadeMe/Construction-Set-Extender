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
			typedef std::function<bool(TESForm*)>
												SecondaryFilterT;			// returns true if the form is to be added
		private:
			class FilterableWindowData
			{
				bgsee::SubclassProcThunk<FilterableWindowData>
										ThunkFormListSubclassProc;
				bgsee::SubclassProcThunk<FilterableWindowData>
										ThunkFilterEditBoxSubclassProc;

				HWND					ParentWindow;
				HWND					FilterEditBox;
				HWND					FormListView;
				HWND					FilterLabel;

				std::string				FilterString;
				std::regex				FilterRegEx;
				UInt32					InputTimeoutThreshold;
				ULONGLONG				TimeCounter;
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

				LRESULT FormListSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);
				LRESULT	FilterEditBoxSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);

				bool	HasRegEx(void) const { return Flags & kFlags_RegEx; }
				bool	HasEditorID(void) const { return Flags & kFlags_SearchEditorID; }
				bool	HasName(void) const { return Flags & kFlags_SearchName; }
				bool	HasDescription(void) const { return Flags & kFlags_SearchDescription; }
				bool	HasFormID(void) const { return Flags & kFlags_SearchFormID; }

				bool	FilterForm(TESForm* Form);		// returns true if the form matches the active filter
				void	HandlePopupMenu(HWND Parent, int X, int Y);
			public:
				FilterableWindowData(HWND Parent, HWND EditBox, HWND FormList, HWND Label, int InputTimeoutThreshold, SecondaryFilterT UserFilter);
				~FilterableWindowData();

				bool	HandleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);		// returns true on timeout
				void	SetEnabled(bool State);

				bool	operator==(HWND FilterEditBox);
			};

			typedef std::vector<FilterableWindowData*>	FilterDataArrayT;

			FilterDataArrayT		ActiveFilters;

			FilterableWindowData*	Lookup(HWND FilterEdit);
		public:
			FilterableFormListManager();
			~FilterableFormListManager();

			bool	Register(HWND FilterEdit, HWND FilterLabel, HWND FormList, HWND ParentWindow, const SecondaryFilterT& UserFilter = nullptr, int InputTimeoutThreshold = 500);
			void	Unregister(HWND FilterEdit);

			bool	HandleMessages(HWND FilterEdit, UINT uMsg, WPARAM wParam, LPARAM lParam);		// returns true to request a refresh of the form list
			void	SetEnabled(HWND FilterEdit, bool State);

			static FilterableFormListManager Instance;
		};

		class FormEnumerationManager
		{
			bool	VisibilityDeletedForms;
			bool	VisibilityUnmodifiedForms;
		public:
			FormEnumerationManager();
			~FormEnumerationManager();

			bool	GetVisibleDeletedForms(void) const;
			bool	GetVisibleUnmodifiedForms(void) const;

			bool	ToggleVisibilityDeletedForms(void);
			bool	ToggleVisibilityUnmodifiedForms(void);

			bool	GetShouldEnumerate(TESForm* Form);
			void	ResetVisibility(void);

			int		CompareActiveForms(TESForm* FormA, TESForm* FormB, int OriginalResult);

			static FormEnumerationManager Instance;
		};


		class DeferredComboBoxController
		{
		public:
			// custom message passed to the combo box by our detour
			// wParam = const char* string, lParam = LPARAM data
			static constexpr UINT		CustomMessageAddItem = WM_USER + 3000;
			static constexpr LRESULT	AddStringMarkerResult = CB_ERRSPACE - 2;
		private:
			struct Message
			{
				UINT			uMsg;
				WPARAM			wParam;
				LPARAM			lParam;
				std::string		StringPayload;

				Message(UINT uMsg = WM_NULL, WPARAM wParam = NULL, LPARAM lParam = NULL);
			};

			struct TrackedData
			{
				std::vector<Message> PendingMessages;
				UInt32 TotalStringLength = 0;
				UInt32 LongestStringLength = 0;
			};


			std::unordered_map<HWND, TrackedData>
						ActiveComboBoxes;
			bgsee::SubclassProcThunk<DeferredComboBoxController>
						ThunkComboBoxSubclassProc;
			bool		Initialized;

			LRESULT		ComboBoxSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);
			void		RegisterComboBox(HWND hWnd);
			void		DeregisterComboBox(HWND hWnd);
			void		FlushQueuedMessages(HWND hWnd, bgsee::WindowSubclasser* Subclasser);
			void		QueueMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			void		SuspendComboBoxUpdates(HWND hWnd, bgsee::WindowSubclasser* Subclasser, bool Suspend) const;

			DeferredComboBoxController();
			~DeferredComboBoxController();
		public:
			void		Initialize();

			static DeferredComboBoxController Instance;
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
#define IDC_CSE_POPUP_GLOBALUNDO				9932
#define IDC_CSE_POPUP_GLOBALREDO				9933
