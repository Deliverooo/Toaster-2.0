#include "vulkan_shader.hpp"

#include "core/log.hpp"
#include "core/hash.hpp"
#include "renderer/renderer.hpp"
#include "platform/application.hpp"
#include "shader_compiler/vulkan_shader_compiler.hpp"

namespace tst
{
	VulkanShader::VulkanShader(const String &path, bool force_compile) : m_filePath(path)
	{
		size_t found = path.find_last_of("/\\");
		m_name       = found != std::string::npos ? path.substr(found + 1) : path;
		found        = path.find_last_of('.');
		m_name       = found != std::string::npos ? m_name.substr(0, found) : m_name;

		reload(force_compile);
	}

	VulkanShader::~VulkanShader()
	{
	}

	void VulkanShader::release()
	{
	}

	void VulkanShader::_reload(bool force_compile)
	{
		#ifndef TST_DISTRIBUTION_BUILD
		if (!VulkanShaderCompiler::recompile(this))
		{
			TST_FATAL("Failed to reload shaders");
		}
		#endif
	}

	void VulkanShader::reload(bool force_compile)
	{
		Renderer::submit([inst = RefPtr<VulkanShader>(this), force_compile]() mutable
		{
			inst->_reload(force_compile);
		});
	}

	uint32 VulkanShader::getHash() const
	{
		return hash::fnvHash(m_filePath.string());
	}

	void VulkanShader::loadAndCreateShaders(const std::map<nvrhi::ShaderType, std::vector<uint32> > &shaderData)
	{
		m_shaderData = shaderData;

		nvrhi::IDevice *device = Application::getGraphicsDevice();

		m_shaderHandles.clear();

		String module_name;
		for (auto [stage, data]: shaderData)
		{
			nvrhi::ShaderDesc desc;
			desc.shaderType        = stage;
			desc.debugName         = m_name;
			desc.entryName         = "main";
			m_shaderHandles[stage] = device->createShader(desc, data.data(), data.size() * sizeof(uint32_t));
		}
	}

	void VulkanShader::createDescriptors()
	{
		nvrhi::DeviceHandle device = Application::getGraphicsDevice();
		m_descriptorSetLayouts.resize(m_reflectionData.shaderDescriptorSets.size());

		for (uint32 set = 0; set < m_reflectionData.shaderDescriptorSets.size(); ++set)
		{
			auto &shader_descriptor_set = m_reflectionData.shaderDescriptorSets[set];

			std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
			nvrhi::BindingLayoutDesc                    binding_desc;
			binding_desc.visibility = nvrhi::ShaderType::None;

			if (set == 0 && !m_reflectionData.pushConstantRanges.empty())
			{
				uint32 binding_index = 0;
				for (const auto &pcr: m_reflectionData.pushConstantRanges)
				{
					binding_desc.visibility = static_cast<nvrhi::ShaderType>(static_cast<uint16>(binding_desc.visibility) | static_cast<uint16>(pcr.shaderStage));

					binding_desc.bindings.push_back(nvrhi::BindingLayoutItem::PushConstants(binding_index++, pcr.size));
				}
			}

			for (auto &[binding, uniform_buffer]: shader_descriptor_set.uniformBuffers)
			{
				RenderInputDeclaration &inputDecl = shader_descriptor_set.inputDeclarations[uniform_buffer.name];
				inputDecl.type                    = ERenderInputType::eUniformBuffer;
				inputDecl.set                     = set;
				inputDecl.binding                 = binding;
				inputDecl.name                    = uniform_buffer.name;
				inputDecl.count                   = 1;

				binding_desc.visibility = static_cast<nvrhi::ShaderType>(static_cast<uint16>(binding_desc.visibility) | static_cast<uint16>(uniform_buffer.shaderStage));
				binding_desc.bindings.push_back(nvrhi::BindingLayoutItem::ConstantBuffer(binding));
			}

			for (auto &[binding, storage_buffer]: shader_descriptor_set.storageBuffers)
			{
				RenderInputDeclaration &inputDecl = shader_descriptor_set.inputDeclarations[storage_buffer.name];
				inputDecl.type                    = ERenderInputType::eStorageBuffer;
				inputDecl.set                     = set;
				inputDecl.binding                 = binding;
				inputDecl.name                    = storage_buffer.name;
				inputDecl.count                   = 1;

				binding_desc.visibility = static_cast<nvrhi::ShaderType>(static_cast<uint16>(binding_desc.visibility) | static_cast<uint16>(storage_buffer.shaderStage));

				if (storage_buffer.readOnly)
					binding_desc.bindings.push_back(nvrhi::BindingLayoutItem::RawBuffer_SRV(binding));
				else
					binding_desc.bindings.push_back(nvrhi::BindingLayoutItem::RawBuffer_UAV(binding));
			}

			for (auto &[binding, image_sampler]: shader_descriptor_set.imageSamplers)
			{
				RenderInputDeclaration &inputDecl = shader_descriptor_set.inputDeclarations[image_sampler.name];
				switch (image_sampler.dimension)
				{
					case 1:
						inputDecl.type = ERenderInputType::eImageSampler1D;
						break;
					case 2:
						inputDecl.type = ERenderInputType::eImageSampler2D;
						break;
					case 3:
						inputDecl.type = ERenderInputType::eImageSampler3D;
						break;
					default: TST_ASSERT(false);
				}

				inputDecl.set     = set;
				inputDecl.binding = binding;
				inputDecl.name    = image_sampler.name;
				inputDecl.count   = image_sampler.arraySize;

				nvrhi::BindingLayoutItem bindingLayoutItem = nvrhi::BindingLayoutItem::Texture_SRV(binding);
				bindingLayoutItem.size                     = image_sampler.arraySize;

				binding_desc.visibility = static_cast<nvrhi::ShaderType>(static_cast<uint16>(binding_desc.visibility) | static_cast<uint16>(image_sampler.shaderStage));

				binding_desc.bindings.push_back(bindingLayoutItem);
			}

			for (auto &[binding, imageSampler]: shader_descriptor_set.separateTextures)
			{
				RenderInputDeclaration &inputDecl = shader_descriptor_set.inputDeclarations[imageSampler.name];
				switch (imageSampler.dimension)
				{
					case 1:
						inputDecl.type = ERenderInputType::eImageSampler1D;
						break;
					case 2:
						inputDecl.type = ERenderInputType::eImageSampler2D;
						break;
					case 3:
						inputDecl.type = ERenderInputType::eImageSampler3D;
						break;
					default: TST_ASSERT(false);
				}
				inputDecl.set     = set;
				inputDecl.binding = binding;
				inputDecl.name    = imageSampler.name;

				inputDecl.count = imageSampler.arraySize;

				nvrhi::BindingLayoutItem bindingLayoutItem = nvrhi::BindingLayoutItem::Texture_SRV(binding);
				bindingLayoutItem.size                     = imageSampler.arraySize;

				binding_desc.visibility = static_cast<nvrhi::ShaderType>(static_cast<uint16>(binding_desc.visibility) | static_cast<uint16>(imageSampler.shaderStage));

				binding_desc.bindings.push_back(bindingLayoutItem);
			}
			for (auto &[binding, imageSampler]: shader_descriptor_set.separateSamplers)
			{
				RenderInputDeclaration &inputDecl = shader_descriptor_set.inputDeclarations[imageSampler.name];
				switch (imageSampler.dimension)
				{
					case 0:
						inputDecl.type = ERenderInputType::eImageSampler;
						break;
					case 1:
						inputDecl.type = ERenderInputType::eImageSampler1D;
						break;
					case 2:
						inputDecl.type = ERenderInputType::eImageSampler2D;
						break;
					case 3:
						inputDecl.type = ERenderInputType::eImageSampler3D;
						break;
					default: TST_ASSERT(false);
				}

				inputDecl.set     = set;
				inputDecl.binding = binding;
				inputDecl.name    = imageSampler.name;
				inputDecl.count   = imageSampler.arraySize;

				nvrhi::BindingLayoutItem bindingLayoutItem = nvrhi::BindingLayoutItem::Sampler(binding);
				bindingLayoutItem.size                     = imageSampler.arraySize;

				binding_desc.visibility = static_cast<nvrhi::ShaderType>(static_cast<uint16>(binding_desc.visibility) | static_cast<uint16>(imageSampler.shaderStage));

				binding_desc.bindings.push_back(bindingLayoutItem);
			}

			for (auto &[binding, imageSampler]: shader_descriptor_set.storageImages)
			{
				RenderInputDeclaration &inputDecl = shader_descriptor_set.inputDeclarations[imageSampler.name];
				switch (imageSampler.dimension)
				{
					case 0:
						inputDecl.type = ERenderInputType::eImageSampler;
						break;
					case 1:
						inputDecl.type = ERenderInputType::eImageSampler1D;
						break;
					case 2:
						inputDecl.type = ERenderInputType::eImageSampler2D;
						break;
					case 3:
						inputDecl.type = ERenderInputType::eImageSampler3D;
						break;
					default: TST_ASSERT(false);
				}

				inputDecl.set     = set;
				inputDecl.binding = binding;
				inputDecl.name    = imageSampler.name;
				inputDecl.count   = imageSampler.arraySize;

				nvrhi::BindingLayoutItem bindingLayoutItem = nvrhi::BindingLayoutItem::Texture_UAV(binding);
				bindingLayoutItem.size                     = imageSampler.arraySize;

				binding_desc.visibility = static_cast<nvrhi::ShaderType>(static_cast<uint16>(binding_desc.visibility) | static_cast<uint16>(imageSampler.shaderStage));

				binding_desc.bindings.push_back(bindingLayoutItem);
			}

			m_descriptorSetLayouts[set] = device->createBindingLayout(binding_desc);
		}
	}

	nvrhi::ShaderHandle VulkanShader::getHandle(nvrhi::ShaderType type) const
	{
		TST_ASSERT(m_shaderHandles.contains(type));
		return m_shaderHandles.at(type);
	}

	const std::unordered_map<std::string, ShaderResourceDeclaration> &VulkanShader::getResources() const
	{
		return m_reflectionData.resources;
	}

	void VulkanShader::serializeReflectionData(StreamWriter *serializer)
	{
		serializer->writeRaw<uint32_t>((uint32_t) m_reflectionData.shaderDescriptorSets.size());
		for (const auto &descriptorSet: m_reflectionData.shaderDescriptorSets)
		{
			serializer->writeMap(descriptorSet.uniformBuffers);
			serializer->writeMap(descriptorSet.storageBuffers);
			serializer->writeMap(descriptorSet.imageSamplers);
			serializer->writeMap(descriptorSet.storageImages);
			serializer->writeMap(descriptorSet.separateTextures);
			serializer->writeMap(descriptorSet.separateSamplers);
			serializer->writeMap(descriptorSet.inputDeclarations);
		}

		serializer->writeMap(m_reflectionData.resources);
		serializer->writeMap(m_reflectionData.constantBuffers);
		serializer->writeArray(m_reflectionData.pushConstantRanges);
	}

	void VulkanShader::setReflectionData(const ShaderReflectionData &reflectionData)
	{
		m_reflectionData = reflectionData;
	}

	bool VulkanShader::tryReadReflectionData(StreamReader *serializer)
	{
		uint32_t shaderDescriptorSetCount;
		serializer->readRaw<uint32_t>(shaderDescriptorSetCount);

		for (uint32_t i = 0; i < shaderDescriptorSetCount; i++)
		{
			auto &descriptorSet = m_reflectionData.shaderDescriptorSets.emplace_back();
			serializer->readMap(descriptorSet.uniformBuffers);
			serializer->readMap(descriptorSet.storageBuffers);
			serializer->readMap(descriptorSet.imageSamplers);
			serializer->readMap(descriptorSet.storageImages);
			serializer->readMap(descriptorSet.separateTextures);
			serializer->readMap(descriptorSet.separateSamplers);
			serializer->readMap(descriptorSet.inputDeclarations);
		}

		serializer->readMap(m_reflectionData.resources);
		serializer->readMap(m_reflectionData.constantBuffers);
		serializer->readArray(m_reflectionData.pushConstantRanges);

		return true;
	}

	void VulkanShader::setShaderMacro(const String &macro_name, const String &value)
	{
	}


}
