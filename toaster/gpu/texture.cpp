#include "texture.hpp"

#include "log.hpp"
#include "platform/application.hpp"
#include "renderer/renderer.hpp"
#include "asset/texture_importer.hpp"
#include "gpu/material.hpp"
#include "gpu/compute_pipeline.hpp"
#include "gpu/compute_pass.hpp"

namespace tst
{
	namespace utils
	{
		static nvrhi::SamplerAddressMode NVRHISamplerWrap(TextureWrap wrap)
		{
			switch (wrap)
			{
				case TextureWrap::Clamp: return nvrhi::SamplerAddressMode::Clamp;
				case TextureWrap::Repeat: return nvrhi::SamplerAddressMode::Repeat;
			}
			TST_ASSERT(false);
			return (nvrhi::SamplerAddressMode) 0;
		}

		static bool NVRHISamplerFilter(TextureFilter filter)
		{
			switch (filter)
			{
				case TextureFilter::Linear:
				case TextureFilter::Cubic: return true;
				case TextureFilter::Nearest: return false;
			}
			TST_ASSERT(false);
			return false;
		}

		static size_t GetMemorySize(ImageFormat format, uint32_t width, uint32_t height)
		{
			switch (format)
			{
				case ImageFormat::RED16UI: return width * height * sizeof(uint16_t);
				case ImageFormat::RG16F: return width * height * 2 * sizeof(uint16_t);
				case ImageFormat::RG32F: return width * height * 2 * sizeof(float);
				case ImageFormat::RED32F: return width * height * sizeof(float);
				case ImageFormat::RED8UN: return width * height;
				case ImageFormat::RED8UI: return width * height;
				case ImageFormat::RGBA: return width * height * 4;
				case ImageFormat::SRGBA: return width * height * 4;
				case ImageFormat::RGBA16F:
					return width * height * 4 * sizeof(uint16_t);
				case ImageFormat::RGBA32F: return width * height * 4 * sizeof(float);
				case ImageFormat::B10R11G11UF: return width * height * sizeof(float);
				case ImageFormat::DEPTH32F: return width * height * sizeof(float);
				case ImageFormat::DEPTH32FSTENCIL8UINT:
					return width * height * (sizeof(float) + sizeof(uint8_t));
				case ImageFormat::DEPTH24STENCIL8:
					return width * height * 4;
			}
			TST_ASSERT(false);
			return 0;
		}

		static bool ValidateSpecification(const TextureSpecInfo &specification)
		{
			bool result = true;

			result = specification.Width > 0 && specification.Height > 0 && specification.Width < 65536 && specification.Height < 65536;
			TST_ASSERT(result);

			return result;
		}
	}

	Texture2D::Texture2D(const TextureSpecInfo &specification, const std::filesystem::path &filepath)
		: m_Specification(specification), m_Path(filepath)
	{
		if (m_Specification.DebugName.empty())
			m_Specification.DebugName = filepath.string();
		CreateFromFile(specification, filepath);
	}

	Texture2D::Texture2D(const TextureSpecInfo &specification, Buffer data)
		: m_Specification(specification)
	{
		CreateFromBuffer(specification, data);
	}

	Texture2D::~Texture2D()
	{
		//	if (m_Image)
		//		m_Image->Release();

		m_ImageData.release();
	}

	RefPtr<Texture2D> Texture2D::CreateFromSRGB(RefPtr<Texture2D> texture)
	{
		// The source texture should already have mipmaps generated (it's RGBA, which supports compute-based mip generation)
		// We copy all mip levels from the source and disable mip generation on the SRGBA texture
		// because SRGB formats cannot be used as storage images in compute shaders

		uint32_t mipCount = texture->GetMipLevelCount();

		TextureSpecInfo spec;
		spec.Width         = texture->GetWidth();
		spec.Height        = texture->GetHeight();
		spec.Format        = ImageFormat::SRGBA;
		spec.GenerateMips  = false; // SRGBA cannot be used as storage image for compute-based mip generation
		spec.SamplerWrap   = texture->m_Specification.SamplerWrap;
		spec.SamplerFilter = texture->m_Specification.SamplerFilter;

		// Create the SRGBA image with all mip levels pre-allocated (matching source texture)
		ImageSpecInfo imageSpec;
		imageSpec.Format        = ImageFormat::SRGBA;
		imageSpec.Width         = spec.Width;
		imageSpec.Height        = spec.Height;
		imageSpec.Mips          = mipCount; // Allocate all mip levels
		imageSpec.CreateSampler = false;
		imageSpec.Transfer      = true;

		RefPtr<Image2D> srgbImage = make_reference<Image2D>(imageSpec);
		srgbImage->invalidate();

		RefPtr<Texture2D> srgbTexture = make_reference<Texture2D>(spec);
		srgbTexture->m_Image          = srgbImage;

		RefPtr<CommandBuffer> commandBuffer = make_reference<CommandBuffer>(1, "CreateFromSRGB-CopyMips");
		commandBuffer->beginRecording();

		Renderer::submit([srgbImage, srgbTexture, mipCount,spec,texture, commandBuffer]() mutable
		{
			nvrhi::DeviceHandle device = Application::getGraphicsDevice();
			nvrhi::SamplerDesc  samplerDesc;
			samplerDesc.minFilter             = samplerDesc.magFilter = samplerDesc.mipFilter = utils::NVRHISamplerFilter(spec.SamplerFilter);
			samplerDesc.addressU              = utils::NVRHISamplerWrap(spec.SamplerWrap);
			samplerDesc.addressV              = samplerDesc.addressW = samplerDesc.addressU;
			srgbImage->getImageInfo().Sampler = device->createSampler(samplerDesc);

			// Copy all mip levels from source texture to SRGBA texture using GPU copy
			nvrhi::CommandListHandle commandList = commandBuffer->getActiveCommandBuffer();

			// Copy each mip level from source to destination
			for (uint32_t mip = 0; mip < mipCount; mip++)
			{
				nvrhi::TextureSlice srcSlice;
				srcSlice.mipLevel = mip;

				nvrhi::TextureSlice dstSlice;
				dstSlice.mipLevel = mip;

				commandList->copyTexture(srgbTexture->GetHandle(), dstSlice, texture->GetHandle(), srcSlice);
			}
		});

		commandBuffer->endRecording();
		commandBuffer->submit();

		return srgbTexture;
	}

	void Texture2D::CreateFromFile(const TextureSpecInfo &specification, const std::filesystem::path &filepath)
	{
		utils::ValidateSpecification(specification);

		TST_INFO("loading image {}", filepath.string());

		//specification.GenerateMips = true;

		m_ImageData = TextureImporter::toBufferFromFile(filepath, m_Specification.Format, m_Specification.Width, m_Specification.Height);
		if (!m_ImageData)
		{
			// TODO(Yan): move this to asset manager
			TST_ERROR("Failed to load texture from file: {}", filepath.string());
			m_ImageData = TextureImporter::toBufferFromFile("res/textures/ErrorTexture.png", m_Specification.Format, m_Specification.Width, m_Specification.Height);
		}

		ImageSpecInfo imageSpec;
		imageSpec.DebugName     = m_Specification.DebugName;
		imageSpec.Format        = m_Specification.Format;
		imageSpec.Width         = m_Specification.Width;
		imageSpec.Height        = m_Specification.Height;
		imageSpec.Mips          = specification.GenerateMips ? GetMipLevelCount() : 1;
		imageSpec.DebugName     = specification.DebugName;
		imageSpec.CreateSampler = false;
		m_Image                 = make_reference<Image2D>(imageSpec);

		TST_ASSERT(m_Specification.Format != ImageFormat::None);

		Invalidate();
	}

	void Texture2D::ReplaceFromFile(const TextureSpecInfo &specification, const std::filesystem::path &filepath)
	{
		utils::ValidateSpecification(specification);

		m_ImageData = TextureImporter::toBufferFromFile(filepath, m_Specification.Format, m_Specification.Width, m_Specification.Height);
		if (!m_ImageData)
		{
			// TODO(Yan): move this to asset manager
			TST_ERROR("Failed to load texture from file: {}", filepath.string());
			m_ImageData = TextureImporter::toBufferFromFile("res/textures/ErrorTexture.png", m_Specification.Format, m_Specification.Width, m_Specification.Height);
		}

		ImageSpecInfo imageSpec;
		imageSpec.Format        = m_Specification.Format;
		imageSpec.Width         = m_Specification.Width;
		imageSpec.Height        = m_Specification.Height;
		imageSpec.Mips          = specification.GenerateMips ? GetMipLevelCount() : 1;
		imageSpec.DebugName     = specification.DebugName;
		imageSpec.CreateSampler = false;
		m_Image                 = make_reference<Image2D>(imageSpec);

		TST_ASSERT(m_Specification.Format != ImageFormat::None);

		RefPtr<Texture2D> instance = this;
		Renderer::submit([instance]() mutable
		{
			instance->Invalidate();
		});
	}

	void Texture2D::CreateFromBuffer(const TextureSpecInfo &specification, Buffer data)
	{
		TST_INFO("loading image from buffer");

		if (m_Specification.Height == 0)
		{
			m_ImageData = TextureImporter::toBufferFromMemory(Buffer(data.data, m_Specification.Width), m_Specification.Format, m_Specification.Width,
															  m_Specification.Height);
			if (!m_ImageData)
			{
				// TODO(Yan): move this to asset manager
				m_ImageData = TextureImporter::toBufferFromFile("res/textures/ErrorTexture.png", m_Specification.Format, m_Specification.Width, m_Specification.Height);
			}

			utils::ValidateSpecification(m_Specification);
		}
		else if (data)
		{
			utils::ValidateSpecification(m_Specification);
			auto size   = (uint32_t) utils::GetMemorySize(m_Specification.Format, m_Specification.Width, m_Specification.Height);
			m_ImageData = Buffer::copy(data.data, size);
		}
		else
		{
			utils::ValidateSpecification(m_Specification);
			auto size = (uint32_t) utils::GetMemorySize(m_Specification.Format, m_Specification.Width, m_Specification.Height);
			m_ImageData.alloc(size);
			m_ImageData.zeroInit();
		}

		ImageSpecInfo imageSpec;
		imageSpec.Format        = m_Specification.Format;
		imageSpec.Width         = m_Specification.Width;
		imageSpec.Height        = m_Specification.Height;
		imageSpec.Mips          = specification.GenerateMips ? Texture2D::GetMipLevelCount() : 1;
		imageSpec.DebugName     = specification.DebugName;
		imageSpec.CreateSampler = false;
		if (specification.Storage)
			imageSpec.Usage = ImageUsage::Storage;
		m_Image = make_reference<Image2D>(imageSpec);

		Invalidate();
	}

	void Texture2D::Resize(const tsm::vec2ui &size)
	{
		Resize(size.x, size.y);
	}

	void Texture2D::Resize(const uint32_t width, const uint32_t height)
	{
		m_Specification.Width  = width;
		m_Specification.Height = height;

		Invalidate();

		/*RefPtr<Texture2D> instance = this;
		Renderer::Submit([instance]() mutable
		{
			instance->Invalidate();
		});*/
	}

	void Texture2D::Invalidate()
	{
		nvrhi::DeviceHandle device = Application::getGraphicsDevice();

		m_Image->release();

		uint32_t mipCount = m_Specification.GenerateMips ? GetMipLevelCount() : 1;

		ImageSpecInfo &imageSpec = m_Image->getSpecInfo();
		imageSpec.Format         = m_Specification.Format;
		imageSpec.Width          = m_Specification.Width;
		imageSpec.Height         = m_Specification.Height;
		imageSpec.Mips           = mipCount;
		imageSpec.CreateSampler  = false;
		imageSpec.Transfer       = true;
		if (!m_ImageData) // TODO(Yan): better management for this, probably from texture spec
			imageSpec.Usage = ImageUsage::Storage;

		RefPtr<Image2D> image = m_Image.as<Image2D>();
		image->_invalidate();

		auto &info = image->getImageInfo();

		if (m_ImageData)
		{
			SetData(m_ImageData);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CREATE TEXTURE SAMPLER (owned by Image)
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		nvrhi::SamplerDesc samplerDesc;
		samplerDesc.minFilter = samplerDesc.magFilter = samplerDesc.mipFilter = utils::NVRHISamplerFilter(m_Specification.SamplerFilter);
		samplerDesc.addressU  = utils::NVRHISamplerWrap(m_Specification.SamplerWrap);
		samplerDesc.addressV  = samplerDesc.addressW = samplerDesc.addressU;

		info.Sampler = device->createSampler(samplerDesc);

		if (m_ImageData && m_Specification.GenerateMips && mipCount > 1)
			GenerateMips();

		// TODO(Yan): option for local storage
		m_ImageData.release();
		m_ImageData = Buffer();
	}

	void Texture2D::SetData(Buffer buffer)
	{
		m_Image->setData(buffer);

		// Generate mips
		uint32_t mipCount = m_Specification.GenerateMips ? GetMipLevelCount() : 1;
		if (m_Specification.GenerateMips && mipCount > 1)
			GenerateMips();
	}

	void Texture2D::Lock()
	{
		if (!m_ImageData)
		{
			auto size = (uint32_t) utils::GetMemorySize(m_Specification.Format, m_Specification.Width, m_Specification.Height);
			m_ImageData.alloc(size);
		}
	}

	void Texture2D::Unlock()
	{
		SetData(m_ImageData);
	}

	Buffer Texture2D::GetWriteableBuffer()
	{
		return m_ImageData;
	}

	const std::filesystem::path &Texture2D::GetPath() const
	{
		return m_Path;
	}

	uint32_t Texture2D::GetMipLevelCount() const
	{
		return utils::CalculateMipCount(m_Specification.Width, m_Specification.Height);
	}

	std::pair<uint32_t, uint32_t> Texture2D::GetMipSize(uint32_t mip) const
	{
		uint32_t width  = m_Specification.Width;
		uint32_t height = m_Specification.Height;
		while (mip != 0)
		{
			width /= 2;
			height /= 2;
			mip--;
		}

		return {width, height};
	}

	void Texture2D::GenerateMips()
	{
		// SRGB/SRGBA formats cannot be used as storage images in compute shaders
		// Skip compute-based mip generation for these formats
		if (m_Specification.Format == ImageFormat::SRGB || m_Specification.Format == ImageFormat::SRGBA)
		{
			TST_WARN("Texture2D::GenerateMips - Skipping compute-based mip generation for SRGB/SRGBA format texture '{}'. "
					 "SRGB formats do not support storage image operations required for compute shaders.", m_Specification.DebugName);
			return;
		}

		RefPtr<CommandBuffer> renderCommandBuffer = make_reference<CommandBuffer>(1, std::format("Texture2D::GenerateMips - {}", m_Specification.DebugName));
		RefPtr<Shader>        shader = Renderer::getShaderLibrary()->get(utils::IsIntegerBased(m_Specification.Format) ? "LinearSampleUInt" : "LinearSample");
		ComputePassSpecInfo   spec{};
		spec.DebugName                  = "LinearSample";
		spec.Pipeline                   = make_reference<ComputePipeline>(shader);
		RefPtr<ComputePass> computePass = make_reference<ComputePass>(spec);
		renderCommandBuffer->beginRecording();
		RefPtr<Texture2D> instance = this;
		Renderer::submit([renderCommandBuffer, instance]()
		{
			TST_WARN("{}: Generating mips for format {}", instance->m_Specification.DebugName, utils::ImageFormatToString(instance->m_Specification.Format));

			nvrhi::CommandListHandle commandList = renderCommandBuffer->getActiveCommandBuffer();

			commandList->setTextureState(instance->m_Image->getImageInfo().ImageHandle, nvrhi::AllSubresources, nvrhi::ResourceStates::UnorderedAccess);
			commandList->commitBarriers();
		});
		Renderer::beginComputePass(renderCommandBuffer, computePass);
		struct PushConstants
		{
			tsm::vec2f TexelSize;
			int        SourceMip;
		}                 pushConstants;
		ImageViewSpecInfo srcImageViewSpec;
		srcImageViewSpec.Image             = m_Image;
		srcImageViewSpec.MipCount          = 1;
		ImageViewSpecInfo dstImageViewSpec = srcImageViewSpec;
		float             targetMipWidth   = static_cast<float>(m_Specification.Width);
		float             targetMipHeight  = static_cast<float>(m_Specification.Height);
		const auto        mipLevels        = GetMipLevelCount();
		for (uint32_t mip = 1; mip < mipLevels; mip++)
		{
			targetMipWidth /= 2;
			targetMipHeight /= 2;

			dstImageViewSpec.Mip = mip;
			srcImageViewSpec.Mip = mip - 1;

			RefPtr<ImageView> srcImageView = make_reference<ImageView>(srcImageViewSpec);
			RefPtr<ImageView> dstImageView = make_reference<ImageView>(dstImageViewSpec);

			RefPtr<Material> material = make_reference<Material>(shader);

			material->set("u_InputTexture", srcImageView);
			material->set("o_OutputTexture", dstImageView);

			pushConstants.TexelSize = {1.0f / targetMipWidth, 1.0f / targetMipHeight};
			pushConstants.SourceMip = mip - 1;

			tsm::vec3ui workGroups{static_cast<uint32>(targetMipWidth / 8), static_cast<uint32>(targetMipHeight / 8), 1};
			workGroups = tsm::max(workGroups, {1});
			Renderer::dispatchCompute(renderCommandBuffer, computePass, material, workGroups, Buffer(&pushConstants, sizeof(pushConstants)));

			Renderer::submit([renderCommandBuffer, instance]()
			{
				nvrhi::CommandListHandle commandList = renderCommandBuffer->getActiveCommandBuffer();
				commandList->commitBarriers();
			});
		}
		Renderer::endComputePass(renderCommandBuffer, computePass);
		Renderer::submit([renderCommandBuffer, instance]()
		{
			nvrhi::CommandListHandle commandList = renderCommandBuffer->getActiveCommandBuffer();

			commandList->setTextureState(instance->m_Image->getImageInfo().ImageHandle, nvrhi::AllSubresources, nvrhi::ResourceStates::ShaderResource);
			commandList->commitBarriers();
		});
		renderCommandBuffer->endRecording();
		renderCommandBuffer->submit();
	}

	void Texture2D::CopyToHostBuffer(Buffer &buffer)
	{
		if (m_Image)
			m_Image->copyToHostBuffer(buffer);
	}
}
