#pragma once
#include "UIManager.h"

namespace cse
{
	namespace uiManager
	{
		std::string ObjectWindowFormListGetColumnText(HWND FormList, TESForm* Form, UInt32 ColumnIndex);

		LRESULT CALLBACK ObjectWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);

		void InitializeObjectWindowOverrides();
	}
}