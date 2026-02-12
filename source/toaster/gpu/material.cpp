#include "material.hpp"

#include <utility>

#include "toast_assert.h"

namespace toaster::gpu
{
	RefPtr<Material> Material::create(const RefPtr<Shader> &p_shader, const std::string &p_name)
	{
		return make_reference<Material>(p_shader, p_name);
	}

	Material::Material(const RefPtr<Shader> &p_shader, std::string p_name) : m_name(std::move(p_name))
	{
		TST_ASSERT_MSG(p_shader != nullptr, "Material shader cannot be null");
		m_shader = p_shader;
	}

	glm::vec3 &Material::getAlbedoColour()
	{
		return m_albedoColour;
	}

	void Material::setAlbedoColour(const glm::vec3 &p_colour)
	{
		m_albedoColour = p_colour;
	}

	float32 &Material::getMetalness()
	{
		return m_metalness;
	}

	void Material::setMetalness(float32 p_metalness)
	{
		m_metalness = p_metalness;
	}

	float32 &Material::getRoughness()
	{
		return m_roughness;
	}

	void Material::setRoughness(float32 p_roughness)
	{
		m_roughness = p_roughness;
	}

	float32 &Material::getOpacity()
	{
		return m_opacity;
	}

	void Material::setOpacity(float32 p_opacity)
	{
		m_opacity = p_opacity;
	}

	RefPtr<Texture2D> Material::getAlbedoMap()
	{
		return m_albedoMap;
	}

	void Material::setAlbedoMap(const RefPtr<Texture2D> &p_albedo_map)
	{
		m_albedoMap = p_albedo_map;
	}

	void Material::clearAlbedoMap()
	{
		m_albedoMap = nullptr;
	}

	RefPtr<Texture2D> Material::getNormalMap()
	{
		return m_normalMap;
	}

	void Material::setNormalMap(const RefPtr<Texture2D> &p_normal_map)
	{
		m_normalMap = p_normal_map;
	}

	void Material::clearNormalMap()
	{
		m_normalMap = nullptr;
	}

	bool Material::isUsingNormalMap() const
	{
		return m_useNormalMap;
	}

	void Material::setUseNormalMap(bool p_use)
	{
		m_useNormalMap = p_use;
	}

	RefPtr<Texture2D> Material::getMetalnessMap()
	{
		return m_metalnessMap;
	}

	void Material::setMetalnessMap(const RefPtr<Texture2D> &p_metalness_map)
	{
		m_metalnessMap = p_metalness_map;
	}

	void Material::clearMetalnessMap()
	{
		m_metalnessMap = nullptr;
	}

	RefPtr<Texture2D> Material::getRoughnessMap()
	{
		return m_roughnessMap;
	}

	void Material::setRoughnessMap(const RefPtr<Texture2D> &p_roughness_map)
	{
		m_roughnessMap = p_roughness_map;
	}

	void Material::clearRoughnessMap()
	{
		m_roughnessMap = nullptr;
	}

	void Material::use() const
	{
		m_shader->bind();

		m_albedoMap->bind();
		m_shader->setUniform("u_AlbedoMap", 0);
		m_shader->setUniform("u_AlbedoColour", m_albedoColour);
	}
}
