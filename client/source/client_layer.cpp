#include "client_layer.hpp"

#include "stb/stb_image.h"
#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "toast_lib/logging.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

#include <imgui.h>
namespace ig = ImGui;

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void ClientLayer::onInit()
	{
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 window_width{swapchain->getExtent().width};
		uint32 window_height{swapchain->getExtent().height};

		m_vertexBufferLayout = {
			{gpu::EShaderDataType::eFloat3, "a_Position"},
			{gpu::EShaderDataType::eFloat3, "a_Colour"},
			{gpu::EShaderDataType::eFloat2, "a_TexCoord"}
		};

		gpu::VKShader::Bytecode    vs_bytecode = io::filesystem::readBinary("shaders/test.vert.glsl.spv");
		gpu::VKShader::Bytecode    ps_bytecode = io::filesystem::readBinary("shaders/test.pixel.glsl.spv");
		gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
		m_shader = make_reference<gpu::VKShader>(ctx, shader_bytecode_map);

		gpu::PipelineCreateInfo pipeline_create_info{};
		pipeline_create_info.vertexBufferLayout = m_vertexBufferLayout;
		pipeline_create_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
		pipeline_create_info.depthFormat        = {swapchain->getDepthFormat()};
		pipeline_create_info.shader             = m_shader;

		m_pipeline = make_reference<gpu::VKPipeline>(ctx, pipeline_create_info);

		gpu::TextureSpecInfo texture_spec_info{};
		m_texture = make_reference<gpu::VKTexture2D>(ctx, texture_spec_info, "../resources/textures/Peeber.png");

		const vk::DeviceSize vertex_buffer_size{sizeof(Vertex) * m_vertices.size()};
		m_vertexBuffer = make_reference<gpu::VKVertexBuffer>(ctx, (void *) m_vertices.data(), vertex_buffer_size);

		const vk::DeviceSize index_buffer_size{sizeof(uint16) * m_indices.size()};
		m_indexBuffer = make_reference<gpu::VKIndexBuffer>(ctx, (void *) m_indices.data(), index_buffer_size);

		_createUniformBuffers();
		_createDescriptorPool();
		_createDescriptorSets();

		m_renderer2D = make_reference<Renderer2D>(ctx, Renderer2DCreateInfo{});
	}

	void ClientLayer::onDestroy()
	{
		for (auto &ubo: m_ubos)
			ubo->unmapMemory();
	}

	void ClientLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index = swapchain->getFrameIndex();

		_recordCommandBuffer(swapchain->getImageIndex());

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4{1.0f}, m_time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
		ubo.view  = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f});
		ubo.proj  = glm::perspective(glm::radians(45.0f), static_cast<float32>(swapchain->getExtent().width) / static_cast<float32>(swapchain->getExtent().height), 0.1f,
									 10.0f);
		ubo.proj[1][1] *= -1.0f;

		std::memcpy(m_mappedUniformBuffers[frame_index], &ubo, sizeof(UniformBufferObject));

		// m_ubCamera->setData(&ubo, sizeof(UniformBufferObject));

		// gpu::RenderPassBeginInfo begin_info{};
		//
		// begin_info.pipeline = m_pipeline;
		// begin_info.width    = static_cast<float32>(swapchain->getExtent().width);
		// begin_info.height   = static_cast<float32>(swapchain->getExtent().height);
		//
		// gpu::VKRenderAttachment colour_attachment{};
		// colour_attachment.type        = gpu::ERenderAttachmentType::eColour;
		// colour_attachment.clearValue  = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		// colour_attachment.targetImage = swapchain->getImageView(swapchain->getImageIndex());
		//
		// gpu::VKRenderAttachment depth_attachment{};
		// colour_attachment.type        = gpu::ERenderAttachmentType::eDepth;
		// colour_attachment.clearValue  = vk::ClearDepthStencilValue{1.0f, 0u};
		// colour_attachment.targetImage = swapchain->getDepthImageView();
		//
		// begin_info.attachments = {colour_attachment, depth_attachment};
		//
		// Renderer::beginRenderPass(swapchain->getCurrentCommandBuffer(), frame_index, begin_info);
		//
		// Renderer::endRenderPass(swapchain->getCurrentCommandBuffer());
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::onKeyPressEvent));
	}

	void ClientLayer::onUIRender()
	{
		ig::Begin("Viewport");
		ig::End();
	}

	bool ClientLayer::onKeyPressEvent(KeyPressEvent &e)
	{
		if (e.getKeyCode() == input::EKeyCode::eEscape)
		{
			getApp().close();
		}

		return false;
	}

	void ClientLayer::_recordCommandBuffer(uint32 p_image_index)
	{
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		auto &command_buffer = swapchain->getCurrentCommandBuffer();

		vk::ClearValue              clear_colour = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};
		vk::RenderingAttachmentInfo colour_attachment_info{};
		colour_attachment_info.clearValue  = clear_colour;
		colour_attachment_info.imageView   = swapchain->getImageView(p_image_index);
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::ClearValue              clear_depth = vk::ClearDepthStencilValue{1.0f, 0u};
		vk::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue  = clear_depth;
		depth_attachment_info.imageView   = swapchain->getDepthImageView();
		depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea           = vk::Rect2D{{0, 0}, swapchain->getExtent()};
		rendering_info.layerCount           = 1;
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments    = &colour_attachment_info;
		rendering_info.pDepthAttachment     = &depth_attachment_info;

		vk::Viewport viewport{};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = static_cast<float32>(swapchain->getExtent().width);
		viewport.height   = static_cast<float32>(swapchain->getExtent().height);

		vk::Rect2D scissor{};
		scissor.offset = vk::Offset2D{0, 0};
		scissor.extent = swapchain->getExtent();

		command_buffer.beginRendering(rendering_info);

		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->getPipeline());

		command_buffer.setViewport(0, viewport);
		command_buffer.setScissor(0, scissor);

		command_buffer.bindVertexBuffers(0, *m_vertexBuffer->getBuffer(), {0});
		command_buffer.bindIndexBuffer(*m_indexBuffer->getBuffer(), 0u, vk::IndexType::eUint16);

		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline->getPipelineLayout(), 0, *m_descriptorSets[swapchain->getFrameIndex()], nullptr);

		command_buffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);

		command_buffer.endRendering();
	}

	void ClientLayer::_createUniformBuffers()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		for (uint32 i{0u}; i < gpu::VKGPUContext::c_maxFramesInFlight; ++i)
		{
			vk::DeviceSize ubo_size{sizeof(UniformBufferObject)};

			auto ubo = make_reference<gpu::VKUniformBuffer>(ctx, ubo_size);
			m_ubos.emplace_back(ubo);

			m_mappedUniformBuffers.emplace_back(ubo->mapMemory(ubo_size, 0));
		}

		// for (uint32 i{0u}; i < gpu::VKGPUContext::c_maxFramesInFlight; ++i)
		// {
		// 	vk::DeviceSize         ubo_size{sizeof(UniformBufferObject)};
		// 	vk::raii::Buffer       ubo{nullptr};
		// 	vk::raii::DeviceMemory ubo_memory{nullptr};
		// 	ctx->createBuffer(ubo_size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		// 					  ubo, ubo_memory);
		// 	m_uniformBuffers.emplace_back(std::move(ubo));
		// 	m_uniformBufferMemories.emplace_back(std::move(ubo_memory));
		// 	m_mappedUniformBuffers.emplace_back(m_uniformBufferMemories[i].mapMemory(0, ubo_size));
		// }
	}

	void ClientLayer::_createDescriptorPool()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		std::array<vk::DescriptorPoolSize, 2> descriptor_pool_sizes{};
		descriptor_pool_sizes[0].descriptorCount = gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_sizes[0].type            = vk::DescriptorType::eUniformBuffer;

		descriptor_pool_sizes[1].descriptorCount = gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_sizes[1].type            = vk::DescriptorType::eCombinedImageSampler;

		vk::DescriptorPoolCreateInfo descriptor_pool_create_info{};
		descriptor_pool_create_info.poolSizeCount = descriptor_pool_sizes.size();
		descriptor_pool_create_info.pPoolSizes    = descriptor_pool_sizes.data();
		descriptor_pool_create_info.maxSets       = gpu::VKGPUContext::c_maxFramesInFlight;

		m_descriptorPool = {ctx->getDevice(), descriptor_pool_create_info};
	}

	void ClientLayer::_createDescriptorSets()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		auto &set_layout = m_shader->getDescriptorSetLayout(0);
		{
			std::vector                   descriptor_set_layouts{gpu::VKGPUContext::c_maxFramesInFlight, *set_layout};
			vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{};
			descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
			descriptor_set_allocate_info.descriptorSetCount = gpu::VKGPUContext::c_maxFramesInFlight;
			descriptor_set_allocate_info.pSetLayouts        = descriptor_set_layouts.data();

			m_descriptorSets = ctx->getDevice().allocateDescriptorSets(descriptor_set_allocate_info);

			for (uint32 i{0u}; i < gpu::VKGPUContext::c_maxFramesInFlight; ++i)
			{
				// vk::DescriptorBufferInfo descriptor_buffer_info{};
				// descriptor_buffer_info.buffer = m_uniformBuffers[i];
				// descriptor_buffer_info.offset = 0;
				// descriptor_buffer_info.range  = sizeof(UniformBufferObject);

				vk::DescriptorImageInfo descriptor_image_info{};
				descriptor_image_info.imageView   = m_texture->getImageView();
				descriptor_image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				descriptor_image_info.sampler     = m_texture->getSampler();

				std::array<vk::WriteDescriptorSet, 2> write_descriptor_sets{};
				write_descriptor_sets[0].descriptorCount = 1;
				write_descriptor_sets[0].descriptorType  = vk::DescriptorType::eUniformBuffer;
				write_descriptor_sets[0].pBufferInfo     = &m_ubos[i]->getDescriptorInfo();
				write_descriptor_sets[0].dstSet          = m_descriptorSets[i];
				write_descriptor_sets[0].dstBinding      = 0;
				write_descriptor_sets[0].dstArrayElement = 0;

				write_descriptor_sets[1].descriptorCount = 1;
				write_descriptor_sets[1].descriptorType  = vk::DescriptorType::eCombinedImageSampler;
				write_descriptor_sets[1].pImageInfo      = &descriptor_image_info;
				write_descriptor_sets[1].dstSet          = m_descriptorSets[i];
				write_descriptor_sets[1].dstBinding      = 1;
				write_descriptor_sets[1].dstArrayElement = 0;

				ctx->getDevice().updateDescriptorSets(write_descriptor_sets, {});
			}
		}
	}
}
