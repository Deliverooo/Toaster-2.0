#pragma once

#include "toaster/toast_gpu/shader.hpp"
#include "toaster/toast_gpu/texture.hpp"

namespace toaster
{
	class Material
	{
	public:
		static RefPtr<Material> create(const RefPtr<gpu::Shader> &p_shader, const std::string &p_name);
		Material(const RefPtr<gpu::Shader> &p_shader, std::string p_name);
		~Material() = default;

		glm::vec3 &getAlbedoColour();
		void       setAlbedoColour(const glm::vec3 &p_colour);

		float32 &getMetalness();
		void     setMetalness(float32 p_metalness);

		float32 &getRoughness();
		void     setRoughness(float32 p_roughness);

		float32 &getOpacity();
		void     setOpacity(float32 p_opacity);

		RefPtr<gpu::Texture2D> getAlbedoMap();
		void                   setAlbedoMap(const RefPtr<gpu::Texture2D> &p_albedo_map);
		void                   clearAlbedoMap();

		RefPtr<gpu::Texture2D> getNormalMap();
		void                   setNormalMap(const RefPtr<gpu::Texture2D> &p_normal_map);
		void                   clearNormalMap();

		[[nodiscard]] bool isUsingNormalMap() const;
		void               setUseNormalMap(bool p_use);

		RefPtr<gpu::Texture2D> getMetalnessMap();
		void                   setMetalnessMap(const RefPtr<gpu::Texture2D> &p_metalness_map);
		void                   clearMetalnessMap();

		RefPtr<gpu::Texture2D> getRoughnessMap();
		void                   setRoughnessMap(const RefPtr<gpu::Texture2D> &p_roughness_map);
		void                   clearRoughnessMap();

		void use() const;

	private:
		std::string m_name;

		RefPtr<gpu::Shader> m_shader;

		RefPtr<gpu::Texture2D> m_albedoMap{nullptr};
		RefPtr<gpu::Texture2D> m_normalMap{nullptr};
		RefPtr<gpu::Texture2D> m_metalnessMap{nullptr};
		RefPtr<gpu::Texture2D> m_roughnessMap{nullptr};

		glm::vec3 m_albedoColour{1.0f};
		float32   m_metalness{0.0f};
		float32   m_roughness{1.0f};
		float32   m_opacity{1.0f};

		bool m_useNormalMap{false}; // sometimes you may not want to use a normal map
	};
}
