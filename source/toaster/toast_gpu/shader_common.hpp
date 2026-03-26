/*!
 * @file shader_common.hpp
 */
#pragma once

#include <span>
#include <string>
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	enum class EShaderType
	{
		eVertex,
		ePixel,
		eCompute,
		eGeometry
	};

	inline std::string shaderStageToString(const EShaderType type)
	{
		switch (type)
		{
			case EShaderType::eVertex: { return "Vertex"; }
			case EShaderType::ePixel: { return "Pixel"; }
			case EShaderType::eCompute: { return "Compute"; }
			case EShaderType::eGeometry: { return "Geometry"; }
		}
		return "";
	}

	class ShaderBlob
	{
	public:
		ShaderBlob(const uint32 *p_data, uint64 p_size) : m_data(p_data), m_size(p_size)
		{
		}

		ShaderBlob(std::span<const uint32> p_data) : m_data(p_data.data()), m_size(p_data.size())
		{
		}

		[[nodiscard]] const uint32 *data() const { return m_data; }
		[[nodiscard]] uint64        size() const { return m_size; }
		[[nodiscard]] uint64        sizeBytes() const { return m_size * sizeof(uint32); }

	private:
		const uint32 *m_data{nullptr};
		uint64        m_size{0u};
	};

	enum class EShaderUniformType
	{
		eBool,
		eInt,
		eUInt,
		eFloat,
		eVec2,
		eVec3,
		eVec4,
		eMat3,
		eMat4
	};
}
