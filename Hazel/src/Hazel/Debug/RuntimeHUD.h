#pragma once

#include <array>
#include <string>

namespace Hazel {

	class RuntimeHUD
	{
	public:
		static constexpr size_t MaxLines = 8;

		static void Clear();
		static void SetLine(size_t index, const std::string& text);
		static const std::array<std::string, MaxLines>& GetLines();

	private:
		static std::array<std::string, MaxLines> s_Lines;
	};

}
