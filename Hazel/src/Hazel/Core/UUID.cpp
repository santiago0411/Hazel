#include "hzpch.h"
#include "Hazel/Core/UUID.h"

#include <random>

namespace Hazel
{
	static std::random_device g_RandomDevice;
	static std::mt19937_64 g_RandomEngine(g_RandomDevice());
	static std::uniform_int_distribution<uint64_t> g_UniformIntDistribution;

	UUID::UUID()
		: m_UUID(g_UniformIntDistribution(g_RandomEngine))
	{
	}

	UUID::UUID(uint64_t uuid)
		: m_UUID(uuid)
	{
	}

	UUID::UUID(const UUID& other)
		: m_UUID(other.m_UUID)
	{
	}
}
