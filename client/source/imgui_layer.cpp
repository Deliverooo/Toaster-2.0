#include "imgui_layer.hpp"

#include <filesystem>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "toaster/toast_kernel/application.hpp"

#include <GLFW/glfw3.h>

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"

namespace toaster
{
	static void checkVKResult(VkResult p_result)
	{
		if (p_result != VK_SUCCESS)
		{
			TST_ASSERT_MSG(false, "Result is not successful");
		}
	}

	ImGuiLayer::ImGuiLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void ImGuiLayer::onInit()
	{
		IMGUI_CHECKVERSION();
		ig::CreateContext();
		ImGuiIO &io = ig::GetIO();

		(void) io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Viewports

		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		vk::DescriptorPoolSize pool_sizes[] = {
			{vk::DescriptorType::eSampler, 500},
			{vk::DescriptorType::eCombinedImageSampler, 500},
			{vk::DescriptorType::eSampledImage, 256},
			{vk::DescriptorType::eStorageImage, 24},
			{vk::DescriptorType::eUniformTexelBuffer, 8},
			{vk::DescriptorType::eStorageTexelBuffer, 8},
			{vk::DescriptorType::eUniformBuffer, 32},
			{vk::DescriptorType::eStorageBuffer, 32},
			{vk::DescriptorType::eUniformBufferDynamic, 16},
			{vk::DescriptorType::eStorageBufferDynamic, 16},
			{vk::DescriptorType::eInputAttachment, 8}
		};

		uint32_t max_sets = 0;
		for (const auto &ps: pool_sizes)
			max_sets += ps.descriptorCount;

		vk::DescriptorPoolCreateInfo descriptor_pool_create_info{};
		descriptor_pool_create_info.pPoolSizes    = pool_sizes;
		descriptor_pool_create_info.poolSizeCount = IM_ARRAYSIZE(pool_sizes);
		descriptor_pool_create_info.maxSets       = max_sets;
		descriptor_pool_create_info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		m_descriptorPool = ctx->getDevice().createDescriptorPool(descriptor_pool_create_info);

		ImGui_ImplGlfw_InitForVulkan(app.getWindow().getNativeWindow(), true);

		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.Instance        = *ctx->getVulkanInstance();
		init_info.PhysicalDevice  = *ctx->getPhysicalDevice();
		init_info.Device          = *ctx->getDevice();
		init_info.QueueFamily     = ctx->getQueueFamilyIndices().graphics;
		init_info.Queue           = *ctx->getGraphicsQueue();
		init_info.PipelineCache   = nullptr;
		init_info.DescriptorPool  = *m_descriptorPool;
		init_info.Allocator       = nullptr;
		init_info.MinImageCount   = swapchain->getMinImageCount();
		init_info.ImageCount      = swapchain->getImageCount();
		init_info.CheckVkResultFn = checkVKResult;

		vk::PipelineRenderingCreateInfo rendering_create_info{};
		rendering_create_info.colorAttachmentCount    = 1;
		vk::Format colour_attachment_format           = swapchain->getSurfaceFormat().format;
		rendering_create_info.pColorAttachmentFormats = &colour_attachment_format;
		rendering_create_info.depthAttachmentFormat   = ctx->findDepthFormat();

		// Dynamic rendering
		init_info.UseDynamicRendering                          = true;
		init_info.PipelineInfoMain.PipelineRenderingCreateInfo = rendering_create_info;

		ImGui_ImplVulkan_Init(&init_info);
	}

	void ImGuiLayer::onDestroy()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ig::DestroyContext();
	}

	void ImGuiLayer::onUpdate(float32 p_dt)
	{
	}

	void ImGuiLayer::onEvent(Event &p_event)
	{
		if (m_blockEvents)
		{
			ImGuiIO &io = ig::GetIO();
			p_event.setHandled(p_event.isHandled() | (p_event.inCategory(EventCategory_Mouse) & io.WantCaptureMouse));
			p_event.setHandled(p_event.isHandled() | (p_event.inCategory(EventCategory_Keyboard) & io.WantCaptureKeyboard));
		}
	}

	void ImGuiLayer::begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ig::NewFrame();
	}

	void ImGuiLayer::end()
	{
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		ig::Render();

		auto &command_buffer = swapchain->getCurrentCommandBuffer();

		vk::ClearValue              clear_colour = vk::ClearColorValue{0.005f, 0.105f, 0.005f, 0.0f};
		vk::RenderingAttachmentInfo colour_attachment_info{};
		colour_attachment_info.clearValue  = clear_colour;
		colour_attachment_info.imageView   = swapchain->getImageView(swapchain->getImageIndex());
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eNone;
		colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::ClearValue              clear_depth = vk::ClearDepthStencilValue{1.0f, 0u};
		vk::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue  = clear_depth;
		depth_attachment_info.imageView   = swapchain->getDepthImageView();
		depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eNone;
		depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eNone;

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

		command_buffer.setViewport(0, viewport);
		command_buffer.setScissor(0, scissor);

		ImDrawData *data = ig::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(data, *command_buffer);

		command_buffer.endRendering();

		ImGuiIO &io = ig::GetIO();
		(void) io;

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ig::UpdatePlatformWindows();
			ig::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiLayer::setBlockEvents(bool p_block)
	{
		m_blockEvents = p_block;
	}
}
