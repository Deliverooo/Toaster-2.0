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
		/*! Creates a shader directly from the shader sources e.g.
		* 	const auto quad_shader = gpu::IShader::create("Quad", {
		* 													{gpu::EShaderType::eVertex,
		* 													io::filesystem::readFile(shader_dir / "quad.vert.glsl").c_str()},
															{gpu::EShaderType::ePixel,
															io::filesystem::readFile(shader_dir / "quad.pixel.glsl").c_str()}
															});
		 */
		static RefPtr<IShader> create(const String &p_name, const std::unordered_map<EShaderType, String> &p_shader_source_map);

		virtual ~IShader() = default;

		virtual uint32 getID() const = 0;

		// Not really used because the OpenGL implementation uses Direct State Access (DSA)
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

		// New interface for reflection and material system
		virtual const ShaderUniformBufferList& getVSMaterialUniforms() const = 0;
		virtual const ShaderUniformBufferList& getPSMaterialUniforms() const = 0;
		virtual const ShaderResourceList& getResources() const = 0;

		virtual ShaderUniformDeclaration* findUniformDeclaration(const String& name) = 0;
		virtual ShaderResourceDeclaration* findResourceDeclaration(const String& name) = 0;

		// Backward compatibility
		virtual const std::vector<ShaderUniformDeclaration *> &getUniformDeclarations() const = 0;
	};
}
