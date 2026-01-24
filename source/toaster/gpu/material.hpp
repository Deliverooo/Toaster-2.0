#pragma once

#include <variant>

#include "shader.hpp"
#include "texture.hpp"

namespace toaster::gpu
{
	using MaterialPropertyType = std::variant<bool, uint32, int32, float32, glm::vec2, glm::vec3, glm::vec4>;

	class Material
	{
	public:
		static RefPtr<Material> create(const RefPtr<Shader> &p_shader, const std::string &p_name);
		virtual                 ~Material() = default;

		virtual void use() const = 0;

		virtual void set(const std::string &p_name, bool p_value) = 0;
		virtual void set(const std::string &p_name, uint32 p_value) = 0;
		virtual void set(const std::string &p_name, int32 p_value) = 0;
		virtual void set(const std::string &p_name, float32 p_value) = 0;
		virtual void set(const std::string &p_name, glm::vec2 p_value) = 0;
		virtual void set(const std::string &p_name, const glm::vec3 &p_value) = 0;
		virtual void set(const std::string &pName, const glm::vec4 &p_value) = 0;
	};
}
