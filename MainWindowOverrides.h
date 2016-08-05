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

		LRESULT CALLBACK		MainWindowMenuInitSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		MainWindowMenuSelectSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		MainWindowMiscSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		MainWindowToolbarSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);

		void InitializeMainWindowOverrides();
	}
}
// custom window messages
#define WM_MAINWINDOW_INITEXTRADATA				(WM_USER + 2003)