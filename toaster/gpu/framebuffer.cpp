#include "framebuffer.hpp"

#include "platform/application.hpp"
#include "renderer/renderer.hpp"

namespace tst
{
	Framebuffer::Framebuffer(const FramebufferSpecInfo &specification)
		: m_specification(specification)
	{
		if (specification.Width == 0)
		{
			m_width  = Application::getInstance().getMainWindow().getWidth();
			m_height = Application::getInstance().getMainWindow().getHeight();
		}
		else
		{
			m_width  = (uint32_t) (specification.Width * m_specification.Scale);
			m_height = (uint32_t) (specification.Height * m_specification.Scale);
		}

		// Create all image objects immediately so we can start referencing them
		// elsewhere
		uint32_t attachmentIndex = 0;
		if (!m_specification.ExistingFramebuffer)
		{
			for (auto &attachmentSpec: m_specification.Attachments.Attachments)
			{
				if (m_specification.ExistingImage)
				{
					if (utils::IsDepthFormat(attachmentSpec.Format))
						m_depthAttachmentImage = m_specification.ExistingImage;
					else
						m_attachmentImages.emplace_back(m_specification.ExistingImage);
				}
				else if (m_specification.ExistingImages.find(attachmentIndex) != m_specification.ExistingImages.end())
				{
					if (utils::IsDepthFormat(attachmentSpec.Format))
						m_depthAttachmentImage = m_specification.ExistingImages.at(attachmentIndex);
					else
						m_attachmentImages.emplace_back(); // This will be set later
				}
				else if (utils::IsDepthFormat(attachmentSpec.Format))
				{
					ImageSpecInfo spec;
					spec.Format = attachmentSpec.Format;
					spec.Usage = ImageUsage::Attachment;
					spec.Transfer = m_specification.transfer;
					spec.Width = (uint32_t) (m_width * m_specification.Scale);
					spec.Height = (uint32_t) (m_height * m_specification.Scale);
					spec.DebugName = std::format("{0}-DepthAttachment{1}", m_specification.DebugName.empty() ? "Unnamed FB" : m_specification.DebugName, attachmentIndex);
					m_depthAttachmentImage = make_reference<Image2D>(spec);
				}
				else
				{
					ImageSpecInfo spec;
					spec.Format    = attachmentSpec.Format;
					spec.Usage     = ImageUsage::Attachment;
					spec.Transfer  = m_specification.transfer;
					spec.Width     = (uint32_t) (m_width * m_specification.Scale);
					spec.Height    = (uint32_t) (m_height * m_specification.Scale);
					spec.DebugName = std::format("{0}-ColorAttachment{1}", m_specification.DebugName.empty() ? "Unnamed FB" : m_specification.DebugName, attachmentIndex);
					m_attachmentImages.emplace_back(make_reference<Image2D>(spec));
				}
				attachmentIndex++;
			}
		}

		TST_ASSERT(specification.Attachments.Attachments.size());
		resize(m_width, m_height, true);
	}

	Framebuffer::~Framebuffer()
	{
		release();
	}

	void Framebuffer::release()
	{

	}

	void Framebuffer::resize(uint32_t width, uint32_t height, bool forceRecreate)
	{
		if (!forceRecreate && (m_width == width && m_height == height))
			return;

		m_width  = (uint32_t) (width * m_specification.Scale);
		m_height = (uint32_t) (height * m_specification.Scale);

		if (m_specification.swapChainTarget)
		{
			// For swapchain targets, we don't recreate - just update dimensions
			// The swapchain itself handles recreation
			for (auto &callback: m_resizeCallbacks)
				callback(this);
			return;
		}

		if (!m_specification.swapChainTarget)
		{
			_invalidate();
		}
		else
		{
			TST_ASSERT(false);
		}

		for (auto &callback: m_resizeCallbacks)
			callback(this);
	}

	void Framebuffer::invalidate()
	{
		RefPtr<Framebuffer> instance = this;
		Renderer::submit([instance]() mutable
		{
			//instance->RT_Invalidate();
		});
		_invalidate();
	}

	void Framebuffer::_invalidate()
	{
		release();

		std::vector<nvrhi::FramebufferAttachment> attachmentDescriptions;

		m_clearValues.resize(m_specification.Attachments.Attachments.size());

		bool createImages = m_attachmentImages.empty();

		if (m_specification.ExistingFramebuffer)
			m_attachmentImages.clear();

		nvrhi::FramebufferDesc framebufferDesc;

		uint32_t attachmentIndex      = 0;
		uint32_t nvrhiAttachmentIndex = 0;
		for (const auto &attachmentSpec: m_specification.Attachments.Attachments)
		{
			if (utils::IsDepthFormat(attachmentSpec.Format))
			{
				if (m_specification.ExistingImage)
				{
					m_depthAttachmentImage = m_specification.ExistingImage;
				}
				else if (m_specification.ExistingFramebuffer)
				{
					RefPtr<Framebuffer> existingFramebuffer = m_specification.ExistingFramebuffer.as<Framebuffer>();
					m_depthAttachmentImage                  = existingFramebuffer->getDepthImage();
				}
				else if (m_specification.ExistingImages.find(attachmentIndex) != m_specification.ExistingImages.end())
				{
					RefPtr<Image2D> existingImage = m_specification.ExistingImages.at(attachmentIndex);
					TST_ASSERT(utils::IsDepthFormat(existingImage->getSpecInfo().Format));
					m_depthAttachmentImage = existingImage;
				}
				else
				{
					RefPtr<Image2D> depthAttachmentImage = m_depthAttachmentImage;
					auto &          spec                 = depthAttachmentImage->getSpecInfo();
					spec.Width                           = (uint32_t) (m_width * m_specification.Scale);
					spec.Height                          = (uint32_t) (m_height * m_specification.Scale);
					depthAttachmentImage->_invalidate(); // Create immediately
				}

				nvrhi::FramebufferAttachment &depthAttachment = framebufferDesc.depthAttachment;
				depthAttachment.texture                       = m_depthAttachmentImage->getHandle();
				depthAttachment.format                        = utils::NVRHIFormat(attachmentSpec.Format);
				if (m_specification.ExistingImageLayer != -1)
				{
					depthAttachment.subresources.baseArraySlice = m_specification.ExistingImageLayer;
					depthAttachment.subresources.numArraySlices = 1;
				}

				m_clearValues[attachmentIndex].DepthStencil = {m_specification.DepthClearValue, 0};
			}
			else
			{
				//HZ_CORE_ASSERT(!m_Specification.ExistingImage, "Not supported for color attachments");

				RefPtr<Image2D> colorAttachmentImage;
				if (m_specification.ExistingFramebuffer)
				{
					RefPtr<Framebuffer> existingFramebuffer = m_specification.ExistingFramebuffer.as<Framebuffer>();
					RefPtr<Image2D>     existingImage       = existingFramebuffer->getImage(attachmentIndex);
					colorAttachmentImage                    = m_attachmentImages.emplace_back(existingImage).as<Image2D>();
				}
				else if (m_specification.ExistingImages.find(attachmentIndex) != m_specification.ExistingImages.end())
				{
					RefPtr<Image2D> existingImage = m_specification.ExistingImages[attachmentIndex];
					TST_ASSERT(!utils::IsDepthFormat(existingImage->getSpecInfo().Format));
					colorAttachmentImage                = existingImage.as<Image2D>();
					m_attachmentImages[attachmentIndex] = existingImage;
				}
				else
				{
					if (createImages)
					{
						ImageSpecInfo spec;
						spec.Format          = attachmentSpec.Format;
						spec.Usage           = ImageUsage::Attachment;
						spec.Transfer        = m_specification.transfer;
						spec.Width           = (uint32_t) (m_width * m_specification.Scale);
						spec.Height          = (uint32_t) (m_height * m_specification.Scale);
						colorAttachmentImage = m_attachmentImages.emplace_back(make_reference<Image2D>(spec)).as<Image2D>();
						TST_ASSERT(false);
					}
					else
					{
						colorAttachmentImage = m_attachmentImages[attachmentIndex];
						ImageSpecInfo &spec  = colorAttachmentImage->getSpecInfo();
						spec.Width           = (uint32_t) (m_width * m_specification.Scale);
						spec.Height          = (uint32_t) (m_height * m_specification.Scale);
						colorAttachmentImage->_invalidate(); // Create immediately
					}
				}

				nvrhi::FramebufferAttachment &colorAttachment = framebufferDesc.colorAttachments.emplace_back();
				colorAttachment.texture                       = colorAttachmentImage->getHandle();
				colorAttachment.format                        = utils::NVRHIFormat(attachmentSpec.Format);
				if (m_specification.ExistingImageLayer != -1)
				{
					colorAttachment.subresources.baseArraySlice = m_specification.ExistingImageLayer;
					colorAttachment.subresources.numArraySlices = 1;
				}

				const auto &clearColor               = m_specification.ClearColor;
				m_clearValues[attachmentIndex].Color = {{clearColor.x, clearColor.y, clearColor.z, clearColor.w}};
			}

			attachmentIndex++;
		}

		nvrhi::DeviceHandle device = Application::getGraphicsDevice();
		m_handle                   = device->createFramebuffer(framebufferDesc);
		m_framebufferDesc          = framebufferDesc;
	}
}
