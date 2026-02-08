#pragma once

#include <unordered_map>

#include "material.hpp"

namespace toaster::gpu
{
	class GLMaterial : public Material
	{
	public:
		GLMaterial(const RefPtr<Shader> &p_shader, const std::string &p_name);
		~GLMaterial() override;

		void use() const override;

		void set(const std::string &p_name, bool p_value) override;
		void set(const std::string &p_name, uint32 p_value) override;
		void set(const std::string &p_name, int32 p_value) override;
		void set(const std::string &p_name, float32 p_value) override;
		void set(const std::string &p_name, glm::vec2 p_value) override;
		void set(const std::string &p_name, const glm::vec3 &p_value) override;
		void set(const std::string &p_name, const glm::vec4 &p_value) override;

	private:
		std::string    m_name;
		RefPtr<Shader> m_shader;

		std::unordered_map<std::string, MaterialPropertyType> m_uniforms;
	};
}
