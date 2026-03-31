/*!
* @file gl_shader.hpp
 */
#pragma once

#include <unordered_map>
#include <openglhpp/opengl.hpp>

#include "../shader.hpp"

namespace toaster::gpu
{
	gl::ShaderStage getShaderStage(EShaderType p_stage);

	class GLShader : public IShader
	{
	public:
		GLShader(String p_name, const std::map<EShaderType, CString> &p_shader_source_map);
		~GLShader() override;

		void bind() const override;
		void unbind() const override;

		void setUniform(const String &p_name, float32 p_value) override;
		void setUniform(const String &p_name, int32 p_value) override;
		void setUniform(const String &p_name, uint32 p_value) override;
		void setUniform(const String &p_name, const glm::vec2 &p_value) override;
		void setUniform(const String &p_name, const glm::vec3 &p_value) override;
		void setUniform(const String &p_name, const glm::vec4 &p_value) override;
		void setUniform(const String &p_name, const glm::mat3 &p_value) override;
		void setUniform(const String &p_name, const glm::mat4 &p_value) override;
		void setUniform(const String &p_name, float32 *p_values, uint32 p_count) override;
		void setUniform(const String &p_name, int32 *p_values, uint32 p_count) override;
		void setUniform(const String &p_name, uint32 *p_values, uint32 p_count) override;

	private:
		String m_name;
		gl::ID m_programId{0u};

		std::unordered_map<String, gl::Int> m_uniformLocations;
	};
}
