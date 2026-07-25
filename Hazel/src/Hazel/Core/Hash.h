#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace Hazel {

	class Hash
	{
	public:
		static constexpr uint32_t GenerateFNVHash(std::string_view str)
		{
			constexpr uint32_t FNV_PRIME = 16777619u;
			constexpr uint32_t OFFSET_BASIS = 2166136261u;

			uint32_t hash = OFFSET_BASIS;
			for (char c : str)
			{
				hash ^= static_cast<uint8_t>(c);
				hash *= FNV_PRIME;
			}
			hash ^= '\0';
			hash *= FNV_PRIME;
			return hash;
		}
	};

}
