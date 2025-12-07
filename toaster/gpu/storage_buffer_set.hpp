#pragma once

#include "storage_buffer.hpp"
#include <map>

namespace tst
{
	class StorageBufferSet : public RefCounted
	{
	public:
		void resize(uint32_t newSize);

		RefPtr<StorageBuffer> get();
		RefPtr<StorageBuffer> get(uint32_t frame);
		RefPtr<StorageBuffer> _get();

		void set(RefPtr<StorageBuffer> storageBuffer, uint32_t frame = 0);

		StorageBufferSet(const StorageBufferSpecInfo &specification, uint32_t size, uint32_t framesInFlight);
		virtual ~StorageBufferSet() = default;

	private:
		StorageBufferSpecInfo                 m_specification;
		uint32_t                                   m_FramesInFlight = 0;
		std::map<uint32_t, RefPtr<StorageBuffer> > m_StorageBuffers;
	};
}
