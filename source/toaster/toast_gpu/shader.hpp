/*!
 * @file shader.hpp
 */
#pragma once

#include "toast_lib/ptr.hpp"
#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"

#include "shader_common.hpp"

#include <map>
#include <glm/glm.hpp>

namespace toaster::gpu
{
	class IShader
	{
	public:
		static RefPtr<IShader> create(const String &p_name, const std::map<EShaderType, CString> &p_shader_source_map);

		virtual ~IShader() = default;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void setUniform(const String &p_name, float32 p_value) = 0;
		virtual void setUniform(const String &p_name, int32 p_value) = 0;
		virtual void setUniform(const String &p_name, uint32 p_value) = 0;
		virtual void setUniform(const String &p_name, const glm::vec2 &p_value) = 0;
		virtual void setUniform(const String &p_name, const glm::vec3 &p_value) = 0;
		virtual void setUniform(const String &p_name, const glm::vec4 &p_value) = 0;
		virtual void setUniform(const String &p_name, const glm::mat3 &p_value) = 0;
		virtual void setUniform(const String &p_name, const glm::mat4 &p_value) = 0;
		virtual void setUniform(const String &p_name, float32 *p_values, uint32 p_count) = 0;
		virtual void setUniform(const String &p_name, int32 *p_values, uint32 p_count) = 0;
		virtual void setUniform(const String &p_name, uint32 *p_values, uint32 p_count) = 0;
	};
}
