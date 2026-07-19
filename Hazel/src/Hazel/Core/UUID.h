#pragma once

#include <cstdint>
#include <functional>

namespace Hazel {

	// 64-bit 随机 ID，用作 AssetHandle（与路径解耦的稳定引用）
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() { return m_UUID; }
		operator uint64_t() const { return m_UUID; }

		bool operator==(const UUID& other) const { return m_UUID == other.m_UUID; }
		bool operator!=(const UUID& other) const { return !(*this == other); }

	private:
		uint64_t m_UUID = 0;
	};

}

namespace std {

	template<>
	struct hash<Hazel::UUID>
	{
		std::size_t operator()(const Hazel::UUID& uuid) const
		{
			return std::hash<uint64_t>()((uint64_t)uuid);
		}
	};

}
