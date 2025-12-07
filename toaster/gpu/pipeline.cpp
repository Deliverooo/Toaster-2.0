#include "pipeline.hpp"

#include "platform/application.hpp"
#include "renderer/renderer.hpp"

namespace tst
{
	namespace utils
	{
		static nvrhi::Format GetNVRHIFormat(ShaderDataType type)
		{
			switch (type)
			{
				case ShaderDataType::Float: return nvrhi::Format::R32_FLOAT;
				case ShaderDataType::Float2: return nvrhi::Format::RG32_FLOAT;
				case ShaderDataType::Float3: return nvrhi::Format::RGB32_FLOAT;
				case ShaderDataType::Float4: return nvrhi::Format::RGBA32_FLOAT;
				case ShaderDataType::Int: return nvrhi::Format::R32_SINT;
				case ShaderDataType::Int2: return nvrhi::Format::RG32_SINT;
				case ShaderDataType::Int3: return nvrhi::Format::RGB32_SINT;
				case ShaderDataType::Int4: return nvrhi::Format::RGBA32_SINT;
				case ShaderDataType::Bool: return nvrhi::Format::RGBA32_FLOAT;
			}

			TST_ASSERT(false);
			return nvrhi::Format::UNKNOWN;
		}

		static nvrhi::PrimitiveType GetNVRHIPrimitiveType(EPrimitiveTopology topology)
		{
			switch (topology)
			{
				case EPrimitiveTopology::ePoints: return nvrhi::PrimitiveType::PointList;
				case EPrimitiveTopology::eLines: return nvrhi::PrimitiveType::LineList;
				case EPrimitiveTopology::eTriangles: return nvrhi::PrimitiveType::TriangleList;
				case EPrimitiveTopology::eTriangleStrip: return nvrhi::PrimitiveType::TriangleStrip;
				case EPrimitiveTopology::eTriangleFan: return nvrhi::PrimitiveType::TriangleFan;
			}

			TST_ASSERT(false);
			return nvrhi::PrimitiveType::PointList;
		}

		static nvrhi::ComparisonFunc GetNVRHICompareOperator(const EDepthCompareOperator compareOp)
		{
			switch (compareOp)
			{
				case EDepthCompareOperator::eNever: return nvrhi::ComparisonFunc::Never;
				case EDepthCompareOperator::eNotEqual: return nvrhi::ComparisonFunc::NotEqual;
				case EDepthCompareOperator::eLess: return nvrhi::ComparisonFunc::Less;
				case EDepthCompareOperator::eLessOrEqual: return nvrhi::ComparisonFunc::LessOrEqual;
				case EDepthCompareOperator::eGreater: return nvrhi::ComparisonFunc::Greater;
				case EDepthCompareOperator::eGreaterOrEqual: return nvrhi::ComparisonFunc::GreaterOrEqual;
				case EDepthCompareOperator::eEqual: return nvrhi::ComparisonFunc::Equal;
				case EDepthCompareOperator::eAlways: return nvrhi::ComparisonFunc::Always;
			}
			TST_ASSERT(false);
			return nvrhi::ComparisonFunc::Never;
		}
	}

	Pipeline::Pipeline(const PipelineSpecInfo &spec)
		: m_specification(spec)
	{
		TST_ASSERT(spec.shader);
		TST_ASSERT(spec.targetFramebuffer);
		invalidate();
		Renderer::registerShaderDependency(spec.shader, this);
	}

	void Pipeline::invalidate()
	{
		RefPtr<Pipeline> instance = this;
		Renderer::submit([instance]() mutable
		{
			instance->_invalidate();
		});
	}

	void Pipeline::_invalidate()
	{
		TST_INFO_TAG("Renderer", "[Pipeline] Creating graphics pipeline: {}", m_specification.shader->getName());

		nvrhi::IDevice *     device       = Application::getGraphicsDevice();
		RefPtr<VulkanShader> vulkanShader = RefPtr<VulkanShader>(m_specification.shader);
		RefPtr<Framebuffer>  framebuffer  = m_specification.targetFramebuffer;

		nvrhi::GraphicsPipelineDesc pipelineDesc;

		#pragma region Shaders

		pipelineDesc.bindingLayouts = vulkanShader->getAllDescriptorSetLayouts();

		const auto &shaderHandles = vulkanShader->getHandles();
		if (shaderHandles.contains(nvrhi::ShaderType::Vertex))
			pipelineDesc.VS = shaderHandles.at(nvrhi::ShaderType::Vertex);
		if (shaderHandles.contains(nvrhi::ShaderType::Pixel))
			pipelineDesc.PS = shaderHandles.at(nvrhi::ShaderType::Pixel);

		pipelineDesc.primType = utils::GetNVRHIPrimitiveType(m_specification.topology);

		#pragma endregion

		#pragma region RasterState

		nvrhi::RasterState &rasterState   = pipelineDesc.renderState.rasterState;
		rasterState.cullMode              = m_specification.backfaceCulling ? nvrhi::RasterCullMode::Back : nvrhi::RasterCullMode::None;
		rasterState.fillMode              = m_specification.wireframe ? nvrhi::RasterFillMode::Line : nvrhi::RasterFillMode::Fill;
		rasterState.frontCounterClockwise = true;
		rasterState.multisampleEnable     = false;

		#pragma endregion

		#pragma region DepthStencilState

		nvrhi::DepthStencilState &depthStencilState = pipelineDesc.renderState.depthStencilState;
		depthStencilState.depthTestEnable           = m_specification.depthTest;
		depthStencilState.depthWriteEnable          = m_specification.depthWrite;
		depthStencilState.depthFunc                 = utils::GetNVRHICompareOperator(m_specification.depthOperator);

		// Stencil off
		depthStencilState.stencilEnable               = false;
		depthStencilState.backFaceStencil.failOp      = nvrhi::StencilOp::Keep;
		depthStencilState.backFaceStencil.passOp      = nvrhi::StencilOp::Keep;
		depthStencilState.backFaceStencil.stencilFunc = nvrhi::ComparisonFunc::Always;
		depthStencilState.frontFaceStencil            = depthStencilState.backFaceStencil;

		#pragma endregion

		#pragma region InputLayout

		// Vertex input descriptor
		VertexBufferLayout &vertexLayout        = m_specification.layout;
		VertexBufferLayout &instanceLayout      = m_specification.instanceLayout;
		VertexBufferLayout &boneInfluenceLayout = m_specification.boneInfluenceLayout;

		nvrhi::static_vector<nvrhi::VertexAttributeDesc, nvrhi::c_MaxVertexAttributes> vertexAttributes;

		uint32_t bufferIndex = 0;
		for (const auto &layout: {vertexLayout, instanceLayout, boneInfluenceLayout})
		{
			for (const VertexBufferElement &element: layout)
			{
				nvrhi::VertexAttributeDesc &attributeDesc = vertexAttributes.emplace_back();
				attributeDesc.bufferIndex                 = bufferIndex;
				attributeDesc.name                        = element.Name;
				attributeDesc.format                      = utils::GetNVRHIFormat(element.Type);
				attributeDesc.offset                      = element.Offset;
				attributeDesc.elementStride               = layout.GetStride();
				attributeDesc.isInstanced                 = layout.IsInstanced();
			}

			bufferIndex++;
		}

		pipelineDesc.inputLayout = device->createInputLayout(vertexAttributes.data(), vertexAttributes.size(), pipelineDesc.VS);

		#pragma endregion

		#pragma region BlendState

		nvrhi::BlendState &blendState           = pipelineDesc.renderState.blendState;
		size_t             colorAttachmentCount = framebuffer->getSpecification().swapChainTarget ? 1 : framebuffer->getColorAttachmentCount();
		if (framebuffer->getSpecification().swapChainTarget)
		{
			nvrhi::BlendState::RenderTarget &renderTarget = blendState.targets[0];
			renderTarget.blendEnable                      = true;
			renderTarget.colorWriteMask                   = nvrhi::ColorMask::All;
			renderTarget.srcBlend                         = nvrhi::BlendFactor::SrcAlpha;
			renderTarget.destBlend                        = nvrhi::BlendFactor::OneMinusSrcAlpha;
			renderTarget.blendOp                          = nvrhi::BlendOp::Add;
			renderTarget.blendOpAlpha                     = nvrhi::BlendOp::Add;
			renderTarget.srcBlendAlpha                    = nvrhi::BlendFactor::One;
			renderTarget.destBlendAlpha                   = nvrhi::BlendFactor::Zero;
		}
		else
		{
			for (size_t i = 0; i < colorAttachmentCount; i++)
			{
				if (!framebuffer->getSpecification().Blend)
					break;

				nvrhi::BlendState::RenderTarget &renderTarget = blendState.targets[i];
				renderTarget.colorWriteMask                   = nvrhi::ColorMask::All;

				const auto &          attachmentSpec = framebuffer->getSpecification().Attachments.Attachments[i];
				EFramebufferBlendMode blendMode      = framebuffer->getSpecification().blendMode == EFramebufferBlendMode::eNone
														   ? attachmentSpec.BlendMode
														   : framebuffer->getSpecification().blendMode;

				renderTarget.blendEnable = attachmentSpec.Blend ? VK_TRUE : VK_FALSE;

				renderTarget.blendOp        = nvrhi::BlendOp::Add;
				renderTarget.blendOpAlpha   = nvrhi::BlendOp::Add;
				renderTarget.srcBlendAlpha  = nvrhi::BlendFactor::One;
				renderTarget.destBlendAlpha = nvrhi::BlendFactor::Zero;

				switch (blendMode)
				{
					case EFramebufferBlendMode::eSrcAlphaOneMinusSrcAlpha:
						renderTarget.srcBlend = nvrhi::BlendFactor::SrcAlpha;
						renderTarget.destBlend      = nvrhi::BlendFactor::OneMinusSrcAlpha;
						renderTarget.srcBlendAlpha  = nvrhi::BlendFactor::SrcAlpha;
						renderTarget.destBlendAlpha = nvrhi::BlendFactor::OneMinusSrcAlpha;
						break;
					case EFramebufferBlendMode::eOneZero:
						renderTarget.srcBlend = nvrhi::BlendFactor::One;
						renderTarget.destBlend = nvrhi::BlendFactor::Zero;
						break;
					case EFramebufferBlendMode::eZero_SrcColor:
						renderTarget.srcBlend = nvrhi::BlendFactor::Zero;
						renderTarget.destBlend = nvrhi::BlendFactor::SrcColor;
						break;
					default: TST_ASSERT(false);
				}
			}
		}

		#pragma endregion

		m_handle = device->createGraphicsPipeline(pipelineDesc, m_specification.targetFramebuffer->getHandle()->getFramebufferInfo());
	}

	bool Pipeline::isDynamicLineWidth() const
	{
		return m_specification.topology == EPrimitiveTopology::eLines || m_specification.topology == EPrimitiveTopology::eLineStrip || m_specification.wireframe;
	}
}
