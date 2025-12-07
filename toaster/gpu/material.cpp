#include "material.hpp"

#include "renderer/renderer.hpp"

namespace tst
{
	Material::Material(RefPtr<Shader> shader, const std::string &name)
		: m_shader(shader), m_name(name)
	{
		init();
		Renderer::registerShaderDependency(shader, this);
	}

	Material::Material(RefPtr<Material> other, const std::string &name)
		: m_shader(other->getShader()), m_name(name)
	{
		if (name.empty())
			m_name = other->getName();

		init();
		Renderer::registerShaderDependency(m_shader, this);

		m_uniformStorageBuffer = Buffer::copy(other->m_uniformStorageBuffer.data, other->m_uniformStorageBuffer.size);
		m_descriptorSetManager = DescriptorSetManager::Copy(other->m_descriptorSetManager);
	}

	void Material::init()
	{
		allocateStorage();

		m_materialFlags |= (uint32_t) MaterialFlag::DepthTest;
		m_materialFlags |= (uint32_t) MaterialFlag::Blend;

		DescriptorSetManagerSpecification dmSpec;
		dmSpec.debugName        = m_name.empty() ? std::format("{} (Material)", m_shader->getName()) : m_name;
		dmSpec.shader           = m_shader.as<VulkanShader>();
		dmSpec.startSet         = 0;
		dmSpec.EndSet           = 0;
		dmSpec.IsDynamic        = false;
		dmSpec.DefaultResources = true;
		m_descriptorSetManager  = DescriptorSetManager(dmSpec);
	}

	void Material::allocateStorage()
	{
		const auto &shaderBuffers = m_shader->getShaderBuffers();

		if (shaderBuffers.size() > 0)
		{
			uint32_t size = 0;
			for (auto [name, shaderBuffer]: shaderBuffers)
				size += shaderBuffer.size;

			m_uniformStorageBuffer.alloc(size);
			m_uniformStorageBuffer.zeroInit();
		}
	}

	void Material::invalidate()
	{
	}

	void Material::onShaderReloaded()
	{
	}

	const ShaderUniform *Material::findUniformDeclaration(const std::string &name)
	{
		const auto &shaderBuffers = m_shader->getShaderBuffers();

		TST_ASSERT(shaderBuffers.size() <= 1);

		if (shaderBuffers.size() > 0)
		{
			const ShaderBuffer &buffer = (*shaderBuffers.begin()).second;
			if (buffer.uniforms.find(name) == buffer.uniforms.end())
				return nullptr;

			return &buffer.uniforms.at(name);
		}
		return nullptr;
	}

	const ShaderResourceDeclaration *Material::findResourceDeclaration(const std::string &name)
	{
		auto &resources = m_shader->getResources();
		if (resources.find(name) != resources.end())
			return &resources.at(name);

		return nullptr;
	}

	void Material::set(const std::string &name, float value)
	{
		set<float>(name, value);
	}

	void Material::set(const std::string &name, int value)
	{
		set<int>(name, value);
	}

	void Material::set(const std::string &name, uint32_t value)
	{
		set<uint32_t>(name, value);
	}

	void Material::set(const std::string &name, bool value)
	{
		// Bools are 4-byte ints
		set<int>(name, (int) value);
	}

	void Material::set(const std::string &name, const tsm::vec2i &value)
	{
		set<tsm::vec2i>(name, value);
	}

	void Material::set(const std::string &name, const tsm::vec3i &value)
	{
		set<tsm::vec3i>(name, value);
	}

	void Material::set(const std::string &name, const tsm::vec4i &value)
	{
		set<tsm::vec4i>(name, value);
	}

	void Material::set(const std::string &name, const tsm::vec2f &value)
	{
		set<tsm::vec2f>(name, value);
	}

	void Material::set(const std::string &name, const tsm::vec3f &value)
	{
		set<tsm::vec3f>(name, value);
	}

	void Material::set(const std::string &name, const tsm::vec4f &value)
	{
		set<tsm::vec4f>(name, value);
	}

	void Material::set(const std::string &name, const tsm::mat3x3f &value)
	{
		set<tsm::mat3x3f>(name, value);
	}

	void Material::set(const std::string &name, const tsm::mat4x4f &value)
	{
		set<tsm::mat4x4f>(name, value);
	}

	void Material::set(const std::string &name, RefPtr<Texture2D> texture)
	{
		m_descriptorSetManager.SetInput(name, texture);
	}

	void Material::set(const std::string &name, RefPtr<Texture2D> texture, uint32_t arrayIndex)
	{
		m_descriptorSetManager.SetInput(name, texture, arrayIndex);
	}

	// void Material::set(const std::string &name, RefPtr<TextureCube> texture)
	// {
	// 	m_DescriptorSetManager.SetInput(name, texture);
	// }

	void Material::set(const std::string &name, RefPtr<Image2D> image)
	{
		m_descriptorSetManager.SetInput(name, image);
	}

	void Material::set(const std::string &name, RefPtr<Image2D> image, uint32_t arrayIndex)
	{
		m_descriptorSetManager.SetInput(name, image, arrayIndex);
	}

	void Material::set(const std::string &name, RefPtr<ImageView> image)
	{
		m_descriptorSetManager.SetInput(name, image);
	}

	void Material::set(const std::string &name, RefPtr<ImageView> image, uint32_t arrayIndex)
	{
		m_descriptorSetManager.SetInput(name, image, arrayIndex);
	}

	float &Material::getFloat(const std::string &name)
	{
		return get<float>(name);
	}

	int32_t &Material::getInt(const std::string &name)
	{
		return get<int32_t>(name);
	}

	uint32_t &Material::getUInt(const std::string &name)
	{
		return get<uint32_t>(name);
	}

	bool &Material::getBool(const std::string &name)
	{
		return get<bool>(name);
	}

	tsm::vec2f &Material::getVector2(const std::string &name)
	{
		return get<tsm::vec2f>(name);
	}

	tsm::vec3f &Material::getVector3(const std::string &name)
	{
		return get<tsm::vec3f>(name);
	}

	tsm::vec4f &Material::getVector4(const std::string &name)
	{
		return get<tsm::vec4f>(name);
	}

	tsm::mat3x3f &Material::getMatrix3(const std::string &name)
	{
		return get<tsm::mat3x3f>(name);
	}

	tsm::mat4x4f &Material::getMatrix4(const std::string &name)
	{
		return get<tsm::mat4x4f>(name);
	}

	RefPtr<Texture2D> Material::getTexture2D(const std::string &name)
	{
		return getResource<Texture2D>(name);
	}

	// RefPtr<TextureCube> Material::TryGetTextureCube(const std::string &name)
	// {
	// 	return TryGetResource<TextureCube>(name);
	// }

	RefPtr<Texture2D> Material::tryGetTexture2D(const std::string &name)
	{
		return tryGetResource<Texture2D>(name);
	}

	// RefPtr<TextureCube> Material::getTextureCube(const std::string &name)
	// {
	// 	return GetResource<TextureCube>(name);
	// }

	void Material::prepare()
	{
		m_descriptorSetManager.InvalidateAndUpdate();
	}

	nvrhi::BindingSetHandle Material::getBindingSet(uint32_t frameIndex) const
	{
		return m_descriptorSetManager.GetBindingSet(frameIndex);
	}
}
