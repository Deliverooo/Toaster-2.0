#include "material.hpp"

#include <utility>

#include "toast_lib/toast_assert.h"

namespace toaster
{
	RefPtr<Material> Material::create(const RefPtr<gpu::IShader> &p_shader)
	{
		return make_reference<Material>(p_shader);
	}

	Material::Material(const RefPtr<gpu::IShader> &p_shader) : m_shader(p_shader)
	{
		TST_ASSERT(m_shader);
		_allocateStorage();
	}

	void Material::_allocateStorage()
	{
		// Allocate storage for vs material uniforms
		auto &vertex_shader_uniforms = m_shader->getVSMaterialUniforms();
		if (!vertex_shader_uniforms.empty())
		{
			auto buffer = vertex_shader_uniforms.front();
			if (buffer)
			{
				m_vsUniformStorageBuffer.allocate(buffer->getSize());
				m_vsUniformStorageBuffer.zeroInitialize();
			}
		}

		// Allocate storage for ps material uniforms
		auto &pixel_shader_uniforms = m_shader->getPSMaterialUniforms();
		if (!pixel_shader_uniforms.empty())
		{
			auto buffer = pixel_shader_uniforms.front();
			if (buffer)
			{
				m_psUniformStorageBuffer.allocate(buffer->getSize());
				m_psUniformStorageBuffer.zeroInitialize();
			}
		}
	}

	void Material::use()
	{
		m_shader->bind();

		// Upload vertex shader material uniforms
		if (m_vsUniformStorageBuffer.data())
		{
			auto &uniforms = m_shader->getVSMaterialUniforms();
			if (!uniforms.empty())
			{
				auto buffer = uniforms.front();
				for (auto uniform: buffer->getUniformDeclarations())
				{
					if (uniform->getLocation() >= 0)
					{
						uint8 *data = static_cast<uint8 *>(m_vsUniformStorageBuffer.data()) + uniform->getOffset();
						_uploadUniformFromBuffer(uniform, data);
					}
				}
			}
		}

		// Upload pixel shader material uniforms
		if (m_psUniformStorageBuffer.data())
		{
			auto &uniforms = m_shader->getPSMaterialUniforms();
			if (!uniforms.empty())
			{
				auto buffer = uniforms.front();
				for (auto uniform: buffer->getUniformDeclarations())
				{
					if (uniform->getLocation() >= 0)
					{
						uint8 *data = static_cast<uint8 *>(m_psUniformStorageBuffer.data()) + uniform->getOffset();
						_uploadUniformFromBuffer(uniform, data);
					}
				}
			}
		}

		// Bind textures
		for (uint32 i{0u}; i < m_textures.size(); i++)
		{
			if (auto &texture = m_textures[i])
				texture->bind(i);
		}
	}

	RefPtr<gpu::IShader> Material::getShader()
	{
		return m_shader;
	}

	const gpu::ShaderUniformBufferList &Material::getVSMaterialUniforms() const
	{
		return m_shader->getVSMaterialUniforms();
	}

	const gpu::ShaderUniformBufferList &Material::getPSMaterialUniforms() const
	{
		return m_shader->getPSMaterialUniforms();
	}

	const gpu::ShaderResourceList &Material::getResources() const
	{
		return m_shader->getResources();
	}

	gpu::ShaderUniformDeclaration *Material::findUniformDeclaration(const String &name)
	{
		return m_shader->findUniformDeclaration(name);
	}

	gpu::ShaderResourceDeclaration *Material::findResourceDeclaration(const String &name)
	{
		return m_shader->findResourceDeclaration(name);
	}

	void Material::setUniform(const String &p_name, float32 p_value)
	{
		m_shader->setUniform(p_name, p_value);
	}

	void Material::setUniform(const String &p_name, int32 p_value)
	{
		m_shader->setUniform(p_name, p_value);
	}

	void Material::setUniform(const String &p_name, uint32 p_value)
	{
		m_shader->setUniform(p_name, p_value);
	}

	void Material::set(const String &p_name, const RefPtr<gpu::ITexture2D> &p_value)
	{
		auto decl = findResourceDeclaration(p_name);
		if (decl)
		{
			uint32 slot = decl->getRegister();
			if (m_textures.size() <= slot)
				m_textures.resize(slot + 1);
			m_textures[slot] = p_value;
		}
		else
		{
			LOG_WARN("Could not find texture resource {}", p_name);
		}
	}

	void Material::setUniform(const String &p_name, const glm::vec2 &p_value)
	{
		m_shader->setUniform(p_name, p_value);
	}

	void Material::setUniform(const String &p_name, const glm::vec3 &p_value)
	{
		m_shader->setUniform(p_name, p_value);
	}

	void Material::setUniform(const String &p_name, const glm::vec4 &p_value)
	{
		m_shader->setUniform(p_name, p_value);
	}

	void Material::setUniform(const String &p_name, const glm::mat3 &p_value)
	{
		m_shader->setUniform(p_name, p_value);
	}

	void Material::setUniform(const String &p_name, const glm::mat4 &p_value)
	{
		m_shader->setUniform(p_name, p_value);
	}

	void Material::setUniform(const String &p_name, float32 *p_values, uint32 p_count)
	{
		m_shader->setUniform(p_name, p_values, p_count);
	}

	void Material::setUniform(const String &p_name, int32 *p_values, uint32 p_count)
	{
		m_shader->setUniform(p_name, p_values, p_count);
	}

	void Material::setUniform(const String &p_name, uint32 *p_values, uint32 p_count)
	{
		m_shader->setUniform(p_name, p_values, p_count);
	}

	gpu::ShaderUniformDeclaration *Material::_findUniformDeclaration(const String &p_name)
	{
		// Use the new shader interface
		return m_shader->findUniformDeclaration(p_name);
	}

	Buffer &Material::_getUniformBufferTarget(gpu::ShaderUniformDeclaration *decl)
	{
		if (decl->getDomain() == gpu::EShaderDomain::eVertex)
			return m_vsUniformStorageBuffer;
		return m_psUniformStorageBuffer;
	}

	void Material::_uploadUniformFromBuffer(gpu::ShaderUniformDeclaration *uniform, uint8 *data)
	{
		int32 location = uniform->getLocation();
		if (location < 0)
		{
			LOG_WARN("[_uploadUniformFromBuffer] Invalid location {} for uniform {}", location, uniform->getName());
			return;
		}

		auto type = uniform->getType();

		switch (type)
		{
			case gpu::EShaderUniformType::eFloat:
			{
				float32 value = *reinterpret_cast<float32 *>(data);
				m_shader->setUniform(uniform->getName(), value);
				break;
			}
			case gpu::EShaderUniformType::eInt:
			{
				int32 value = *reinterpret_cast<int32 *>(data);
				m_shader->setUniform(uniform->getName(), value);
				break;
			}
			case gpu::EShaderUniformType::eUInt:
			{
				uint32 value = *reinterpret_cast<uint32 *>(data);
				m_shader->setUniform(uniform->getName(), value);
				break;
			}
			case gpu::EShaderUniformType::eVec2:
			{
				glm::vec2 value = *reinterpret_cast<glm::vec2 *>(data);
				m_shader->setUniform(uniform->getName(), value);
				break;
			}
			case gpu::EShaderUniformType::eVec3:
			{
				glm::vec3 value = *reinterpret_cast<glm::vec3 *>(data);
				m_shader->setUniform(uniform->getName(), value);
				break;
			}
			case gpu::EShaderUniformType::eVec4:
			{
				glm::vec4 value = *reinterpret_cast<glm::vec4 *>(data);
				m_shader->setUniform(uniform->getName(), value);
				break;
			}
			case gpu::EShaderUniformType::eMat3:
			{
				glm::mat3 value = *reinterpret_cast<glm::mat3 *>(data);
				m_shader->setUniform(uniform->getName(), value);
				break;
			}
			case gpu::EShaderUniformType::eMat4:
			{
				glm::mat4 value = *reinterpret_cast<glm::mat4 *>(data);
				m_shader->setUniform(uniform->getName(), value);
				break;
			}
			default: LOG_WARN("[_uploadUniformFromBuffer] Unsupported uniform type {} for {}", static_cast<int>(type), uniform->getName());
				break;
		}
	}
}
