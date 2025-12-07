#pragma once
#include "image.hpp"
#include "ref_ptr.hpp"
#include "math/vector.hpp"

#include <filesystem/filesystem.hpp>

namespace tst
{
	struct TextureSpecInfo
	{
		ImageFormat   Format        = ImageFormat::RGBA;
		uint32_t      Width         = 1;
		uint32_t      Height        = 1;
		TextureWrap   SamplerWrap   = TextureWrap::Repeat;
		TextureFilter SamplerFilter = TextureFilter::Linear;

		bool GenerateMips = true;
		bool Storage      = false;
		bool StoreLocally = false;

		std::string DebugName;
	};

	class Texture : public RendererResource
	{
	public:
		virtual ~Texture() = default;

		virtual nvrhi::TextureHandle GetHandle() const = 0;

		virtual ImageFormat    GetFormat() const = 0;
		virtual uint32_t       GetWidth() const = 0;
		virtual uint32_t       GetHeight() const = 0;
		virtual tsm::vec2ui GetSize() const = 0;

		virtual uint32_t                      GetMipLevelCount() const = 0;
		virtual std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const = 0;

		virtual uint64_t GetHash() const = 0;

		virtual TextureType GetType() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		virtual ResourceDescriptorInfo getDescriptorInfo() const override { return m_Image->getDescriptorInfo(); }

		// reinterpret the given texture's data as if it was sRGB
		static RefPtr<Texture2D> CreateFromSRGB(RefPtr<Texture2D> texture);

		void CreateFromFile(const TextureSpecInfo &specification, const std::filesystem::path &filepath);
		void CreateFromBuffer(const TextureSpecInfo &specification, Buffer data = Buffer());

		void ReplaceFromFile(const TextureSpecInfo &specification, const std::filesystem::path &filepath);

		void Invalidate();

		void Resize(const tsm::vec2ui &size);
		void Resize(const uint32_t width, const uint32_t height);

		virtual nvrhi::TextureHandle GetHandle() const override { return m_Image->getHandle(); }
		virtual ImageFormat          GetFormat() const override { return m_Specification.Format; }
		virtual uint32_t             GetWidth() const override { return m_Specification.Width; }
		virtual uint32_t             GetHeight() const override { return m_Specification.Height; }
		virtual tsm::vec2ui       GetSize() const override { return {m_Specification.Width, m_Specification.Height}; }

		RefPtr<Image2D> GetImage() const { return m_Image; }

		void Lock();
		void Unlock();

		Buffer GetWriteableBuffer();

		virtual TextureType GetType() const override { return TextureType::Texture2D; }

		bool                                  Loaded() const { return m_Image && m_Image->isValid(); }
		const std::filesystem::path &         GetPath() const;
		uint32_t                              GetMipLevelCount() const override;
		virtual std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const override;

		void GenerateMips();
		void CopyToHostBuffer(Buffer &buffer);

		void SetData(Buffer buffer);

		// static AssetType  GetStaticType() { return AssetType::Texture; }
		// virtual AssetType GetAssetType() const override { return GetStaticType(); }

		virtual uint64_t GetHash() const { return (uint64_t) m_Image->getHandle().Get(); }

	public:
		Texture2D(const TextureSpecInfo &specification, const std::filesystem::path &filepath);
		Texture2D(const TextureSpecInfo &specification, Buffer data = Buffer());

		virtual ~Texture2D();

	private:
		TextureSpecInfo       m_Specification;
		std::filesystem::path m_Path;

		Buffer m_ImageData;

		RefPtr<Image2D> m_Image;
	};
}
