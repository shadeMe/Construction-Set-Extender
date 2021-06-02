#pragma once
#include "UIManager.h"

namespace cse
{
	namespace uiManager
	{
#define IDC_OBJECTWINDOW_TAG_SEARCH_LABEL		3001
#define IDC_OBJECTWINDOW_TAG_SEARCH_EDIT		3002

		LRESULT CALLBACK ObjectWindowSplitterSubclassOverrideProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		class ObjectWindowExtraState : public bgsee::WindowExtraData
		{
		public:
			POINT	LastDialogDimensions;

			ObjectWindowExtraState(HWND ObjectWindow);
			virtual ~ObjectWindowExtraState() = default;

			enum { kTypeID = 'XOBJ' };
		};

		class ObjectWindowCommonHandlers
		{
		public:
			static void		OnInit(HWND hWnd, bgsee::WindowExtraDataCollection* ExtraData);
			static void		OnDestroy(HWND hWnd, UINT uMsg, bgsee::WindowExtraDataCollection* ExtraData);
			static void		OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData);
			static void		OnWindowPosChanging(HWND hWnd, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData);
			static void		OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam);
			static void		HandleFormListFilter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowExtraDataCollection* ExtraData);
		};

		LRESULT CALLBACK ObjectWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);

		void InitializeObjectWindowOverrides();
	}
}