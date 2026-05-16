#pragma once

#include "system_types.h"

#include <new>

namespace toaster
{
	class ArenaAllocator
	{
	public:
		ArenaAllocator(uint64 p_size)
		{
			m_buffer = new uint8[p_size];
			m_ptr    = m_buffer;
		}

		~ArenaAllocator()
		{
			delete[] m_buffer;
		}

		auto alloc(uint64 p_size) -> void *
		{
			void *ptr{m_ptr};
			m_ptr += p_size;
			return ptr;
		}

		auto reset() -> void
		{
			m_ptr = m_buffer;
		}

		auto getBuffer() -> void * { return m_ptr; }

	private:
		uint8 *m_buffer{nullptr};
		uint8 *m_ptr{nullptr};

		friend class CommandQueue;
	};

	class CommandQueue
	{
	public:
		using CommandFunc = void(*)(void *);

		CommandQueue(const uint64 p_size = 10485760u /*10 MB*/) : m_allocator(p_size)
		{
		}

		template<typename TFunc> requires std::is_trivially_destructible_v<TFunc> && std::invocable<TFunc>
		auto enqueue(TFunc &&p_func) -> void
		{
			auto cmd{
				+[](void *p_ptr) -> void
				{
					auto func{(TFunc *) p_ptr};
					(*func)();

					func->~TFunc();
				}
			};
			*reinterpret_cast<CommandFunc *>(m_allocator.m_ptr) = cmd;
			m_allocator.m_ptr                                   += sizeof(CommandFunc);
			*reinterpret_cast<uint32 *>(m_allocator.m_ptr)      = sizeof(p_func);
			m_allocator.m_ptr                                   += sizeof(uint32);

			void *memory{m_allocator.alloc(sizeof(p_func))};

			++m_commandCount;

			new(memory) TFunc(std::forward<TFunc>(p_func));
		}

		auto execute() -> void
		{
			uint8 *buffer{m_allocator.m_buffer};
			for (uint32 i{0u}; i < m_commandCount; ++i)
			{
				CommandFunc func{*static_cast<CommandFunc *>(static_cast<void *>(buffer))};
				buffer += sizeof(CommandFunc);

				uint32 size{*(uint32 *) buffer};
				buffer += sizeof(uint32);
				func(buffer);
				buffer += size;
			}
			m_commandCount = 0u;
			m_allocator.reset();
		}

	private:
		ArenaAllocator m_allocator;
		uint32         m_commandCount{0u};
	};
}
