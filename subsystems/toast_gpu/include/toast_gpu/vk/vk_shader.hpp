#pragma once

#include "../toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	using ShaderBytecode = std::vector<uint32>;

	class TST_GPU_API VKShader
	{
		TST_GPU_OBJECT
	public:
		VKShader() = default;
		auto operator=(VKShader &&p_other) noexcept -> VKShader &;
		VKShader(VKGPUContext &          p_gpu_ctx, const ShaderBytecode &p_bytecode, vk::ShaderStageFlagBits p_stage,
				 vk::ShaderStageFlagBits p_next_stage = vk::ShaderStageFlagBits{0u});
		~VKShader();

		[[nodiscard]] auto getShader() const -> vk::ShaderEXT;
		[[nodiscard]] auto getStage() const -> vk::ShaderStageFlagBits;
		[[nodiscard]] auto getNextStage() const -> vk::ShaderStageFlagBits;
		[[nodiscard]] auto getBytecode() const -> const ShaderBytecode &;

	private:
		vk::raii::ShaderEXT     m_shader{nullptr};
		vk::ShaderStageFlagBits m_stage{};
		vk::ShaderStageFlagBits m_nextStage{};

		ShaderBytecode m_bytecode;
	};

	TST_GPU_DEFINE_HANDLE(VKShader, Shader)

	#define TST_PUSH_CONSTANT_BLOCK(__name) struct  alignas(16) __name
}
