#pragma once

namespace cse
{
	namespace uiManager
	{
		class CellViewExtraData : public bgsee::WindowExtraData
		{
		public:
			RECT	RefFilterEditBox;		// init bounds of the new controls
			RECT	RefFilterLabel;
			RECT	XLabel;
			RECT	YLabel;
			RECT	XEdit;
			RECT	YEdit;
			RECT	GoBtn;
			RECT	CellFilterEditBox;
			RECT	CellFilterLabel;
			RECT	VisibleOnlyCheckBox;
			RECT	SelectionOnlyCheckBox;
			RECT	RefreshRefListBtn;

			CellViewExtraData();
			virtual ~CellViewExtraData();

			enum { kTypeID = 'XCVD' };

			enum
			{
				kExtraRefListColumn_Persistent = 5,
				kExtraRefListColumn_Disabled,
				kExtraRefListColumn_VWD,
				kExtraRefListColumn_EnableParent,
				kExtraRefListColumn_Count
			};

			static int CALLBACK CustomFormListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
			static bool RefListFilter(TESForm* Form);
		};

		LRESULT CALLBACK		CellViewWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);

		void InitializeCellViewWindowOverrides();
	}
}

// custom control IDs, as baked into the dialog templates
#define IDC_CSE_CELLVIEW_XLBL					9929
#define IDC_CSE_CELLVIEW_YLBL					9930
#define IDC_CSE_CELLVIEW_XEDIT					9931
#define IDC_CSE_CELLVIEW_YEDIT					9932
#define IDC_CSE_CELLVIEW_GOBTN					9933
#define IDC_CSE_CELLVIEW_VISIBLEONLYBTN			9938
#define IDC_CSE_CELLVIEW_SELECTEDONLYBTN		9939
#define IDC_CSE_CELLVIEW_REFRESHREFSBTN			9940
// the IDC_CSEFILTERABLEFORMLIST_XXX IDs are used for the ref list filter
#define IDC_CSE_CELLVIEW_CELLFILTEREDIT			9936
#define IDC_CSE_CELLVIEW_CELLFILTERLBL			9937
