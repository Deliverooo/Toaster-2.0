#include "renderer.hpp"

#include "globals.hpp"
#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_renderer.hpp"

namespace toaster::render
{
	auto createEnvironmentMap(gpu::VKLogicalDevice *p_device, const Globals *p_globals, const io::filesystem::Path &p_path) -> RefPtr<gpu::VKTexture3D>
	{
		RefPtr<gpu::VKTexture2D> env_tex{nullptr};
		RefPtr<gpu::VKTexture3D> env_map{nullptr};

		gpu::TextureSpecInfo skybox_texture_spec_info{};
		env_tex = p_device->alloc<gpu::VKTexture2D>(skybox_texture_spec_info, p_path);

		constexpr uint32     skybox_resolution{2048};
		gpu::TextureSpecInfo skybox_texture_map_spec_info{};
		skybox_texture_map_spec_info.width  = skybox_resolution;
		skybox_texture_map_spec_info.height = skybox_resolution;
		skybox_texture_map_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
		env_map                             = p_device->alloc<gpu::VKTexture3D>(skybox_texture_map_spec_info);

		auto equirectangular_to_cubemap_pipeline{p_device->alloc<gpu::VKComputePipeline>(p_globals->shaderLibrary().get("Equirectangular_To_CubeMap"))};
		auto equirectangular_to_cubemap_pass{p_device->alloc<gpu::VKComputePass>(equirectangular_to_cubemap_pipeline)};
		equirectangular_to_cubemap_pass->setInput("u_EquirectangularMap", env_tex);
		equirectangular_to_cubemap_pass->setInput("o_Cubemap", env_map);
		equirectangular_to_cubemap_pass->bake();

		auto command_buffer{p_device->alloc<gpu::VKCommandBuffer>(vk::QueueFlagBits::eCompute)};
		command_buffer->begin();
		gpu::render::beginCompute(command_buffer->getVulkanCommandBuffer(), 0, equirectangular_to_cubemap_pass);
		gpu::render::endCompute(command_buffer->getVulkanCommandBuffer(), 0, equirectangular_to_cubemap_pass);
		gpu::render::dispatchCompute(command_buffer->getVulkanCommandBuffer(), 0, equirectangular_to_cubemap_pass, nullptr, skybox_resolution / 32,
									 skybox_resolution / 32, 6);
		command_buffer->end();
		command_buffer->submit();
		command_buffer->waitForFence();

		return env_map;
	}

	auto renderFullscreenQuad( const Globals *p_globals, const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKPipeline> &p_pipeline,
							  const RefPtr<gpu::VKMaterial> &p_material) -> void
	{
		TST_ASSERT_MSG(*p_command_buffer, "Command buffer is null");

		if (p_material) // You technically don't need to use a material if you don't want to
		{
			if (p_material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{p_material->getDescriptorSet(p_frame_index)};
				p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set, {});
			}

			const auto &push_constants{p_material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_pipeline->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eFragment;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = 0u;
				push_constants_info.pValues    = push_constants.data();
				p_command_buffer.pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_globals->fullscreenQuadVertexBuffer()->bind(p_command_buffer);
		p_globals->fullscreenQuadIndexBuffer()->bind(p_command_buffer, vk::IndexType::eUint32);

		// Finally, draw indexed :)
		p_command_buffer.drawIndexed(p_globals->fullscreenQuadIndices().size(), 1, 0, 0, 0);
	}
}
