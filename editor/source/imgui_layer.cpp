#include "imgui_layer.hpp"

#include <filesystem>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "toaster/toast_kernel/application.hpp"

#include <GLFW/glfw3.h>

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"
#include "ui/colours.hpp"

#include <ImGuizmo.h>
namespace igz = ImGuizmo;

namespace toaster
{
	static auto checkVKResult(VkResult p_result) -> void
	{
		if (p_result != VK_SUCCESS)
		{
			TST_ASSERT_MSG(false, "Result is not successful");
		}
	}

	ImGuiLayer::ImGuiLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	auto ImGuiLayer::onInit() -> void
	{
		IMGUI_CHECKVERSION();
		ig::CreateContext();
		ImGuiIO &io{ig::GetIO()};
		(void) io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		// io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Viewports

		io.FontDefault = io.Fonts->AddFontFromFileTTF("../resources/fonts/Noto_Sans_JP/static/NotoSansJP-Regular.ttf", 0, nullptr, io.Fonts->GetGlyphRangesJapanese());
		io.Fonts->AddFontFromFileTTF("../resources/fonts/Noto_Sans_JP/static/NotoSansJP-Bold.ttf", 0, nullptr, io.Fonts->GetGlyphRangesJapanese());

		auto &style{ImGui::GetStyle()};
		auto &colours{ImGui::GetStyle().Colors};

		// Headers
		colours[ImGuiCol_Header]        = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::groupHeader);
		colours[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::groupHeader);
		colours[ImGuiCol_HeaderActive]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::groupHeader);

		// Buttons
		colours[ImGuiCol_Button]        = ImColor(56, 56, 56, 200);
		colours[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
		colours[ImGuiCol_ButtonActive]  = ImColor(56, 56, 56, 150);

		// Frame BG
		colours[ImGuiCol_FrameBg]        = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::propertyField);
		colours[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::propertyField);
		colours[ImGuiCol_FrameBgActive]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::propertyField);

		// Tabs
		colours[ImGuiCol_Tab]                = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_TabHovered]         = ImColor(255, 225, 135, 30);
		colours[ImGuiCol_TabActive]          = ImColor(255, 225, 135, 60);
		colours[ImGuiCol_TabUnfocused]       = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_TabUnfocusedActive] = colours[ImGuiCol_TabHovered];

		// Title
		colours[ImGuiCol_TitleBg]          = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_TitleBgActive]    = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Resize Grip
		colours[ImGuiCol_ResizeGrip]        = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colours[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colours[ImGuiCol_ResizeGripActive]  = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Scrollbar
		colours[ImGuiCol_ScrollbarBg]          = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colours[ImGuiCol_ScrollbarGrab]        = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colours[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colours[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

		// Check Mark
		colours[ImGuiCol_CheckMark] = ImColor(200, 200, 200, 255);

		// Slider
		colours[ImGuiCol_SliderGrab]       = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colours[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

		// Text
		colours[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::text);

		// Checkbox
		colours[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::text);

		// Separator
		colours[ImGuiCol_Separator]        = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::backgroundDark);
		colours[ImGuiCol_SeparatorActive]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::highlight);
		colours[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);

		// Window Background
		colours[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_ChildBg]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::background);
		colours[ImGuiCol_PopupBg]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::backgroundPopup);
		colours[ImGuiCol_Border]   = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::backgroundDark);

		// Tables
		colours[ImGuiCol_TableHeaderBg]    = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::groupHeader);
		colours[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::backgroundDark);

		// Menubar
		colours[ImGuiCol_MenuBarBg] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};

		/// Style
		style.FrameRounding   = 2.5f;
		style.FrameBorderSize = 1.0f;
		style.IndentSpacing   = 11.0f;

		const auto &app{getApp()};
		auto        device{app.getWindow().getLogicalDevice()};
		const auto  swapchain{app.getWindow().getSwapchain()};

		vk::DescriptorPoolSize pool_sizes[] = {
			{vk::DescriptorType::eSampler, 1000},
			{vk::DescriptorType::eCombinedImageSampler, 1000},
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

		m_descriptorPool = device->getVulkanLogicalDevice().createDescriptorPool(descriptor_pool_create_info);

		ImGui_ImplGlfw_InitForVulkan(app.getWindow().getNativeWindow(), true);

		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.Instance        = static_cast<vk::Instance>(static_cast<vk::raii::Instance &>(*device->getPhysicalDevice()->getInstance()));
		init_info.PhysicalDevice  = *device->getPhysicalDevice()->getVulkanPhysicalDevice();
		init_info.Device          = static_cast<vk::Device>(static_cast<vk::raii::Device &>(*device));
		init_info.QueueFamily     = device->getQueueFamilyIndices().graphics;
		init_info.Queue           = *device->getGraphicsQueue();
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
		rendering_create_info.depthAttachmentFormat   = device->getPhysicalDevice()->getDepthFormat();

		// Dynamic rendering
		init_info.UseDynamicRendering                          = true;
		init_info.PipelineInfoMain.PipelineRenderingCreateInfo = rendering_create_info;

		ImGui_ImplVulkan_Init(&init_info);
	}

	auto ImGuiLayer::onDestroy() -> void
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ig::DestroyContext();
	}

	auto ImGuiLayer::onUpdate(float32 p_dt) -> void
	{
	}

	auto ImGuiLayer::onEvent(Event &p_event) -> void
	{
		if (m_blockEvents)
		{
			ImGuiIO &io = ig::GetIO();
			(void) io;
			p_event.setHandled(p_event.isHandled() | (p_event.inCategory(EventCategory_Mouse) & io.WantCaptureMouse));
			p_event.setHandled(p_event.isHandled() | (p_event.inCategory(EventCategory_Keyboard) & io.WantCaptureKeyboard));
		}
	}

	auto ImGuiLayer::begin() -> void
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ig::NewFrame();
		igz::BeginFrame();
	}

	auto ImGuiLayer::end() -> void
	{
		const auto &app{getApp()};
		const auto  swapchain{app.getWindow().getSwapchain()};

		ig::Render();

		auto &command_buffer{swapchain->getCurrentCommandBuffer()};

		vk::RenderingAttachmentInfo colour_attachment_info{};
		colour_attachment_info.clearValue  = vk::ClearColorValue{0.005f, 0.105f, 0.005f, 1.0f};
		colour_attachment_info.imageView   = swapchain->getCurrentImageView();
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue  = vk::ClearDepthStencilValue{1.0f, 0u};
		depth_attachment_info.imageView   = swapchain->getDepthImageView();
		depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eNone;

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea           = vk::Rect2D{{0, 0}, swapchain->getExtent()};
		rendering_info.layerCount           = 1;
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments    = &colour_attachment_info;
		rendering_info.pDepthAttachment     = &depth_attachment_info;

		const vk::Viewport viewport{0.0f, 0.0f, static_cast<float32>(swapchain->getExtent().width), static_cast<float32>(swapchain->getExtent().height), 0.0f, 1.0f};
		const vk::Rect2D   scissor{vk::Offset2D{0, 0}, swapchain->getExtent()};

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

	auto ImGuiLayer::setBlockEvents(bool p_block) -> void
	{
		m_blockEvents = p_block;
	}
}
