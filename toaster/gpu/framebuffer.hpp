#pragma once

#include <functional>
#include <string>

#include "image.hpp"
#include "ref_ptr.hpp"

namespace tst
{
	class Framebuffer;

	enum class EFramebufferBlendMode
	{
		eNone = 0,
		eOneZero,
		eSrcAlphaOneMinusSrcAlpha,
		eAdditive,
		eZero_SrcColor
	};

	enum class EAttachmentLoadOp
	{
		eInherit = 0, eClear = 1, eLoad = 2
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;

		FramebufferTextureSpecification(ImageFormat format) : Format(format)
		{
		}

		ImageFormat           Format;
		bool                  Blend     = true;
		EFramebufferBlendMode BlendMode = EFramebufferBlendMode::eSrcAlphaOneMinusSrcAlpha;
		EAttachmentLoadOp     LoadOp    = EAttachmentLoadOp::eInherit;
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;

		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification> &attachments)
			: Attachments(attachments)
		{
		}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecInfo
	{
		float         Scale            = 1.0f;
		uint32_t      Width            = 0;
		uint32_t      Height           = 0;
		tsm::vec4f ClearColor       = {0.0f, 0.0f, 0.0f, 1.0f};
		float         DepthClearValue  = 0.0f;
		bool          ClearColorOnLoad = true;
		bool          ClearDepthOnLoad = true;

		FramebufferAttachmentSpecification Attachments;
		uint32_t                           Samples = 1; // multisampling

		bool NoResize = false;

		// Master switch (individual attachments can be disabled in FramebufferTextureSpecification)
		bool Blend = true;
		// None means use BlendMode in FramebufferTextureSpecification
		EFramebufferBlendMode blendMode = EFramebufferBlendMode::eNone;

		// SwapChainTarget = screen buffer (i.e. no framebuffer)
		bool swapChainTarget = false;

		// Will it be used for transfer ops?
		bool transfer = false;

		// Note: these are used to attach multi-layered color/depth images
		RefPtr<Image2D> ExistingImage;
		uint32_t        ExistingImageLayer = (uint32_t) -1; // -1 means all (no specific layer)

		// Specify existing images to attach instead of creating
		// new images. attachment index -> image
		std::map<uint32_t, RefPtr<Image2D> > ExistingImages;

		// At the moment this will just create a new render pass
		// with an existing framebuffer
		RefPtr<Framebuffer> ExistingFramebuffer;

		std::string DebugName;
	};

	typedef union ClearColorValue
	{
		float    float32[4];
		int32_t  int32[4];
		uint32_t uint32[4];
	} ClearColorValue;

	typedef struct ClearDepthStencilValue
	{
		float    Depth;
		uint32_t Stencil;
	} ClearDepthStencilValue;

	typedef union ClearValue
	{
		ClearColorValue        Color;
		ClearDepthStencilValue DepthStencil;
	} ClearValue;

	class Framebuffer : public RefCounted
	{
	public:
		void resize(uint32_t width, uint32_t height, bool forceRecreate = false);
		void addResizeCallback(const std::function<void(RefPtr<Framebuffer>)> &func);

		uint32_t getWidth() const { return m_width; }
		uint32_t getHeight() const { return m_height; }

		RefPtr<Image2D> getImage(uint32_t attachmentIndex = 0) const
		{
			TST_ASSERT(attachmentIndex < m_attachmentImages.size());
			return m_attachmentImages[attachmentIndex];
		}

		RefPtr<Image2D> getDepthImage() const { return m_depthAttachmentImage; }
		size_t          getColorAttachmentCount() const { return m_specification.swapChainTarget ? 1 : m_attachmentImages.size(); }
		bool            hasDepthAttachment() const { return (bool) m_depthAttachmentImage; }

		nvrhi::FramebufferHandle      getHandle() const { return m_handle; }
		const nvrhi::FramebufferDesc &getFramebufferDesc() const { return m_framebufferDesc; }

		const std::vector<ClearValue> &getClearValues() const { return m_clearValues; }

		virtual const FramebufferSpecInfo &getSpecification() const { return m_specification; }

		void invalidate();
		void _invalidate();
		void release();

		Framebuffer(const FramebufferSpecInfo &spec);

		virtual ~Framebuffer();

	private:
		FramebufferSpecInfo m_specification;

		nvrhi::FramebufferHandle m_handle = nullptr;
		nvrhi::FramebufferDesc   m_framebufferDesc;

		uint32 m_width  = 0;
		uint32 m_height = 0;

		std::vector<RefPtr<Image2D> > m_attachmentImages;
		RefPtr<Image2D>               m_depthAttachmentImage;

		std::vector<ClearValue> m_clearValues;

		std::vector<std::function<void(RefPtr<Framebuffer>)> > m_resizeCallbacks;
	};
}
