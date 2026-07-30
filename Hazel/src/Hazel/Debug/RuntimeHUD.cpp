#include "hzpch.h"
#include "RuntimeHUD.h"

namespace Hazel {

	std::array<std::string, RuntimeHUD::MaxLines> RuntimeHUD::s_Lines;

	void RuntimeHUD::Clear()
	{
		for (auto& line : s_Lines)
			line.clear();
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

}
