#pragma once

#include <string>
#include <utility>
#include <vector>

#include "system_types.h"
#include "toast_assert.h"

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
			case EShaderDataType::eFloat: { return 4; }
			case EShaderDataType::eFloat2: { return 4 * 2; }
			case EShaderDataType::eFloat3: { return 4 * 3; }
			case EShaderDataType::eFloat4: { return 4 * 4; }
			case EShaderDataType::eMat3: { return 4 * 3 * 3; }
			case EShaderDataType::eMat4: { return 4 * 4 * 4; }
			case EShaderDataType::eInt: { return 4; }
			case EShaderDataType::eInt2: { return 4 * 2; }
			case EShaderDataType::eInt3: { return 4 * 3; }
			case EShaderDataType::eInt4: { return 4 * 4; }
			case EShaderDataType::eBool: { return 1; }
		}
		return UINT32_MAX;
	}

	struct VertexBufferElement
	{
		std::string     name;
		EShaderDataType type;
		uint32          size;
		uint32          offset;
		bool            normalized;

		VertexBufferElement(EShaderDataType p_type, std::string p_name, bool p_normalized = false)
			: name(std::move(p_name)), type(p_type), size(shaderDataTypeSize(p_type)), offset(0), normalized(p_normalized)
		{
		}

		[[nodiscard]] uint32 getComponentCount() const
		{
			switch (type)
			{
				case EShaderDataType::eFloat: { return 1; }
				case EShaderDataType::eFloat2: { return 2; }
				case EShaderDataType::eFloat3: { return 3; }
				case EShaderDataType::eFloat4: { return 4; }
				case EShaderDataType::eMat3: { return 3; }
				case EShaderDataType::eMat4: { return 4; }
				case EShaderDataType::eInt: { return 1; }
				case EShaderDataType::eInt2: { return 2; }
				case EShaderDataType::eInt3: { return 3; }
				case EShaderDataType::eInt4: { return 4; }
				case EShaderDataType::eBool: { return 1; }
			}
			TST_ASSERT_MSG(false, "Unknown shader data type");

			return 0;
		}
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() = default;

		VertexBufferLayout(std::initializer_list<VertexBufferElement> p_elements) : m_elements(p_elements)
		{
			size_t offset = 0;
			for (auto &element: m_elements)
			{
				element.offset = offset;
				offset += element.size;
				m_stride += element.size;
			}
		}

		[[nodiscard]] uint32                                  getStride() const { return m_stride; }
		[[nodiscard]] const std::vector<VertexBufferElement> &getElements() const { return m_elements; }

		std::vector<VertexBufferElement>::iterator                     begin() { return m_elements.begin(); }
		std::vector<VertexBufferElement>::iterator                     end() { return m_elements.end(); }
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const { return m_elements.begin(); }
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const { return m_elements.end(); }

	private:
		std::vector<VertexBufferElement> m_elements;
		uint32                           m_stride{0u};
	};
}
