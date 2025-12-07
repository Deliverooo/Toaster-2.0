#include "image.hpp"

#include "math/math_funcs.hpp"
#include "platform/application.hpp"
#include "renderer/renderer.hpp"

namespace tst
{
	static std::map<nvrhi::ITexture *, CachedPtr<Image2D> > s_ImageReferences;

	Image2D::Image2D(const ImageSpecInfo &specification)
		: m_specification(specification)
	{
		TST_ASSERT(m_specification.Width > 0 && m_specification.Height > 0);
	}

	Image2D::~Image2D()
	{
		release();
	}

	void Image2D::invalidate()
	{
		_invalidate();
	}

	void Image2D::release()
	{
		s_ImageReferences.erase(m_Info.ImageHandle.Get());
	}

	int Image2D::getClosestMipLevel(uint32_t width, uint32_t height) const
	{
		if (width > m_specification.Width / 2 || height > m_specification.Height / 2)
			return 0;

		int a = tsm::log2(tsm::min(m_specification.Width, m_specification.Height));
		int b = tsm::log2(tsm::min(width, height));
		return a - b;
	}

	std::pair<uint32_t, uint32_t> Image2D::getMipLevelSize(int mipLevel) const
	{
		uint32_t width  = m_specification.Width;
		uint32_t height = m_specification.Height;
		return {width >> mipLevel, height >> mipLevel};
	}

	void Image2D::_invalidate()
	{
		TST_ASSERT(m_specification.Width > 0 && m_specification.Height > 0);
		nvrhi::DeviceHandle device = Application::getGraphicsDevice();

		release();

		// Safety net since nvrhi has both Texture2D and Texture2DArray, but we should
		// use the latter if image has many layers since subresource resolution will
		// result in not using anything above the first layer if Texture2D is set
		if (m_specification.Layers > 1 && m_specification.Dimension == nvrhi::TextureDimension::Texture2D)
			m_specification.Dimension = nvrhi::TextureDimension::Texture2DArray;

		nvrhi::TextureDesc textureDesc;
		textureDesc.debugName = m_specification.DebugName;
		textureDesc.dimension = m_specification.Dimension;
		textureDesc.format    = utils::NVRHIFormat(m_specification.Format);
		textureDesc.width     = m_specification.Width;
		textureDesc.height    = m_specification.Height;
		textureDesc.mipLevels = m_specification.Mips;
		textureDesc.arraySize = m_specification.Layers;

		// NOTE(Yan): tiling?

		textureDesc.isRenderTarget = m_specification.Usage == ImageUsage::Attachment;
		textureDesc.isUAV          = m_specification.Usage == ImageUsage::Storage;

		// Always has source/dest?
		if (m_specification.Usage == ImageUsage::Texture)
		{
			textureDesc.initialState     = nvrhi::ResourceStates::ShaderResource;
			textureDesc.keepInitialState = true;
		}
		else if (m_specification.Usage == ImageUsage::Attachment)
		{
			textureDesc.initialState     = utils::IsDepthFormat(m_specification.Format) ? nvrhi::ResourceStates::DepthWrite : nvrhi::ResourceStates::RenderTarget;
			textureDesc.keepInitialState = true;
		}
		else if (m_specification.Usage == ImageUsage::Storage)
		{
			textureDesc.initialState     = utils::IsDepthFormat(m_specification.Format) ? nvrhi::ResourceStates::DepthWrite : nvrhi::ResourceStates::UnorderedAccess;
			textureDesc.keepInitialState = true;
		}

		//textureDesc.initialState = textureDesc.initialState | nvrhi::ResourceStates::UnorderedAccess;
		if (!utils::IsDepthFormat(m_specification.Format) && m_specification.Format != ImageFormat::SRGB && m_specification.Format != ImageFormat::SRGBA)
			textureDesc.isUAV = true;

		if (textureDesc.isUAV)
		{
			TST_ASSERT(!utils::IsDepthFormat(m_specification.Format));
			TST_ASSERT(m_specification.Format != ImageFormat::SRGB);
			TST_ASSERT(m_specification.Format != ImageFormat::SRGBA);
		}

		m_Info.ImageHandle = device->createTexture(textureDesc);

		s_ImageReferences[m_Info.ImageHandle.Get()] = this;

		if (m_specification.CreateSampler)
		{
			nvrhi::SamplerDesc samplerDesc;
			samplerDesc.minFilter = samplerDesc.magFilter = samplerDesc.mipFilter = !utils::IsIntegerBased(m_specification.Format);
			samplerDesc.addressU  = nvrhi::SamplerAddressMode::ClampToEdge;
			samplerDesc.addressV  = samplerDesc.addressW = samplerDesc.addressU;

			m_Info.Sampler = device->createSampler(samplerDesc);

			// VKutils::SetDebugutilsObjectName(device, VK_OBJECT_TYPE_SAMPLER, std::format("{} default sampler", m_Specification.DebugName), m_Info.Sampler);
		}

		m_Info.Dimension = textureDesc.dimension;
	}

	// void Image2D::CreatePerLayerImageViews()
	// {
	// 	RefPtr<Image2D> instance = this;
	// 	Renderer::submit([instance]() mutable
	// 	{
	// 		//instance->RT_CreatePerLayerImageViews();
	// 	});
	// 	RT_CreatePerLayerImageViews();
	// }

	void Image2D::_createPerLayerImageViews()
	{
		TST_ASSERT(m_specification.Layers > 1);

		m_PerLayerImageViews.resize(m_specification.Layers);
		for (uint32_t layer = 0; layer < m_specification.Layers; layer++)
		{
			nvrhi::TextureSubresourceSet &tss = m_PerLayerImageViews[layer];
			tss.baseMipLevel                  = 0;
			tss.numMipLevels                  = m_specification.Mips;
			tss.baseArraySlice                = layer;
			tss.numArraySlices                = 1;
		}
	}

	nvrhi::TextureSubresourceSet Image2D::getMipImageView(uint32_t mip)
	{
		auto it = m_PerMipImageViews.find(mip);
		if (it != m_PerMipImageViews.end())
			return it->second;

		nvrhi::TextureSubresourceSet &tss = m_PerMipImageViews[mip];
		tss.baseMipLevel                  = mip;
		tss.numMipLevels                  = 1;
		tss.baseArraySlice                = 0;
		tss.numArraySlices                = 1;
		return tss;
	}

	void Image2D::_createPerSpecificLayerImageViews(const std::vector<uint32_t> &layerIndices)
	{
		TST_ASSERT(m_specification.Layers > 1);

		if (m_PerLayerImageViews.empty())
			m_PerLayerImageViews.resize(m_specification.Layers);

		for (uint32_t layer: layerIndices)
		{
			nvrhi::TextureSubresourceSet &tss = m_PerLayerImageViews[layer];
			tss.baseMipLevel                  = 0;
			tss.numMipLevels                  = m_specification.Mips;
			tss.baseArraySlice                = layer;
			tss.numArraySlices                = 1;
		}
	}

	const std::map<nvrhi::ITexture *, CachedPtr<Image2D> > &Image2D::getImageRefs()
	{
		return s_ImageReferences;
	}

	void Image2D::setData(Buffer buffer)
	{
		TST_ASSERT(m_specification.Transfer);

		if (buffer)
		{
			if (!m_CommandList)
				m_CommandList = make_reference<CommandBuffer>(1, "Image2D");

			m_CommandList->_beginRecording();

			m_CommandList->getActiveCommandBuffer()->writeTexture(m_Info.ImageHandle, 0, 0, buffer.data,
																  utils::GetImageMemoryRowPitch(m_specification.Format, m_specification.Width));

			m_CommandList->_endRecording();
			m_CommandList->_submit();

			m_Info.State = nvrhi::ResourceStates::ShaderResource;
		}
	}

	void Image2D::copyToHostBuffer(Buffer &buffer) const
	{
		nvrhi::IDevice *device = Application::getInstance().getMainWindow().getDeviceManager()->GetDevice();

		if (!m_CommandList)
			m_CommandList = make_reference<CommandBuffer>(1, "Image2D");

		auto stagingDesc = nvrhi::TextureDesc().setFormat(utils::NVRHIFormat(m_specification.Format)).setDimension(nvrhi::TextureDimension::Texture2D).
				setWidth(m_specification.Width).setHeight(m_specification.Height).setDepth(1).setMipLevels(m_specification.Mips).setArraySize(m_specification.Layers);

		nvrhi::StagingTextureHandle stagingTexture = device->createStagingTexture(stagingDesc, nvrhi::CpuAccessMode::Read);

		nvrhi::TextureSlice textureSlice;

		uint32_t mipCount = 1; // TODO(Yan): not sure what the orig code was doing...
		// mipCount = m_Specification.Mips;

		m_CommandList->_beginRecording();

		//commandList->beginTrackingTextureState(m_Info.ImageHandle, nvrhi::AllSubresources, m_Info.State);
		//commandList->setTextureState(m_Info.ImageHandle, nvrhi::AllSubresources, nvrhi::ResourceStates::CopySource);
		for (uint32_t mip = 0; mip < mipCount; mip++)
		{
			textureSlice.mipLevel = mip;
			m_CommandList->getActiveCommandBuffer()->copyTexture(stagingTexture, textureSlice, m_Info.ImageHandle, textureSlice);
		}
		//commandList->setTextureState(m_Info.ImageHandle, nvrhi::AllSubresources, m_Info.State);

		m_CommandList->_endRecording();
		m_CommandList->_submit();

		uint64_t bufferSize = m_specification.Width * m_specification.Height * utils::GetImageFormatBPP(m_specification.Format);
		buffer.alloc(bufferSize);
		for (uint32_t mip = 0; mip < mipCount; mip++)
		{
			textureSlice.mipLevel = mip;
			size_t rowPitch;
			void * data = device->mapStagingTexture(stagingTexture, textureSlice, nvrhi::CpuAccessMode::Read, &rowPitch);
			TST_ASSERT(bufferSize == (rowPitch * m_specification.Height));
			memcpy(buffer.data, data, buffer.size);
			device->unmapStagingTexture(stagingTexture);
		}
	}

	ImageView::ImageView(const ImageViewSpecInfo &specification)
		: m_Specification(specification)
	{
		Invalidate();
	}

	void ImageView::Invalidate()
	{
		RT_Invalidate();
	}

	void ImageView::RT_Invalidate()
	{
		m_ImageInfo = m_Specification.Image->getImageInfo();

		m_TextureSubresourceSet.baseMipLevel = m_Specification.Mip;
		m_TextureSubresourceSet.numMipLevels = m_Specification.MipCount == 0 ? m_Specification.Image->getSpecInfo().Mips : m_Specification.MipCount;

		m_TextureSubresourceSet.baseArraySlice = m_Specification.Layer;
		m_TextureSubresourceSet.numArraySlices = m_Specification.LayerCount == 0 ? m_Specification.Image->getSpecInfo().Layers : m_Specification.LayerCount;

		m_ImageInfo.ImageView = m_TextureSubresourceSet;
		m_ImageInfo.Dimension = m_Specification.Dimension;
	}

	Sampler::Sampler(const SamplerSpecification &specification)
		: m_Specification(specification)
	{
		Invalidate();
	}

	void Sampler::Invalidate()
	{
		const auto desc = nvrhi::SamplerDesc().setMipBias(m_Specification.MipBias).setAllAddressModes(m_Specification.AddressMode).setMinFilter(m_Specification.MinFilter)
				.setMagFilter(m_Specification.MagFilter).setMipFilter(m_Specification.MipFilter);

		auto device = Application::getGraphicsDevice();
		m_handle    = device->createSampler(desc);
	}
}
