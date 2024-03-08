#pragma once

namespace ImGui
{
	bool	IsItemHoveredRect();
	void	SetNextWindowPosCenter(ImGuiCond cond = 0);
	void	PushStyleCompact();
	void	PopStyleCompact();
	void	ShowHelpPopup(const char* fmt, ...);
	bool	CheckBoxTristate(const char* label, int* v_tristate);
	void	MoveNextWindowToSafeZone(const ImVec2& WindowPos, const ImVec2& WindowSize);
	void	ResetMouseKeys();
	void	ResetKeyboardKeys();
	ImGuiKey VirtualKeyToImGuiKey(WPARAM wParam);
	SHORT ImGuiKeyToVirtualKey(ImGuiKey Key);
	const std::initializer_list<ImGuiKey> &NamedKeys();


	int		TableGetHoveredColumn();	// Expose IMGUI-internal function
	bool	TableGetColumnIsVisible();
}

// ugly macros to add tooltips to widgets
#define COLORTEXT_WITH_TOOLTIP(color, ...)		ImGui::TextColored(color, __VA_ARGS__); if (ImGui::IsItemHovered()) { ImGui::SetTooltip(__VA_ARGS__); }
#define TEXT_WITH_TOOLTIP(...)					ImGui::Text(__VA_ARGS__); if (ImGui::IsItemHovered()) { ImGui::SetTooltip(__VA_ARGS__); }


// There is no distinct VK_xxx for keypad enter, instead it is VK_RETURN + KF_EXTENDED, we assign it an arbitrary value to make code more readable (VK_ codes go up to 255)
#define IM_VK_KEYPAD_ENTER      (VK_RETURN + 256)
