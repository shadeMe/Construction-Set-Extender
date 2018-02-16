#pragma once

namespace ImGui
{
	bool					BeginPopupWithStyling(const char* str_id, ImGuiWindowFlags extra_flags = 0, float window_rounding = 0); // support for window styling
	static inline bool      IsMouseHoveringWindow() { return IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem); }
	static inline bool      IsItemHoveredRect() { return IsItemHovered(ImGuiHoveredFlags_RectOnly); }
	static inline void      SetNextWindowPosCenter(ImGuiCond cond = 0) { SetNextWindowPos(ImVec2(GetIO().DisplaySize.x * 0.5f, GetIO().DisplaySize.y * 0.5f), cond, ImVec2(0.5f, 0.5f)); }
}