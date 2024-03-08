#include "imgui.h"
#include "imgui_internal.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"

namespace ImGui
{
	bool IsItemHoveredRect()
	{
		return IsItemHovered(ImGuiHoveredFlags_RectOnly);
	}

	void SetNextWindowPosCenter(ImGuiCond cond /*= 0*/)
	{
		const auto& DisplaySize(GetIO().DisplaySize);
		SetNextWindowPos(ImVec2(DisplaySize.x * 0.5f, DisplaySize.y * 0.5f), cond, ImVec2(0.5f, 0.5f));
	}

	void PushStyleCompact()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.70f)));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.70f)));
	}

	void PopStyleCompact()
	{
		ImGui::PopStyleVar(2);
	}

	void ShowHelpPopup(const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);

		const char* Icon = " " ICON_MD_HELP;

		ImGui::TextDisabled("%s", Icon);
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextV(fmt, va);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}

		va_end(va);
	}

	bool CheckBoxTristate(const char* label, int* v_tristate)
	{
		bool ret;
		if (*v_tristate == -1)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);
			bool b = false;
			ret = ImGui::Checkbox(label, &b);
			if (ret)
				*v_tristate = 1;
			ImGui::PopItemFlag();
		}
		else
		{
			bool b = (*v_tristate != 0);
			ret = ImGui::Checkbox(label, &b);
			if (ret)
				*v_tristate = (int)b;
		}
		return ret;
	}

	void MoveNextWindowToSafeZone(const ImVec2& WindowPos, const ImVec2& WindowSize)
	{
		const ImVec2 SafeZoneMargin(30, 30);
		if (WindowPos.x + WindowSize.x <= 0 || WindowPos.x >= *TESRenderWindow::ScreenWidth - SafeZoneMargin.x)
			SetNextWindowPosCenter();
		else if (WindowPos.y + WindowSize.y <= 0 || WindowPos.y >= *TESRenderWindow::ScreenHeight - SafeZoneMargin.y)
			SetNextWindowPosCenter();
	}

	void ResetMouseKeys()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
		io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
		io.AddMouseButtonEvent(ImGuiMouseButton_Right, false);
		io.AddMouseButtonEvent(ImGuiMouseButton_Middle, false);
	}

	void ResetKeyboardKeys()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.AddKeyEvent(ImGuiKey_Tab, false);
		io.AddKeyEvent(ImGuiKey_LeftArrow, false);
		io.AddKeyEvent(ImGuiKey_RightArrow, false);
		io.AddKeyEvent(ImGuiKey_UpArrow, false);
		io.AddKeyEvent(ImGuiKey_DownArrow, false);
		io.AddKeyEvent(ImGuiKey_PageUp, false);
		io.AddKeyEvent(ImGuiKey_PageDown, false);
		io.AddKeyEvent(ImGuiKey_Home, false);
		io.AddKeyEvent(ImGuiKey_End, false);
		io.AddKeyEvent(ImGuiKey_Insert, false);
		io.AddKeyEvent(ImGuiKey_Delete, false);
		io.AddKeyEvent(ImGuiKey_Backspace, false);
		io.AddKeyEvent(ImGuiKey_Space, false);
		io.AddKeyEvent(ImGuiKey_Enter, false);
		io.AddKeyEvent(ImGuiKey_Escape, false);
		io.AddKeyEvent(ImGuiKey_Apostrophe, false);
		io.AddKeyEvent(ImGuiKey_Comma, false);
		io.AddKeyEvent(ImGuiKey_Minus, false);
		io.AddKeyEvent(ImGuiKey_Period, false);
		io.AddKeyEvent(ImGuiKey_Slash, false);
		io.AddKeyEvent(ImGuiKey_Semicolon, false);
		io.AddKeyEvent(ImGuiKey_Equal, false);
		io.AddKeyEvent(ImGuiKey_LeftBracket, false);
		io.AddKeyEvent(ImGuiKey_Backslash, false);
		io.AddKeyEvent(ImGuiKey_RightBracket, false);
		io.AddKeyEvent(ImGuiKey_GraveAccent, false);
		io.AddKeyEvent(ImGuiKey_CapsLock, false);
		io.AddKeyEvent(ImGuiKey_ScrollLock, false);
		io.AddKeyEvent(ImGuiKey_NumLock, false);
		io.AddKeyEvent(ImGuiKey_PrintScreen, false);
		io.AddKeyEvent(ImGuiKey_Pause, false);
		io.AddKeyEvent(ImGuiKey_Keypad0, false);
		io.AddKeyEvent(ImGuiKey_Keypad1, false);
		io.AddKeyEvent(ImGuiKey_Keypad2, false);
		io.AddKeyEvent(ImGuiKey_Keypad3, false);
		io.AddKeyEvent(ImGuiKey_Keypad4, false);
		io.AddKeyEvent(ImGuiKey_Keypad5, false);
		io.AddKeyEvent(ImGuiKey_Keypad6, false);
		io.AddKeyEvent(ImGuiKey_Keypad7, false);
		io.AddKeyEvent(ImGuiKey_Keypad8, false);
		io.AddKeyEvent(ImGuiKey_Keypad9, false);
		io.AddKeyEvent(ImGuiKey_KeypadDecimal, false);
		io.AddKeyEvent(ImGuiKey_KeypadDivide, false);
		io.AddKeyEvent(ImGuiKey_KeypadMultiply, false);
		io.AddKeyEvent(ImGuiKey_KeypadSubtract, false);
		io.AddKeyEvent(ImGuiKey_KeypadAdd, false);
		io.AddKeyEvent(ImGuiKey_KeypadEnter, false);
		io.AddKeyEvent(ImGuiKey_LeftShift, false);
		io.AddKeyEvent(ImGuiKey_LeftCtrl, false);
		io.AddKeyEvent(ImGuiKey_LeftAlt, false);
		io.AddKeyEvent(ImGuiKey_LeftSuper, false);
		io.AddKeyEvent(ImGuiKey_RightShift, false);
		io.AddKeyEvent(ImGuiKey_RightCtrl, false);
		io.AddKeyEvent(ImGuiKey_RightAlt, false);
		io.AddKeyEvent(ImGuiKey_RightSuper, false);
		io.AddKeyEvent(ImGuiKey_Menu, false);
		io.AddKeyEvent(ImGuiKey_0, false);
		io.AddKeyEvent(ImGuiKey_1, false);
		io.AddKeyEvent(ImGuiKey_2, false);
		io.AddKeyEvent(ImGuiKey_3, false);
		io.AddKeyEvent(ImGuiKey_4, false);
		io.AddKeyEvent(ImGuiKey_5, false);
		io.AddKeyEvent(ImGuiKey_6, false);
		io.AddKeyEvent(ImGuiKey_7, false);
		io.AddKeyEvent(ImGuiKey_8, false);
		io.AddKeyEvent(ImGuiKey_9, false);
		io.AddKeyEvent(ImGuiKey_A, false);
		io.AddKeyEvent(ImGuiKey_B, false);
		io.AddKeyEvent(ImGuiKey_C, false);
		io.AddKeyEvent(ImGuiKey_D, false);
		io.AddKeyEvent(ImGuiKey_E, false);
		io.AddKeyEvent(ImGuiKey_F, false);
		io.AddKeyEvent(ImGuiKey_G, false);
		io.AddKeyEvent(ImGuiKey_H, false);
		io.AddKeyEvent(ImGuiKey_I, false);
		io.AddKeyEvent(ImGuiKey_J, false);
		io.AddKeyEvent(ImGuiKey_K, false);
		io.AddKeyEvent(ImGuiKey_L, false);
		io.AddKeyEvent(ImGuiKey_M, false);
		io.AddKeyEvent(ImGuiKey_N, false);
		io.AddKeyEvent(ImGuiKey_O, false);
		io.AddKeyEvent(ImGuiKey_P, false);
		io.AddKeyEvent(ImGuiKey_Q, false);
		io.AddKeyEvent(ImGuiKey_R, false);
		io.AddKeyEvent(ImGuiKey_S, false);
		io.AddKeyEvent(ImGuiKey_T, false);
		io.AddKeyEvent(ImGuiKey_U, false);
		io.AddKeyEvent(ImGuiKey_V, false);
		io.AddKeyEvent(ImGuiKey_W, false);
		io.AddKeyEvent(ImGuiKey_X, false);
		io.AddKeyEvent(ImGuiKey_Y, false);
		io.AddKeyEvent(ImGuiKey_Z, false);
		io.AddKeyEvent(ImGuiKey_F1, false);
		io.AddKeyEvent(ImGuiKey_F2, false);
		io.AddKeyEvent(ImGuiKey_F3, false);
		io.AddKeyEvent(ImGuiKey_F4, false);
		io.AddKeyEvent(ImGuiKey_F5, false);
		io.AddKeyEvent(ImGuiKey_F6, false);
		io.AddKeyEvent(ImGuiKey_F7, false);
		io.AddKeyEvent(ImGuiKey_F8, false);
		io.AddKeyEvent(ImGuiKey_F9, false);
		io.AddKeyEvent(ImGuiKey_F10, false);
		io.AddKeyEvent(ImGuiKey_F11, false);
		io.AddKeyEvent(ImGuiKey_F12, false);
		io.AddKeyEvent(ImGuiKey_F13, false);
		io.AddKeyEvent(ImGuiKey_F14, false);
		io.AddKeyEvent(ImGuiKey_F15, false);
		io.AddKeyEvent(ImGuiKey_F16, false);
		io.AddKeyEvent(ImGuiKey_F17, false);
		io.AddKeyEvent(ImGuiKey_F18, false);
		io.AddKeyEvent(ImGuiKey_F19, false);
		io.AddKeyEvent(ImGuiKey_F20, false);
		io.AddKeyEvent(ImGuiKey_F21, false);
		io.AddKeyEvent(ImGuiKey_F22, false);
		io.AddKeyEvent(ImGuiKey_F23, false);
		io.AddKeyEvent(ImGuiKey_F24, false);
		io.AddKeyEvent(ImGuiKey_AppBack, false);
		io.AddKeyEvent(ImGuiKey_AppForward, false);
	}

	ImGuiKey VirtualKeyToImGuiKey(WPARAM wParam)
	{
		switch (wParam)
		{
		case VK_TAB: return ImGuiKey_Tab;
		case VK_LEFT: return ImGuiKey_LeftArrow;
		case VK_RIGHT: return ImGuiKey_RightArrow;
		case VK_UP: return ImGuiKey_UpArrow;
		case VK_DOWN: return ImGuiKey_DownArrow;
		case VK_PRIOR: return ImGuiKey_PageUp;
		case VK_NEXT: return ImGuiKey_PageDown;
		case VK_HOME: return ImGuiKey_Home;
		case VK_END: return ImGuiKey_End;
		case VK_INSERT: return ImGuiKey_Insert;
		case VK_DELETE: return ImGuiKey_Delete;
		case VK_BACK: return ImGuiKey_Backspace;
		case VK_SPACE: return ImGuiKey_Space;
		case VK_RETURN: return ImGuiKey_Enter;
		case VK_ESCAPE: return ImGuiKey_Escape;
		case VK_OEM_7: return ImGuiKey_Apostrophe;
		case VK_OEM_COMMA: return ImGuiKey_Comma;
		case VK_OEM_MINUS: return ImGuiKey_Minus;
		case VK_OEM_PERIOD: return ImGuiKey_Period;
		case VK_OEM_2: return ImGuiKey_Slash;
		case VK_OEM_1: return ImGuiKey_Semicolon;
		case VK_OEM_PLUS: return ImGuiKey_Equal;
		case VK_OEM_4: return ImGuiKey_LeftBracket;
		case VK_OEM_5: return ImGuiKey_Backslash;
		case VK_OEM_6: return ImGuiKey_RightBracket;
		case VK_OEM_3: return ImGuiKey_GraveAccent;
		case VK_CAPITAL: return ImGuiKey_CapsLock;
		case VK_SCROLL: return ImGuiKey_ScrollLock;
		case VK_NUMLOCK: return ImGuiKey_NumLock;
		case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
		case VK_PAUSE: return ImGuiKey_Pause;
		case VK_NUMPAD0: return ImGuiKey_Keypad0;
		case VK_NUMPAD1: return ImGuiKey_Keypad1;
		case VK_NUMPAD2: return ImGuiKey_Keypad2;
		case VK_NUMPAD3: return ImGuiKey_Keypad3;
		case VK_NUMPAD4: return ImGuiKey_Keypad4;
		case VK_NUMPAD5: return ImGuiKey_Keypad5;
		case VK_NUMPAD6: return ImGuiKey_Keypad6;
		case VK_NUMPAD7: return ImGuiKey_Keypad7;
		case VK_NUMPAD8: return ImGuiKey_Keypad8;
		case VK_NUMPAD9: return ImGuiKey_Keypad9;
		case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
		case VK_DIVIDE: return ImGuiKey_KeypadDivide;
		case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
		case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
		case VK_ADD: return ImGuiKey_KeypadAdd;
		case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
		case VK_LSHIFT: return ImGuiKey_LeftShift;
		case VK_LCONTROL: return ImGuiKey_LeftCtrl;
		case VK_LMENU: return ImGuiKey_LeftAlt;
		case VK_LWIN: return ImGuiKey_LeftSuper;
		case VK_RSHIFT: return ImGuiKey_RightShift;
		case VK_RCONTROL: return ImGuiKey_RightCtrl;
		case VK_RMENU: return ImGuiKey_RightAlt;
		case VK_RWIN: return ImGuiKey_RightSuper;
		case VK_APPS: return ImGuiKey_Menu;
		case '0': return ImGuiKey_0;
		case '1': return ImGuiKey_1;
		case '2': return ImGuiKey_2;
		case '3': return ImGuiKey_3;
		case '4': return ImGuiKey_4;
		case '5': return ImGuiKey_5;
		case '6': return ImGuiKey_6;
		case '7': return ImGuiKey_7;
		case '8': return ImGuiKey_8;
		case '9': return ImGuiKey_9;
		case 'A': return ImGuiKey_A;
		case 'B': return ImGuiKey_B;
		case 'C': return ImGuiKey_C;
		case 'D': return ImGuiKey_D;
		case 'E': return ImGuiKey_E;
		case 'F': return ImGuiKey_F;
		case 'G': return ImGuiKey_G;
		case 'H': return ImGuiKey_H;
		case 'I': return ImGuiKey_I;
		case 'J': return ImGuiKey_J;
		case 'K': return ImGuiKey_K;
		case 'L': return ImGuiKey_L;
		case 'M': return ImGuiKey_M;
		case 'N': return ImGuiKey_N;
		case 'O': return ImGuiKey_O;
		case 'P': return ImGuiKey_P;
		case 'Q': return ImGuiKey_Q;
		case 'R': return ImGuiKey_R;
		case 'S': return ImGuiKey_S;
		case 'T': return ImGuiKey_T;
		case 'U': return ImGuiKey_U;
		case 'V': return ImGuiKey_V;
		case 'W': return ImGuiKey_W;
		case 'X': return ImGuiKey_X;
		case 'Y': return ImGuiKey_Y;
		case 'Z': return ImGuiKey_Z;
		case VK_F1: return ImGuiKey_F1;
		case VK_F2: return ImGuiKey_F2;
		case VK_F3: return ImGuiKey_F3;
		case VK_F4: return ImGuiKey_F4;
		case VK_F5: return ImGuiKey_F5;
		case VK_F6: return ImGuiKey_F6;
		case VK_F7: return ImGuiKey_F7;
		case VK_F8: return ImGuiKey_F8;
		case VK_F9: return ImGuiKey_F9;
		case VK_F10: return ImGuiKey_F10;
		case VK_F11: return ImGuiKey_F11;
		case VK_F12: return ImGuiKey_F12;
		case VK_F13: return ImGuiKey_F13;
		case VK_F14: return ImGuiKey_F14;
		case VK_F15: return ImGuiKey_F15;
		case VK_F16: return ImGuiKey_F16;
		case VK_F17: return ImGuiKey_F17;
		case VK_F18: return ImGuiKey_F18;
		case VK_F19: return ImGuiKey_F19;
		case VK_F20: return ImGuiKey_F20;
		case VK_F21: return ImGuiKey_F21;
		case VK_F22: return ImGuiKey_F22;
		case VK_F23: return ImGuiKey_F23;
		case VK_F24: return ImGuiKey_F24;
		case VK_BROWSER_BACK: return ImGuiKey_AppBack;
		case VK_BROWSER_FORWARD: return ImGuiKey_AppForward;
		default: return ImGuiKey_None;
		}
	}

	SHORT ImGuiKeyToVirtualKey(ImGuiKey Key)
	{
		switch (Key)
		{
		case ImGuiKey_Tab: return VK_TAB;
		case ImGuiKey_LeftArrow: return VK_LEFT;
		case ImGuiKey_RightArrow: return VK_RIGHT;
		case ImGuiKey_UpArrow: return VK_UP;
		case ImGuiKey_DownArrow: return VK_DOWN;
		case ImGuiKey_PageUp: return VK_PRIOR;
		case ImGuiKey_PageDown: return VK_NEXT;
		case ImGuiKey_Home: return VK_HOME;
		case ImGuiKey_End: return VK_END;
		case ImGuiKey_Insert: return VK_INSERT;
		case ImGuiKey_Delete: return VK_DELETE;
		case ImGuiKey_Backspace: return VK_BACK;
		case ImGuiKey_Space: return VK_SPACE;
		case ImGuiKey_Enter: return VK_RETURN;
		case ImGuiKey_Escape: return VK_ESCAPE;
		case ImGuiKey_Apostrophe: return VK_OEM_7;
		case ImGuiKey_Comma: return VK_OEM_COMMA;
		case ImGuiKey_Minus: return VK_OEM_MINUS;
		case ImGuiKey_Period: return VK_OEM_PERIOD;
		case ImGuiKey_Slash: return VK_OEM_2;
		case ImGuiKey_Semicolon: return VK_OEM_1;
		case ImGuiKey_Equal: return VK_OEM_PLUS;
		case ImGuiKey_LeftBracket: return VK_OEM_4;
		case ImGuiKey_Backslash: return VK_OEM_5;
		case ImGuiKey_RightBracket: return VK_OEM_6;
		case ImGuiKey_GraveAccent: return VK_OEM_3;
		case ImGuiKey_CapsLock: return VK_CAPITAL;
		case ImGuiKey_ScrollLock: return VK_SCROLL;
		case ImGuiKey_NumLock: return VK_NUMLOCK;
		case ImGuiKey_PrintScreen: return VK_SNAPSHOT;
		case ImGuiKey_Pause: return VK_PAUSE;
		case ImGuiKey_Keypad0: return VK_NUMPAD0;
		case ImGuiKey_Keypad1: return VK_NUMPAD1;
		case ImGuiKey_Keypad2: return VK_NUMPAD2;
		case ImGuiKey_Keypad3: return VK_NUMPAD3;
		case ImGuiKey_Keypad4: return VK_NUMPAD4;
		case ImGuiKey_Keypad5: return VK_NUMPAD5;
		case ImGuiKey_Keypad6: return VK_NUMPAD6;
		case ImGuiKey_Keypad7: return VK_NUMPAD7;
		case ImGuiKey_Keypad8: return VK_NUMPAD8;
		case ImGuiKey_Keypad9: return VK_NUMPAD9;
		case ImGuiKey_KeypadDecimal: return VK_DECIMAL;
		case ImGuiKey_KeypadDivide: return VK_DIVIDE;
		case ImGuiKey_KeypadMultiply: return VK_MULTIPLY;
		case ImGuiKey_KeypadSubtract: return VK_SUBTRACT;
		case ImGuiKey_KeypadAdd: return VK_ADD;
		case ImGuiKey_KeypadEnter: return IM_VK_KEYPAD_ENTER;
		case ImGuiKey_LeftShift: return VK_LSHIFT;
		case ImGuiKey_LeftCtrl: return VK_LCONTROL;
		case ImGuiKey_LeftAlt: return VK_LMENU;
		case ImGuiKey_LeftSuper: return VK_LWIN;
		case ImGuiKey_RightShift: return VK_RSHIFT;
		case ImGuiKey_RightCtrl: return VK_RCONTROL;
		case ImGuiKey_RightAlt: return VK_RMENU;
		case ImGuiKey_RightSuper: return VK_RWIN;
		case ImGuiKey_Menu: return VK_APPS;
		case ImGuiKey_0: return '0';
		case ImGuiKey_1: return '1';
		case ImGuiKey_2: return '2';
		case ImGuiKey_3: return '3';
		case ImGuiKey_4: return '4';
		case ImGuiKey_5: return '5';
		case ImGuiKey_6: return '6';
		case ImGuiKey_7: return '7';
		case ImGuiKey_8: return '8';
		case ImGuiKey_9: return '9';
		case ImGuiKey_A: return 'A';
		case ImGuiKey_B: return 'B';
		case ImGuiKey_C: return 'C';
		case ImGuiKey_D: return 'D';
		case ImGuiKey_E: return 'E';
		case ImGuiKey_F: return 'F';
		case ImGuiKey_G: return 'G';
		case ImGuiKey_H: return 'H';
		case ImGuiKey_I: return 'I';
		case ImGuiKey_J: return 'J';
		case ImGuiKey_K: return 'K';
		case ImGuiKey_L: return 'L';
		case ImGuiKey_M: return 'M';
		case ImGuiKey_N: return 'N';
		case ImGuiKey_O: return 'O';
		case ImGuiKey_P: return 'P';
		case ImGuiKey_Q: return 'Q';
		case ImGuiKey_R: return 'R';
		case ImGuiKey_S: return 'S';
		case ImGuiKey_T: return 'T';
		case ImGuiKey_U: return 'U';
		case ImGuiKey_V: return 'V';
		case ImGuiKey_W: return 'W';
		case ImGuiKey_X: return 'X';
		case ImGuiKey_Y: return 'Y';
		case ImGuiKey_Z: return 'Z';
		case ImGuiKey_F1: return VK_F1;
		case ImGuiKey_F2: return VK_F2;
		case ImGuiKey_F3: return VK_F3;
		case ImGuiKey_F4: return VK_F4;
		case ImGuiKey_F5: return VK_F5;
		case ImGuiKey_F6: return VK_F6;
		case ImGuiKey_F7: return VK_F7;
		case ImGuiKey_F8: return VK_F8;
		case ImGuiKey_F9: return VK_F9;
		case ImGuiKey_F10: return VK_F10;
		case ImGuiKey_F11: return VK_F11;
		case ImGuiKey_F12: return VK_F12;
		case ImGuiKey_F13: return VK_F13;
		case ImGuiKey_F14: return VK_F14;
		case ImGuiKey_F15: return VK_F15;
		case ImGuiKey_F16: return VK_F16;
		case ImGuiKey_F17: return VK_F17;
		case ImGuiKey_F18: return VK_F18;
		case ImGuiKey_F19: return VK_F19;
		case ImGuiKey_F20: return VK_F20;
		case ImGuiKey_F21: return VK_F21;
		case ImGuiKey_F22: return VK_F22;
		case ImGuiKey_F23: return VK_F23;
		case ImGuiKey_F24: return VK_F24;
		case ImGuiKey_AppBack: return VK_BROWSER_BACK;
		case ImGuiKey_AppForward: return VK_BROWSER_FORWARD;
		default: return 0;
		};
	}

	const std::initializer_list<ImGuiKey> &NamedKeys()
	{
		static auto Keys = {
			ImGuiKey_Tab,
			ImGuiKey_LeftArrow,
			ImGuiKey_RightArrow,
			ImGuiKey_UpArrow,
			ImGuiKey_DownArrow,
			ImGuiKey_PageUp,
			ImGuiKey_PageDown,
			ImGuiKey_Home,
			ImGuiKey_End,
			ImGuiKey_Insert,
			ImGuiKey_Delete,
			ImGuiKey_Backspace,
			ImGuiKey_Space,
			ImGuiKey_Enter,
			ImGuiKey_Escape,
			ImGuiKey_Apostrophe,
			ImGuiKey_Comma,
			ImGuiKey_Minus,
			ImGuiKey_Period,
			ImGuiKey_Slash,
			ImGuiKey_Semicolon,
			ImGuiKey_Equal,
			ImGuiKey_LeftBracket,
			ImGuiKey_Backslash,
			ImGuiKey_RightBracket,
			ImGuiKey_GraveAccent,
			ImGuiKey_CapsLock,
			ImGuiKey_ScrollLock,
			ImGuiKey_NumLock,
			ImGuiKey_PrintScreen,
			ImGuiKey_Pause,
			ImGuiKey_Keypad0,
			ImGuiKey_Keypad1,
			ImGuiKey_Keypad2,
			ImGuiKey_Keypad3,
			ImGuiKey_Keypad4,
			ImGuiKey_Keypad5,
			ImGuiKey_Keypad6,
			ImGuiKey_Keypad7,
			ImGuiKey_Keypad8,
			ImGuiKey_Keypad9,
			ImGuiKey_KeypadDecimal,
			ImGuiKey_KeypadDivide,
			ImGuiKey_KeypadMultiply,
			ImGuiKey_KeypadSubtract,
			ImGuiKey_KeypadAdd,
			ImGuiKey_KeypadEnter,
			ImGuiKey_LeftShift,
			ImGuiKey_LeftCtrl,
			ImGuiKey_LeftAlt,
			ImGuiKey_LeftSuper,
			ImGuiKey_RightShift,
			ImGuiKey_RightCtrl,
			ImGuiKey_RightAlt,
			ImGuiKey_RightSuper,
			ImGuiKey_Menu,
			ImGuiKey_0,
			ImGuiKey_1,
			ImGuiKey_2,
			ImGuiKey_3,
			ImGuiKey_4,
			ImGuiKey_5,
			ImGuiKey_6,
			ImGuiKey_7,
			ImGuiKey_8,
			ImGuiKey_9,
			ImGuiKey_A,
			ImGuiKey_B,
			ImGuiKey_C,
			ImGuiKey_D,
			ImGuiKey_E,
			ImGuiKey_F,
			ImGuiKey_G,
			ImGuiKey_H,
			ImGuiKey_I,
			ImGuiKey_J,
			ImGuiKey_K,
			ImGuiKey_L,
			ImGuiKey_M,
			ImGuiKey_N,
			ImGuiKey_O,
			ImGuiKey_P,
			ImGuiKey_Q,
			ImGuiKey_R,
			ImGuiKey_S,
			ImGuiKey_T,
			ImGuiKey_U,
			ImGuiKey_V,
			ImGuiKey_W,
			ImGuiKey_X,
			ImGuiKey_Y,
			ImGuiKey_Z,
			ImGuiKey_F1,
			ImGuiKey_F2,
			ImGuiKey_F3,
			ImGuiKey_F4,
			ImGuiKey_F5,
			ImGuiKey_F6,
			ImGuiKey_F7,
			ImGuiKey_F8,
			ImGuiKey_F9,
			ImGuiKey_F10,
			ImGuiKey_F11,
			ImGuiKey_F12,
			ImGuiKey_F13,
			ImGuiKey_F14,
			ImGuiKey_F15,
			ImGuiKey_F16,
			ImGuiKey_F17,
			ImGuiKey_F18,
			ImGuiKey_F19,
			ImGuiKey_F20,
			ImGuiKey_F21,
			ImGuiKey_F22,
			ImGuiKey_F23,
			ImGuiKey_F24,
			ImGuiKey_AppBack,
			ImGuiKey_AppForward,
		};

		return Keys;
	}

	bool TableGetColumnIsVisible()
	{
		return TableGetColumnFlags() & ImGuiTableColumnFlags_IsVisible;
	}

}