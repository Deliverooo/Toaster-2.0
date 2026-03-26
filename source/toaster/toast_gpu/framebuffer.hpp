#pragma once

#include "toast_lib/ptr.hpp"
#include "toast_lib/system_types.h"

#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	struct FramebufferCreateInfo
	{
		uint32 width;
		uint32 height;
	};

	class IFramebuffer
	{
	public:
		static RefPtr<IFramebuffer> create(const FramebufferCreateInfo &p_framebuffer_create_info);
		virtual                    ~IFramebuffer() = default;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void resize(uint32 p_width, uint32 p_height) = 0;

		virtual void saveImageToFile([[maybe_unused]]const io::filesystem::Path& p_path) const
		{
		}

		[[nodiscard]] virtual uint32 getID() const = 0;
		[[nodiscard]] virtual uint32 getColourAttachmentID() const = 0;
		[[nodiscard]] virtual uint32 getDepthStencilAttachmentID() const = 0;

		[[nodiscard]] virtual const FramebufferCreateInfo &getCreateInfo() const = 0;
	};
}
