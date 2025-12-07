#include "shader.hpp"
#include "vulkan/vulkan_shader.hpp"
#include "vulkan/shader_compiler/vulkan_shader_compiler.hpp"

namespace tst
{
	RefPtr<Shader> Shader::create(const String &name, bool force_compile)
	{
		return make_reference<VulkanShader>(name, force_compile);
	}

	void ShaderLibrary::add(const RefPtr<Shader> &shader)
	{
		auto &name = shader->getName();
		TST_ASSERT(m_shaders.find(name) == m_shaders.end());
		m_shaders[name] = shader;
	}

	void ShaderLibrary::load(const std::filesystem::path& path, bool forceCompile)
	{
		RefPtr<Shader> shader;
		#ifndef TST_DISTRIBUTION_BUILD
		shader = VulkanShaderCompiler::compile(path, forceCompile);
		#endif

		auto &name = shader->getName();
		TST_ASSERT(m_shaders.find(name) == m_shaders.end());
		m_shaders[name] = shader;
	}

	void ShaderLibrary::load(std::string_view name, const std::filesystem::path& path)
	{
		TST_ASSERT(m_shaders.find(std::string(name)) == m_shaders.end());
		m_shaders[std::string(name)] = Shader::create(path.string());
	}

	const RefPtr<Shader> &ShaderLibrary::get(const std::string &name) const
	{
		TST_ASSERT(m_shaders.contains(name));
		return m_shaders.at(name);
	}
}
