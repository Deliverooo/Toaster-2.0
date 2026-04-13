#pragma once

#include <utility>
#include <vector>

#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	enum class EBufferDataType
	{
		eUnknown,
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

	constexpr auto bufferDataTypeSize(const EBufferDataType p_type) -> uint32
	{
		switch (p_type)
		{
			case EBufferDataType::eUnknown: return 0u;
			case EBufferDataType::eFloat: return 4u;
			case EBufferDataType::eFloat2: return 4u * 2u;
			case EBufferDataType::eFloat3: return 4u * 3u;
			case EBufferDataType::eFloat4: return 4u * 4u;
			case EBufferDataType::eMat3: return 4u * 3u * 3u;
			case EBufferDataType::eMat4: return 4u * 4u * 4u;
			case EBufferDataType::eInt: return 4u;
			case EBufferDataType::eInt2: return 4u * 2u;
			case EBufferDataType::eInt3: return 4u * 3u;
			case EBufferDataType::eInt4: return 4u * 4u;
			case EBufferDataType::eBool: return 1u;
		}
		TST_ASSERT_MSG(false, "Unknown buffer data type!!!");
		return UINT32_MAX;
	}

	struct BufferElement
	{
		String          name{};
		EBufferDataType type{EBufferDataType::eUnknown};
		uint32          size{0u};
		uint32          offset{0u};
		bool            normalized{false};

		BufferElement(EBufferDataType p_type, String p_name, bool p_normalized = false)
			: name(std::move(p_name)), type(p_type), size(bufferDataTypeSize(p_type)), offset(0), normalized(p_normalized)
		{
		}

		[[nodiscard]] auto getComponentCount() const -> uint32
		{
			switch (type)
			{
				case EBufferDataType::eUnknown: return 0u;
				case EBufferDataType::eFloat: return 1u;
				case EBufferDataType::eFloat2: return 2u;
				case EBufferDataType::eFloat3: return 3u;
				case EBufferDataType::eFloat4: return 4u;
				case EBufferDataType::eMat3: return 3u;
				case EBufferDataType::eMat4: return 4u;
				case EBufferDataType::eInt: return 1u;
				case EBufferDataType::eInt2: return 2u;
				case EBufferDataType::eInt3: return 3u;
				case EBufferDataType::eInt4: return 4u;
				case EBufferDataType::eBool: return 1u;
			}
			TST_ASSERT_MSG(false, "Unknown buffer data type!!!");
			return UINT32_MAX;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() = default;

		BufferLayout(std::initializer_list<BufferElement> p_elements) : m_elements(p_elements)
		{
			uint32 offset = 0;
			for (auto &element: m_elements)
			{
				element.offset = offset;
				offset         += element.size;
				m_stride       += element.size;
			}
		}

		[[nodiscard]] auto getStride() const -> uint32 { return m_stride; }
		[[nodiscard]] auto getElements() const -> const std::vector<BufferElement> & { return m_elements; }

		auto               begin() -> std::vector<BufferElement>::iterator { return m_elements.begin(); }
		auto               end() -> std::vector<BufferElement>::iterator { return m_elements.end(); }
		[[nodiscard]] auto begin() const -> std::vector<BufferElement>::const_iterator { return m_elements.begin(); }
		[[nodiscard]] auto end() const -> std::vector<BufferElement>::const_iterator { return m_elements.end(); }

	private:
		std::vector<BufferElement> m_elements;
		uint32                     m_stride{0u};
	};

	using VertexBufferLayout = BufferLayout;
	using InstanceLayout     = BufferLayout;
}
