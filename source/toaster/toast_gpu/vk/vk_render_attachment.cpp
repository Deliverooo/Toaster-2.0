#include "vk_render_attachment.hpp"

namespace toaster::gpu
{
	VKRenderAttachment::VKRenderAttachment(VKGPUContext *p_ctx, const RenderAttachmentSpecInfo &p_spec_info) : m_ctx(p_ctx), m_specInfo(p_spec_info)
	{
	}
}
