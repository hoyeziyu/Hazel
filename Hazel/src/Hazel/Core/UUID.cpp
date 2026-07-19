#include "hzpch.h"
#include "UUID.h"

#include <random>

namespace Hazel {

	UUID::UUID()
		: m_UUID(0)
	{
		std::random_device rd;
		std::mt19937_64 gen(rd());
		std::uniform_int_distribution<uint64_t> dist;
		m_UUID = dist(gen);
	}

	UUID::UUID(uint64_t uuid)
		: m_UUID(uuid)
	{
	}

}
