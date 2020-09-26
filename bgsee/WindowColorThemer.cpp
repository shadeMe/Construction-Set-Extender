#include "WindowColorThemer.h"

namespace bgsee
{
#define WINDOWTHEMER_INISECTION				"ColorTheme"

	WindowThemer::Settings::Settings() :
		Enabled("Enabled", WINDOWTHEMER_INISECTION, "Enable CS color theming", (UInt32)0),
		Colors()
	{
		// Order-dependent
		Colors.emplace_back("DefaultColorBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");
		Colors.emplace_back("DefaultColorText", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "255,255,255");
		Colors.emplace_back("DefaultColorTextBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");
		Colors.emplace_back("ListViewColorBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
		Colors.emplace_back("ListViewColorText", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "255,255,255");
		Colors.emplace_back("ListViewColorTextBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
		Colors.emplace_back("ListViewColorGridRightBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "65,65,65");
		Colors.emplace_back("ListViewColorGridRightBorderShadow", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "29,38,48");
		Colors.emplace_back("ListViewHeaderColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "77,77,77");
		Colors.emplace_back("ListViewHeaderColorRightBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "65,65,65");
		Colors.emplace_back("ListViewHeaderColorBottomBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "29,38,48");
		Colors.emplace_back("ButtonColorFace", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");
		Colors.emplace_back("ButtonColorText", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "255,255,255");
		Colors.emplace_back("ButtonColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
		Colors.emplace_back("ButtonColorBorderHighlighted", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "0,120,215");
		Colors.emplace_back("ButtonColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
		Colors.emplace_back("ButtonColorPressed", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "83,83,83");
		Colors.emplace_back("ScrollbarColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "77,77,77");
		Colors.emplace_back("ScrollbarColorFillHighlighted", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "122,122,122");
		Colors.emplace_back("ScrollbarColorBackground", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "23,23,23");
		Colors.emplace_back("StatusBarColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
		Colors.emplace_back("StatusBarColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");
		Colors.emplace_back("TextBoxColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
		Colors.emplace_back("TextBoxColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
		Colors.emplace_back("ComboBoxColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
		Colors.emplace_back("ComboBoxColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "32,32,32");
		Colors.emplace_back("TabControlColorBorder", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "130,135,144");
		Colors.emplace_back("TabControlColorFill", WINDOWTHEMER_INISECTION, "Comma-separated RGB value", "56,56,56");

		SME_ASSERT(Colors.size() == kColor__MAX);
	}

	void WindowThemer::Settings::ParseColors(COLORREF* OutColors)
	{
		int i = 0;
		for (const auto& Setting : Colors)
		{
			OutColors[i] = SME::StringHelpers::GetRGB(Setting().s);
			++i;
		}
	}

	std::unordered_map<std::string, bgsee::WindowThemer::ThemeType> WindowThemer::WindowClassThemeTypes
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

}

