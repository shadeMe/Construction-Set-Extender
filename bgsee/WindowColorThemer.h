#pragma once

#include "Main.h"
#include "UICommon.h"
#include "HookUtil.h"
#include "WindowSubclasser.h"

namespace bgsee
{
	class WindowColorThemer
	{
		static SME::INI::INISetting		kEnabled;
		static SME::INI::INISetting		kDefaultColorBackground;
		static SME::INI::INISetting		kDefaultColorText;
		static SME::INI::INISetting		kDefaultColorTextBackground;
		static SME::INI::INISetting		kListViewColorBackground;
		static SME::INI::INISetting		kListViewColorText;
		static SME::INI::INISetting		kListViewColorTextBackground;
		static SME::INI::INISetting		kListViewColorGridRightBorder;
		static SME::INI::INISetting		kListViewColorGridRightBorderShadow;
		static SME::INI::INISetting		kListViewHeaderColorFill;
		static SME::INI::INISetting		kListViewHeaderColorRightBorder;
		static SME::INI::INISetting		kListViewHeaderColorBottomBorder;
		static SME::INI::INISetting		kTreeViewColorBackground;
		static SME::INI::INISetting		kTreeViewColorText;
		static SME::INI::INISetting		kButtonColorFace;
		static SME::INI::INISetting		kButtonColorText;
		static SME::INI::INISetting		kButtonColorBorder;
		static SME::INI::INISetting		kButtonColorBorderHighlighted;
		static SME::INI::INISetting		kButtonColorFill;
		static SME::INI::INISetting		kButtonColorPressed;
		static SME::INI::INISetting		kScrollbarColorFill;
		static SME::INI::INISetting		kScrollbarColorFillHighlighted;
		static SME::INI::INISetting		kScrollbarColorBackground;
		static SME::INI::INISetting		kScrollbarArrowButtonColorFill;
		static SME::INI::INISetting		kStatusBarColorBorder;
		static SME::INI::INISetting		kStatusBarColorFill;
		static SME::INI::INISetting		kTextBoxColorBorder;
		static SME::INI::INISetting		kTextBoxColorFill;
		static SME::INI::INISetting		kComboBoxColorBorder;
		static SME::INI::INISetting		kComboBoxColorFill;
		static SME::INI::INISetting		kTabControlColorBorder;
		static SME::INI::INISetting		kTabControlColorFill;

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

		static const std::unordered_map<std::string, ThemeType>	WindowClassThemeTypes;

		enum
		{
			kColor__BEGIN = -1,

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

			kColor_TreeViewColorBackground,
			kColor_TreeViewColorText,

			kColor_ButtonColorFace,
			kColor_ButtonColorText,
			kColor_ButtonColorBorder,
			kColor_ButtonColorBorderHighlighted,
			kColor_ButtonColorFill,
			kColor_ButtonColorPressed,

			kColor_ScrollbarColorFill,
			kColor_ScrollbarColorFillHighlighted,
			kColor_ScrollbarColorBackground,
			kColor_ScrollbarArrowButtonColorFill,

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

		enum
		{
			kIATPatch__BEGIN = -1,

			kIATPatch_GetSysColor = 0,
			kIATPatch_GetSysColorBrush,
			kIATPatch_DrawThemeText,
			kIATPatch_DrawThemeBackground,

			kIATPatch__MAX
		};

		struct WindowThemeData
		{
			std::string	ClassName;
			ThemeType	Type;
			HTHEME		WindowThemeHandle;
			HTHEME		ScrollbarThemeHandle;
			union
			{
				struct
				{
					COLORREF TextColor;
				} RichEdit;

				struct
				{
					COLORREF TextColor;
					COLORREF TextBackgroundColor;
					COLORREF BackgroundColor;
				} ListView;

				struct
				{
					COLORREF TextColor;
					COLORREF BackgroundColor;
				} TreeView;

				struct
				{
					LONG_PTR Style;
				} TabControl;
			} InitialParams;

			WindowThemeData();
			WindowThemeData(const WindowThemeData& rhs);

			void	CacheInitialParams(HWND hWnd);
			void	HandleThemeInit(HWND hWnd, const COLORREF* Colors) const;
			void	HandleThemeDeinit(HWND hWnd) const;
		};

		DWORD		OwnerThreadID;
		SME::INI::INISetting*
					ColorSettings[kColor__MAX];
		COLORREF	Colors[kColor__MAX];
		std::unordered_map<UInt32, HBRUSH>
					Brushes;
		std::unordered_map<HTHEME, ThemeType>
					ActiveThemeHandles;
		std::unordered_map<HWND, WindowThemeData>
					ActiveThemedWindows;
		util::ThunkStdCall<WindowColorThemer, LRESULT, HWND, UINT, WPARAM, LPARAM, bool&, WindowExtraDataCollection*, WindowSubclasser*>
					ThunkThemeOverrideSubclass;
		util::ThunkStdCall<WindowColorThemer, DWORD, int>
					ThunkHookComctl32GetSysColor;
		util::ThunkStdCall<WindowColorThemer, HBRUSH, int>
					ThunkHookComctl32GetSysColorBrush;
		util::ThunkStdCall<WindowColorThemer, HRESULT, HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT>
					ThunkHookComctl32DrawThemeText;
		util::ThunkStdCall<WindowColorThemer, HRESULT, HTHEME, HDC, int, int, LPCRECT, LPCRECT>
					ThunkHookComctl32DrawThemeBackground;
		util::IATPatchData
					Comctl32IATPatches[kIATPatch__MAX];

		LRESULT		ThemeOverrideSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, WindowExtraDataCollection* ExtraData, WindowSubclasser* Subclasser);
		DWORD		HookComctl32GetSysColor(int nIndex);
		HBRUSH		HookComctl32GetSysColorBrush(int nIndex);
		HRESULT		HookComctl32DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect);
		HRESULT		HookComctl32DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect);

		void		InitializeIATPatches();
		void		ToggleIATPatches(bool Enabled) const;
		void		ReloadColors();
		void		RefreshActiveWindows() const;
	public:
		WindowColorThemer(WindowSubclasser* Subclasser);
		~WindowColorThemer();

		void		Enable();
		void		Disable();
		void		Refresh();
		bool		IsEnabled() const;

		static void	RegisterINISettings(INISettingDepotT& Depot);
	};
}
