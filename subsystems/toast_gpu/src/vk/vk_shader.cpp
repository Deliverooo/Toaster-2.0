#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster::gpu
{
	auto VKShader::operator=(VKShader &&p_other) noexcept -> VKShader &
	{
		if (this != &p_other)
		{
			m_gpuCtx    = p_other.m_gpuCtx;
			m_shader    = std::move(p_other.m_shader);
			m_stage     = p_other.m_stage;
			m_nextStage = p_other.m_stage;
		}
		return *this;
	}

	VKShader::VKShader(VKGPUContext &          p_gpu_ctx, const ShaderBytecode &p_bytecode, vk::ShaderStageFlagBits p_stage,
									 vk::ShaderStageFlagBits p_next_stage) : m_gpuCtx(&p_gpu_ctx), m_stage(p_stage), m_nextStage(p_next_stage), m_bytecode(p_bytecode)
	{
		TST_PERMA_ASSERT_MSG(!p_bytecode.empty(), "Your shader compilation failed, or you have the wrong path");

		vk::ShaderCreateInfoEXT shader_create_info{};
		shader_create_info.stage     = m_stage;
		shader_create_info.nextStage = m_nextStage;
		shader_create_info.codeSize  = m_bytecode.size() * sizeof(uint32);
		shader_create_info.pCode     = m_bytecode.data();
		shader_create_info.codeType  = vk::ShaderCodeTypeEXT::eSpirv;
		shader_create_info.pName     = "main";
		shader_create_info.flags     = vk::ShaderCreateFlagBitsEXT::eDescriptorHeap;
		// if (m_stage == vk::ShaderStageFlagBits::eMeshEXT)
		// shader_create_info.flags |= vk::ShaderCreateFlagBitsEXT::eNoTaskShader;

		m_shader = m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice().createShaderEXT(shader_create_info);
	}

	VKShader::~VKShader()
	{
		m_gpuCtx->deferDestruction([shader = std::move(m_shader)]() mutable -> void
		{
			shader = nullptr;
		});
	}

	auto VKShader::getShader() const -> vk::ShaderEXT
	{
		return *m_shader;
	}

	auto VKShader::getStage() const -> vk::ShaderStageFlagBits
	{
		return m_stage;
	}

	auto VKShader::getNextStage() const -> vk::ShaderStageFlagBits
	{
		return m_nextStage;
	}

	auto VKShader::getBytecode() const -> const ShaderBytecode &
	{
		return m_bytecode;
	}
}
