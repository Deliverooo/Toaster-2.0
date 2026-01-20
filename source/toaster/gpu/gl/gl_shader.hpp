#pragma once

#include <openglhpp/opengl.hpp>

#include "shader.hpp"

namespace toaster::gpu
{
	gl::ShaderStage getShaderStage(EShaderType p_stage);

	class GLShader : public Shader
	{
	public:
		GLShader(std::string p_name, const std::map<EShaderType, ShaderBlob> &p_shader_bytecode_map);
		GLShader(std::string p_name, const std::map<EShaderType, const char *> &p_shader_source_map);
		~GLShader() override;

		void bind() const override;
		void unbind() const override;

		void setUniform(const std::string &p_name, float p_value) override;
		void setUniform(const std::string &p_name, int p_value) override;
		void setUniform(const std::string &p_name, uint32 p_value) override;
		void setUniform(const std::string &p_name, const glm::vec2 &p_value) override;
		void setUniform(const std::string &p_name, const glm::vec3 &p_value) override;
		void setUniform(const std::string &p_name, const glm::vec4 &p_value) override;
		void setUniform(const std::string &p_name, const glm::mat3 &p_value) override;
		void setUniform(const std::string &p_name, const glm::mat4 &p_value) override;

	private:
		std::string m_name;

		gl::UInt m_programId{0u};

		std::unordered_map<std::string, gl::Int> m_uniformLocations;
	};
}
