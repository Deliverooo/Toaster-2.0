#pragma once

#include "shader_reflection.hpp"
#include "toast_render.hpp"
#include "toast_lib/buffer.hpp"

namespace toaster::render
{
	[[nodiscard]] TST_RENDER_API auto findConstantBufferDeclaration(const reflection::ReflectionData &p_reflection_data) -> const reflection::ReflectedStruct *;

	class TST_RENDER_API ConstantBuffer
	{
	public:
		ConstantBuffer(const reflection::ReflectedStruct &p_constant_buffer_declaration);
		~ConstantBuffer();

		template<typename Type>
		auto set(const String &p_name, const Type &p_value) -> void
		{
			_set(p_name, static_cast<const void *>(&p_value));
		}

		auto getBuffer() const -> const Buffer &;

	private:
		auto _set(const String &p_name, const void *p_value) -> void;

		reflection::ReflectedStruct m_constantBufferDeclaration;

		Buffer m_constantBufferData;
	};

	TST_RENDER_DEFINE_HANDLE(ConstantBuffer, ConstantBuffer)
}
