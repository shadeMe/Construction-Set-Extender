#pragma once
#include "UIManager.h"

#define IDC_CSE_OBJECTWINDOW_VERTICAL_SPLITTER			10002
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_TREEVIEW		10000
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_LISTVIEW		10001
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_SPLITTER		10003
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_FILTERLBL		10004
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_FILTEREDIT		10005

namespace cse
{
	namespace uiManager
	{
		class ObjectWindowExtraState;

		enum ObjectWindowControlID : UInt32
		{
			kForm_FilterLabel = IDC_CSEFILTERABLEFORMLIST_FILTERLBL,
			kForm_FilterEdit = IDC_CSEFILTERABLEFORMLIST_FILTEREDIT,
			kForm_ListView = TESObjectWindow::kFormListCtrlID,
			kForm_TreeView = TESObjectWindow::kTreeViewCtrlID,
			kForm_Splitter = TESObjectWindow::kSplitterCtrlID,
			kTag_FilterLabel = IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_FILTERLBL,
			kTag_FilterEdit = IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_FILTEREDIT,
			kTag_ListView = IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_LISTVIEW,
			kTag_TreeView = IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_TREEVIEW,
			kTag_Splitter = IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_SPLITTER,
			kVerticalSplitter = IDC_CSE_OBJECTWINDOW_VERTICAL_SPLITTER,
		};

		struct SplitterState
		{
			ObjectWindowExtraState*
						WindowState;
			HWND		SplitterHandle;
			RECT*		SplitterBounds;
			bool		Enabled;
			bool		MouseOver;
			bool		Dragging;
			POINT		DragOrigin;
			ObjectWindowControlID
						ID;

			SplitterState(ObjectWindowExtraState* WindowState, ObjectWindowControlID ID);
			virtual ~SplitterState() = default;

			bool			IsMouseOver() const;
			void			UpdateCursor() const;
			virtual void	Move(LONG Delta) = 0;
			virtual LPCSTR	GetSizingCursor() const = 0;
		};

		struct HorizontalSplitter : public SplitterState
		{
			ObjectWindowControlID	FilterLabel;
			ObjectWindowControlID	FilterEdit;
			ObjectWindowControlID	TreeView;
			ObjectWindowControlID	ListView;

			HorizontalSplitter(ObjectWindowExtraState* WindowState, ObjectWindowControlID ID);
			virtual ~HorizontalSplitter() override = default;

			virtual void	Move(LONG Delta) override;
			virtual LPCSTR	GetSizingCursor() const override;
		};

		struct VerticalSplitter : public SplitterState
		{
		private:
			static constexpr auto kHidePanelMinOffset = 75;
			static constexpr auto kHidePanelMaxOffset = 75;

			void	ResizeControl(ObjectWindowControlID Control, LONG Delta) const;
		public:
			bool	TopHidden;
			bool	BottomHidden;

			VerticalSplitter(ObjectWindowExtraState* WindowState);
			virtual ~VerticalSplitter() override = default;

			virtual void	Move(LONG Delta) override;
			virtual LPCSTR	GetSizingCursor() const override;
		};

		class ObjectWindowExtraState : public bgsee::WindowExtraData
		{
			LRESULT		SplitterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);
			void		SetActiveSplitter(ObjectWindowControlID Splitter, bool ActiveState);
		public:
			HWND			Parent;
			std::map<ObjectWindowControlID, HWND>
							Handles;
			std::map<ObjectWindowControlID, RECT>
							CurrentRects;
			POINT			CurrentDialogSize;
			std::unique_ptr<HorizontalSplitter>
							FormViewSplitter;
			std::unique_ptr<HorizontalSplitter>
							TagViewSplitter;
			std::unique_ptr<VerticalSplitter>
							VerticalSplitter;
			bgsee::SubclassProcThunk<ObjectWindowExtraState>
							ThunkSplitterSubclassProc;

			ObjectWindowExtraState(HWND ObjectWindow);
			virtual ~ObjectWindowExtraState();

			enum { kTypeID = 'XOBJ' };

			void	UpdateRect(ObjectWindowControlID Control);
			void	UpdateRects();
			void	UpdateDialogSize(POINT * Delta = nullptr);
			bool	OnWindowPosChanging(WINDOWPOS* PosParams) const;
		};

		class ObjectWindowStateManager
		{
		public:
			static void		OnInit(HWND hWnd, bgsee::WindowExtraDataCollection* ExtraData);
			static void		OnDestroy(HWND hWnd, UINT uMsg, bgsee::WindowExtraDataCollection* ExtraData);
			static void		OnWindowPosChanging(HWND hWnd, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData);
			static void		OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData);
			static void		OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam);
			static void		HandleFilters(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData);
		};



		LRESULT CALLBACK ObjectWindowPrimarySubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);

		void InitializeObjectWindowOverrides();
	}
}

