#pragma once
#include "UIManager.h"

#define IDC_CSE_OBJECTWINDOW_VERTICAL_SPLITTER		10002
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_TREEVIEW		10000
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_LISTVIEW		10001
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_SPLITTER		10003
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_FILTERLBL		10004
#define IDC_CSE_OBJECTWINDOW_CUSTOM_TAGS_FILTEREDIT		10005

namespace cse
{
	namespace uiManager
	{
		class ObjectWindowExtraState : public bgsee::WindowExtraData
		{
			static constexpr auto kMinHorizontalSplitterWidth = 65;
			static constexpr auto kMinVerticalSplitterHeight = 65;
		public:
			enum ControlID : UInt32
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
				bool		Enabled = true;
				bool		MouseOver = false;
				bool		Dragging = false;
				POINT		DragOrigin = { 0 };
				ControlID	ID = kForm_Splitter;

				void	UpdateCursor() const;
			};
		private:
			LRESULT		SplitterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);

			bool		IsMouseOverSplitter(HWND hWnd) const;
			void		SetActiveSplitter(ControlID Splitter, bool ActiveState);
		public:
			HWND						Parent;
			std::map<ControlID, HWND>	Handles;
			std::map<ControlID, RECT>	CurrentRects;
			POINT						CurrentDialogSize;
			SplitterState				FormViewSplitterState;
			SplitterState				TagViewSplitterState;
			SplitterState				VerticalSplitterState;

			bgsee::SubclassProcThunk<ObjectWindowExtraState>
										ThunkSplitterSubclassProc;

			ObjectWindowExtraState(HWND ObjectWindow);
			virtual ~ObjectWindowExtraState();

			enum { kTypeID = 'XOBJ' };

			void	UpdateRects();
			void	UpdateDialogSize(POINT * Delta = nullptr);
			RECT	GetRelativeBounds(ControlID Control) const;
			bool	OnWindowPosChanging(WINDOWPOS* PosParams) const;
		};

		class ObjectWindowStateManager
		{
		public:
			static void		OnInit(HWND hWnd, bgsee::WindowExtraDataCollection* ExtraData);
			static void		OnDestroy(HWND hWnd, bgsee::WindowExtraDataCollection* ExtraData);
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

