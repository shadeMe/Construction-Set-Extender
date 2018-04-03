#include "imgui.h"
#include "imgui_internal.h"

namespace ImGui
{
	// Warning! Invloves a lot of code duplication
	static inline void ClearSetNextWindowData()
	{
		ImGuiContext& g = *GImGui;
		g.SetNextWindowPosCond = g.SetNextWindowSizeCond = g.SetNextWindowContentSizeCond = g.SetNextWindowCollapsedCond = 0;
		g.SetNextWindowSizeConstraint = g.SetNextWindowFocus = false;
	}

	static bool IsPopupOpen(ImGuiID id)
	{
		ImGuiContext& g = *GImGui;
		const bool is_open = g.OpenPopupStack.Size > g.CurrentPopupStack.Size && g.OpenPopupStack[g.CurrentPopupStack.Size].PopupId == id;
		return is_open;
	}

	IMGUI_API bool BeginPopupWithStyling(const char* str_id, ImGuiWindowFlags extra_flags /*= 0*/, float window_rounding)
	{
		if (GImGui->OpenPopupStack.Size <= GImGui->CurrentPopupStack.Size)
		{
			ClearSetNextWindowData();
			return false;
		}

		// copy-paste: BeginPopupEx()
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		const ImGuiID id = window->GetID(str_id);
		if (!IsPopupOpen(id))
		{
			ClearSetNextWindowData(); // We behave like Begin() and need to consume those values
			return false;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, window_rounding);
		ImGuiWindowFlags flags = extra_flags | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;

		char name[32];
		if (flags & ImGuiWindowFlags_ChildMenu)
			ImFormatString(name, 20, "##menu_%d", g.CurrentPopupStack.Size);    // Recycle windows based on depth
		else
			ImFormatString(name, 20, "##popup_%08x", id); // Not recycling, so we can close/open during the same frame

		bool is_open = ImGui::Begin(name, NULL, flags);
		if (!is_open) // NB: is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
			ImGui::EndPopup();

		return is_open;
	}

}