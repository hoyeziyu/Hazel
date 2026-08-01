#include "hzpch.h"
#include "RuntimeHUD.h"

namespace Hazel {

	std::array<std::string, RuntimeHUD::MaxLines> RuntimeHUD::s_Lines;
	std::array<RuntimeHUD::WorldLabelEntry, RuntimeHUD::MaxWorldLabels> RuntimeHUD::s_WorldLabels;

	void RuntimeHUD::Clear()
	{
		for (auto& line : s_Lines)
			line.clear();
		ClearWorldLabels();
	}

	void RuntimeHUD::SetLine(size_t index, const std::string& text)
	{
		if (index >= MaxLines)
			return;
		s_Lines[index] = text;
	}

	const std::array<std::string, RuntimeHUD::MaxLines>& RuntimeHUD::GetLines()
	{
		return s_Lines;
	}

	void RuntimeHUD::ClearWorldLabels()
	{
		for (auto& label : s_WorldLabels)
		{
			label.Text.clear();
			label.Active = false;
		}
	}

	void RuntimeHUD::SetWorldLabel(size_t index, const glm::vec3& position, const std::string& text, const glm::vec4& color)
	{
		if (index >= MaxWorldLabels)
			return;

		s_WorldLabels[index].Position = position;
		s_WorldLabels[index].Text = text;
		s_WorldLabels[index].Color = color;
		s_WorldLabels[index].Active = !text.empty();
	}

	const std::array<RuntimeHUD::WorldLabelEntry, RuntimeHUD::MaxWorldLabels>& RuntimeHUD::GetWorldLabels()
	{
		return s_WorldLabels;
	}

}
