#pragma once
#include "UIManager.h"

namespace cse
{
	namespace uiManager
	{
		class MainWindowMiscData : public bgsee::WindowExtraData
		{
		public:
			Subwindow*			ToolbarExtras;

			MainWindowMiscData();
			virtual ~MainWindowMiscData();

			enum { kTypeID = 'XMWM' };
		};

		class MainWindowToolbarData : public bgsee::WindowExtraData
		{
		public:
			bool				SettingTODSlider;

			MainWindowToolbarData();
			virtual ~MainWindowToolbarData();

			enum { kTypeID = 'XMTD' };
		};

		LRESULT CALLBACK		MainWindowMenuInitSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);
		LRESULT CALLBACK		MainWindowMenuSelectSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);
		LRESULT CALLBACK		MainWindowMiscSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);
		LRESULT CALLBACK		MainWindowToolbarSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);

		void InitializeMainWindowOverrides();
	}
}
// custom window messages
#define WM_MAINWINDOW_INIT_DIALOG				(WM_USER + 2003)
#define WM_MAINWINDOW_INIT_EXTRADATA			(WM_USER + 2004)

#define WM_MAINTOOLBAR_INIT						(WM_USER + 2005)

// wParam = position
#define WM_MAINTOOLBAR_SETTOD					(WM_USER + 2006)