#pragma once
#include "ref_ptr.hpp"
#include "math/vector.hpp"
#include "buffer.hpp"

#include <nvrhi/nvrhi.h>

#include "command_buffer.hpp"
#include <map>

#include "render_resource.hpp"
#include "math/math_funcs.hpp"

namespace tst
{
	enum class ImageFormat
	{
		None = 0,
		RED8UN,
		RED8UI,
		RED16UI,
		RED32UI,
		RED32F,
		RG8,
		RG16F,
		RG32F,
		RGB,
		RGBA,
		RGBA16F,
		RGBA32F,
		B10R11G11UF,
		SRGB,
		SRGBA,
		DEPTH32FSTENCIL8UINT,
		DEPTH32F,
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8,
	};

	enum class ImageUsage
	{
		None = 0,
		Texture,
		Attachment,
		Storage,
		HostRead
	};

	enum class TextureWrap
	{
		None = 0, Clamp, Repeat
	};

	enum class TextureFilter
	{
		None = 0,
		Linear,
		Nearest,
		Cubic
	};

	enum class TextureType
	{
		None = 0, Texture2D, TextureCube
	};

	struct ImageSpecInfo
	{
		std::string DebugName;

		nvrhi::TextureDimension Dimension     = nvrhi::TextureDimension::Texture2D;
		ImageFormat             Format        = ImageFormat::RGBA;
		ImageUsage              Usage         = ImageUsage::Texture;
		bool                    Transfer      = false; // Will it be used for transfer ops?
		uint32_t                Width         = 1;
		uint32_t                Height        = 1;
		uint32_t                Mips          = 1;
		uint32_t                Layers        = 1;
		bool                    CreateSampler = true;
	};

	struct ImageInfo
	{
		nvrhi::TextureHandle         ImageHandle = nullptr;
		nvrhi::TextureSubresourceSet ImageView   = nvrhi::AllSubresources;
		nvrhi::SamplerHandle         Sampler     = nullptr;
		nvrhi::ResourceStates        State       = nvrhi::ResourceStates::Unknown;
		nvrhi::TextureDimension      Dimension   = nvrhi::TextureDimension::Unknown;
	};

	class Image : public RendererResource
	{
	public:
		virtual ~Image() = default;

		virtual void resize(const tsm::vec2ui &size) = 0;
		virtual void resize(const uint32_t width, const uint32_t height) = 0;
		virtual void invalidate() = 0;
		virtual void release() = 0;

		virtual nvrhi::TextureHandle getHandle() const = 0;

		virtual uint32_t    getWidth() const = 0;
		virtual uint32_t    getHeight() const = 0;
		virtual tsm::vec2ui getSize() const = 0;
		virtual bool        hasMips() const = 0;

		virtual float getAspectRatio() const = 0;

		virtual ImageSpecInfo &      getSpecInfo() = 0;
		virtual const ImageSpecInfo &getSpecInfo() const = 0;

		virtual Buffer  getBuffer() const = 0;
		virtual Buffer &getBuffer() = 0;

		virtual void setData(Buffer buffer) = 0;
		virtual void copyToHostBuffer(Buffer &buffer) const = 0;
	};

	class Image2D : public Image
	{
	public:
		bool isValid() const { return m_Info.ImageHandle != nullptr; }

		void resize(const tsm::vec2ui &size) override
		{
			resize(size.x, size.y);
		}

		void resize(const uint32_t width, const uint32_t height) override
		{
			m_specification.Width  = width;
			m_specification.Height = height;
			invalidate();
		}

		void invalidate() override;
		void release() override;

		nvrhi::TextureHandle getHandle() const override { return m_Info.ImageHandle; }
		uint32_t             getWidth() const override { return m_specification.Width; }
		uint32_t             getHeight() const override { return m_specification.Height; }
		tsm::vec2ui          getSize() const override { return {m_specification.Width, m_specification.Height}; }
		bool                 hasMips() const override { return m_specification.Mips > 1; }

		float getAspectRatio() const override { return (float) m_specification.Width / (float) m_specification.Height; }

		int                           getClosestMipLevel(uint32_t width, uint32_t height) const;
		std::pair<uint32_t, uint32_t> getMipLevelSize(int mipLevel) const;

		ImageSpecInfo &      getSpecInfo() override { return m_specification; }
		const ImageSpecInfo &getSpecInfo() const override { return m_specification; }

		void _invalidate();

		// virtual void CreatePerLayerImageViews() override;
		void _createPerLayerImageViews();
		void _createPerSpecificLayerImageViews(const std::vector<uint32_t> &layerIndices);

		virtual nvrhi::TextureSubresourceSet getLayerImageView(uint32_t layer)
		{
			TST_ASSERT(layer < m_PerLayerImageViews.size());
			return m_PerLayerImageViews[layer];
		}

		nvrhi::TextureSubresourceSet getMipImageView(uint32_t mip);

		ImageInfo &      getImageInfo() { return m_Info; }
		const ImageInfo &getImageInfo() const { return m_Info; }

		ResourceDescriptorInfo getDescriptorInfo() const override { return (ResourceDescriptorInfo) &m_Info; }

		Buffer  getBuffer() const override { return m_ImageData; }
		Buffer &getBuffer() override { return m_ImageData; }

		// virtual uint64_t GetHash() const override { return (uint64_t) m_Info.ImageHandle.Get(); }

		// Debug
		static const std::map<nvrhi::ITexture *, CachedPtr<Image2D> > &getImageRefs();

		virtual void setData(Buffer buffer) override;
		virtual void copyToHostBuffer(Buffer &buffer) const override;

		Image2D(const ImageSpecInfo &specification);

		virtual ~Image2D();

	private:
		ImageSpecInfo m_specification;

		Buffer m_ImageData;

		ImageInfo m_Info;
		uint64_t  m_GPUAllocationSize = 0;

		mutable RefPtr<CommandBuffer> m_CommandList;

		std::vector<nvrhi::TextureSubresourceSet>        m_PerLayerImageViews;
		std::map<uint32_t, nvrhi::TextureSubresourceSet> m_PerMipImageViews;
	};

	struct ImageViewSpecInfo
	{
		RefPtr<Image2D> Image;
		uint32_t        Mip        = 0;
		uint32_t        MipCount   = 0; // 0 means all
		uint32_t        Layer      = 0;
		uint32_t        LayerCount = 0; // 0 means all

		// Unknown will take dimension from Texture
		nvrhi::TextureDimension Dimension = nvrhi::TextureDimension::Unknown;

		std::string DebugName;
	};

	class ImageView : public RendererResource
	{
	public:
		void Invalidate();
		void RT_Invalidate();

		const nvrhi::TextureSubresourceSet &GetImageView() const { return m_TextureSubresourceSet; }
		virtual ResourceDescriptorInfo      getDescriptorInfo() const override { return (ResourceDescriptorInfo) &m_ImageInfo; }

	public:
		ImageView(const ImageViewSpecInfo &specification);

		virtual ~ImageView() = default;

	private:
		ImageViewSpecInfo m_Specification;
		ImageInfo         m_ImageInfo;

		nvrhi::TextureSubresourceSet m_TextureSubresourceSet;
	};

	struct SamplerSpecification
	{
		float                     MipBias     = 0.0f;
		nvrhi::SamplerAddressMode AddressMode = nvrhi::SamplerAddressMode::Clamp;
		bool                      MinFilter   = true;
		bool                      MagFilter   = true;
		bool                      MipFilter   = true;
	};

	class Sampler : public RendererResource
	{
	public:
		void Invalidate();

		nvrhi::SamplerHandle           getHandle() const { return m_handle; }
		virtual ResourceDescriptorInfo getDescriptorInfo() const override { return ResourceDescriptorInfo(this); }

	public:
		Sampler(const SamplerSpecification &specification = {});

		virtual ~Sampler() = default;

	private:
		SamplerSpecification m_Specification;

		nvrhi::SamplerHandle m_handle;
	};

	namespace utils
	{
		inline nvrhi::Format NVRHIFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RED8UN: return nvrhi::Format::R8_UNORM;
				case ImageFormat::RED8UI: return nvrhi::Format::R8_UINT;
				case ImageFormat::RED16UI: return nvrhi::Format::R16_UINT;
				case ImageFormat::RED32UI: return nvrhi::Format::R32_UINT;
				case ImageFormat::RED32F: return nvrhi::Format::R32_FLOAT;
				case ImageFormat::RG8: return nvrhi::Format::RG8_UNORM;
				case ImageFormat::RG16F: return nvrhi::Format::RG16_FLOAT;
				case ImageFormat::RG32F: return nvrhi::Format::RG32_FLOAT;
				case ImageFormat::RGBA: return nvrhi::Format::RGBA8_UNORM;
				case ImageFormat::SRGBA: return nvrhi::Format::SRGBA8_UNORM;
				case ImageFormat::RGBA16F: return nvrhi::Format::RGBA16_FLOAT;
				case ImageFormat::RGBA32F: return nvrhi::Format::RGBA32_FLOAT;
				case ImageFormat::B10R11G11UF: return nvrhi::Format::R11G11B10_FLOAT;
				case ImageFormat::DEPTH32FSTENCIL8UINT: return nvrhi::Format::D32S8;
				case ImageFormat::DEPTH32F: return nvrhi::Format::D32;
				case ImageFormat::DEPTH24STENCIL8: return nvrhi::Format::D24S8;
			}

			TST_ASSERT(false);
			return nvrhi::Format::UNKNOWN;
		}

		inline uint32_t GetImageFormatBPP(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RED8UN: return 1;
				case ImageFormat::RED8UI: return 1;
				case ImageFormat::RED16UI: return 2;
				case ImageFormat::RED32UI: return 4;
				case ImageFormat::RED32F: return 4;
				case ImageFormat::RGB:
				case ImageFormat::SRGB: return 3;
				case ImageFormat::RGBA: return 4;
				case ImageFormat::SRGBA: return 4;
				case ImageFormat::RGBA16F: return 2 * 4;
				case ImageFormat::RGBA32F: return 4 * 4;
				case ImageFormat::B10R11G11UF: return 4;
					return 4;
				case ImageFormat::DEPTH32FSTENCIL8UINT:
					return 8;
				case ImageFormat::DEPTH32F:
					return 4;
				case ImageFormat::DEPTH24STENCIL8:
					return 4;
			}
			TST_ASSERT(false);
			return 0;
		}

		inline bool IsIntegerBased(const ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RED16UI:
				case ImageFormat::RED32UI:
				case ImageFormat::RED8UI:
				case ImageFormat::DEPTH32FSTENCIL8UINT:
					return true;
				case ImageFormat::DEPTH32F:
				case ImageFormat::RED8UN:
				case ImageFormat::RGBA32F:
				case ImageFormat::B10R11G11UF:
				case ImageFormat::RG16F:
				case ImageFormat::RG32F:
				case ImageFormat::RED32F:
				case ImageFormat::RG8:
				case ImageFormat::RGBA:
				case ImageFormat::RGBA16F:
				case ImageFormat::RGB:
				case ImageFormat::SRGB:
				case ImageFormat::SRGBA:
				case ImageFormat::DEPTH24STENCIL8:
					return false;
			}
			TST_ASSERT(false);
			return false;
		}

		inline uint32_t CalculateMipCount(uint32_t width, uint32_t height)
		{
			return (uint32_t) tsm::floor(tsm::log2(tsm::min(width, height))) + 1;
		}

		inline uint32_t GetImageMemorySize(ImageFormat format, uint32_t width, uint32_t height)
		{
			return width * height * GetImageFormatBPP(format);
		}

		inline uint32_t GetImageMemoryRowPitch(ImageFormat format, uint32_t width)
		{
			return width * GetImageFormatBPP(format);
		}

		inline bool IsDepthFormat(ImageFormat format)
		{
			if (format == ImageFormat::DEPTH24STENCIL8 || format == ImageFormat::DEPTH32F || format == ImageFormat::DEPTH32FSTENCIL8UINT)
				return true;

			return false;
		}

		inline std::string_view ImageFormatToString(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::None: return "None";
				case ImageFormat::RED8UN: return "RED8UN";
				case ImageFormat::RED8UI: return "RED8UI";
				case ImageFormat::RED16UI: return "RED16UI";
				case ImageFormat::RED32UI: return "RED32UI";
				case ImageFormat::RED32F: return "RED32F";
				case ImageFormat::RG8: return "RG8";
				case ImageFormat::RG16F: return "RG16F";
				case ImageFormat::RG32F: return "RG32F";
				case ImageFormat::RGB: return "RGB";
				case ImageFormat::RGBA: return "RGBA";
				case ImageFormat::RGBA16F: return "RGBA16F";
				case ImageFormat::RGBA32F: return "RGBA32F";
				case ImageFormat::B10R11G11UF: return "B10R11G11UF";
				case ImageFormat::SRGB: return "SRGB";
				case ImageFormat::SRGBA: return "SRGBA";
				case ImageFormat::DEPTH32FSTENCIL8UINT: return "DEPTH32FSTENCIL8UINT";
				case ImageFormat::DEPTH32F: return "DEPTH32F";
				case ImageFormat::DEPTH24STENCIL8: return "DEPTH24STENCIL8";
			}

			return "<Unknown>";
		}
	}
}
