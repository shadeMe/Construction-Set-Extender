#pragma once
#include "UIManager.h"

namespace cse
{
	namespace uiManager
	{
		LRESULT CALLBACK ObjectWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);

		void InitializeObjectWindowOverrides();
	}
}