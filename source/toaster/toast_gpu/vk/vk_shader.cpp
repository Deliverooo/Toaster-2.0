#include "vk_shader.hpp"
#include "vk_gpu_context.hpp"

#include <ranges>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "toast_lib/logging.hpp"

namespace toaster::gpu
{
	VKShader::VKShader(VKGPUContext *p_ctx, const BytecodeMap &p_bytecode_map) : m_ctx(p_ctx), m_shaderBytecodeMap(p_bytecode_map)
	{
		for (auto &[stage, code]: p_bytecode_map)
		{
			m_shaderModules.insert({stage, m_ctx->createShaderModule(code)});

			vk::PipelineShaderStageCreateInfo &create_info = m_shaderCreateInfos[stage];
			create_info                                    = vk::PipelineShaderStageCreateInfo{};
			create_info.module                             = *m_shaderModules.at(stage);
			create_info.stage                              = stage;
			create_info.pName                              = "main";
		}

		for (auto &[stage, code]: p_bytecode_map)
			_reflect(stage, code);

		_createDescriptors();
	}

	const VKShader::PipelineCreateInfoMap &VKShader::getPipelineShaderStageCreateInfoMap() const
	{
		return m_shaderCreateInfos;
	}

	std::vector<vk::PipelineShaderStageCreateInfo> VKShader::getPipelineShaderStageCreateInfos() const
	{
		std::vector<vk::PipelineShaderStageCreateInfo> result;
		for (const auto &info: m_shaderCreateInfos | std::views::values)
			result.emplace_back(info);
		return result;
	}

	const vk::PipelineShaderStageCreateInfo &VKShader::getPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits p_stage) const
	{
		TST_ASSERT_MSG(m_shaderCreateInfos.contains(p_stage), "Stage is not present in shader create infos map");
		return m_shaderCreateInfos.at(p_stage);
	}

	const VKShader::BytecodeMap &VKShader::getShaderBytecodeMap() const
	{
		return m_shaderBytecodeMap;
	}

	const VKShader::Bytecode &VKShader::getShaderBytecode(vk::ShaderStageFlagBits p_stage) const
	{
		TST_ASSERT_MSG(m_shaderBytecodeMap.contains(p_stage), "Stage is not present in bytecode map");
		return m_shaderBytecodeMap.at(p_stage);
	}

	std::vector<vk::DescriptorSetLayout> VKShader::getDescriptorSetLayouts() const
	{
		std::vector<vk::DescriptorSetLayout> result;
		for (const auto &layout: m_descriptorSetLayouts)
			result.emplace_back(layout);
		return result;
	}

	const vk::raii::DescriptorSetLayout &VKShader::getDescriptorSetLayout(uint32 p_set_index) const
	{
		TST_ASSERT_MSG(p_set_index < m_descriptorSetLayouts.size(), "Set index out of bounds");
		return m_descriptorSetLayouts.at(p_set_index);
	}

	void VKShader::_reflect(vk::ShaderStageFlagBits p_stage, Bytecode p_bytecode)
	{
		const Bytecode            copy = {p_bytecode.begin(), p_bytecode.end()};
		spirv_cross::CompilerGLSL compiler{copy};
		auto                      resources{compiler.get_shader_resources()};

		LOG_INFO("Shader stage: {}\n", vk::to_string(p_stage));

		if (!resources.uniform_buffers.empty())
			LOG_INFO("Uniform buffers:");
		for (const auto &resource: resources.uniform_buffers)
		{
			const auto &name = resource.name;

			auto & buffer_type = compiler.get_type(resource.base_type_id);
			uint32 size        = compiler.get_declared_struct_size(buffer_type);

			uint32 member_count = buffer_type.member_types.size();

			uint32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 set     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

			if (set >= m_reflectionData.descriptorSets.size())
				m_reflectionData.descriptorSets.resize(set + 1);

			auto &        descriptorSet = m_reflectionData.descriptorSets[set];
			UniformBuffer uniform_buffer{};
			uniform_buffer.size    = size;
			uniform_buffer.stage   = p_stage;
			uniform_buffer.name    = name;
			uniform_buffer.binding = binding;

			descriptorSet.uniformBuffers[binding] = uniform_buffer;

			LOG_TRACE("\t{} | Set: {} | Binding: {}", name, set, binding);
			LOG_TRACE("\t\tMember count: {}", member_count);
			LOG_TRACE("\t\tSize: {}", size);
		}

		if (!resources.sampled_images.empty())
			LOG_INFO("Combined image samplers:");
		for (const auto &resource: resources.sampled_images)
		{
			const auto &name = resource.name;

			uint32 binding    = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 set        = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			auto & type       = compiler.get_type(resource.base_type_id);
			uint32 array_size = type.array.empty() ? 1 : type.array[0];
			if (array_size == 0)
				array_size = 1;

			if (set >= m_reflectionData.descriptorSets.size())
				m_reflectionData.descriptorSets.resize(set + 1);

			auto &       descriptorSet = m_reflectionData.descriptorSets[set];
			ImageSampler image_sampler{};
			image_sampler.stage     = p_stage;
			image_sampler.name      = name;
			image_sampler.binding   = binding;
			image_sampler.arraySize = array_size;

			descriptorSet.imageSamplers[binding] = image_sampler;

			LOG_TRACE("\t{} | Set: {} | Binding: {}", name, set, binding);
			LOG_TRACE("\t\tArray size: {}", array_size);
		}
		LOG_INFO("");
	}

	void VKShader::_createDescriptors()
	{
		for (uint32 set{0u}; set < m_reflectionData.descriptorSets.size(); ++set)
		{
			auto &descriptor_set = m_reflectionData.descriptorSets.at(set);

			if (!descriptor_set.uniformBuffers.empty())
			{
				vk::DescriptorPoolSize &pool_size = m_poolSizes[set].emplace_back();
				pool_size.type                    = vk::DescriptorType::eUniformBuffer;
				pool_size.descriptorCount         = descriptor_set.uniformBuffers.size();
			}

			if (!descriptor_set.imageSamplers.empty())
			{
				vk::DescriptorPoolSize &pool_size = m_poolSizes[set].emplace_back();
				pool_size.type                    = vk::DescriptorType::eCombinedImageSampler;
				pool_size.descriptorCount         = descriptor_set.imageSamplers.size();
			}

			std::vector<vk::DescriptorSetLayoutBinding> layout_bindings{};
			for (auto &[binding, uniform_buffer]: descriptor_set.uniformBuffers)
			{
				vk::DescriptorSetLayoutBinding &layout_binding = layout_bindings.emplace_back();
				layout_binding.binding                         = binding;
				layout_binding.descriptorCount                 = 1;
				layout_binding.descriptorType                  = vk::DescriptorType::eUniformBuffer;
				layout_binding.pImmutableSamplers              = nullptr;
				layout_binding.stageFlags                      = uniform_buffer.stage;

				vk::WriteDescriptorSet &write_descriptor = descriptor_set.writeDescriptorSets[uniform_buffer.name];
				write_descriptor                         = vk::WriteDescriptorSet{};
				write_descriptor.descriptorCount         = 1;
				write_descriptor.descriptorType          = vk::DescriptorType::eUniformBuffer;
				write_descriptor.dstBinding              = binding;
			}

			for (auto &[binding, image_sampler]: descriptor_set.imageSamplers)
			{
				vk::DescriptorSetLayoutBinding &layout_binding = layout_bindings.emplace_back();
				layout_binding.binding                         = binding;
				layout_binding.descriptorCount                 = image_sampler.arraySize;
				layout_binding.descriptorType                  = vk::DescriptorType::eCombinedImageSampler;
				layout_binding.pImmutableSamplers              = nullptr;
				layout_binding.stageFlags                      = image_sampler.stage;

				vk::WriteDescriptorSet &write_descriptor = descriptor_set.writeDescriptorSets[image_sampler.name];
				write_descriptor                         = vk::WriteDescriptorSet{};
				write_descriptor.descriptorCount         = image_sampler.arraySize;
				write_descriptor.descriptorType          = vk::DescriptorType::eCombinedImageSampler;
				write_descriptor.dstBinding              = binding;
			}

			vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
			descriptor_set_layout_create_info.bindingCount = layout_bindings.size();
			descriptor_set_layout_create_info.pBindings    = layout_bindings.data();

			if (set >= m_descriptorSetLayouts.size())
			{
				m_descriptorSetLayouts.reserve(set + 1);
				while (m_descriptorSetLayouts.size() <= set)
				{
					m_descriptorSetLayouts.emplace_back(nullptr);
				}
			}

			m_descriptorSetLayouts[set] = {m_ctx->getDevice(), descriptor_set_layout_create_info};
		}
	}
}
