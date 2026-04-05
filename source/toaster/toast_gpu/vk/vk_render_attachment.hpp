#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	struct VKRenderAttachment
	{
		vk::ClearValue clearValue{};
		vk::Format     format{vk::Format::eUndefined};
	};
}
