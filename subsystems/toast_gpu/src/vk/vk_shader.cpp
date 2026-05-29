#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_lib/logging.hpp"

#include <ranges>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#define TST_SHADER_LOG_TRACE(...) do { if(m_device->getSpecInfo().printShaderDebugInfo) { LOG_TRACE(__VA_ARGS__); } } while(false)
#define TST_SHADER_LOG_INFO(...) do { if(m_device->getSpecInfo().printShaderDebugInfo) { LOG_INFO(__VA_ARGS__); } } while(false)

namespace toaster::gpu
{
	VKShader::VKShader(VKLogicalDevice *p_device, const BytecodeMap &p_bytecode_map, const String &p_name) : m_device(p_device), m_name(p_name),
																											 m_shaderBytecodeMap(p_bytecode_map)
	{
		TST_PERMA_ASSERT_MSG(p_device, "Device cannot be null");

		for (auto &[stage, code]: p_bytecode_map)
		{
			m_shaderModules.insert({stage, m_device->createShaderModule(code)});

			vk::PipelineShaderStageCreateInfo &create_info = m_shaderCreateInfos[stage];
			create_info                                    = vk::PipelineShaderStageCreateInfo{};
			create_info.module                             = *m_shaderModules.at(stage);
			create_info.stage                              = stage;
			create_info.pName                              = "main";
		}

		TST_SHADER_LOG_TRACE("Shader: {} [", m_name);
		for (auto &[stage, code]: p_bytecode_map)
			_reflect(stage, code);
		TST_SHADER_LOG_TRACE("]");

		_createDescriptors();
	}

	VKShader::VKShader(VKLogicalDevice *p_device, const InitialiserList<vk::ShaderStageFlagBits> &p_stages, const InitialiserList<Bytecode> &p_bytecodes,
					   const String &   p_name) : m_device(p_device), m_name(p_name)
	{
		TST_PERMA_ASSERT_MSG(p_device, "Device cannot be null");

		auto kit = p_stages.begin();
		auto vit = p_bytecodes.begin();
		while (kit != p_stages.end() && vit != p_bytecodes.end())
		{
			m_shaderBytecodeMap[*kit] = *vit;
			++kit;
			++vit;
		}

		for (auto &[stage, code]: m_shaderBytecodeMap)
		{
			m_shaderModules.insert({stage, m_device->createShaderModule(code)});

			vk::PipelineShaderStageCreateInfo &create_info = m_shaderCreateInfos[stage];
			create_info                                    = vk::PipelineShaderStageCreateInfo{};
			create_info.module                             = *m_shaderModules.at(stage);
			create_info.stage                              = stage;
			create_info.pName                              = "main";
		}

		TST_SHADER_LOG_TRACE("Shader: {} [", m_name);
		for (auto &[stage, code]: m_shaderBytecodeMap)
			_reflect(stage, code);
		TST_SHADER_LOG_TRACE("]");

		_createDescriptors();
	}

	auto VKShader::operator=(VKShader &&p_other) noexcept -> VKShader &
	{
		if (this != &p_other)
		{
			m_device               = p_other.m_device;
			m_name                 = p_other.m_name;
			m_shaderCreateInfos    = p_other.m_shaderCreateInfos;
			m_shaderModules        = std::move(p_other.m_shaderModules);
			m_shaderBytecodeMap    = p_other.m_shaderBytecodeMap;
			m_descriptorSetLayouts = std::move(p_other.m_descriptorSetLayouts);
			m_reflectionData       = p_other.m_reflectionData;
			m_poolSizes            = p_other.m_poolSizes;
		}

		return *this;
	}

	auto VKShader::getPipelineShaderStageCreateInfoMap() const -> const PipelineCreateInfoMap &
	{
		return m_shaderCreateInfos;
	}

	auto VKShader::getPipelineShaderStageCreateInfos() const -> std::vector<vk::PipelineShaderStageCreateInfo>
	{
		std::vector<vk::PipelineShaderStageCreateInfo> result;
		for (const auto &info: m_shaderCreateInfos | std::views::values)
			result.emplace_back(info);
		return result;
	}

	auto VKShader::getPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits p_stage) const -> const vk::PipelineShaderStageCreateInfo &
	{
		TST_PERMA_ASSERT_MSG(m_shaderCreateInfos.contains(p_stage), "Stage is not present in shader create infos map");
		return m_shaderCreateInfos.at(p_stage);
	}

	auto VKShader::getShaderStageFlags() const -> vk::ShaderStageFlags
	{
		vk::ShaderStageFlags result{0u};
		for (const auto &[stage, code]: m_shaderBytecodeMap)
			result |= stage;
		return result;
	}

	auto VKShader::getShaderBytecodeMap() const -> const BytecodeMap &
	{
		return m_shaderBytecodeMap;
	}

	auto VKShader::getShaderBytecode(vk::ShaderStageFlagBits p_stage) const -> const Bytecode &
	{
		TST_PERMA_ASSERT_MSG(m_shaderBytecodeMap.contains(p_stage), "Stage is not present in bytecode map");
		return m_shaderBytecodeMap.at(p_stage);
	}

	auto VKShader::getDescriptorSetLayouts() const -> std::vector<vk::DescriptorSetLayout>
	{
		std::vector<vk::DescriptorSetLayout> result;
		for (const auto &layout: m_descriptorSetLayouts)
			result.emplace_back(layout);
		return result;
	}

	auto VKShader::getDescriptorSetLayout(uint32 p_set_index) const -> const vk::raii::DescriptorSetLayout &
	{
		TST_PERMA_ASSERT_MSG(p_set_index < m_descriptorSetLayouts.size(), "Set index out of bounds");
		return m_descriptorSetLayouts.at(p_set_index);
	}

	auto VKShader::getReflectedShaderDescriptorSets() const -> const std::vector<DescriptorSet> &
	{
		return m_reflectionData.descriptorSets;
	}

	auto VKShader::getReflectedShaderResources() const -> const std::unordered_map<String, ShaderResource> &
	{
		return m_reflectionData.resources;
	}

	auto VKShader::getReflectedPushConstantRanges() const -> const std::vector<PushConstantRange> &
	{
		return m_reflectionData.pushConstantRanges;
	}

	auto VKShader::getReflectedPushConstantBuffers() const -> const std::unordered_map<String, PushConstantBuffer> &
	{
		return m_reflectionData.pushConstantBuffers;
	}

	auto VKShader::getDescriptorPoolSizes(uint32 p_set_index) const -> const std::vector<vk::DescriptorPoolSize> &
	{
		TST_PERMA_ASSERT_MSG(m_poolSizes.contains(p_set_index), "Set index out of bounds");
		return m_poolSizes.at(p_set_index);
	}

	auto VKShader::_reflect(vk::ShaderStageFlagBits p_stage, Bytecode p_bytecode) -> void
	{
		const Bytecode            copy = {p_bytecode.begin(), p_bytecode.end()};
		spirv_cross::CompilerGLSL compiler{copy};
		auto                      resources{compiler.get_shader_resources()};

		TST_SHADER_LOG_INFO("Shader stage: {}\n", vk::to_string(p_stage));

		TST_SHADER_LOG_INFO("Uniform buffers:");
		for (const auto &resource: resources.uniform_buffers)
		{
			const String &name = resource.name;

			auto & buffer_type = compiler.get_type(resource.base_type_id);
			uint32 size        = compiler.get_declared_struct_size(buffer_type);

			uint32 member_count = buffer_type.member_types.size();

			uint32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 set     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

			if (set >= m_reflectionData.descriptorSets.size())
				m_reflectionData.descriptorSets.resize(set + 1);

			DescriptorSet &descriptorSet = m_reflectionData.descriptorSets[set];

			UniformBuffer &uniform_buffer = descriptorSet.uniformBuffers[binding];
			uniform_buffer.size           = size;
			uniform_buffer.stage          = p_stage;
			uniform_buffer.name           = name;
			uniform_buffer.binding        = binding;

			TST_SHADER_LOG_TRACE("\t{} | Set: {} | Binding: {}", name, set, binding);
			TST_SHADER_LOG_TRACE("\t\tMember count: {}", member_count);
			TST_SHADER_LOG_TRACE("\t\tSize: {}", size);
		}

		TST_SHADER_LOG_INFO("Storage buffers:");
		for (const auto &resource: resources.storage_buffers)
		{
			const String &name = resource.name;

			auto & buffer_type = compiler.get_type(resource.base_type_id);
			uint32 size        = compiler.get_declared_struct_size(buffer_type);

			uint32 member_count = buffer_type.member_types.size();

			uint32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 set     = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

			if (set >= m_reflectionData.descriptorSets.size())
				m_reflectionData.descriptorSets.resize(set + 1);

			DescriptorSet &descriptorSet = m_reflectionData.descriptorSets[set];

			StorageBuffer &uniform_buffer = descriptorSet.storageBuffers[binding];
			uniform_buffer.size           = size;
			uniform_buffer.name           = name;
			uniform_buffer.binding        = binding;

			TST_SHADER_LOG_TRACE("\t{} | Set: {} | Binding: {}", name, set, binding);
			TST_SHADER_LOG_TRACE("\t\tMember count: {}", member_count);
			TST_SHADER_LOG_TRACE("\t\tSize: {}", size);
		}

		TST_SHADER_LOG_INFO("Storage images:");
		for (const auto &resource: resources.storage_images)
		{
			const String &name = resource.name;

			uint32 binding    = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 set        = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			auto & type       = compiler.get_type(resource.type_id);
			uint32 array_size = type.array.size() > 0 ? type.array[0] : 1;
			if (array_size == 0)
				array_size = 1;

			if (set >= m_reflectionData.descriptorSets.size())
				m_reflectionData.descriptorSets.resize(set + 1);

			DescriptorSet &descriptorSet = m_reflectionData.descriptorSets[set];

			ImageSampler &image_sampler = descriptorSet.storageImages[binding];
			image_sampler.stage         = p_stage;
			image_sampler.name          = name;
			image_sampler.binding       = binding;
			image_sampler.arraySize     = array_size;
			image_sampler.dimension     = static_cast<EImageDimension>(type.image.dim);

			LOG_INFO("{}", imageDimensionToString(image_sampler.dimension));

			ShaderResource &image_sampler_resource = m_reflectionData.resources[name];
			image_sampler_resource.name            = name;
			image_sampler_resource.set             = set;
			image_sampler_resource.binding         = binding;
			image_sampler_resource.arraySize       = array_size;

			TST_SHADER_LOG_TRACE("\t{} | Set: {} | Binding: {}", name, set, binding);
			TST_SHADER_LOG_TRACE("\t\tArray size: {}", array_size);
		}

		TST_SHADER_LOG_INFO("Separate images:");
		for (const auto &resource: resources.separate_images)
		{
			const String &name = resource.name;

			uint32 binding    = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 set        = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			auto & type       = compiler.get_type(resource.type_id);
			uint32 array_size = type.array.size() > 0 ? type.array[0] : 1;
			if (array_size == 0)
				array_size = 1;

			if (set >= m_reflectionData.descriptorSets.size())
				m_reflectionData.descriptorSets.resize(set + 1);

			DescriptorSet &descriptorSet = m_reflectionData.descriptorSets[set];

			ImageSampler &image_sampler = descriptorSet.separateImages[binding];
			image_sampler.stage         = p_stage;
			image_sampler.name          = name;
			image_sampler.binding       = binding;
			image_sampler.arraySize     = array_size;
			image_sampler.dimension     = static_cast<EImageDimension>(type.image.dim);

			ShaderResource &image_sampler_resource = m_reflectionData.resources[name];
			image_sampler_resource.name            = name;
			image_sampler_resource.set             = set;
			image_sampler_resource.binding         = binding;
			image_sampler_resource.arraySize       = array_size;

			TST_SHADER_LOG_TRACE("\t{} | Set: {} | Binding: {}", name, set, binding);
			TST_SHADER_LOG_TRACE("\t\tArray size: {}", array_size);
		}

		TST_SHADER_LOG_INFO("Combined image samplers:");
		for (const auto &resource: resources.sampled_images)
		{
			const String &name = resource.name;

			uint32 binding    = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 set        = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			auto & type       = compiler.get_type(resource.type_id);
			uint32 array_size = type.array.size() > 0 ? type.array[0] : 1;
			if (array_size == 0)
				array_size = 1;

			if (set >= m_reflectionData.descriptorSets.size())
				m_reflectionData.descriptorSets.resize(set + 1);

			DescriptorSet &descriptorSet = m_reflectionData.descriptorSets[set];

			ImageSampler &image_sampler = descriptorSet.imageSamplers[binding];
			image_sampler.stage         = p_stage;
			image_sampler.name          = name;
			image_sampler.binding       = binding;
			image_sampler.arraySize     = array_size;
			image_sampler.dimension     = static_cast<EImageDimension>(type.image.dim);

			ShaderResource &image_sampler_resource = m_reflectionData.resources[name];
			image_sampler_resource.name            = name;
			image_sampler_resource.set             = set;
			image_sampler_resource.binding         = binding;
			image_sampler_resource.arraySize       = array_size;

			TST_SHADER_LOG_TRACE("\t{} | Set: {} | Binding: {}", name, set, binding);
			TST_SHADER_LOG_TRACE("\t\tArray size: {}", array_size);
		}

		TST_SHADER_LOG_INFO("Push constant buffers:");
		for (const auto &resource: resources.push_constant_buffers)
		{
			TST_SHADER_LOG_TRACE("Stage: {}", vk::to_string((p_stage)));

			const String &name = resource.name;

			auto & buffer_type  = compiler.get_type(resource.base_type_id);
			uint32 size         = compiler.get_declared_struct_size(buffer_type);
			uint32 member_count = buffer_type.member_types.size();

			uint32 offset{0u};
			if (!m_reflectionData.pushConstantRanges.empty())
				offset = m_reflectionData.pushConstantRanges.back().offset + m_reflectionData.pushConstantRanges.back().size;

			PushConstantRange &push_constant_range{m_reflectionData.pushConstantRanges.emplace_back()};
			push_constant_range.stage  = p_stage;
			push_constant_range.size   = size;
			push_constant_range.offset = offset;

			TST_SHADER_LOG_TRACE("PCR: Name: {} | Size: {} | Offset: {}", name, size, offset);

			if (name.starts_with("_"))
				continue;

			PushConstantBuffer &push_constant_buffer{m_reflectionData.pushConstantBuffers[name]};
			push_constant_buffer.name = name;
			push_constant_buffer.size = size;

			for (uint32 i{0u}; i < member_count; ++i)
			{
				auto &      type{compiler.get_type(buffer_type.member_types[i])};
				const auto &member_name{compiler.get_member_name(buffer_type.self, i)};
				auto        member_size{compiler.get_declared_struct_member_size(buffer_type, i)};
				auto        member_offset{compiler.type_struct_member_offset(buffer_type, i)};

				member_offset -= offset;

				TST_SHADER_LOG_TRACE("Member size: {}", member_size);
				TST_SHADER_LOG_TRACE("Member offset: {}", member_offset);

				push_constant_buffer.pushConstants[fmt::format("{}.{}", name, member_name)] = PushConstant{member_name, static_cast<uint32>(member_size), member_offset};
			}
		}
	}

	auto VKShader::_createDescriptors() -> void
	{
		for (uint32 set{0u}; set < m_reflectionData.descriptorSets.size(); ++set)
		{
			DescriptorSet &descriptor_set = m_reflectionData.descriptorSets.at(set);

			if (!descriptor_set.uniformBuffers.empty())
			{
				vk::DescriptorPoolSize &pool_size = m_poolSizes[set].emplace_back();
				pool_size.type                    = vk::DescriptorType::eUniformBuffer;
				pool_size.descriptorCount         = descriptor_set.uniformBuffers.size();
			}

			if (!descriptor_set.storageBuffers.empty())
			{
				vk::DescriptorPoolSize &pool_size = m_poolSizes[set].emplace_back();
				pool_size.type                    = vk::DescriptorType::eStorageBuffer;
				pool_size.descriptorCount         = descriptor_set.storageBuffers.size();
			}

			if (!descriptor_set.imageSamplers.empty())
			{
				vk::DescriptorPoolSize &pool_size = m_poolSizes[set].emplace_back();
				pool_size.type                    = vk::DescriptorType::eCombinedImageSampler;
				pool_size.descriptorCount         = descriptor_set.imageSamplers.size();
			}

			if (!descriptor_set.storageImages.empty())
			{
				vk::DescriptorPoolSize &pool_size = m_poolSizes[set].emplace_back();
				pool_size.type                    = vk::DescriptorType::eStorageImage;
				pool_size.descriptorCount         = descriptor_set.storageBuffers.size();
			}

			if (!descriptor_set.separateImages.empty())
			{
				vk::DescriptorPoolSize &pool_size = m_poolSizes[set].emplace_back();
				pool_size.type                    = vk::DescriptorType::eSampledImage;
				pool_size.descriptorCount         = descriptor_set.storageBuffers.size();
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

			for (auto &[binding, storage_buffer]: descriptor_set.storageBuffers)
			{
				vk::DescriptorSetLayoutBinding &layout_binding = layout_bindings.emplace_back();
				layout_binding.binding                         = binding;
				layout_binding.descriptorCount                 = 1;
				layout_binding.descriptorType                  = vk::DescriptorType::eStorageBuffer;
				layout_binding.pImmutableSamplers              = nullptr;
				layout_binding.stageFlags                      = vk::ShaderStageFlagBits::eCompute;

				vk::WriteDescriptorSet &write_descriptor = descriptor_set.writeDescriptorSets[storage_buffer.name];
				write_descriptor                         = vk::WriteDescriptorSet{};
				write_descriptor.descriptorCount         = 1;
				write_descriptor.descriptorType          = vk::DescriptorType::eStorageBuffer;
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

			for (auto &[binding, storage_image]: descriptor_set.storageImages)
			{
				vk::DescriptorSetLayoutBinding &layout_binding = layout_bindings.emplace_back();
				layout_binding.binding                         = binding;
				layout_binding.descriptorCount                 = storage_image.arraySize;
				layout_binding.descriptorType                  = vk::DescriptorType::eStorageImage;
				layout_binding.pImmutableSamplers              = nullptr;
				layout_binding.stageFlags                      = storage_image.stage;

				vk::WriteDescriptorSet &write_descriptor = descriptor_set.writeDescriptorSets[storage_image.name];
				write_descriptor                         = vk::WriteDescriptorSet{};
				write_descriptor.descriptorCount         = storage_image.arraySize;
				write_descriptor.descriptorType          = vk::DescriptorType::eStorageImage;
				write_descriptor.dstBinding              = binding;
			}

			for (auto &[binding, separate_image]: descriptor_set.separateImages)
			{
				vk::DescriptorSetLayoutBinding &layout_binding = layout_bindings.emplace_back();
				layout_binding.binding                         = binding;
				layout_binding.descriptorCount                 = separate_image.arraySize;
				layout_binding.descriptorType                  = vk::DescriptorType::eSampledImage;
				layout_binding.pImmutableSamplers              = nullptr;
				layout_binding.stageFlags                      = separate_image.stage;

				vk::WriteDescriptorSet &write_descriptor = descriptor_set.writeDescriptorSets[separate_image.name];
				write_descriptor                         = vk::WriteDescriptorSet{};
				write_descriptor.descriptorCount         = separate_image.arraySize;
				write_descriptor.descriptorType          = vk::DescriptorType::eSampledImage;
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

			m_descriptorSetLayouts[set] = {*m_device, descriptor_set_layout_create_info};
		}
	}
}

#undef TST_SHADER_LOG_TRACE
#undef TST_SHADER_LOG_INFO
