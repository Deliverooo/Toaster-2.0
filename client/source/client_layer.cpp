#include "client_layer.hpp"

#include "stb/stb_image.h"
#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "toast_lib/logging.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void ClientLayer::onInit()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		m_vertexBufferLayout = {
			{gpu::EShaderDataType::eFloat3, "a_Position"},
			{gpu::EShaderDataType::eFloat3, "a_Colour"},
			{gpu::EShaderDataType::eFloat2, "a_TexCoord"}
		};

		_createDescriptorSetLayout();
		_createGraphicsPipeline();

		_createTextureImage();
		_createTextureImageView();
		_createTextureSampler();

		_createVertexBuffer();
		_createIndexBuffer();
		_createUniformBuffers();
		_createDescriptorPool();
		_createDescriptorSets();
	}

	void ClientLayer::onDestroy()
	{
	}

	void ClientLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		_recordCommandBuffer(swapchain->getImageIndex());

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4{1.0f}, m_time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
		ubo.view  = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f});
		ubo.proj  = glm::perspective(glm::radians(45.0f), static_cast<float32>(swapchain->getExtent().width) / static_cast<float32>(swapchain->getExtent().height), 0.1f,
									 10.0f);
		ubo.proj[1][1] *= -1.0f;

		std::memcpy(m_mappedUniformBuffers[swapchain->getFrameIndex()], &ubo, sizeof(UniformBufferObject));
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::onKeyPressEvent));
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

		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphicsPipeline);

		command_buffer.setViewport(0, viewport);
		command_buffer.setScissor(0, scissor);

		command_buffer.bindVertexBuffers(0, *m_vertexBuffer, {0});
		command_buffer.bindIndexBuffer(m_indexBuffer, 0u, vk::IndexType::eUint16);

		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, *m_descriptorSets[swapchain->getFrameIndex()], nullptr);
		command_buffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);

		command_buffer.endRendering();
	}

	vk::Format ClientLayer::_getVulkanAttribType(gpu::EShaderDataType p_type)
	{
		switch (p_type)
		{
			case gpu::EShaderDataType::eFloat: return vk::Format::eR32Sfloat;
			case gpu::EShaderDataType::eFloat2: return vk::Format::eR32G32Sfloat;
			case gpu::EShaderDataType::eFloat3: return vk::Format::eR32G32B32Sfloat;
			case gpu::EShaderDataType::eFloat4: return vk::Format::eR32G32B32A32Sfloat;
			case gpu::EShaderDataType::eInt: return vk::Format::eR32Sint;
			case gpu::EShaderDataType::eInt2: return vk::Format::eR32G32Sint;
			case gpu::EShaderDataType::eInt3: return vk::Format::eR32G32B32Sint;
			case gpu::EShaderDataType::eInt4: return vk::Format::eR32G32B32A32Sint;
			case gpu::EShaderDataType::eBool: return vk::Format::eR32Sint;
			default: return vk::Format::eUndefined;
		}
		TST_ASSERT_MSG(false, "Unsupported shader data type");
		return vk::Format::eUndefined;
	}

	void ClientLayer::_createDescriptorSetLayout()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		std::array<vk::DescriptorSetLayoutBinding, 2> bindings{};

		bindings[0].binding         = 0;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType  = vk::DescriptorType::eUniformBuffer;
		bindings[0].stageFlags      = vk::ShaderStageFlagBits::eVertex;

		bindings[1].binding         = 1;
		bindings[1].descriptorCount = 1;
		bindings[1].descriptorType  = vk::DescriptorType::eCombinedImageSampler;
		bindings[1].stageFlags      = vk::ShaderStageFlagBits::eFragment;

		vk::DescriptorSetLayoutCreateInfo layout_create_info{};
		layout_create_info.bindingCount = bindings.size();
		layout_create_info.pBindings    = bindings.data();

		m_descriptorSetLayout = {ctx->getDevice(), layout_create_info};
	}

	void ClientLayer::_createGraphicsPipeline()
	{
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		vk::raii::ShaderModule vertex_shader_module = ctx->createShaderModule(io::filesystem::readBinary("shaders/test.vert.glsl.spv"));
		vk::raii::ShaderModule pixel_shader_module  = ctx->createShaderModule(io::filesystem::readBinary("shaders/test.pixel.glsl.spv"));

		vk::PipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
		vertex_shader_stage_create_info.stage  = vk::ShaderStageFlagBits::eVertex;
		vertex_shader_stage_create_info.module = vertex_shader_module;
		vertex_shader_stage_create_info.pName  = "main";

		vk::PipelineShaderStageCreateInfo pixel_shader_stage_create_info{};
		pixel_shader_stage_create_info.stage  = vk::ShaderStageFlagBits::eFragment;
		pixel_shader_stage_create_info.module = pixel_shader_module;
		pixel_shader_stage_create_info.pName  = "main";

		vk::PipelineShaderStageCreateInfo shader_stage_create_infos[] = {vertex_shader_stage_create_info, pixel_shader_stage_create_info};

		vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
		vk::VertexInputBindingDescription      vertex_input_binding_description{};
		vertex_input_binding_description.binding                     = 0;
		vertex_input_binding_description.stride                      = m_vertexBufferLayout.getStride();
		vertex_input_binding_description.inputRate                   = vk::VertexInputRate::eVertex;
		vertex_input_state_create_info.pVertexBindingDescriptions    = &vertex_input_binding_description;
		vertex_input_state_create_info.vertexBindingDescriptionCount = 1;

		std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions;
		vertex_input_attribute_descriptions.resize(m_vertexBufferLayout.getElements().size());

		uint32 location{0u};
		for (const auto &element: m_vertexBufferLayout)
		{
			vertex_input_attribute_descriptions[location].binding  = 0;
			vertex_input_attribute_descriptions[location].format   = _getVulkanAttribType(element.type);
			vertex_input_attribute_descriptions[location].location = location;
			vertex_input_attribute_descriptions[location].offset   = element.offset;
			++location;
		}

		vertex_input_state_create_info.pVertexAttributeDescriptions    = vertex_input_attribute_descriptions.data();
		vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32>(vertex_input_attribute_descriptions.size());

		vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
		input_assembly_state_create_info.topology = vk::PrimitiveTopology::eTriangleList;

		vk::PipelineViewportStateCreateInfo viewport_state_create_info{};
		viewport_state_create_info.viewportCount = 1;
		viewport_state_create_info.scissorCount  = 1;

		vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{};
		rasterization_state_create_info.depthClampEnable        = false;
		rasterization_state_create_info.rasterizerDiscardEnable = false;
		rasterization_state_create_info.polygonMode             = vk::PolygonMode::eFill;
		rasterization_state_create_info.cullMode                = vk::CullModeFlagBits::eBack;
		rasterization_state_create_info.frontFace               = vk::FrontFace::eCounterClockwise;
		rasterization_state_create_info.depthBiasEnable         = false;
		rasterization_state_create_info.lineWidth               = 1.0f;

		vk::PipelineColorBlendAttachmentState colour_blend_attachment_state{};
		colour_blend_attachment_state.blendEnable    = false;
		colour_blend_attachment_state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
													   vk::ColorComponentFlagBits::eA;

		vk::PipelineColorBlendStateCreateInfo colour_blend_state_create_info{};
		colour_blend_state_create_info.logicOpEnable   = false;
		colour_blend_state_create_info.logicOp         = vk::LogicOp::eCopy;
		colour_blend_state_create_info.attachmentCount = 1;
		colour_blend_state_create_info.pAttachments    = &colour_blend_attachment_state;

		std::array                         dynamic_states{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{};
		dynamic_state_create_info.pDynamicStates    = dynamic_states.data();
		dynamic_state_create_info.dynamicStateCount = dynamic_states.size();

		vk::PipelineMultisampleStateCreateInfo multisample_state_create_info{};
		multisample_state_create_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisample_state_create_info.sampleShadingEnable  = false;

		vk::PipelineRenderingCreateInfo rendering_create_info{};
		rendering_create_info.colorAttachmentCount    = 1;
		vk::Format colour_attachment_format           = swapchain->getSurfaceFormat().format;
		rendering_create_info.pColorAttachmentFormats = &colour_attachment_format;
		rendering_create_info.depthAttachmentFormat   = ctx->findDepthFormat();

		vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
		depth_stencil_state_create_info.depthTestEnable       = true;
		depth_stencil_state_create_info.depthWriteEnable      = true;
		depth_stencil_state_create_info.depthCompareOp        = vk::CompareOp::eLess;
		depth_stencil_state_create_info.depthBoundsTestEnable = false;
		depth_stencil_state_create_info.stencilTestEnable     = false;

		vk::DescriptorSetLayout      set_layouts[] = {m_descriptorSetLayout};
		vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.setLayoutCount         = 1;
		pipeline_layout_create_info.pSetLayouts            = set_layouts;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		m_pipelineLayout                                   = {ctx->getDevice(), pipeline_layout_create_info};

		vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info{};
		graphics_pipeline_create_info.stageCount          = 2;
		graphics_pipeline_create_info.pStages             = shader_stage_create_infos;
		graphics_pipeline_create_info.pVertexInputState   = &vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
		graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
		graphics_pipeline_create_info.pMultisampleState   = &multisample_state_create_info;
		graphics_pipeline_create_info.pColorBlendState    = &colour_blend_state_create_info;
		graphics_pipeline_create_info.pDynamicState       = &dynamic_state_create_info;
		graphics_pipeline_create_info.pDepthStencilState  = &depth_stencil_state_create_info;
		graphics_pipeline_create_info.layout              = m_pipelineLayout;
		graphics_pipeline_create_info.renderPass          = nullptr;
		graphics_pipeline_create_info.pNext               = &rendering_create_info;

		m_graphicsPipeline = {ctx->getDevice(), nullptr, graphics_pipeline_create_info};
	}

	void ClientLayer::_createTextureImage()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		int32          tex_width;
		int32          tex_height;
		int32          tex_channels;
		auto           pixels     = stbi_load("../resources/textures/Peeber.png", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
		vk::DeviceSize image_size = tex_width * tex_height * 4;

		if (!pixels)
			TST_ASSERT_MSG(false, "failed to load texture image");

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		ctx->createBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
						  staging_buffer, staging_buffer_memory);

		void *data = staging_buffer_memory.mapMemory(0, image_size);
		std::memcpy(data, pixels, image_size);
		staging_buffer_memory.unmapMemory();

		stbi_image_free(pixels);

		ctx->createImage(tex_width, tex_height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
						 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, m_textureImage,
						 m_textureImageMemory);

		ctx->transitionImageLayout(m_textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		ctx->copyBufferToImage(staging_buffer, m_textureImage, tex_width, tex_height);
		ctx->transitionImageLayout(m_textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	void ClientLayer::_createTextureImageView()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		m_textureImageView = ctx->createImageView(m_textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
	}

	void ClientLayer::_createTextureSampler()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		auto props = ctx->getPhysicalDevice().getProperties();

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.magFilter               = vk::Filter::eLinear;
		sampler_create_info.minFilter               = vk::Filter::eLinear;
		sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.mipLodBias              = 0.0f;
		sampler_create_info.anisotropyEnable        = true;
		sampler_create_info.maxAnisotropy           = props.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable           = false;
		sampler_create_info.compareOp               = vk::CompareOp::eAlways;
		sampler_create_info.minLod                  = 0.0f;
		sampler_create_info.maxLod                  = 0.0f;
		sampler_create_info.borderColor             = vk::BorderColor::eFloatCustomEXT;
		sampler_create_info.unnormalizedCoordinates = false;

		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		m_textureImageSampler = {ctx->getDevice(), sampler_create_info};
	}

	void ClientLayer::_createVertexBuffer()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		const vk::DeviceSize vertex_buffer_size{sizeof(Vertex) * m_vertices.size()};

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		ctx->createBuffer(vertex_buffer_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
						  staging_buffer, staging_buffer_memory);

		void *data = staging_buffer_memory.mapMemory(0, vertex_buffer_size);
		std::memcpy(data, m_vertices.data(), vertex_buffer_size);
		staging_buffer_memory.unmapMemory();

		ctx->createBuffer(vertex_buffer_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal,
						  m_vertexBuffer, m_vertexBufferMemory);

		ctx->copyBuffer(staging_buffer, m_vertexBuffer, vertex_buffer_size);
	}

	void ClientLayer::_createIndexBuffer()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		const vk::DeviceSize index_buffer_size{sizeof(uint16) * m_indices.size()};

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		ctx->createBuffer(index_buffer_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
						  staging_buffer, staging_buffer_memory);

		void *data = staging_buffer_memory.mapMemory(0, index_buffer_size);
		std::memcpy(data, m_indices.data(), index_buffer_size);
		staging_buffer_memory.unmapMemory();

		ctx->createBuffer(index_buffer_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal,
						  m_indexBuffer, m_indexBufferMemory);

		ctx->copyBuffer(staging_buffer, m_indexBuffer, index_buffer_size);
	}

	void ClientLayer::_createUniformBuffers()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		for (uint32 i{0u}; i < gpu::VKGPUContext::c_maxFramesInFlight; ++i)
		{
			vk::DeviceSize         ubo_size{sizeof(UniformBufferObject)};
			vk::raii::Buffer       ubo{nullptr};
			vk::raii::DeviceMemory ubo_memory{nullptr};
			ctx->createBuffer(ubo_size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							  ubo, ubo_memory);
			m_uniformBuffers.emplace_back(std::move(ubo));
			m_uniformBufferMemories.emplace_back(std::move(ubo_memory));
			m_mappedUniformBuffers.emplace_back(m_uniformBufferMemories[i].mapMemory(0, ubo_size));
		}
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

		{
			std::vector                   descriptor_set_layouts{gpu::VKGPUContext::c_maxFramesInFlight, *m_descriptorSetLayout};
			vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{};
			descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
			descriptor_set_allocate_info.descriptorSetCount = gpu::VKGPUContext::c_maxFramesInFlight;
			descriptor_set_allocate_info.pSetLayouts        = descriptor_set_layouts.data();

			m_descriptorSets = ctx->getDevice().allocateDescriptorSets(descriptor_set_allocate_info);

			for (uint32 i{0u}; i < gpu::VKGPUContext::c_maxFramesInFlight; ++i)
			{
				vk::DescriptorBufferInfo descriptor_buffer_info{};
				descriptor_buffer_info.buffer = m_uniformBuffers[i];
				descriptor_buffer_info.offset = 0;
				descriptor_buffer_info.range  = sizeof(UniformBufferObject);

				vk::DescriptorImageInfo descriptor_image_info{};
				descriptor_image_info.imageView   = m_textureImageView;
				descriptor_image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				descriptor_image_info.sampler     = m_textureImageSampler;

				std::array<vk::WriteDescriptorSet, 2> write_descriptor_sets{};
				write_descriptor_sets[0].descriptorCount = 1;
				write_descriptor_sets[0].descriptorType  = vk::DescriptorType::eUniformBuffer;
				write_descriptor_sets[0].pBufferInfo     = &descriptor_buffer_info;
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
