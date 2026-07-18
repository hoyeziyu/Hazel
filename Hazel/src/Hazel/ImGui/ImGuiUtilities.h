#pragma once

#include <imgui.h>

#include <cstdarg>

namespace Hazel::UI {

	// ImGuiHoveredFlags_ForTooltip (SetItemTooltip) needs stationary mouse + delay.
	// Use these flags for immediate, reliable tooltips on buttons/menus/text rows.
	inline constexpr ImGuiHoveredFlags TooltipHoverFlags =
		ImGuiHoveredFlags_AllowWhenBlockedByPopup |
		ImGuiHoveredFlags_AllowWhenDisabled |
		ImGuiHoveredFlags_AllowWhenOverlapped;

	inline void SetTooltip(const char* fmt, ...)
	{
		if (!ImGui::IsItemHovered(TooltipHoverFlags))
			return;

		va_list args;
		va_start(args, fmt);
		ImGui::SetTooltipV(fmt, args);
		va_end(args);
	}

	// Text for display; extend hover rect to the full row width for tooltips.
	inline void HoverRow(const char* rowLabel, const char* tooltipFmt, ...)
	{
		ImGui::TextUnformatted(rowLabel);

		const ImVec2 rowMin = ImGui::GetItemRectMin();
		const ImVec2 rowMax = {
			ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x,
			ImGui::GetItemRectMax().y
		};

		if (!ImGui::IsMouseHoveringRect(rowMin, rowMax))
			return;

		va_list args;
		va_start(args, tooltipFmt);
		ImGui::SetTooltipV(tooltipFmt, args);
		va_end(args);
	}

}
