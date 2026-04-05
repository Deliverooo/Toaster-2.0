#pragma once

#include "vk_image.hpp"

namespace toaster::gpu
{
	class VKFramebuffer
	{
	public:
		VKFramebuffer(VKGPUContext *p_ctx);

		RefPtr<VKImage2D> getImage(uint32 p_index);

	private:
		VKGPUContext *m_ctx{nullptr};

		std::vector<RefPtr<VKImage2D> > m_attachmentImages{};
	};
}
