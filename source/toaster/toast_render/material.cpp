#include "material.hpp"

#include <utility>

#include "toast_lib/toast_assert.h"

namespace toaster
{
	RefPtr<MMaterial> MMaterial::create(const RefPtr<gpu::IShader> &p_shader, const std::string &p_name)
	{
		return make_reference<MMaterial>(p_shader, p_name);
	}

	MMaterial::MMaterial(const RefPtr<gpu::IShader> &p_shader, std::string p_name) : m_name(std::move(p_name))
	{
		TST_ASSERT_MSG(p_shader != nullptr, "Material shader cannot be null");
		m_shader = p_shader;
	}

	glm::vec3 &MMaterial::getAlbedoColour()
	{
		return m_albedoColour;
	}

	void MMaterial::setAlbedoColour(const glm::vec3 &p_colour)
	{
		m_albedoColour = p_colour;
	}

	float32 &MMaterial::getMetalness()
	{
		return m_metalness;
	}

	void MMaterial::setMetalness(float32 p_metalness)
	{
		m_metalness = p_metalness;
	}

	float32 &MMaterial::getRoughness()
	{
		return m_roughness;
	}

	void MMaterial::setRoughness(float32 p_roughness)
	{
		m_roughness = p_roughness;
	}

	float32 &MMaterial::getOpacity()
	{
		return m_opacity;
	}

	void MMaterial::setOpacity(float32 p_opacity)
	{
		m_opacity = p_opacity;
	}

	RefPtr<gpu::ITexture2D> MMaterial::getAlbedoMap()
	{
		return m_albedoMap;
	}

	void MMaterial::setAlbedoMap(const RefPtr<gpu::ITexture2D> &p_albedo_map)
	{
		m_albedoMap = p_albedo_map;
	}

	void MMaterial::clearAlbedoMap()
	{
		m_albedoMap = nullptr;
	}

	RefPtr<gpu::ITexture2D> MMaterial::getNormalMap()
	{
		return m_normalMap;
	}

	void MMaterial::setNormalMap(const RefPtr<gpu::ITexture2D> &p_normal_map)
	{
		m_normalMap = p_normal_map;
	}

	void MMaterial::clearNormalMap()
	{
		m_normalMap = nullptr;
	}

	bool MMaterial::isUsingNormalMap() const
	{
		return m_useNormalMap;
	}

	void MMaterial::setUseNormalMap(bool p_use)
	{
		m_useNormalMap = p_use;
	}

	RefPtr<gpu::ITexture2D> MMaterial::getMetalnessMap()
	{
		return m_metalnessMap;
	}

	void MMaterial::setMetalnessMap(const RefPtr<gpu::ITexture2D> &p_metalness_map)
	{
		m_metalnessMap = p_metalness_map;
	}

	void MMaterial::clearMetalnessMap()
	{
		m_metalnessMap = nullptr;
	}

	RefPtr<gpu::ITexture2D> MMaterial::getRoughnessMap()
	{
		return m_roughnessMap;
	}

	void MMaterial::setRoughnessMap(const RefPtr<gpu::ITexture2D> &p_roughness_map)
	{
		m_roughnessMap = p_roughness_map;
	}

	void MMaterial::clearRoughnessMap()
	{
		m_roughnessMap = nullptr;
	}

	void MMaterial::use() const
	{
		m_shader->bind();

		m_albedoMap->bind();
		m_shader->setUniform("u_AlbedoMap", 0);
		m_shader->setUniform("u_AlbedoColour", m_albedoColour);
	}

	RefPtr<Material> Material::create(const RefPtr<gpu::IShader> &p_shader)
	{
		return make_reference<Material>(p_shader);
	}

	Material::Material(const RefPtr<gpu::IShader> &p_shader) : m_shader(p_shader)
	{
	}

	void Material::use()
	{
		m_shader->bind();
	}

	RefPtr<gpu::IShader> Material::getShader()
	{
		return m_shader;
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
}
