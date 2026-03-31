#pragma once

#include "toast_lib/ptr.hpp"
#include "toast_lib/system_types.h"

#include "toast_lib/io/filesystem.hpp"

#include "image.hpp"

namespace toaster::gpu
{
	struct FramebufferTextureCreateInfo
	{
		FramebufferTextureCreateInfo() = default;

		FramebufferTextureCreateInfo(EImageFormat p_format) : format(p_format)
		{
		}

		EImageFormat format;
	};

	struct FramebufferAttachmentsCreateInfo
	{
		FramebufferAttachmentsCreateInfo() = default;

		FramebufferAttachmentsCreateInfo(const std::initializer_list<FramebufferTextureCreateInfo> &p_attachments) : attachments(p_attachments)
		{
		}

		std::vector<FramebufferTextureCreateInfo> attachments;
	};

	struct FramebufferCreateInfo
	{
		FramebufferAttachmentsCreateInfo attachments;

		uint32 width{0u};
		uint32 height{0u};

		uint32 samples{1u}; // Invalid if < 1
	};

	class IFramebuffer
	{
	public:
		static RefPtr<IFramebuffer> create(const FramebufferCreateInfo &p_framebuffer_create_info);
		virtual                     ~IFramebuffer() = default;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void resize(uint32 p_width, uint32 p_height) = 0;

		[[nodiscard]] virtual uint32 getID() const = 0;
		[[nodiscard]] virtual uint32 getColourAttachmentID(uint32 p_attachment_index = 0) const = 0;
		[[nodiscard]] virtual uint32 getDepthStencilAttachmentID() const = 0;

		[[nodiscard]] virtual const FramebufferCreateInfo &getCreateInfo() const = 0;

		virtual int32 readPixel(uint32 p_attachment_index, int32 p_x, int32 p_y) = 0;
	};
}
