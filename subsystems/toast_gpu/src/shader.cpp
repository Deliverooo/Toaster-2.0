#include "toast_gpu/shader.hpp"

#if !defined(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
#error "fjakfjkljflk"
#endif
namespace toaster::gpu
{
	ShaderManager::ShaderManager(LogicalDevice &p_device) : m_device(&p_device)
	{
		m_pool.setUserData(this);
		m_pool.setDestroyCallback(+[](void *p_user_data, ShaderHandle p_handle) -> void
		{
			auto        ts{static_cast<ShaderManager *>(p_user_data)};
			ShaderData *shader_data{&ts->m_pool._data[p_handle.id]};

			ts->_destroyData(shader_data);
		});
	}

	ShaderManager::~ShaderManager()
	{
		// For safety...
		for (uint32 i{0u}; i < m_pool.getSize(); ++i)
		{
			if (m_pool._alive[i])
				_destroyData(&m_pool._data[i]);
		}
	}

	auto ShaderManager::createShader(const vk::ShaderCreateInfoEXT &p_create_info) -> ShaderHandle
	{
		ShaderData out_data{};
		const auto res{m_device->getDevice().createShaderEXT(p_create_info, nullptr, FunctionDispatcher::get())};
		if (res.result != vk::Result::eSuccess)
			TST_PERMA_ASSERT(false);
		out_data.shader = res.value;
		out_data.stage  = p_create_info.stage;

		return m_pool.create(out_data);
	}

	auto ShaderManager::destroyShader(ShaderHandle p_handle) -> void
	{
		m_pool.destroy(p_handle);
	}

	auto ShaderManager::bindShaders(vk::CommandBuffer p_cmd, const InitialiserList<const ShaderHandle> &p_shaders) const -> void
	{
		std::vector<vk::ShaderEXT>           shaders(p_shaders.size());
		std::vector<vk::ShaderStageFlagBits> shader_stages(p_shaders.size());

		for (uint32 i{0u}; i < p_shaders.size(); ++i)
		{
			const ShaderData *data{getData(p_shaders.data()[i])};
			shaders[i]       = data->shader;
			shader_stages[i] = data->stage;
		}

		p_cmd.bindShadersEXT(shader_stages, shaders, FunctionDispatcher::get());
	}

	auto ShaderManager::_destroyData(ShaderData *p_data) -> void
	{
		if (p_data->shader)
		{
			m_device->getDevice().destroyShaderEXT(p_data->shader, nullptr, FunctionDispatcher::get());
			p_data->shader = nullptr;
		}
	}
}
