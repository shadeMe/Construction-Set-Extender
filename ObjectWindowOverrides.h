#pragma once
#include "UIManager.h"

namespace cse
{
	namespace uiManager
	{
		LRESULT CALLBACK ObjectWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser);

		void InitializeObjectWindowOverrides();
	}
}