#pragma once

#include "gpu_common.hpp"
#include "toast_lib/pool.hpp"
#include "logical_device.hpp"

namespace toaster::gpu
{
	TST_DECLARE_HANDLE(Shader);

	struct TST_GPU_API ShaderData
	{
		vk::ShaderEXT           shader{nullptr};
		vk::ShaderStageFlagBits stage{vk::ShaderStageFlagBits::eAll};
	};

	class TST_GPU_API ShaderManager
	{
		TST_REGISTER_DEPENDENCY(LogicalDevice, Device, device)
	public:
		ShaderManager(LogicalDevice &p_device);
		~ShaderManager();

		ShaderManager(const ShaderManager &)            = delete;
		ShaderManager(ShaderManager &&)                 = delete;
		ShaderManager &operator=(const ShaderManager &) = delete;
		ShaderManager &operator=(ShaderManager &&)      = delete;

		[[nodiscard]] auto createShader(const vk::ShaderCreateInfoEXT &p_create_info) -> ShaderHandle;
		auto               destroyShader(ShaderHandle p_handle) -> void;

		auto getData(ShaderHandle p_handle) const -> const ShaderData * { return m_pool.getData(p_handle); }
		auto getData(ShaderHandle p_handle) -> ShaderData * { return m_pool.getData(p_handle); }

		auto bindShaders(vk::CommandBuffer p_cmd, const InitialiserList<const ShaderHandle> &p_shaders) const -> void;

	private:
		auto _destroyData(ShaderData *p_data) -> void;

		Pool<ShaderTag, ShaderData> m_pool;
	};
}
