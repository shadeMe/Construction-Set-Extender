#pragma once

namespace cse
{
// refreshes the imposter's form list
#define WM_OBJECTWINDOWIMPOSTER_REFRESHFORMLIST		(WM_USER + 2400)
// refreshes the imposter's tree view
#define WM_OBJECTWINDOWIMPOSTER_REFRESHTREEVIEW		(WM_USER + 2404)
// initializes the imposter's controls and data
#define WM_OBJECTWINDOWIMPOSTER_INITIALIZE			(WM_USER + 2402)
// initializes the extra fittings subclass, works around the delayed subclassing
// handled elsewhere by the subclass
#define WM_OBJECTWINDOWIMPOSTER_INITIALIZEXTRA		(WM_USER + 2403)

	// this implementation isn't without its share of bugs
	// most of them are related to form creation, control updates, i.e., failed sorting
	// hence we pick up the slack whenever necessary
	class ObjectWindowImposterManager
	{
		INT_PTR ImposterDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		struct ImposterData
		{
			UInt32				TreeSelection;			// last selected tree node
			int					SortColumns[TESObjectWindow::TreeEntryInfo::kTreeEntryCount];

			ImposterData();
		};

		// stores the original values of the caches and restores them on destruction
		// an added advantage is that the changes are transparent to the existing object window related additions
		// limitations: the addition of newly created forms will not be propagated immediately through all instances
		class CacheOperator
		{
			HWND				MainWindow;
			HWND				FormList;
			HWND				TreeView;
			HWND				Splitter;

			ImposterData*		ParentData;
			UInt32				TreeSelection;
			int					SortColumns[TESObjectWindow::TreeEntryInfo::kTreeEntryCount];
		public:
			CacheOperator(HWND Imposter);
			~CacheOperator();
		};

		typedef std::map<HWND, ImposterData*> ImposterTableT;

		ImposterTableT ImposterRegistry;
		bgsee::util::ThunkStdCall<ObjectWindowImposterManager, INT_PTR, HWND, UINT, WPARAM, LPARAM> ThunkImposterDlgProc;

		void										DisposeImposter(HWND Imposter);
		ImposterData*								GetImposterData(HWND Imposter) const;
	public:
		ObjectWindowImposterManager();
		~ObjectWindowImposterManager();

		void	SpawnImposter(void);
		void	RefreshImposters(void) const;
		void	DestroyImposters(void);
		bool	IsImposter(HWND hWnd) const;

		void	HandleObjectWindowSizing(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const;
		void	HandleObjectWindowActivating(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const;

		static ObjectWindowImposterManager			Instance;
	};

#define WM_PREVIEWWINDOWIMPOSTER_INITIALIZE			(WM_USER + 2405)

	// same as above but for the preview window
	class PreviewWindowImposterManager
	{
		INT_PTR ImposterDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		struct ImposterData
		{
			DWORD						InitTickCount;
			TESObjectREFR*				PreviewRef;
			TESObjectSTAT*				PreviewGround;
			TESPreviewControl*			Renderer;
			RECT						Bounds;
			TESBoundObject*				PreviewSource;
			BaseExtraList*				DialogExtraList;

			ImposterData();
			~ImposterData();
		};

		class CacheOperator
		{
			HWND					MainWindow;
			HWND					AnimationList;
			DWORD					InitTicks;
			TESObjectREFR*			PreviewRef;
			TESObjectSTAT*			PreviewGround;
			TESPreviewControl*		Renderer;

			ImposterData*			ParentData;
		public:
			CacheOperator(HWND Imposter);
			~CacheOperator();
		};

		typedef std::map<HWND, ImposterData*> ImposterTableT;

		ImposterTableT ImposterRegistry;
		bgsee::util::ThunkStdCall<PreviewWindowImposterManager, INT_PTR, HWND, UINT, WPARAM, LPARAM> ThunkImposterDlgProc;
		bool Enabled;

		void										DisposeImposterData(HWND Imposter);
		ImposterData*								GetImposterData(HWND Imposter) const;
	public:
		PreviewWindowImposterManager();
		~PreviewWindowImposterManager();

		void	SpawnImposter(TESBoundObject* Object);
		void	DestroyImposters(void);
		bool	IsImposter(HWND hWnd) const;

		bool	GetEnabled(void) const;
		void	SetEnabled(bool State);

		static PreviewWindowImposterManager			Instance;
	};
}