#pragma once

#include "toaster/toast_kernel/layer.hpp"

#include <imgui.h>
namespace ig = ImGui;

#include <vulkan/vulkan_raii.hpp>

namespace toaster
{
	class ImGuiLayer final : public IAppLayer
	{
	public:
		explicit ImGuiLayer(Application *p_app);
		virtual  ~ImGuiLayer() override = default;

		virtual auto onInit() -> void override;
		virtual auto onDestroy() -> void override;

		virtual auto onUpdate(float32 p_dt) -> void override;
		virtual auto onEvent(Event &p_event) -> void override;

		auto begin() -> void;
		auto end() -> void;

		auto setBlockEvents(bool p_block) -> void;
		auto getDescriptorPool() -> vk::raii::DescriptorPool &;

	private:
		bool m_blockEvents{false};

		vk::raii::DescriptorPool m_descriptorPool{nullptr};
	};
}
