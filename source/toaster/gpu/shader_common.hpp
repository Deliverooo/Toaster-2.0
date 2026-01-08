#pragma once

#include <span>
#include <vector>
#include "system_types.h"

namespace toaster::gpu
{
	// class ShaderBlob
	// {
	// public:
	// 	ShaderBlob(const uint32 *p_data, uint64 p_size) : m_data(p_data), m_size(p_size)
	// 	{
	// 	}
	//
	// 	const uint32 *data() const { return m_data; }
	// 	uint64        size() const { return m_size; }
	//
	// private:
	// 	const uint32 *m_data{nullptr};
	// 	uint64        m_size{0u};
	// };

	using ShaderBlob = std::span<const uint32>;
}
