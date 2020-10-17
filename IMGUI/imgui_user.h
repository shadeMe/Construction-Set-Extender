#pragma once

namespace ImGui
{
	bool	IsItemHoveredRect();
	void	SetNextWindowPosCenter(ImGuiCond cond = 0);
	void	PushStyleCompact();
	void	PopStyleCompact();
	void	ShowHelpPopup(const char* fmt, ...);
	bool	CheckBoxTristate(const char* label, int* v_tristate);
}

// ugly macros to add tooltips to widgets
#define COLORTEXT_WITH_TOOLTIP(color, ...)		ImGui::TextColored(color, __VA_ARGS__); if (ImGui::IsItemHovered()) { ImGui::SetTooltip(__VA_ARGS__); }
#define TEXT_WITH_TOOLTIP(...)					ImGui::Text(__VA_ARGS__); if (ImGui::IsItemHovered()) { ImGui::SetTooltip(__VA_ARGS__); }