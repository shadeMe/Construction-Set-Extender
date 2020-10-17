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
		SetNextWindowPos(ImVec2(GetIO().DisplaySize.x * 0.5f, GetIO().DisplaySize.y * 0.5f), cond, ImVec2(0.5f, 0.5f));
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

}