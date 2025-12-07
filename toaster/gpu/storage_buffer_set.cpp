#include "storage_buffer_set.hpp"

#include "renderer/renderer.hpp"

namespace tst
{
	StorageBufferSet::StorageBufferSet(const StorageBufferSpecInfo &specification, uint32_t size, uint32_t framesInFlight)
		: m_specification(specification), m_FramesInFlight(framesInFlight)
	{
		if (framesInFlight == 0)
			m_FramesInFlight = Renderer::getConfigInfo().maxFramesInFlight;

		for (uint32_t frame         = 0; frame < m_FramesInFlight; frame++)
			m_StorageBuffers[frame] = make_reference<StorageBuffer>(size, specification);
	}

	void StorageBufferSet::resize(uint32_t newSize)
	{
		for (uint32_t frame = 0; frame < m_FramesInFlight; frame++)
			m_StorageBuffers.at(frame)->resize(newSize);
	}

	RefPtr<StorageBuffer> StorageBufferSet::get()
	{
		uint32_t frame = Renderer::getCurrentFrameIndex();
		return get(frame);
	}

	RefPtr<StorageBuffer> StorageBufferSet::_get()
	{
		uint32_t frame = Renderer::_getCurrentFrameIndex();
		return get(frame);
	}

	RefPtr<StorageBuffer> StorageBufferSet::get(uint32_t frame)
	{
		TST_ASSERT(m_StorageBuffers.find(frame) != m_StorageBuffers.end());
		return m_StorageBuffers.at(frame);
	}

	void StorageBufferSet::set(RefPtr<StorageBuffer> storageBuffer, uint32_t frame)
	{
		m_StorageBuffers[frame] = storageBuffer;
	}
}
