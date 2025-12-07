#include "compute_pipeline.hpp"

#include "image.hpp"
#include "platform/application.hpp"
#include "renderer/renderer.hpp"
#include "vulkan/vulkan_shader.hpp"

namespace tst
{
	static nvrhi::ResourceStates MapAccessFlagsToResourceState(EResourceAccessFlags accessFlags)
	{
		auto state = nvrhi::ResourceStates::Unknown;

		if (static_cast<uint32_t>(accessFlags) & static_cast<uint32_t>(EResourceAccessFlags::eShaderRead))
			state = state | nvrhi::ResourceStates::ShaderResource;
		if (static_cast<uint32_t>(accessFlags) & static_cast<uint32_t>(EResourceAccessFlags::eShaderWrite))
			state = state | nvrhi::ResourceStates::UnorderedAccess;
		if (static_cast<uint32_t>(accessFlags) & static_cast<uint32_t>(EResourceAccessFlags::eTransferRead))
			state = state | nvrhi::ResourceStates::CopySource;
		if (static_cast<uint32_t>(accessFlags) & static_cast<uint32_t>(EResourceAccessFlags::eTransferWrite))
			state = state | nvrhi::ResourceStates::CopyDest;
		if (static_cast<uint32_t>(accessFlags) & static_cast<uint32_t>(EResourceAccessFlags::eColorAttachmentWrite))
			state = state | nvrhi::ResourceStates::RenderTarget;
		if (static_cast<uint32_t>(accessFlags) & static_cast<uint32_t>(EResourceAccessFlags::eDepthStencilAttachmentRead))
			state = state | nvrhi::ResourceStates::DepthRead;
		if (static_cast<uint32_t>(accessFlags) & static_cast<uint32_t>(EResourceAccessFlags::eDepthStencilAttachmentWrite))
			state = state | nvrhi::ResourceStates::DepthWrite;

		// Default to ShaderResource if no specific flags matched
		if (state == nvrhi::ResourceStates::Unknown)
			state = nvrhi::ResourceStates::ShaderResource;

		return state;
	}

	ComputePipeline::ComputePipeline(RefPtr<Shader> computeShader)
		: m_shader(computeShader)
	{
		_createPipeline();
		Renderer::registerShaderDependency(computeShader, this);
	}

	void ComputePipeline::_createPipeline()
	{
		TST_INFO_TAG("Renderer", "[ComputePipeline] Creating compute pipeline: {}", m_shader->getName());

		nvrhi::ComputePipelineDesc desc;
		desc.CS             = m_shader->getHandle();
		desc.bindingLayouts = m_shader.as<VulkanShader>()->getAllDescriptorSetLayouts();

		nvrhi::DeviceHandle device = Application::getGraphicsDevice();
		m_handle                   = device->createComputePipeline(desc);

		m_commandList = make_reference<CommandBuffer>(1, "ComputePipeline");
	}

	void ComputePipeline::begin(RefPtr<CommandBuffer> renderCommandBuffer)
	{
	}

	void ComputePipeline::_begin(RefPtr<CommandBuffer> renderCommandBuffer)
	{
	}

	void ComputePipeline::end()
	{
	}

	void ComputePipeline::bufferMemoryBarrier(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<StorageBuffer> storageBuffer, EResourceAccessFlags fromAccess,
											  EResourceAccessFlags  toAccess)
	{
		bufferMemoryBarrier(renderCommandBuffer, storageBuffer, PipelineStage::ComputeShader, fromAccess, PipelineStage::AllCommands, toAccess);
	}

	void ComputePipeline::bufferMemoryBarrier(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<StorageBuffer> storageBuffer, PipelineStage  fromStage,
											  EResourceAccessFlags  fromAccess, PipelineStage                  toStage, EResourceAccessFlags toAccess)
	{
		Renderer::submit([renderCommandBuffer, storageBuffer, toAccess]() mutable
		{
			nvrhi::CommandListHandle commandList = renderCommandBuffer->getActiveCommandBuffer();
			nvrhi::ResourceStates    targetState = MapAccessFlagsToResourceState(toAccess);
			commandList->setBufferState(storageBuffer->getHandle(), targetState);
		});
	}

	void ComputePipeline::imageMemoryBarrier(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<Image2D> image, EResourceAccessFlags fromAccess,
											 EResourceAccessFlags  toAccess)
	{
		imageMemoryBarrier(renderCommandBuffer, image, PipelineStage::ComputeShader, fromAccess, PipelineStage::AllCommands, toAccess);
	}

	void ComputePipeline::imageMemoryBarrier(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<Image2D> image, PipelineStage fromStage, EResourceAccessFlags fromAccess,
											 PipelineStage         toStage, EResourceAccessFlags        toAccess)
	{
		Renderer::submit([renderCommandBuffer, image, toAccess]() mutable
		{
			nvrhi::CommandListHandle commandList = renderCommandBuffer->getActiveCommandBuffer();
			nvrhi::ResourceStates    targetState = MapAccessFlagsToResourceState(toAccess);
			commandList->setTextureState(image->getHandle(), nvrhi::AllSubresources, targetState);
		});
	}

	void ComputePipeline::execute(void *descriptorSets, uint32_t descriptorSetCount, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		nvrhi::DeviceHandle device = Application::getGraphicsDevice();

		nvrhi::ComputeState computeState;
		computeState.pipeline = m_handle;

		m_commandList->_beginRecording();
		m_commandList->getActiveCommandBuffer()->setComputeState(computeState);
		m_commandList->getActiveCommandBuffer()->dispatch(groupCountX, groupCountY, groupCountZ);

		m_commandList->_endRecording();
		m_commandList->_submit();
	}

	void ComputePipeline::setPushConstants(Buffer constants) const
	{
	}

	void ComputePipeline::createPipeline()
	{
	}
}
