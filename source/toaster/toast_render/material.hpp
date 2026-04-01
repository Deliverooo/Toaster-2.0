#pragma once

#include "toast_gpu/shader.hpp"
#include "toast_gpu/texture.hpp"
#include "toast_lib/logging.hpp"
#include "toast_lib/buffer.hpp"

#include <unordered_map>

#include "toast_gpu/buffer.hpp"

namespace toaster
{
	class Material
	{
	public:
		static RefPtr<Material> create(const RefPtr<gpu::IShader> &p_shader);
		Material(const RefPtr<gpu::IShader> &p_shader);

		void use();

		RefPtr<gpu::IShader> getShader();

		template<typename Type>
		void set(const String &p_name, const Type &p_value)
		{
			auto decl = _findUniformDeclaration(p_name);
			if (decl)
			{
				auto& buffer = _getUniformBufferTarget(decl);
				buffer.write((uint8*)&p_value, decl->getSize(), decl->getOffset());
			}
			else
			{
				LOG_ERROR("Could not find uniform {}", p_name);
			}
		}

		// Texture convenience overload so callers can do: material->set("u_Texture", texture);
		void set(const String &p_name, const RefPtr<gpu::ITexture2D> &p_value);

		void setUniform(const String &p_name, float32 p_value);
		void setUniform(const String &p_name, int32 p_value);
		void setUniform(const String &p_name, uint32 p_value);
		void setUniform(const String &p_name, const glm::vec2 &p_value);
		void setUniform(const String &p_name, const glm::vec3 &p_value);
		void setUniform(const String &p_name, const glm::vec4 &p_value);
		void setUniform(const String &p_name, const glm::mat3 &p_value);
		void setUniform(const String &p_name, const glm::mat4 &p_value);
		void setUniform(const String &p_name, float32 *p_values, uint32 p_count);
		void setUniform(const String &p_name, int32 *p_values, uint32 p_count);
		void setUniform(const String &p_name, uint32 *p_values, uint32 p_count);

		// New Hazel-style interface
		const gpu::ShaderUniformBufferList& getVSMaterialUniforms() const;
		const gpu::ShaderUniformBufferList& getPSMaterialUniforms() const;
		const gpu::ShaderResourceList& getResources() const;

		gpu::ShaderUniformDeclaration* findUniformDeclaration(const String& name);
		gpu::ShaderResourceDeclaration* findResourceDeclaration(const String& name);

	private:
		void _allocateStorage();
		gpu::ShaderUniformDeclaration *_findUniformDeclaration(const String &p_name);
		Buffer& _getUniformBufferTarget(gpu::ShaderUniformDeclaration* decl);
		void _uploadUniformFromBuffer(gpu::ShaderUniformDeclaration* uniform, uint8* data);

		RefPtr<gpu::IShader> m_shader;
		std::vector<RefPtr<gpu::ITexture2D>> m_textures;

		// Uniform buffers for material storage
		Buffer m_vsUniformStorageBuffer;
		Buffer m_psUniformStorageBuffer;
	};
}

