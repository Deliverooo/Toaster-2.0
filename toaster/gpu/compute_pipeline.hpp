#pragma once
#include "ref_ptr.hpp"

#include "gpu/shader.hpp"
#include "gpu/image.hpp"
#include "gpu/command_buffer.hpp"
#include "gpu/storage_buffer.hpp"
#include "pipeline_spec_info.hpp"

namespace tst
{
	class ComputePipeline : public RefCounted
	{
	public:
		void begin(RefPtr<CommandBuffer> renderCommandBuffer = nullptr);
		void _begin(RefPtr<CommandBuffer> renderCommandBuffer = nullptr);
		void end();

		void bufferMemoryBarrier(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<StorageBuffer> storageBuffer, EResourceAccessFlags fromAccess,
								 EResourceAccessFlags  toAccess);
		void bufferMemoryBarrier(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<StorageBuffer> storageBuffer, PipelineStage fromStage, EResourceAccessFlags fromAccess,
								 PipelineStage         toStage, EResourceAccessFlags              toAccess);

		void imageMemoryBarrier(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<Image2D> image, EResourceAccessFlags fromAccess, EResourceAccessFlags toAccess);
		void imageMemoryBarrier(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<Image2D> image, PipelineStage fromStage, EResourceAccessFlags fromAccess,
								PipelineStage         toStage, EResourceAccessFlags        toAccess);

		void execute(void *descriptorSets, uint32_t descriptorSetCount, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

		void setPushConstants(Buffer constants) const;
		void createPipeline();

		nvrhi::ComputePipelineHandle getHandle() const { return m_handle; }
		RefPtr<Shader>               getShader() const { return m_shader; }

		ComputePipeline(RefPtr<Shader> computeShader);

	private:
		void _createPipeline();

		RefPtr<Shader>               m_shader;
		nvrhi::ComputePipelineHandle m_handle = nullptr;

		RefPtr<CommandBuffer> m_commandList;

		bool m_usingGraphicsQueue = false;
	};
}
