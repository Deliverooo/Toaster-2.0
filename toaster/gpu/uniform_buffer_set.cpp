#include "uniform_buffer_set.hpp"

#include "renderer/renderer.hpp"

namespace tst
{
	UniformBufferSet::UniformBufferSet(uint32_t size, uint32_t framesInFlight)
		: m_FramesInFlight(framesInFlight)
	{
		if (framesInFlight == 0)
			m_FramesInFlight = Renderer::getConfigInfo().maxFramesInFlight;

		for (uint32_t frame         = 0; frame < m_FramesInFlight; frame++)
			m_UniformBuffers[frame] = make_reference<UniformBuffer>(size);
	}

	RefPtr<UniformBuffer> UniformBufferSet::Get()
	{
		uint32_t frame = Renderer::getCurrentFrameIndex();
		return Get(frame);
	}

	RefPtr<UniformBuffer> UniformBufferSet::RT_Get()
	{
		uint32_t frame = Renderer::_getCurrentFrameIndex();
		return Get(frame);
	}

	RefPtr<UniformBuffer> UniformBufferSet::Get(uint32_t frame)
	{
		TST_ASSERT(m_UniformBuffers.find(frame) != m_UniformBuffers.end());
		return m_UniformBuffers.at(frame);
	}

	void UniformBufferSet::Set(RefPtr<UniformBuffer> uniformBuffer, uint32_t frame)
	{
		m_UniformBuffers[frame] = uniformBuffer;
	}
}
