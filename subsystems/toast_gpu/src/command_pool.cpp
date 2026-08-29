#include "toast_gpu/command_pool.hpp"

#include "toast_gpu/command_list.hpp"

namespace toaster::gpu
{
	CommandPool::CommandPool(Device &p_device, vk::CommandPool p_pool, ECommandPoolFlags p_flags) : m_device(&p_device),
	m_commandPool(p_pool), m_commandPoolFlags(p_flags)
	{
		TST_ASSERT(m_commandPool);
	}

	CommandPool::~CommandPool()
	{
		_destroy();
	}

	CommandPool::CommandPool(CommandPool &&p_other) noexcept
	: m_device(p_other.m_device), m_commandPool(p_other.m_commandPool),
													 m_commandPoolFlags(p_other.m_commandPoolFlags)
	{
		p_other.m_device = nullptr;
		p_other.m_commandPool = nullptr;
		p_other.m_commandPoolFlags = ECommandPoolFlags(0u);
	}

	CommandPool &CommandPool::operator=(CommandPool &&p_other) noexcept
	{
		if (this != &p_other)
		{
			_destroy();

			m_device = p_other.m_device;
			m_commandPool = p_other.m_commandPool;
			m_commandPoolFlags = p_other.m_commandPoolFlags;

			p_other.m_device = nullptr;
			p_other.m_commandPool = nullptr;
			p_other.m_commandPoolFlags = static_cast<ECommandPoolFlags>(0u);
		}
		return *this;
	}

	auto CommandPool::reset() -> void
	{
		TST_ASSERT(m_device && m_commandPool);
		m_device->getDevice().getDevice().resetCommandPool(m_commandPool);
	}

	auto CommandPool::createCommandList() -> CommandList
	{
		TST_ASSERT(m_device && m_commandPool);

		vk::CommandBufferAllocateInfo cmd_alloc_info{};
		cmd_alloc_info.commandPool        = m_commandPool;
		cmd_alloc_info.commandBufferCount = 1u;
		cmd_alloc_info.level              = vk::CommandBufferLevel::ePrimary;

		vk::CommandBuffer cmd{m_device->getDevice().getDevice().allocateCommandBuffers(cmd_alloc_info).front()};

		return {*this,*m_device, cmd};
	}

	auto CommandPool::createCommandLists(uint32 p_count) -> std::vector<CommandList>
	{
		TST_ASSERT(m_device && m_commandPool);

		vk::CommandBufferAllocateInfo cmd_alloc_info{};
		cmd_alloc_info.commandPool        = m_commandPool;
		cmd_alloc_info.commandBufferCount = p_count;
		cmd_alloc_info.level              = vk::CommandBufferLevel::ePrimary;

		const std::vector<vk::CommandBuffer> command_buffers{m_device->getDevice().getDevice().allocateCommandBuffers(cmd_alloc_info)};

		std::vector<CommandList> out_lists;
		for (const auto cmd: command_buffers)
			out_lists.emplace_back(*this,*m_device, cmd);
		return out_lists;
	}

	auto CommandPool::_destroy() -> void
	{
		if (m_device && m_commandPool)
		{
			m_device->getDevice().getDevice().destroyCommandPool(m_commandPool);
			m_commandPool = nullptr;
		}
	}
}
