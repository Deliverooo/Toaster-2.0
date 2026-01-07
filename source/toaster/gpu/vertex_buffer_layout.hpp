#pragma once

#include"system_types.h"

namespace toaster::gpu
{
	enum class EShaderDataType
	{
		eFloat,
		eFloat2,
		eFloat3,
		eFloat4,
		eMat3,
		eMat4,
		eInt,
		eInt2,
		eInt3,
		eInt4,
		eBool
	};

	constexpr uint32 shaderDataTypeSize(const EShaderDataType p_type)
	{
		switch (p_type)
		{
			case EShaderDataType::eFloat: return 4;
			case EShaderDataType::eFloat2: return 4 * 2;
			case EShaderDataType::eFloat3: return 4 * 3;
			case EShaderDataType::eFloat4: return 4 * 4;
			case EShaderDataType::eMat3: return 4 * 3 * 3;
			case EShaderDataType::eMat4: return 4 * 4 * 4;
			case EShaderDataType::eInt: return 4;
			case EShaderDataType::eInt2: return 4 * 2;
			case EShaderDataType::eInt3: return 4 * 3;
			case EShaderDataType::eInt4: return 4 * 4;
			case EShaderDataType::eBool: return 1;
		}
		return UINT32_MAX;
	}

	struct VertexBufferElement
	{

	};
}
