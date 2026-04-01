/*!
* @file gl_shader.hpp
 */
#pragma once

#include <unordered_map>
#include <openglhpp/opengl.hpp>

#include "../shader.hpp"
#include "gl_shader_uniform.hpp"

namespace toaster::gpu
{
	gl::ShaderStage getShaderStage(EShaderType p_stage);

	class GLShader : public IShader
	{
	public:
		GLShader(String p_name, const std::unordered_map<EShaderType, String> &p_shader_source_map);
		~GLShader() override;

		uint32_t getID() const override;

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

		// New reflection interface
		const ShaderUniformBufferList& getVSMaterialUniforms() const override { return m_vsMaterialUniforms; }
		const ShaderUniformBufferList& getPSMaterialUniforms() const override { return m_psMaterialUniforms; }
		const ShaderResourceList& getResources() const override { return m_resources; }

		ShaderUniformDeclaration* findUniformDeclaration(const String& name) override;
		ShaderResourceDeclaration* findResourceDeclaration(const String& name) override;

		// Backward compatibility
		const std::vector<ShaderUniformDeclaration *> &getUniformDeclarations() const override;

	private:
		void _reflect();
		void _populateUniformBuffers();

		String m_name;
		gl::ID m_programId{0u};

		std::unordered_map<EShaderType, String> m_shaderSourceMap;
		std::unordered_map<String, gl::Int> m_uniformLocations;

		// New system: organized by domain and type
		ShaderUniformBufferList m_vsMaterialUniforms;
		ShaderUniformBufferList m_psMaterialUniforms;
		ShaderResourceList m_resources;
	};
}

