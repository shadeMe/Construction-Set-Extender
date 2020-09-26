#pragma once

#include "UICommon.h"

namespace bgsee
{
	class WindowThemer
	{
		enum class ThemeType
		{
			None,
			ScrollBar,
			StatusBar,
			MDIClient,
			Static,
			Edit,
			RichEdit,
			Button,
			ComboBox,
			Header,
			ListView,
			TreeView,
			TabControl,
		};

		static std::unordered_map<std::string, ThemeType>	WindowClassThemeTypes;


		enum
		{
			kColor_DefaultColorBackground = 0,
			kColor_DefaultColorText,
			kColor_DefaultColorTextBackground,

			kColor_ListViewColorBackground,
			kColor_ListViewColorText,
			kColor_ListViewColorTextBackground,
			kColor_ListViewColorGridRightBorder,
			kColor_ListViewColorGridRightBorderShadow,

			kColor_ListViewHeaderColorFill,
			kColor_ListViewHeaderColorRightBorder,
			kColor_ListViewHeaderColorBottomBorder,

			kColor_ButtonColorFace,
			kColor_ButtonColorText,
			kColor_ButtonColorBorder,
			kColor_ButtonColorBorderHighlighted,
			kColor_ButtonColorFill,
			kColor_ButtonColorPressed,

			kColor_ScrollbarColorFill,
			kColor_ScrollbarColorFillHighlighted,
			kColor_ScrollbarColorBackground,

			kColor_StatusBarColorBorder,
			kColor_StatusBarColorFill,

			kColor_TextBoxColorBorder,
			kColor_TextBoxColorFill,

			kColor_ComboBoxColorBorder,
			kColor_ComboBoxColorFill,

			kColor_TabControlColorBorder,
			kColor_TabControlColorFill,

			kColor__MAX
		};

		struct Settings
		{
			INISetting				Enabled;
			std::vector<INISetting>	Colors;

			Settings();

			void ParseColors(COLORREF* OutColors);
		};

		LRESULT		CallbackThemeOverride(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT		CallbackCallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
		DWORD		HookComctl32GetSysColor(int nIndex);
		HBRUSH		HookComctl32GetSysColorBrush(int nIndex);
		HRESULT		HookComctl32DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect);
		HRESULT		HookComctl32DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect);


		Settings	Settings;
		COLORREF	Colors[kColor__MAX];
		std::unordered_map<HTHEME, ThemeType>
					ThemeHandles;

		void		PatchComctl32();
	public:
		WindowThemer();
		~WindowThemer();

		void		Initialize();
	};
}
