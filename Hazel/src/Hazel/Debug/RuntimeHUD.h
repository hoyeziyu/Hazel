#pragma once

#include <array>
#include <string>
#include <glm/glm.hpp>

namespace Hazel {

	class RuntimeHUD
	{
	public:
		struct WorldLabelEntry
		{
			glm::vec3 Position = glm::vec3(0.0f);
			std::string Text;
			bool Active = false;
		};

		static constexpr size_t MaxLines = 8;
		static constexpr size_t MaxWorldLabels = 16;

		static void Clear();
		static void SetLine(size_t index, const std::string& text);
		static const std::array<std::string, MaxLines>& GetLines();

		static void ClearWorldLabels();
		static void SetWorldLabel(size_t index, const glm::vec3& position, const std::string& text);
		static const std::array<WorldLabelEntry, MaxWorldLabels>& GetWorldLabels();

	private:
		static std::array<std::string, MaxLines> s_Lines;
		static std::array<WorldLabelEntry, MaxWorldLabels> s_WorldLabels;
	};

}
