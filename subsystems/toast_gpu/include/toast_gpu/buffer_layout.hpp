#pragma once

#include "toast_gpu.hpp"

#include <utility>
#include <vector>

#include "toast_lib/core_basic.hpp"

#include <vulkan/vulkan_raii.hpp>

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

	constexpr auto getVulkanAttribType(EBufferDataType p_type) -> vk::Format
	{
		switch (p_type)
		{
			case EBufferDataType::eFloat: return vk::Format::eR32Sfloat;
			case EBufferDataType::eFloat2: return vk::Format::eR32G32Sfloat;
			case EBufferDataType::eFloat3: return vk::Format::eR32G32B32Sfloat;
			case EBufferDataType::eFloat4: return vk::Format::eR32G32B32A32Sfloat;
			case EBufferDataType::eMat3: return vk::Format::eR32G32B32A32Sfloat; // TODO: If I ever want to do instanced rendering, I will need to look into ts
			case EBufferDataType::eMat4: return vk::Format::eR32G32B32A32Sfloat;
			case EBufferDataType::eInt: return vk::Format::eR32Sint;
			case EBufferDataType::eInt2: return vk::Format::eR32G32Sint;
			case EBufferDataType::eInt3: return vk::Format::eR32G32B32Sint;
			case EBufferDataType::eInt4: return vk::Format::eR32G32B32A32Sint;
			case EBufferDataType::eBool: return vk::Format::eR32Sint;
			default: return vk::Format::eUndefined;
		}
		TST_ASSERT_MSG(false, "Unsupported shader data type");
		return vk::Format::eUndefined;
	}

	struct TST_GPU_API BufferElement
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

	class TST_GPU_API BufferLayout
	{
	public:
		constexpr BufferLayout() = default;

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

		[[nodiscard]] auto getBindingDescription() const -> vk::VertexInputBindingDescription2EXT
		{
			vk::VertexInputBindingDescription2EXT vertex_input_binding_description{};
			vertex_input_binding_description.binding   = 0;
			vertex_input_binding_description.stride    = m_stride;
			vertex_input_binding_description.inputRate = vk::VertexInputRate::eVertex;
			vertex_input_binding_description.divisor   = 1;
			return vertex_input_binding_description;
		}

		[[nodiscard]] auto getAttributeDescriptions(uint32 p_target_binding = 0u) const -> std::vector<vk::VertexInputAttributeDescription2EXT>
		{
			std::vector<vk::VertexInputAttributeDescription2EXT> vertex_input_attribute_descriptions;
			vertex_input_attribute_descriptions.resize(m_elements.size());

			uint32 location{0u};
			for (const auto &element: m_elements)
			{
				vertex_input_attribute_descriptions[location].binding  = p_target_binding;
				vertex_input_attribute_descriptions[location].format   = getVulkanAttribType(element.type);
				vertex_input_attribute_descriptions[location].location = location;
				vertex_input_attribute_descriptions[location].offset   = element.offset;
				++location;
			}
			return vertex_input_attribute_descriptions;
		}

	private:
		std::vector<BufferElement> m_elements;
		uint32                     m_stride{0u};
	};

	using VertexBufferLayout = BufferLayout;
	using InstanceLayout     = BufferLayout;
}
