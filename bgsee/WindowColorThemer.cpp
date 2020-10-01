#include "WindowColorThemer.h"
#include "UIManager.h"

namespace bgsee
{
	const std::unordered_map<std::string, bgsee::WindowColorThemer::ThemeType> WindowColorThemer::WindowClassThemeTypes
	{
		{ "msctls_statusbar32", ThemeType::StatusBar },
		{ "MDIClient", ThemeType::MDIClient },
		{ "Static", ThemeType::Static },
		{ "Edit", ThemeType::Edit },
		{ "RichEdit20A", ThemeType::RichEdit },
		{ "RICHEDIT50W", ThemeType::RichEdit },
		{ "Button", ThemeType::Button },
		{ "ComboBox", ThemeType::ComboBox },
		{ "SysHeader32", ThemeType::Header },
		{ "SysListView32", ThemeType::ListView },
		{ "SysTreeView32", ThemeType::TreeView },
		{ "SysTabControl32", ThemeType::TabControl },
	};

#define WINDOWTHEMER_INISECTION				"ColorTheme"

	SME::INI::INISetting    WindowColorThemer::kEnabled("Enabled", WINDOWTHEMER_INISECTION, "Enable CS color theming", (SInt32)0);
	SME::INI::INISetting    WindowColorThemer::kDefaultColorBackground("DefaultColorBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");
	SME::INI::INISetting    WindowColorThemer::kDefaultColorText("DefaultColorText", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "255,255,255");
	SME::INI::INISetting    WindowColorThemer::kDefaultColorTextBackground("DefaultColorTextBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");
	SME::INI::INISetting    WindowColorThemer::kListViewColorBackground("ListViewColorBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
	SME::INI::INISetting    WindowColorThemer::kListViewColorText("ListViewColorText", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "255,255,255");
	SME::INI::INISetting    WindowColorThemer::kListViewColorTextBackground("ListViewColorTextBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
	SME::INI::INISetting    WindowColorThemer::kListViewColorGridRightBorder("ListViewColorGridRightBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "65,65,65");
	SME::INI::INISetting    WindowColorThemer::kListViewColorGridRightBorderShadow("ListViewColorGridRightBorderShadow", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "29,38,48");
	SME::INI::INISetting    WindowColorThemer::kListViewHeaderColorFill("ListViewHeaderColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "77,77,77");
	SME::INI::INISetting    WindowColorThemer::kListViewHeaderColorRightBorder("ListViewHeaderColorRightBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "65,65,65");
	SME::INI::INISetting    WindowColorThemer::kListViewHeaderColorBottomBorder("ListViewHeaderColorBottomBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "29,38,48");
	SME::INI::INISetting    WindowColorThemer::kTreeViewColorBackground("TreeViewColorBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
	SME::INI::INISetting    WindowColorThemer::kTreeViewColorText("TreeViewColorText", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "255,255,255");
	SME::INI::INISetting    WindowColorThemer::kButtonColorFace("ButtonColorFace", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");
	SME::INI::INISetting    WindowColorThemer::kButtonColorText("ButtonColorText", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "255,255,255");
	SME::INI::INISetting    WindowColorThemer::kButtonColorBorder("ButtonColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
	SME::INI::INISetting    WindowColorThemer::kButtonColorBorderHighlighted("ButtonColorBorderHighlighted", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "0,120,215");
	SME::INI::INISetting    WindowColorThemer::kButtonColorFill("ButtonColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
	SME::INI::INISetting    WindowColorThemer::kButtonColorPressed("ButtonColorPressed", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "83,83,83");
	SME::INI::INISetting    WindowColorThemer::kScrollbarColorFill("ScrollbarColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "77,77,77");
	SME::INI::INISetting    WindowColorThemer::kScrollbarColorFillHighlighted("ScrollbarColorFillHighlighted", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "122,122,122");
	SME::INI::INISetting    WindowColorThemer::kScrollbarColorBackground("ScrollbarColorBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "23,23,23");
	SME::INI::INISetting    WindowColorThemer::kScrollbarArrowButtonColorFill("ScrollbarArrowButtonColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "100,100,100");
	SME::INI::INISetting    WindowColorThemer::kStatusBarColorBorder("StatusBarColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
	SME::INI::INISetting    WindowColorThemer::kStatusBarColorFill("StatusBarColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");
	SME::INI::INISetting    WindowColorThemer::kTextBoxColorBorder("TextBoxColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
	SME::INI::INISetting    WindowColorThemer::kTextBoxColorFill("TextBoxColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
	SME::INI::INISetting    WindowColorThemer::kComboBoxColorBorder("ComboBoxColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
	SME::INI::INISetting    WindowColorThemer::kComboBoxColorFill("ComboBoxColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
	SME::INI::INISetting    WindowColorThemer::kTabControlColorBorder("TabControlColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
	SME::INI::INISetting    WindowColorThemer::kTabControlColorFill("TabControlColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");


	WindowColorThemer::WindowThemeData::WindowThemeData()
	{
		ClassName = "";
		Type = ThemeType::None;
		WindowThemeHandle = ScrollbarThemeHandle = nullptr;
		ZeroMemory(&InitialParams, sizeof(InitialParams));
	}

	WindowColorThemer::WindowThemeData::WindowThemeData(const WindowThemeData& rhs)
	{
		ClassName = rhs.ClassName;
		Type = rhs.Type;
		WindowThemeHandle = rhs.WindowThemeHandle;
		ScrollbarThemeHandle = rhs.ScrollbarThemeHandle;
		memcpy(&InitialParams, &rhs.InitialParams, sizeof(InitialParams));
	}

	void WindowColorThemer::WindowThemeData::CacheInitialParams(HWND hWnd)
	{
		switch (Type)
		{
		case ThemeType::RichEdit:
		{
			CHARFORMAT2A Format = {};
			Format.cbSize = sizeof(Format);
			Format.dwMask = CFM_COLOR;
			SendMessage(hWnd, EM_GETCHARFORMAT, SCF_DEFAULT, reinterpret_cast<LPARAM>(&Format));
			InitialParams.RichEdit.TextColor = Format.crTextColor;

			break;
		}
		case ThemeType::ListView:
			InitialParams.ListView.TextColor = ListView_GetTextColor(hWnd);
			InitialParams.ListView.TextBackgroundColor = ListView_GetTextBkColor(hWnd);
			InitialParams.ListView.BackgroundColor = ListView_GetBkColor(hWnd);

			break;
		case ThemeType::TreeView:
			InitialParams.TreeView.TextColor = TreeView_GetTextColor(hWnd);
			InitialParams.TreeView.BackgroundColor = TreeView_GetBkColor(hWnd);

			break;
		case ThemeType::TabControl:
			InitialParams.TabControl.Style = GetWindowLongPtrA(hWnd, GWL_STYLE);

			break;
		}
	}

	void WindowColorThemer::WindowThemeData::HandleThemeInit(HWND hWnd, const COLORREF* Colors) const
	{
		switch (Type)
		{
		case ThemeType::RichEdit:
		{
			CHARFORMAT2A Format = {};
			Format.cbSize = sizeof(Format);
			Format.dwMask = CFM_COLOR;
			Format.crTextColor = Colors[kColor_DefaultColorText];
			SendMessage(hWnd, EM_SETCHARFORMAT, SCF_ALL, reinterpret_cast<LPARAM>(&Format));
			SendMessage(hWnd, EM_SETBKGNDCOLOR, 0, Colors[kColor_DefaultColorBackground]);

			break;
		}
		case ThemeType::ListView:
			ListView_SetTextColor(hWnd, Colors[kColor_ListViewColorText]);
			ListView_SetTextBkColor(hWnd, Colors[kColor_ListViewColorTextBackground]);
			ListView_SetBkColor(hWnd, Colors[kColor_ListViewColorBackground]);

			break;
		case ThemeType::TreeView:
			TreeView_SetTextColor(hWnd, Colors[kColor_TreeViewColorText]);
			TreeView_SetBkColor(hWnd, Colors[kColor_TreeViewColorBackground]);

			break;
		case ThemeType::TabControl:
			SetWindowLongPtr(hWnd, GWL_STYLE, (InitialParams.TabControl.Style & ~TCS_BUTTONS) | TCS_TABS);
			SetWindowTheme(hWnd, nullptr, nullptr);
			break;
		}

	}

	void WindowColorThemer::WindowThemeData::HandleThemeDeinit(HWND hWnd) const
	{
		switch (Type)
		{
		case ThemeType::RichEdit:
		{
			CHARFORMAT2A Format = {};
			Format.cbSize = sizeof(Format);
			Format.dwMask = CFM_COLOR;
			Format.crTextColor = InitialParams.RichEdit.TextColor;
			SendMessage(hWnd, EM_SETCHARFORMAT, SCF_ALL, reinterpret_cast<LPARAM>(&Format));
			SendMessage(hWnd, EM_SETBKGNDCOLOR, TRUE, 0);

			break;
		}
		case ThemeType::ListView:
			ListView_SetTextColor(hWnd, InitialParams.ListView.TextColor);
			ListView_SetTextBkColor(hWnd, InitialParams.ListView.TextBackgroundColor);
			ListView_SetBkColor(hWnd, InitialParams.ListView.BackgroundColor);

			break;
		case ThemeType::TreeView:
			TreeView_SetTextColor(hWnd, InitialParams.TreeView.TextColor);
			TreeView_SetBkColor(hWnd, InitialParams.TreeView.BackgroundColor);

			break;
		case ThemeType::TabControl:
			SetWindowLongPtr(hWnd, GWL_STYLE, (InitialParams.TabControl.Style & ~TCS_TABS) | TCS_BUTTONS);
			break;
		}
	}

	LRESULT WindowColorThemer::ThemeOverrideSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, WindowExtraDataCollection* ExtraData, WindowSubclasser* Subclasser)
	{
		LRESULT CallbackResult = FALSE;

		switch (uMsg)
		{
		case WM_SUBCLASSER_POST_WM_INITDIALOG:
		case WM_SUBCLASSER_POST_WM_CREATE:
		{
			if (ActiveThemedWindows.find(hWnd) != ActiveThemedWindows.end())
				break;

			WindowThemeData ThemeData;
			char ClassName[256] = {};
			GetClassName(hWnd, ClassName, ARRAYSIZE(ClassName));
			ThemeData.ClassName = ClassName;
			ThemeData.WindowThemeHandle = GetWindowTheme(hWnd);

			auto ClassThemeMatch = WindowClassThemeTypes.find(ClassName);
			if (ClassThemeMatch != WindowClassThemeTypes.end())
				ThemeData.Type = ClassThemeMatch->second;

			if (ThemeData.Type == ThemeType::None)
				break;

			switch (ThemeData.Type)
			{
			case ThemeType::ListView:
			case ThemeType::TreeView:
				ThemeData.ScrollbarThemeHandle = OpenThemeData(hWnd, VSCLASS_SCROLLBAR);
				break;
			}

			ThemeData.CacheInitialParams(hWnd);

			if (IsEnabled())
			{
				ThemeData.HandleThemeInit(hWnd, Colors);
				if (ThemeData.Type == ThemeType::TabControl)
					ThemeData.WindowThemeHandle = GetWindowTheme(hWnd);
			}

			if (ThemeData.WindowThemeHandle)
				ActiveThemeHandles.emplace(ThemeData.WindowThemeHandle, ThemeData.Type);

			if (ThemeData.ScrollbarThemeHandle)
				ActiveThemeHandles.emplace(ThemeData.ScrollbarThemeHandle, ThemeType::ScrollBar);

			ActiveThemedWindows.emplace(hWnd, ThemeData);

			break;
		}
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSTATIC:
		{
			if (!IsEnabled())
				break;

			Return = true;
			HDC hdc = reinterpret_cast<HDC>(wParam);
			if (hdc)
			{
				SetTextColor(hdc, Colors[kColor_DefaultColorText]);
				SetBkColor(hdc, Colors[kColor_DefaultColorBackground]);
			}

			CallbackResult = reinterpret_cast<INT_PTR>(Brushes[kColor_DefaultColorBackground]);
			break;
		}
		case LVM_SETEXTENDEDLISTVIEWSTYLE:
		{
			if (!IsEnabled())
				break;

			const auto ThemeDataMatch = ActiveThemedWindows.find(hWnd);
			if (ThemeDataMatch == ActiveThemedWindows.end())
				break;

			auto& ThemeData = ThemeDataMatch->second;

			if (ThemeData.Type == ThemeType::ListView)
			{
				// Prevent the OS grid separators from drawing
				Return = true;

				wParam &= ~static_cast<WPARAM>(LVS_EX_GRIDLINES);
				lParam &= ~static_cast<LPARAM>(LVS_EX_GRIDLINES);

				CallbackResult = Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);
			}

			break;
		}
		case WM_PAINT:
		{
			if (!IsEnabled())
				break;

			const auto ThemeDataMatch = ActiveThemedWindows.find(hWnd);
			if (ThemeDataMatch == ActiveThemedWindows.end())
				break;

			auto& ThemeData = ThemeDataMatch->second;

			switch (ThemeData.Type)
			{
			case ThemeType::MDIClient:
			{
				HDC hdc = GetDC(hWnd);
				if (hdc)
				{
					RECT windowArea;
					GetClientRect(hWnd, &windowArea);

					FillRect(hdc, &windowArea, Brushes[kColor_DefaultColorBackground]);
					ReleaseDC(hWnd, hdc);
				}

				break;
			}
			case ThemeType::ListView:
			{
				// Paint normally, then apply custom grid lines
				Return = true;
				CallbackResult = Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);

				RECT headerRect;
				GetClientRect(ListView_GetHeader(hWnd), &headerRect);

				RECT listRect;
				GetClientRect(hWnd, &listRect);

				HDC hdc = GetDC(hWnd);
				if (hdc)
				{
					HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(DC_PEN));
					int x = 0 - GetScrollPos(hWnd, SB_HORZ);

					LVCOLUMN colInfo = {};
					colInfo.mask = LVCF_WIDTH;

					for (int col = 0; ListView_GetColumn(hWnd, col, &colInfo); col++)
					{
						x += colInfo.cx;

						// Stop drawing if outside the listview client area
						if (x >= listRect.right)
							break;

						// Right border
						POINT verts[2]
						{
							{ x - 2, headerRect.bottom },
							{ x - 2, listRect.bottom },
						};

						SetDCPenColor(hdc, Colors[kColor_ListViewColorGridRightBorder]);
						Polyline(hdc, verts, 2);

						// Right border shadow
						verts[0].x += 1;
						verts[1].x += 1;

						SetDCPenColor(hdc, Colors[kColor_ListViewColorGridRightBorderShadow]);
						Polyline(hdc, verts, 2);
					}

					SelectObject(hdc, oldPen);
					ReleaseDC(hWnd, hdc);
				}

				break;
			}
			}

			break;
		}
		case WM_DESTROY:
			// theme handles are shared between windows, and there aren't too many of them
			// so we can hold on to them for the entire session without releasing them
			auto Match = ActiveThemedWindows.find(hWnd);
			if (Match != ActiveThemedWindows.end())
				ActiveThemedWindows.erase(hWnd);

			break;
		}


		return CallbackResult;
	}

	DWORD WindowColorThemer::HookComctl32GetSysColor(int nIndex)
	{
		if (OwnerThreadID != GetCurrentThreadId() || BGSEEUI->GetSubclasser()->GetMostRecentWindowHandle() == nullptr)
			return GetSysColor(nIndex);

		switch (nIndex)
		{
		case COLOR_BTNFACE:
			return Colors[kColor_ButtonColorFace];
		case COLOR_BTNTEXT:
			return Colors[kColor_ButtonColorText];
		}

		return GetSysColor(nIndex);
	}

	HBRUSH WindowColorThemer::HookComctl32GetSysColorBrush(int nIndex)
	{
		if (OwnerThreadID != GetCurrentThreadId() || BGSEEUI->GetSubclasser()->GetMostRecentWindowHandle() == nullptr)
			return GetSysColorBrush(nIndex);

		switch (nIndex)
		{
		case COLOR_BTNFACE:
			return Brushes[kColor_ButtonColorFace];
		case COLOR_BTNTEXT:
			return Brushes[kColor_ButtonColorText];
		}

		return GetSysColorBrush(nIndex);
	}

	HRESULT WindowColorThemer::HookComctl32DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect)
	{
		if (OwnerThreadID != GetCurrentThreadId() || BGSEEUI->GetSubclasser()->GetMostRecentWindowHandle() == nullptr)
			return DrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, pRect);

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, Colors[kColor_DefaultColorText]);

		RECT temp = *pRect;
		DrawTextW(hdc, pszText, cchText, &temp, dwTextFlags);

		return S_OK;
	}

	HRESULT WindowColorThemer::HookComctl32DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect)
	{
		if (OwnerThreadID != GetCurrentThreadId() || BGSEEUI->GetSubclasser()->GetMostRecentWindowHandle() == nullptr)
			return DrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);

		ThemeType ThemeType = ThemeType::None;

		const auto ThemeHandleMatch = ActiveThemeHandles.find(hTheme);
		if (ThemeHandleMatch == ActiveThemeHandles.end())
			return DrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);

		ThemeType = ThemeHandleMatch->second;

		switch (ThemeType)
		{
		case ThemeType::ScrollBar:
		{
			auto scrollbarFill = Brushes[kColor_ScrollbarColorFill];
			auto scrollbarFillHighlighted = Brushes[kColor_ScrollbarColorFillHighlighted];
			auto scrollbarBackground = Brushes[kColor_ScrollbarColorBackground];
			auto scrollbarArrowButtonFill = Brushes[kColor_ScrollbarArrowButtonColorFill];

			switch (iPartId)
			{
			case SBP_THUMBBTNHORZ:	// Horizontal drag bar
			case SBP_THUMBBTNVERT:	// Vertical drag bar
			{
				if (iStateId == SCRBS_HOT || iStateId == SCRBS_PRESSED)
					FillRect(hdc, pRect, scrollbarFillHighlighted);
				else
					FillRect(hdc, pRect, scrollbarFill);
			}
			return S_OK;

			case SBP_LOWERTRACKHORZ:// Horizontal background
			case SBP_UPPERTRACKHORZ:// Horizontal background
			case SBP_LOWERTRACKVERT:// Vertical background
			case SBP_UPPERTRACKVERT:// Vertical background
			{
				FillRect(hdc, pRect, scrollbarBackground);
			}
			return S_OK;

			case SBP_ARROWBTN:		// Arrow button
			{
				switch (iStateId)
				{
				case ABS_UPHOT:
				case ABS_DOWNHOT:
				case ABS_LEFTHOT:
				case ABS_RIGHTHOT:
					FillRect(hdc, pRect, scrollbarFillHighlighted);
					break;
				default:
					FillRect(hdc, pRect, scrollbarArrowButtonFill);
				}
				return S_OK;

			}
			case SBP_GRIPPERHORZ:	// Horizontal resize scrollbar
			case SBP_GRIPPERVERT:	// Vertical resize scrollbar
			case SBP_SIZEBOX:		// Resize box, bottom right
			case SBP_SIZEBOXBKGND:	// Resize box, background, unused
				break;
			}

			break;
		}
		case ThemeType::StatusBar:
		{
			auto statusBarBorder = Brushes[kColor_StatusBarColorBorder];
			auto statusBarFill = Brushes[kColor_StatusBarColorFill];

			switch (iPartId)
			{
			case 0:
			{
				// Outside border (top, right)
				FillRect(hdc, pRect, statusBarBorder);
			}
			return S_OK;

			case SP_PANE:
			case SP_GRIPPERPANE:
			case SP_GRIPPER:
			{
				// Everything else
				FillRect(hdc, pRect, statusBarFill);
			}
			return S_OK;
			}

			break;
		}
		case ThemeType::Edit:
		{
			auto editControlBorder = Brushes[kColor_TextBoxColorBorder];
			auto editControlFill = Brushes[kColor_TextBoxColorFill];

			switch (iPartId)
			{
			case EP_EDITBORDER_NOSCROLL:
			{
				FillRect(hdc, pRect, editControlFill);
				FrameRect(hdc, pRect, editControlBorder);
			}
			return S_OK;
			}

			break;
		}
		case ThemeType::Button:
		{
			auto buttonBorder = Brushes[kColor_ButtonColorBorder];
			auto buttonBorderHighlighted = Brushes[kColor_ButtonColorBorderHighlighted];
			auto buttonFill = Brushes[kColor_ButtonColorFill];
			auto buttonPressed = Brushes[kColor_ButtonColorPressed];

			switch (iPartId)
			{
			case BP_PUSHBUTTON:
			{
				HBRUSH frameColor = buttonBorder;
				HBRUSH fillColor = buttonFill;

				switch (iStateId)
				{
				case PBS_HOT:
				case PBS_DEFAULTED:
					frameColor = buttonBorderHighlighted;
					break;

				case PBS_DISABLED:
					fillColor = buttonBorder;
					break;

				case PBS_PRESSED:
					fillColor = buttonPressed;
					break;
				}

				FillRect(hdc, pRect, fillColor);
				FrameRect(hdc, pRect, frameColor);
			}
			return S_OK;

			case BP_CHECKBOX:
			{
				if (iStateId == CBS_UNCHECKEDDISABLED ||
					iStateId == CBS_CHECKEDDISABLED ||
					iStateId == CBS_MIXEDDISABLED ||
					iStateId == CBS_IMPLICITDISABLED ||
					iStateId == CBS_EXCLUDEDDISABLED)
				{
					FrameRect(hdc, pRect, buttonBorder);
					return S_OK;
				}
			}
			break;
			}

			break;
		}
		case ThemeType::ComboBox:
		{
			auto comboBoxBorder = Brushes[kColor_ComboBoxColorBorder];
			auto comboBoxFill =	Brushes[kColor_ComboBoxColorFill];

			switch (iPartId)
			{
			case CP_READONLY:			// Main control
			{
				FillRect(hdc, pRect, iStateId == CBRO_DISABLED ? comboBoxBorder : comboBoxFill);
				FrameRect(hdc, pRect, comboBoxBorder);
			}
			return S_OK;

			case CP_BORDER:				// Main control with text edit
			{
				// Special case: dropdown arrow needs to be drawn
				DrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
				FillRect(hdc, pRect, comboBoxFill);

				if (iStateId == CBB_DISABLED)
					FillRect(hdc, pRect, comboBoxBorder);
				else
					FrameRect(hdc, pRect, comboBoxBorder);
			}
			return S_OK;

			case CP_DROPDOWNBUTTONRIGHT:// Dropdown arrow
			case CP_DROPDOWNBUTTONLEFT:	// Dropdown arrow
				break;

			case CP_DROPDOWNBUTTON:
			case CP_BACKGROUND:
			case CP_TRANSPARENTBACKGROUND:
			case CP_CUEBANNER:
			case CP_DROPDOWNITEM:
				return S_OK;
			}

			break;
		}
		case ThemeType::Header:
		{
			auto headerFill = Brushes[kColor_ListViewHeaderColorFill];

			switch (iPartId)
			{
			case 0:
			case HP_HEADERITEM:
			case HP_HEADERITEMLEFT:
			case HP_HEADERITEMRIGHT:
			{
				HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(DC_PEN));

				for (int i = 0; i < 2; i++)
				{
					if (i == 0)
						SetDCPenColor(hdc, Colors[kColor_ListViewColorGridRightBorder]);
					else
						SetDCPenColor(hdc, Colors[kColor_ListViewColorGridRightBorderShadow]);

					DWORD counts[2] = { 2, 2 };
					POINT verts[4] =
					{
						// Right border
						{ pRect->right - 2 + i, pRect->top },
						{ pRect->right - 2 + i, pRect->bottom },

						// Bottom border
						{ pRect->left - 1, pRect->bottom - 2 + i },
						{ pRect->right - 2, pRect->bottom - 2 + i },
					};

					PolyPolyline(hdc, verts, counts, 2);
				}

				// Fill background on hover (1px padding for border shadow)
				if ((iPartId == 0 && iStateId == HIS_HOT) ||
					(iPartId == HP_HEADERITEM && iStateId == HIS_HOT) ||
					(iPartId == HP_HEADERITEMLEFT && iStateId == HILS_HOT) ||
					(iPartId == HP_HEADERITEMRIGHT && iStateId == HIRS_HOT))
				{
					RECT padded = { pRect->left - 1, pRect->top, pRect->right - 1, pRect->bottom - 1 };

					FillRect(hdc, &padded, headerFill);
				}

				SelectObject(hdc, oldPen);
			}
			return S_OK;
			}

			break;
		}
		case ThemeType::TabControl:
		{
			auto tabControlButtonBorder =	Brushes[kColor_TabControlColorBorder];
			auto tabControlButtonFill =		Brushes[kColor_TabControlColorFill];

			switch (iPartId)
			{
			case TABP_TABITEM:				// TCS_MULTILINE middle buttons
			case TABP_TABITEMLEFTEDGE:		// TCS_MULTILINE leftmost button
			case TABP_TABITEMRIGHTEDGE:		// TCS_MULTILINE rightmost button
			case TABP_TABITEMBOTHEDGE:		// TCS_MULTILINE ???
			case TABP_TOPTABITEM:			// Middle buttons
			case TABP_TOPTABITEMLEFTEDGE:	// Leftmost button
			case TABP_TOPTABITEMRIGHTEDGE:	// Rightmost button
			case TABP_TOPTABITEMBOTHEDGE:	// ???
			{
				RECT paddedRect = *pRect;
				RECT insideRect = { pRect->left + 1, pRect->top + 1, pRect->right - 1, pRect->bottom - 1 };

				bool isHover = (iPartId == TABP_TABITEM && iStateId == TIS_HOT) ||
					(iPartId == TABP_TABITEMLEFTEDGE && iStateId == TILES_HOT) ||
					(iPartId == TABP_TABITEMRIGHTEDGE && iStateId == TIRES_HOT) ||
					(iPartId == TABP_TABITEMBOTHEDGE && iStateId == TIBES_HOT) ||
					(iPartId == TABP_TOPTABITEM && iStateId == TTIS_HOT) ||
					(iPartId == TABP_TOPTABITEMLEFTEDGE && iStateId == TTILES_HOT) ||
					(iPartId == TABP_TOPTABITEMRIGHTEDGE && iStateId == TTIRES_HOT) ||
					(iPartId == TABP_TOPTABITEMBOTHEDGE && iStateId == TTIBES_HOT);

				if ((iPartId == TABP_TABITEM && iStateId == TIS_SELECTED) ||
					(iPartId == TABP_TABITEMLEFTEDGE && iStateId == TILES_SELECTED) ||
					(iPartId == TABP_TABITEMRIGHTEDGE && iStateId == TIRES_SELECTED) ||
					(iPartId == TABP_TABITEMBOTHEDGE && iStateId == TIBES_SELECTED) ||
					(iPartId == TABP_TOPTABITEM && iStateId == TTIS_SELECTED) ||
					(iPartId == TABP_TOPTABITEMLEFTEDGE && iStateId == TTILES_SELECTED) ||
					(iPartId == TABP_TOPTABITEMRIGHTEDGE && iStateId == TTIRES_SELECTED) ||
					(iPartId == TABP_TOPTABITEMBOTHEDGE && iStateId == TTIBES_SELECTED))
				{
					paddedRect.top += 1;
					paddedRect.bottom -= 2;

					// Allow the rect to overlap so the bottom border outline is removed
					insideRect.top += 1;
					insideRect.bottom += 1;
				}

				FrameRect(hdc, &paddedRect, tabControlButtonBorder);
				FillRect(hdc, &insideRect, isHover ? tabControlButtonBorder : tabControlButtonFill);
			}
			return S_OK;

			case TABP_PANE:
				FrameRect(hdc, pRect, tabControlButtonBorder);

				return S_OK;
			}

			break;
		}
		}

		return DrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
	}

	void WindowColorThemer::InitializeIATPatches()
	{
		auto ModuleBase = reinterpret_cast<uintptr_t>(GetModuleHandle("comctl32.dll"));
		SME_ASSERT(ModuleBase);

		for (int i = kIATPatch__BEGIN + 1; i < kIATPatch__MAX; ++i)
		{
			const char* ImportName = nullptr;
			const char* DLLName = nullptr;
			void* CallbackFunction = nullptr;
			bool DelayedHook = false;

			switch (i)
			{
			case kIATPatch_GetSysColor:
				DLLName = "USER32.dll";
				ImportName = "GetSysColor";
				CallbackFunction = ThunkHookComctl32GetSysColor();
				break;
			case kIATPatch_GetSysColorBrush:
				DLLName = "USER32.dll";
				ImportName = "GetSysColorBrush";
				CallbackFunction = ThunkHookComctl32GetSysColorBrush();
				break;
			case kIATPatch_DrawThemeText:
				DLLName = "UxTheme.dll";
				ImportName = "DrawThemeText";
				CallbackFunction = ThunkHookComctl32DrawThemeText();
				DelayedHook = true;
				break;
			case kIATPatch_DrawThemeBackground:
				DLLName = "UxTheme.dll";
				ImportName = "DrawThemeBackground";
				CallbackFunction = ThunkHookComctl32DrawThemeBackground();
				DelayedHook = true;
				break;
			}

			auto& Patch = Comctl32IATPatches[i];
			Patch.ModuleBase = ModuleBase;
			Patch.ImportModule = DLLName;
			Patch.ImportName = ImportName;
			Patch.Location = 0;
			Patch.Delayed = DelayedHook;
			Patch.CallbackFunction = reinterpret_cast<uintptr_t>(CallbackFunction);

			auto PatchSuccessful = Patch.Initialize();
			SME_ASSERT(PatchSuccessful);
		}
	}

	void WindowColorThemer::ToggleIATPatches(bool Enabled) const
	{
		// ### TODO this is potentially dangerous operation in a threaded context
		// make writes to memory atomic

		for (int i = kIATPatch__BEGIN + 1; i < kIATPatch__MAX; ++i)
		{
			if (Enabled)
				Comctl32IATPatches[i].Replace();
			else
				Comctl32IATPatches[i].Reset();
		}
	}

	void WindowColorThemer::ReloadColors()
	{
		for (const auto& Itr: Brushes)
			DeleteObject(Itr.second);

		Brushes.clear();

		for (int i = kColor__BEGIN + 1; i < kColor__MAX; ++i)
		{
			const auto& Setting = ColorSettings[i];
			Colors[i] = SME::StringHelpers::GetRGB(Setting->GetData().s);
			Brushes.emplace(i, CreateSolidBrush(Colors[i]));
		}
	}

	void WindowColorThemer::RefreshActiveWindows() const
	{
		for (const auto& Itr : ActiveThemedWindows)
		{
			Itr.second.HandleThemeDeinit(Itr.first);

			if (IsEnabled())
				Itr.second.HandleThemeInit(Itr.first, Colors);

			RedrawWindow(Itr.first, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}

	WindowColorThemer::WindowColorThemer(WindowSubclasser* Subclasser) :
		ThunkThemeOverrideSubclass(this, &WindowColorThemer::ThemeOverrideSubclass),
		ThunkHookComctl32GetSysColor(this, &WindowColorThemer::HookComctl32GetSysColor),
		ThunkHookComctl32GetSysColorBrush(this, &WindowColorThemer::HookComctl32GetSysColorBrush),
		ThunkHookComctl32DrawThemeText(this, &WindowColorThemer::HookComctl32DrawThemeText),
		ThunkHookComctl32DrawThemeBackground(this, &WindowColorThemer::HookComctl32DrawThemeBackground)
	{
		OwnerThreadID = GetCurrentThreadId();

		ColorSettings[kColor_DefaultColorBackground] = &kDefaultColorBackground;
		ColorSettings[kColor_DefaultColorText] = &kDefaultColorText;
		ColorSettings[kColor_DefaultColorTextBackground] = &kDefaultColorTextBackground;
		ColorSettings[kColor_ListViewColorBackground] = &kListViewColorBackground;
		ColorSettings[kColor_ListViewColorText] = &kListViewColorText;
		ColorSettings[kColor_ListViewColorTextBackground] = &kListViewColorTextBackground;
		ColorSettings[kColor_ListViewColorGridRightBorder] = &kListViewColorGridRightBorder;
		ColorSettings[kColor_ListViewColorGridRightBorderShadow] = &kListViewColorGridRightBorderShadow;
		ColorSettings[kColor_ListViewHeaderColorFill] = &kListViewHeaderColorFill;
		ColorSettings[kColor_ListViewHeaderColorRightBorder] = &kListViewHeaderColorRightBorder;
		ColorSettings[kColor_ListViewHeaderColorBottomBorder] = &kListViewHeaderColorBottomBorder;
		ColorSettings[kColor_TreeViewColorBackground] = &kTreeViewColorBackground;
		ColorSettings[kColor_TreeViewColorText] = &kTreeViewColorText;
		ColorSettings[kColor_ButtonColorFace] = &kButtonColorFace;
		ColorSettings[kColor_ButtonColorText] = &kButtonColorText;
		ColorSettings[kColor_ButtonColorBorder] = &kButtonColorBorder;
		ColorSettings[kColor_ButtonColorBorderHighlighted] = &kButtonColorBorderHighlighted;
		ColorSettings[kColor_ButtonColorFill] = &kButtonColorFill;
		ColorSettings[kColor_ButtonColorPressed] = &kButtonColorPressed;
		ColorSettings[kColor_ScrollbarColorFill] = &kScrollbarColorFill;
		ColorSettings[kColor_ScrollbarColorFillHighlighted] = &kScrollbarColorFillHighlighted;
		ColorSettings[kColor_ScrollbarColorBackground] = &kScrollbarColorBackground;
		ColorSettings[kColor_ScrollbarArrowButtonColorFill] = &kScrollbarArrowButtonColorFill;
		ColorSettings[kColor_StatusBarColorBorder] = &kStatusBarColorBorder;
		ColorSettings[kColor_StatusBarColorFill] = &kStatusBarColorFill;
		ColorSettings[kColor_TextBoxColorBorder] = &kTextBoxColorBorder;
		ColorSettings[kColor_TextBoxColorFill] = &kTextBoxColorFill;
		ColorSettings[kColor_ComboBoxColorBorder] = &kComboBoxColorBorder;
		ColorSettings[kColor_ComboBoxColorFill] = &kComboBoxColorFill;
		ColorSettings[kColor_TabControlColorBorder] = &kTabControlColorBorder;
		ColorSettings[kColor_TabControlColorFill] = &kTabControlColorFill;

		InitializeIATPatches();
		ReloadColors();

		if (IsEnabled())
			ToggleIATPatches(true);

		Subclasser->RegisterGlobalSubclass(ThunkThemeOverrideSubclass(), -9999);
	}

	WindowColorThemer::~WindowColorThemer()
	{
		if (IsEnabled())
			ToggleIATPatches(false);

		for (const auto& Itr: Brushes)
			DeleteObject(Itr.second);

		Brushes.clear();
	}

	void WindowColorThemer::Enable()
	{
		if (IsEnabled())
			return;

		kEnabled.SetInt(1);

		ToggleIATPatches(true);
		ReloadColors();
		RefreshActiveWindows();
	}

	void WindowColorThemer::Disable()
	{
		if (!IsEnabled())
			return;

		kEnabled.SetInt(0);

		ToggleIATPatches(false);
		RefreshActiveWindows();
	}

	void WindowColorThemer::Refresh()
	{
		ReloadColors();
		RefreshActiveWindows();
	}

	bool WindowColorThemer::IsEnabled() const
	{
		return kEnabled().i != 0;
	}

	void WindowColorThemer::RegisterINISettings(INISettingDepotT& Depot)
	{
		Depot.push_back(&kEnabled);

		Depot.push_back(&kDefaultColorBackground);
		Depot.push_back(&kDefaultColorText);
		Depot.push_back(&kDefaultColorTextBackground);
		Depot.push_back(&kListViewColorBackground);
		Depot.push_back(&kListViewColorText);
		Depot.push_back(&kListViewColorTextBackground);
		Depot.push_back(&kListViewColorGridRightBorder);
		Depot.push_back(&kListViewColorGridRightBorderShadow);
		Depot.push_back(&kListViewHeaderColorFill);
		Depot.push_back(&kListViewHeaderColorRightBorder);
		Depot.push_back(&kListViewHeaderColorBottomBorder);
		Depot.push_back(&kTreeViewColorBackground);
		Depot.push_back(&kTreeViewColorText);
		Depot.push_back(&kButtonColorFace);
		Depot.push_back(&kButtonColorText);
		Depot.push_back(&kButtonColorBorder);
		Depot.push_back(&kButtonColorBorderHighlighted);
		Depot.push_back(&kButtonColorFill);
		Depot.push_back(&kButtonColorPressed);
		Depot.push_back(&kScrollbarColorFill);
		Depot.push_back(&kScrollbarColorFillHighlighted);
		Depot.push_back(&kScrollbarColorBackground);
		Depot.push_back(&kScrollbarArrowButtonColorFill);
		Depot.push_back(&kStatusBarColorBorder);
		Depot.push_back(&kStatusBarColorFill);
		Depot.push_back(&kTextBoxColorBorder);
		Depot.push_back(&kTextBoxColorFill);
		Depot.push_back(&kComboBoxColorBorder);
		Depot.push_back(&kComboBoxColorFill);
		Depot.push_back(&kTabControlColorBorder);
		Depot.push_back(&kTabControlColorFill);
	}

}

