#include "gl/gl_material.hpp"

namespace toaster::gpu
{
	// https://en.cppreference.com/w/cpp/utility/variant/visit
	template<class... Ts>
	struct overloads : Ts...
	{
		using Ts::operator()...;
	};

	struct MaterialUniformUploader
	{
		const RefPtr<Shader> &shader;
		const std::string &   uniformName;

		void operator()(float value) const
		{
			shader->setUniform(uniformName, value);
		}

		void operator()(int value) const
		{
			shader->setUniform(uniformName, value);
		}

		void operator()(bool value) const
		{
			shader->setUniform(uniformName, value);
		}

		void operator()(const glm::vec3 &value) const
		{
			shader->setUniform(uniformName, value);
		}

		void operator()(const glm::vec4 &value) const
		{
			shader->setUniform(uniformName, value);
		}
	};

	GLMaterial::GLMaterial(const RefPtr<Shader> &p_shader, const std::string &p_name) : m_name(p_name), m_shader(p_shader)
	{
	}

	GLMaterial::~GLMaterial()
	{
	}

	void GLMaterial::use() const
	{
		for (const auto &[name, value]: m_uniforms)
		{
			const auto visitor = overloads{
				[this, name](bool v) { m_shader->setUniform(name, v); },
				[this, name](uint32 v) { m_shader->setUniform(name, v); },
				[this, name](int32 v) { m_shader->setUniform(name, v); },
				[this, name](float32 v) { m_shader->setUniform(name, v); },
				[this, name](glm::vec2 v) { m_shader->setUniform(name, v); },
				[this, name](const glm::vec3 &v) { m_shader->setUniform(name, v); },
				[this, name](const glm::vec4 &v) { m_shader->setUniform(name, v); },
			};

			std::visit(visitor, value);
		}
	}

	void GLMaterial::set(const std::string &p_name, bool p_value)
	{
		m_uniforms[p_name] = p_value;
	}

	void GLMaterial::set(const std::string &p_name, uint32 p_value)
	{
		m_uniforms[p_name] = p_value;
	}

	void GLMaterial::set(const std::string &p_name, int32 p_value)
	{
		m_uniforms[p_name] = p_value;
	}

	void GLMaterial::set(const std::string &p_name, float32 p_value)
	{
		m_uniforms[p_name] = p_value;
	}

	void GLMaterial::set(const std::string &p_name, glm::vec2 p_value)
	{
		m_uniforms[p_name] = p_value;
	}

	void GLMaterial::set(const std::string &p_name, const glm::vec3 &p_value)
	{
		m_uniforms[p_name] = p_value;
	}

	void GLMaterial::set(const std::string &p_name, const glm::vec4 &p_value)
	{
		m_uniforms[p_name] = p_value;
	}
}
