#include "shader_reflection.hpp"

#include <spirv_cross/spirv_glsl.hpp>

#include "logging.hpp"

namespace toaster::gpu::reflection
{
	void reflectShaderStage(nvrhi::ShaderType p_stage, const ShaderBytecode p_shader_binary, ReflectionData &p_out_reflection_data)
	{
		std::vector<uint32>   spv_code = {p_shader_binary.begin(), p_shader_binary.end()};
		spirv_cross::Compiler compiler{spv_code};
		auto                  resources = compiler.get_shader_resources();

		for (auto ubo: resources.uniform_buffers)
		{
			auto active_buffers = compiler.get_active_buffer_ranges(ubo.id);

			if (!active_buffers.empty())
			{
				const auto &ubo_name              = ubo.name;
				auto &      ubo_type              = compiler.get_type(ubo.base_type_id);
				int32       member_count          = ubo_type.member_types.size();
				uint32      binding_index         = compiler.get_decoration(ubo.id, spv::DecorationBinding);
				uint32      parent_descriptor_set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
				uint32      size                  = compiler.get_declared_struct_size(ubo_type);

				if (parent_descriptor_set >= p_out_reflection_data.descriptorSets.size())
				{
					p_out_reflection_data.descriptorSets.resize(parent_descriptor_set + 1);
				}

				DescriptorSet &descriptor_set = p_out_reflection_data.descriptorSets[parent_descriptor_set];

				UniformBuffer uniform_buffer{};
				uniform_buffer.binding     = binding_index;
				uniform_buffer.size        = size;
				uniform_buffer.name        = ubo_name;
				uniform_buffer.shaderStage = nvrhi::ShaderType::All;

				descriptor_set.uniformBuffers[binding_index] = uniform_buffer;

				LOG_INFO("{}, ({} : {})", ubo_name, parent_descriptor_set, binding_index);
				LOG_INFO("{}", member_count);
				LOG_INFO("{}", size);
			}
		}
	}
}
