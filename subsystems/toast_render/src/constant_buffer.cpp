#include "toast_render/constant_buffer.hpp"

namespace toaster::render
{
	auto findConstantBufferDeclaration(const reflection::ReflectionData &p_reflection_data) -> const reflection::ReflectedStruct *
	{
		auto it{
			std::ranges::find_if(p_reflection_data.structs, [](const auto &pair) -> bool
			{
				if (pair.first.starts_with("TSTC__"))
					return true;
				return false;
			})
		};

		if (it != p_reflection_data.structs.end())
			return &it->second;

		// LOG_ERROR("Failed to find toaster constant buffer declaration!");
		return nullptr;
	}

	ConstantBuffer::ConstantBuffer(const reflection::ReflectedStruct &p_constant_buffer_declaration) : m_constantBufferDeclaration(p_constant_buffer_declaration)
	{
		m_constantBufferData.allocate(m_constantBufferDeclaration.size);
		m_constantBufferData.zeroInitialize();
	}

	ConstantBuffer::~ConstantBuffer()
	{
		m_constantBufferData.release();
	}

	auto ConstantBuffer::getBuffer() const -> const Buffer &
	{
		return m_constantBufferData;
	}

	auto ConstantBuffer::_set(const String &p_name, const void *p_value) -> void
	{
		auto it{m_constantBufferDeclaration.members.find(p_name)};
		if (it == m_constantBufferDeclaration.members.end())
		{
			// LOG_ERROR("Material '{}' does not contain member '{}' | struct: {}", m_name, p_name, m_internalMaterialStructName);
			return;
		}
		uintptr offset{(uintptr) it->second.offset};
		uint32  size{it->second.size};
		m_constantBufferData.write(p_value, size, offset);
	}
}
