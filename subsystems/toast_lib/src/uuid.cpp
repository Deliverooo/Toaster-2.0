#include "toast_lib/uuid.hpp"

#include <random>

namespace toaster
{
	static std::random_device                    s_randomDevice{};
	static std::mt19937_64                       s_engine{s_randomDevice()};
	static std::uniform_int_distribution<uint64> s_uniformIntDistribution{};

	UUID::UUID() : m_uuid(s_uniformIntDistribution(s_engine))
	{
	}

	UUID::operator uint64() const
	{
		return m_uuid;
	}
}
