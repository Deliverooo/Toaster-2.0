#pragma once

#include "uniform_buffer.hpp"
#include <map>

namespace tst
{
	class UniformBufferSet : public RefCounted
	{
	public:
		RefPtr<UniformBuffer> Get();
		RefPtr<UniformBuffer> RT_Get();

		RefPtr<UniformBuffer> Get(uint32_t frame);

		void Set(RefPtr<UniformBuffer> uniformBuffer, uint32_t frame = 0);

		UniformBufferSet(uint32_t size, uint32_t framesInFlight = 0);
		virtual ~UniformBufferSet() = default;

	private:
		uint32_t                                   m_FramesInFlight = 0;
		std::map<uint32_t, RefPtr<UniformBuffer> > m_UniformBuffers;
	};
}
