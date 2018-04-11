#pragma once

namespace ImGui
{
	static inline bool      IsMouseHoveringWindow() { return IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem); }
	static inline bool      IsItemHoveredRect() { return IsItemHovered(ImGuiHoveredFlags_RectOnly); }
	static inline void      SetNextWindowPosCenter(ImGuiCond cond = 0) { SetNextWindowPos(ImVec2(GetIO().DisplaySize.x * 0.5f, GetIO().DisplaySize.y * 0.5f), cond, ImVec2(0.5f, 0.5f)); }
}

// ugly macros to add tooltips to widgets
#define COLORTEXT_WITH_TOOLTIP(color, ...)		ImGui::TextColored(color, __VA_ARGS__); if (ImGui::IsItemHovered()) { ImGui::SetTooltip(__VA_ARGS__); }
#define TEXT_WITH_TOOLTIP(...)					ImGui::Text(__VA_ARGS__); if (ImGui::IsItemHovered()) { ImGui::SetTooltip(__VA_ARGS__); }