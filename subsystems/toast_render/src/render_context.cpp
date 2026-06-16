#include "toast_render/render_context.hpp"
#include "toast_render/globals.hpp"

#include "toast_render/material.hpp"
#include "toast_render/mesh.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_lib/os/terminal.hpp"
#include "toast_render/compute_pass.hpp"
#include "toast_render/image.hpp"
#include "toast_render/renderer_2d.hpp"
#include "toast_render/render_pass.hpp"

#include "toast_render/shader_compiler.hpp"

namespace toaster::render
{
	#define TST_GET_VALID_CMD_BUFFER() gpu::VKCommandBuffer* command_buffer{(!p_command_buffer) ? getCurrentSwapchainCommandBuffer() : p_command_buffer}
	#define TST_GET_VALID_FRAME_INDEX() uint32 frame_index{(p_frame_index == UINT32_MAX) ? getCurrentFrameIndex() : p_frame_index}
	#define TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX() TST_GET_VALID_CMD_BUFFER(); TST_GET_VALID_FRAME_INDEX()

	RenderContext::RenderContext(const RenderContextSpecInfo &p_spec_info) : m_specInfo(p_spec_info)
	{
		bool use_present{m_specInfo.instanceExtensions.contains(VK_KHR_SURFACE_EXTENSION_NAME)};
		#pragma region create vulkan objects
		gpu::VKInstanceSpecInfo vk_instance_spec_info{};
		vk_instance_spec_info.appName            = "Toaster-2.0 -> Vulkan";
		vk_instance_spec_info.requiredExtensions = m_specInfo.instanceExtensions;
		vk_instance_spec_info.printDebugInfo     = m_specInfo.printDebugInfo;
		m_backendInstance                        = new gpu::VKInstance{vk_instance_spec_info};

		std::unordered_set<String> required_device_extensions{
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRMaintenance6ExtensionName,
			vk::KHRLoadStoreOpNoneExtensionName,
			vk::EXTShaderObjectExtensionName,
			vk::KHRBufferDeviceAddressExtensionName,
			vk::EXTDescriptorHeapExtensionName,
			vk::KHRShaderUntypedPointersExtensionName
		};
		if (use_present)
			required_device_extensions.insert(vk::KHRSwapchainExtensionName);

		gpu::VKPhysicalDeviceSpecInfo vk_physical_device_spec_info{};
		vk_physical_device_spec_info.requiredExtensions = required_device_extensions;
		vk_physical_device_spec_info.printDebugInfo     = m_specInfo.printDebugInfo;

		m_physicalDevice = new gpu::VKPhysicalDevice{m_backendInstance, vk_physical_device_spec_info};

		gpu::VKLogicalDeviceSpecInfo vk_logical_device_spec_info{};
		vk_logical_device_spec_info.usePresent           = use_present;
		vk_logical_device_spec_info.requiredExtensions   = required_device_extensions;
		vk_logical_device_spec_info.printDebugInfo       = m_specInfo.printDebugInfo;
		vk_logical_device_spec_info.printShaderDebugInfo = m_specInfo.printDebugInfo;
		auto features{gpu::VKLogicalDeviceSpecInfo::getDefaultFeatures()};
		vk_logical_device_spec_info.pNext = features.get<vk::PhysicalDeviceFeatures2>();

		m_logicalDevice = new gpu::VKLogicalDevice{m_physicalDevice, vk_logical_device_spec_info};
		#pragma endregion

		m_descriptorHeap = new gpu::VKDescriptorHeap{m_logicalDevice};

		const auto physical_device_props = m_physicalDevice->getVulkanPhysicalDevice().getProperties();
		{
			vk::SamplerCreateInfo default_sampler_create_info{};

			default_sampler_create_info.magFilter               = vk::Filter::eLinear;
			default_sampler_create_info.minFilter               = vk::Filter::eLinear;
			default_sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
			default_sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
			default_sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
			default_sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
			default_sampler_create_info.mipLodBias              = 0.0f;
			default_sampler_create_info.anisotropyEnable        = true;
			default_sampler_create_info.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
			default_sampler_create_info.compareEnable           = false;
			default_sampler_create_info.compareOp               = vk::CompareOp::eAlways;
			default_sampler_create_info.minLod                  = 0.0f;
			default_sampler_create_info.maxLod                  = vk::LodClampNone;
			default_sampler_create_info.borderColor             = vk::BorderColor::eFloatOpaqueWhite;
			default_sampler_create_info.unnormalizedCoordinates = false;

			m_samplers[ESamplerType::eDefault] = m_descriptorHeap->allocSampler(default_sampler_create_info);
		}
		{
			vk::SamplerCreateInfo nearest_sampler_create_info{};

			nearest_sampler_create_info.magFilter               = vk::Filter::eNearest;
			nearest_sampler_create_info.minFilter               = vk::Filter::eNearest;
			nearest_sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eNearest;
			nearest_sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
			nearest_sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
			nearest_sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
			nearest_sampler_create_info.mipLodBias              = 0.0f;
			nearest_sampler_create_info.anisotropyEnable        = true;
			nearest_sampler_create_info.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
			nearest_sampler_create_info.compareEnable           = false;
			nearest_sampler_create_info.compareOp               = vk::CompareOp::eAlways;
			nearest_sampler_create_info.minLod                  = 0.0f;
			nearest_sampler_create_info.maxLod                  = vk::LodClampNone;
			nearest_sampler_create_info.borderColor             = vk::BorderColor::eFloatOpaqueWhite;
			nearest_sampler_create_info.unnormalizedCoordinates = false;

			m_samplers[ESamplerType::eNearest] = m_descriptorHeap->allocSampler(nearest_sampler_create_info);
		}

		m_shaderCompiler = toaster::make_unique<ShaderCompiler>(*this);

		if (m_specInfo.createGlobals)
			m_globals = new Globals{*this, io::filesystem::exists(m_specInfo.sdkDir) ? m_specInfo.sdkDir : os::getBinaryDirectory()};
	}

	RenderContext::~RenderContext()
	{
		delete m_globals;

		delete m_descriptorHeap;

		// Clean up any remaining objects
		performGarbageCollection();

		delete m_logicalDevice;
		delete m_physicalDevice;
		delete m_backendInstance;
	}

	auto RenderContext::getBackendInstance() const -> gpu::VKInstance *
	{
		return m_backendInstance;
	}

	auto RenderContext::getPhysicalDevice() const -> gpu::VKPhysicalDevice *
	{
		return m_physicalDevice;
	}

	auto RenderContext::getLogicalDevice() const -> gpu::VKLogicalDevice *
	{
		return m_logicalDevice;
	}

	auto RenderContext::getDescriptorHeap() const -> gpu::VKDescriptorHeap *
	{
		return m_descriptorHeap;
	}

	auto RenderContext::getGlobals() const -> const Globals *
	{
		return m_globals;
	}

	auto RenderContext::gpuWaitIdle() const -> void
	{
		m_logicalDevice->getVulkanLogicalDevice().waitIdle();
	}

	auto RenderContext::getCurrentFrameIndex() const -> uint32
	{
		return m_logicalDevice->getCurrentFrameIndex();
	}

	auto RenderContext::setCurrentFrameIndex(uint32 p_index) -> void
	{
		TST_PERMA_ASSERT_MSG(p_index < maxFramesInFlight, "Index is out of bounds!");
		m_logicalDevice->setCurrentFrameIndex(p_index);
	}

	auto RenderContext::performGarbageCollection() const -> void
	{
		m_logicalDevice->performGarbageCollection();
	}

	auto RenderContext::getCurrentSwapchainCommandBuffer() const -> gpu::CommandBuffer *
	{
		return m_logicalDevice->getCurrentCommandBuffer();
	}

	auto RenderContext::setCurrentSwapchainCommandBuffer(gpu::CommandBuffer *p_cmd) -> void
	{
		m_logicalDevice->setCurrentCommandBuffer(p_cmd);
	}

	auto RenderContext::getSampler(ESamplerType p_type) const -> gpu::DescriptorSlot
	{
		return m_samplers.at(p_type);
	}

	auto RenderContext::createImageRef(const io::filesystem::Path &p_path) -> RefPtr<Image>
	{
		ImageSpecInfo image_spec_info{};
		Buffer        image_data{gpu::util::loadTextureIntoBuffer(p_path, image_spec_info.format, image_spec_info.size.x, image_spec_info.size.y)};
		if (!image_data)
			TST_PERMA_ASSERT(false);
		auto out_image{createRef<Image>(image_spec_info, image_data)}; // The image takes ownership of the image data from here...
		return out_image;
	}

	auto RenderContext::createImageUnique(const io::filesystem::Path &p_path) -> UniquePtr<Image>
	{
		ImageSpecInfo image_spec_info{};
		Buffer        image_data{gpu::util::loadTextureIntoBuffer(p_path, image_spec_info.format, image_spec_info.size.x, image_spec_info.size.y)};
		if (!image_data)
			TST_PERMA_ASSERT(false);
		auto out_image{createUnique<Image>(image_spec_info, image_data)}; // The image takes ownership of the image data from here...
		return std::move(out_image);
	}

	auto RenderContext::loadTextureIntoImage(const io::filesystem::Path &p_path) const -> gpu::RawImageHandle
	{
		gpu::ImageSpecInfo image_spec_info{};
		Buffer             texture_data{gpu::util::loadTextureIntoBuffer(p_path, image_spec_info.format, image_spec_info.size.x, image_spec_info.size.y)};
		if (!texture_data)
		{
			TST_ASSERT(false);
		}

		image_spec_info.usage       = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		image_spec_info.mipCount    = 1;
		image_spec_info.sampleCount = vk::SampleCountFlagBits::e1;
		auto out_image{createGPURef<gpu::RawImage>(image_spec_info)};

		gpu::util::toTransferDst(out_image.get());
		out_image->setData(texture_data);

		texture_data.release();

		m_logicalDevice->generateMipmaps(out_image->getImage(), {image_spec_info.size.x, image_spec_info.size.y, 1u}, 1);
		out_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Generate mips leaves the image in the eShaderReadOnlyOptimal layout

		return out_image;
	}

	auto RenderContext::createAttachmentImageRaw(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags, vk::Format p_format) const -> gpu::RawImageHandle
	{
		if (p_format == vk::Format::eUndefined)
			p_format = gpu::util::getDefaultFormat(p_image_aspect_flags);

		gpu::ImageSpecInfo attachment_image_spec_info{};
		attachment_image_spec_info.size   = p_size;
		attachment_image_spec_info.format = p_format;
		attachment_image_spec_info.usage  = gpu::util::getImageUsageFlags(p_image_aspect_flags);
		return createGPURef<gpu::RawImage>(attachment_image_spec_info);
	}

	auto RenderContext::createMultisampleAttachmentImage(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags, vk::Format p_format) const -> gpu::RawImageHandle
	{
		if (p_format == vk::Format::eUndefined)
			p_format = gpu::util::getDefaultFormat(p_image_aspect_flags);

		gpu::ImageSpecInfo attachment_image_spec_info{};
		attachment_image_spec_info.size        = p_size;
		attachment_image_spec_info.format      = p_format;
		attachment_image_spec_info.sampleCount = m_physicalDevice->getMaxUsableSampleCount();
		attachment_image_spec_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | gpu::util::getImageUsageFlags(p_image_aspect_flags);
		return createGPURef<gpu::RawImage>(attachment_image_spec_info);
	}

	auto RenderContext::createAttachmentImage(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags, vk::Format p_format) -> RefPtr<Image>
	{
		if (p_format == vk::Format::eUndefined)
			p_format = gpu::util::getDefaultFormat(p_image_aspect_flags);

		gpu::ImageSpecInfo image_spec_info{};
		image_spec_info.size  = p_size;
		image_spec_info.usage = gpu::util::getImageUsageFlags(p_image_aspect_flags);
		image_spec_info.usage |= vk::ImageUsageFlagBits::eSampled; // An attachment image is only used if it needs to be sampled from, else just create a raw image.

		image_spec_info.format = p_format;
		return createRef<Image>(createGPURef<gpu::RawImage>(image_spec_info));
	}

	auto RenderContext::createAttachmentTexture(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags, vk::Format p_format) const -> gpu::Texture2DHandle
	{
		if (p_format == vk::Format::eUndefined)
			p_format = gpu::util::getDefaultFormat(p_image_aspect_flags);

		gpu::TextureSpecInfo attachment_texture_spec_info{};
		attachment_texture_spec_info.size         = p_size;
		attachment_texture_spec_info.format       = p_format;
		attachment_texture_spec_info.generateMips = false;
		return createGPURef<gpu::Texture2D>(attachment_texture_spec_info);
	}

	auto RenderContext::createEnvironmentMapImage(const io::filesystem::Path &p_path) -> RefPtr<Image>
	{
		auto env_input{createImageRef(p_path)};

		constexpr uint32 skybox_resolution{2048};

		ImageSpecInfo env_output_spec_info{};
		env_output_spec_info.size       = {skybox_resolution};
		env_output_spec_info.format     = vk::Format::eR16G16B16A16Sfloat;
		env_output_spec_info.layerCount = 6u;
		env_output_spec_info.storage    = true;

		auto env_output{createRef<Image>(env_output_spec_info)};

		gpu::CommandBuffer command_buffer{m_logicalDevice, vk::QueueFlagBits::eCompute};
		command_buffer.begin();

		m_descriptorHeap->bind(&command_buffer);

		m_globals->dynamicShaderLibrary().get("Equirectangular_To_Cube_Map")->bind(&command_buffer);

		Globals::EquirectangularToCubeMapConstants equirectangular_to_cube_map_constants{};
		equirectangular_to_cube_map_constants.equirectangularMapId = env_input->getAlignedShaderReadHeapID();
		equirectangular_to_cube_map_constants.cubeMapId            = env_output->getAlignedStorageHeapID();
		equirectangular_to_cube_map_constants.samplerId            = m_samplers.at(ESamplerType::eDefault);

		command_buffer.pushData(equirectangular_to_cube_map_constants);
		command_buffer.getVulkanCommandBuffer().dispatch(skybox_resolution / 32, skybox_resolution / 32, 6);
		command_buffer.endAndSubmit();

		env_output->toShaderReadOptimal();

		return env_output;
	}

	auto RenderContext::createEnvironmentMap(const io::filesystem::Path &p_path) -> gpu::Texture3DHandle
	{
		auto env_tex{createGPURef<gpu::Texture2D>(gpu::TextureSpecInfo{}, p_path)};

		constexpr uint32     skybox_resolution{2048};
		gpu::TextureSpecInfo skybox_texture_map_spec_info{};
		skybox_texture_map_spec_info.size   = {skybox_resolution};
		skybox_texture_map_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
		RefPtr<gpu::Texture3D> env_map      = createGPURef<gpu::Texture3D>(skybox_texture_map_spec_info);

		auto equirectangular_to_cube_map_pipeline{createGPURef<gpu::ComputePipeline>(m_globals->shaderLibrary().get("Equirectangular_To_CubeMap"))};
		auto equirectangular_to_cube_map_pass{createRef<ComputePass>(equirectangular_to_cube_map_pipeline)};
		equirectangular_to_cube_map_pass->setInput("u_EquirectangularMap", env_tex).setInput("o_CubeMap", env_map).bake();

		gpu::CommandBuffer command_buffer{m_logicalDevice, vk::QueueFlagBits::eCompute};
		command_buffer.begin();

		beginCompute(equirectangular_to_cube_map_pass, &command_buffer, 0);
		dispatchCompute(equirectangular_to_cube_map_pass, nullptr, {skybox_resolution / 32, skybox_resolution / 32, 6}, &command_buffer, 0);

		command_buffer.endAndSubmit();
		return env_map;
	}

	auto RenderContext::createEnvironmentMap(const gpu::TextureSpecInfo &p_spec_info, const Buffer &p_data) -> gpu::Texture3DHandle
	{
		auto env_tex{make_reference<gpu::VKTexture2D>(m_logicalDevice, p_spec_info, p_data.data(), p_data.size())};

		constexpr uint32     skybox_resolution{2048};
		gpu::TextureSpecInfo skybox_texture_map_spec_info{};
		skybox_texture_map_spec_info.size   = {skybox_resolution};
		skybox_texture_map_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
		RefPtr<gpu::Texture3D> env_map      = createGPURef<gpu::Texture3D>(skybox_texture_map_spec_info);

		auto equirectangular_to_cube_map_pipeline{createGPURef<gpu::ComputePipeline>(m_globals->shaderLibrary().get("Equirectangular_To_CubeMap"))};
		auto equirectangular_to_cube_map_pass{createRef<ComputePass>(equirectangular_to_cube_map_pipeline)};
		equirectangular_to_cube_map_pass->setInput("u_EquirectangularMap", env_tex).setInput("o_CubeMap", env_map).bake();

		gpu::CommandBuffer command_buffer{m_logicalDevice, vk::QueueFlagBits::eCompute};
		command_buffer.begin();

		beginCompute(equirectangular_to_cube_map_pass, &command_buffer, 0);
		dispatchCompute(equirectangular_to_cube_map_pass, nullptr, {skybox_resolution / 32, skybox_resolution / 32, 6}, &command_buffer, 0);

		command_buffer.endAndSubmit();
		return env_map;
	}

	auto RenderContext::createDiffuseIrradianceMap(const gpu::Texture3DHandle &p_environment_map) -> gpu::Texture3DHandle
	{
		static constexpr uint32 c_diffuse_irradiance_resolution{32u};

		gpu::TextureSpecInfo irradiance_map_spec_info{};
		irradiance_map_spec_info.size   = {c_diffuse_irradiance_resolution};
		irradiance_map_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
		auto out_irradiance_map{createGPURef<gpu::Texture3D>(irradiance_map_spec_info)};

		auto irradiance_convolution_pipeline{createGPURef<gpu::ComputePipeline>(m_globals->shaderLibrary().get("Diffuse_Irradiance_Convolution"))};
		auto irradiance_convolution_pass{createRef<ComputePass>(irradiance_convolution_pipeline)};
		irradiance_convolution_pass->setInput("u_EnvironmentMap", p_environment_map).setInput("o_Irradiance", out_irradiance_map).bake();

		gpu::CommandBuffer command_buffer{m_logicalDevice, vk::QueueFlagBits::eCompute};
		command_buffer.begin();

		beginCompute(irradiance_convolution_pass, &command_buffer, 0);
		dispatchCompute(irradiance_convolution_pass, nullptr, {tsm::uint2{(c_diffuse_irradiance_resolution + 15u) / 16u}, 6u}, &command_buffer, 0);

		command_buffer.endAndSubmit();
		return out_irradiance_map;
	}

	auto RenderContext::createShader(const io::filesystem::Path &p_path, EShaderStage p_stage, EShaderStage p_next_stage,
									 EShaderLanguage             p_shader_lang) const -> gpu::DynamicShaderHandle
	{
		return m_shaderCompiler->compileToShaderFromPath(p_path, p_stage, p_next_stage, p_shader_lang);
	}

	auto RenderContext::createShaderFromSpirV(const io::filesystem::Path &p_spir_v_path, EShaderStage p_stage,
											  EShaderStage                p_next_stage) const -> gpu::DynamicShaderHandle
	{
		return createGPURef<gpu::DynamicShader>(io::filesystem::readBinary(p_spir_v_path), getVulkanShaderStage(p_stage), getVulkanShaderStage((p_next_stage)));
	}

	auto RenderContext::beginRenderPass(const RenderingInfo &p_rendering_info, RenderPass *p_render_pass, gpu::CommandBuffer *p_command_buffer,
										uint32               p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		std::vector<vk::RenderingAttachmentInfo> colour_rendering_attachment_infos{};
		for (const auto &rendering_attachment: p_rendering_info.colourAttachments)
		{
			auto &info{colour_rendering_attachment_infos.emplace_back()};

			auto image{rendering_attachment.image};
			if (rendering_attachment.image != nullptr)
			{
				info.imageView = image->getImageView();

				// Perform the layout transition on sampled attachment images
				if ((image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					gpu::util::transitionImageLayout(image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
													 command_buffer->getVulkanCommandBuffer());
				}

				info.imageLayout = image->getCurrentImageLayout();
			}
			else
			{
				info.imageView   = rendering_attachment.imageView;
				info.imageLayout = rendering_attachment.imageLayout;
			}

			auto resolve_image{rendering_attachment.resolveImage};
			if (resolve_image != nullptr)
			{
				info.resolveImageView = resolve_image->getImageView();

				// Perform the layout transition on sampled attachment images
				if ((resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						resolve_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					gpu::util::transitionImageLayout(resolve_image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
													 command_buffer->getVulkanCommandBuffer());
				}

				info.resolveImageLayout = resolve_image->getCurrentImageLayout();
			}
			else
			{
				info.resolveImageView   = rendering_attachment.resolveImageView;
				info.resolveImageLayout = rendering_attachment.resolveImageLayout;
			}

			info.resolveMode = rendering_attachment.resolveMode;

			info.loadOp     = getLoadOp(rendering_attachment.attachmentOp);
			info.storeOp    = getStoreOp(rendering_attachment.attachmentOp);
			info.clearValue = rendering_attachment.clearValue;
		}

		vk::RenderingAttachmentInfo depth_attachment_info{};
		if (p_rendering_info.depthAttachment.has_value())
		{
			auto depth_image{p_rendering_info.depthAttachment->image};
			if (depth_image != nullptr)
			{
				depth_attachment_info.imageView = depth_image->getImageView();

				// Perform the layout transition on sampled attachment images
				if ((depth_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						depth_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					gpu::util::transitionImageLayout(depth_image, vk::ImageLayout::eShaderReadOnlyOptimal,
													 p_rendering_info.depthReadOnly ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal,
													 command_buffer->getVulkanCommandBuffer());
				}

				depth_attachment_info.imageLayout = depth_image->getCurrentImageLayout();
			}
			else
			{
				depth_attachment_info.imageView   = p_rendering_info.depthAttachment->imageView;
				depth_attachment_info.imageLayout = p_rendering_info.depthAttachment->imageLayout;
			}

			auto depth_resolve_image{p_rendering_info.depthAttachment->resolveImage};
			if (depth_resolve_image != nullptr)
			{
				depth_attachment_info.resolveImageView = depth_resolve_image->getImageView();
				// Perform the layout transition on sampled attachment images
				if ((depth_resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						depth_resolve_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					gpu::util::transitionImageLayout(depth_resolve_image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthAttachmentOptimal,
													 command_buffer->getVulkanCommandBuffer());
				}

				depth_attachment_info.resolveImageLayout = depth_resolve_image->getCurrentImageLayout();
			}
			else
			{
				depth_attachment_info.resolveImageView   = p_rendering_info.depthAttachment->resolveImageView;
				depth_attachment_info.resolveImageLayout = p_rendering_info.depthAttachment->resolveImageLayout;
			}

			depth_attachment_info.resolveMode = p_rendering_info.depthAttachment->resolveMode;

			depth_attachment_info.loadOp     = getLoadOp(p_rendering_info.depthAttachment->attachmentOp);
			depth_attachment_info.storeOp    = getStoreOp(p_rendering_info.depthAttachment->attachmentOp);
			depth_attachment_info.clearValue = p_rendering_info.depthAttachment->clearValue;
		}

		vk::RenderingAttachmentInfo stencil_attachment_info{};
		if (p_rendering_info.pStencilAttachment != nullptr)
		{
			if (p_rendering_info.pStencilAttachment->image != nullptr)
			{
				stencil_attachment_info.imageView   = p_rendering_info.pStencilAttachment->image->getImageView();
				stencil_attachment_info.imageLayout = p_rendering_info.pStencilAttachment->image->getCurrentImageLayout();
			}
			else
			{
				stencil_attachment_info.imageView   = p_rendering_info.pStencilAttachment->imageView;
				stencil_attachment_info.imageLayout = p_rendering_info.pStencilAttachment->imageLayout;
			}

			if (p_rendering_info.pStencilAttachment->resolveImage != nullptr)
			{
				stencil_attachment_info.resolveImageView   = p_rendering_info.pStencilAttachment->resolveImage->getImageView();
				stencil_attachment_info.resolveImageLayout = p_rendering_info.pStencilAttachment->resolveImage->getCurrentImageLayout();
			}
			else
			{
				stencil_attachment_info.resolveImageView   = p_rendering_info.pStencilAttachment->resolveImageView;
				stencil_attachment_info.resolveImageLayout = p_rendering_info.pStencilAttachment->resolveImageLayout;
			}

			stencil_attachment_info.resolveMode = p_rendering_info.pStencilAttachment->resolveMode;

			stencil_attachment_info.loadOp     = getLoadOp(p_rendering_info.pStencilAttachment->attachmentOp);
			stencil_attachment_info.storeOp    = getStoreOp(p_rendering_info.pStencilAttachment->attachmentOp);
			stencil_attachment_info.clearValue = p_rendering_info.pStencilAttachment->clearValue;
		}

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea           = p_rendering_info.renderArea;
		rendering_info.layerCount           = p_rendering_info.layerCount;
		rendering_info.colorAttachmentCount = colour_rendering_attachment_infos.empty() ? 0u : p_rendering_info.colourAttachments.size();
		rendering_info.pColorAttachments    = colour_rendering_attachment_infos.empty() ? nullptr : colour_rendering_attachment_infos.data();
		rendering_info.pDepthAttachment     = p_rendering_info.depthAttachment.has_value() ? &depth_attachment_info : nullptr;
		rendering_info.pStencilAttachment   = p_rendering_info.pStencilAttachment ? &stencil_attachment_info : nullptr;

		command_buffer->getVulkanCommandBuffer().beginRendering(rendering_info);
		if (p_render_pass)
		{
			command_buffer->getVulkanCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipeline());

			command_buffer->getVulkanCommandBuffer().setViewport(0, p_rendering_info.getViewport());
			command_buffer->getVulkanCommandBuffer().setScissor(0, p_rendering_info.getScissor());

			p_render_pass->update(frame_index);

			const auto descriptor_sets = p_render_pass->getDescriptorSets(frame_index);
			if (!descriptor_sets.empty())
				command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipelineLayout(),
																			p_render_pass->getStartSetIndex(), descriptor_sets, nullptr);
		}
	}

	auto RenderContext::endRenderPass(const RenderingInfo &p_rendering_info, gpu::CommandBuffer *p_command_buffer) const -> void
	{
		TST_GET_VALID_CMD_BUFFER();

		command_buffer->getVulkanCommandBuffer().endRendering();

		// Perform the layout transition on sampled attachment images
		for (const auto &rendering_attachment: p_rendering_info.colourAttachments)
		{
			auto image{rendering_attachment.image};
			if ((image != nullptr) && (image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(image, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												 command_buffer->getVulkanCommandBuffer());
			}
			auto resolve_image{rendering_attachment.resolveImage};
			if ((resolve_image != nullptr) && (resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(resolve_image, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												 command_buffer->getVulkanCommandBuffer());
			}
		}

		if (p_rendering_info.depthAttachment.has_value())
		{
			auto depth_image{p_rendering_info.depthAttachment->image};

			if ((depth_image != nullptr) && (depth_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(depth_image,
												 p_rendering_info.depthReadOnly ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal,
												 vk::ImageLayout::eShaderReadOnlyOptimal, command_buffer->getVulkanCommandBuffer());
			}
			auto depth_resolve_image{p_rendering_info.depthAttachment->resolveImage};
			if ((depth_resolve_image != nullptr) && (depth_resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(depth_resolve_image, vk::ImageLayout::eDepthAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												 command_buffer->getVulkanCommandBuffer());
			}
		}
	}

	auto RenderContext::beginCompute(ComputePass *p_compute_pass, gpu::CommandBuffer *p_command_buffer, uint32 p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();
		command_buffer->getVulkanCommandBuffer().bindPipeline(vk::PipelineBindPoint::eCompute, *p_compute_pass->getPipeline());
		p_compute_pass->update(frame_index);

		const auto descriptor_sets = p_compute_pass->getDescriptorSets(frame_index);
		if (!descriptor_sets.empty())
			command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eCompute, p_compute_pass->getPipeline()->getPipelineLayout(),
																		p_compute_pass->getStartSetIndex(), descriptor_sets, nullptr);
	}

	auto RenderContext::dispatchCompute(const ComputePass *p_compute_pass, Material *p_material, const tsm::uint3 &p_work_groups, gpu::CommandBuffer *p_command_buffer,
										uint32             p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		if (p_material)
		{
			if (p_material->hasDescriptorSets())
				if (const auto descriptor_set{p_material->getDescriptorSet(frame_index)})
					command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eCompute, p_compute_pass->getPipeline()->getPipelineLayout(), 0,
																				descriptor_set, nullptr);

			const auto &push_constants{p_material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_compute_pass->getPipeline()->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eCompute;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = 0u;
				push_constants_info.pValues    = push_constants.data();

				command_buffer->getVulkanCommandBuffer().pushConstants2(push_constants_info);
			}
		}

		command_buffer->getVulkanCommandBuffer().dispatch(p_work_groups.x, p_work_groups.y, p_work_groups.z);
	}

	auto RenderContext::renderGeometry(gpu::Pipeline *p_pipeline, gpu::VertexBuffer *p_vertex_buffer, gpu::IndexBuffer *p_index_buffer, uint32 p_index_count,
									   Material *     p_material, Dx::FXMMATRIX p_transform, gpu::CommandBuffer *p_command_buffer, uint32 p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		// Push the constants
		Dx::XMFLOAT4X4 transform;
		Dx::XMStoreFloat4x4(&transform, p_transform);
		command_buffer->getVulkanCommandBuffer().pushConstants<Dx::XMFLOAT4X4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, transform);

		if (p_material)
		{
			if (p_material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{p_material->getDescriptorSet(frame_index)};
				command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set,
																			{});
			}

			const auto &push_constants{p_material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_pipeline->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eFragment;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = sizeof(tsm::float4x4);
				push_constants_info.pValues    = push_constants.data();

				command_buffer->getVulkanCommandBuffer().pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_vertex_buffer->bind(command_buffer);
		p_index_buffer->bind(command_buffer, vk::IndexType::eUint32);

		// Finally, draw indexed :)
		command_buffer->getVulkanCommandBuffer().drawIndexed(p_index_count, 1, 0, 0, 0);
	}

	auto RenderContext::renderFullscreenQuad(const RenderPass *p_render_pass, Material *p_material, gpu::CommandBuffer *p_command_buffer,
											 uint32            p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		if (p_material) // You technically don't need to use a material if you don't want to
		{
			if (p_material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{p_material->getDescriptorSet(frame_index)};
				command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipelineLayout(), 0,
																			material_descriptor_set, {});
			}

			const auto &push_constants{p_material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_render_pass->getPipeline()->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eFragment;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = 0u;
				push_constants_info.pValues    = push_constants.data();
				command_buffer->getVulkanCommandBuffer().pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		m_globals->fullscreenQuadVertexBuffer()->bind(command_buffer);
		m_globals->fullscreenQuadIndexBuffer()->bind(command_buffer, vk::IndexType::eUint32);

		// Finally, draw indexed :)
		command_buffer->getVulkanCommandBuffer().drawIndexed(m_globals->fullscreenQuadIndices().size(), 1, 0, 0, 0);
	}

	auto RenderContext::renderMesh(const MeshData *    p_mesh, uint32           p_submesh_index, gpu::Pipeline *p_pipeline, Dx::FXMMATRIX p_transform,
								   gpu::CommandBuffer *p_command_buffer, uint32 p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		// Push the constants
		Dx::XMFLOAT4X4 transform;
		Dx::XMStoreFloat4x4(&transform, p_transform);
		command_buffer->getVulkanCommandBuffer().pushConstants<Dx::XMFLOAT4X4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, transform);

		const auto &submesh{p_mesh->getSubmeshes()[p_submesh_index]};
		auto        material{p_mesh->getMaterials().getMaterial(submesh.materialIndex).material};

		if (material) // You technically don't need to use a material if you don't want to
		{
			if (material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{material->getDescriptorSet(frame_index)};
				command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set,
																			{});
			}

			const auto &push_constants{material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_pipeline->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eFragment;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = sizeof(tsm::float4x4);
				push_constants_info.pValues    = push_constants.data();

				command_buffer->getVulkanCommandBuffer().pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_mesh->getVertexBuffer()->bind(command_buffer);
		p_mesh->getIndexBuffer()->bind(command_buffer, vk::IndexType::eUint32);

		// Finally, draw indexed :)
		command_buffer->getVulkanCommandBuffer().drawIndexed(submesh.indexCount, 1, submesh.baseIndex, submesh.baseVertex, 0);
	}

	auto RenderContext::renderMesh(const MeshData *p_mesh, uint32 p_submesh_index, gpu::Pipeline *p_pipeline, Dx::FXMMATRIX p_transform, Material *p_override_material,
								   gpu::CommandBuffer *p_command_buffer, uint32 p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		// Push the constants
		Dx::XMFLOAT4X4 transform;
		Dx::XMStoreFloat4x4(&transform, p_transform);
		command_buffer->getVulkanCommandBuffer().pushConstants<Dx::XMFLOAT4X4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, transform);

		const auto &submesh{p_mesh->getSubmeshes()[p_submesh_index]};

		if (p_override_material) // You technically don't need to use a material if you don't want to
		{
			if (p_override_material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{p_override_material->getDescriptorSet(frame_index)};
				command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set,
																			{});
			}

			const auto &push_constants{p_override_material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_pipeline->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eFragment;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = sizeof(tsm::float4x4);
				push_constants_info.pValues    = push_constants.data();

				command_buffer->getVulkanCommandBuffer().pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_mesh->getVertexBuffer()->bind(command_buffer);
		p_mesh->getIndexBuffer()->bind(command_buffer, vk::IndexType::eUint32);

		// Finally, draw indexed :)
		command_buffer->getVulkanCommandBuffer().drawIndexed(submesh.indexCount, 1, submesh.baseIndex, static_cast<int32>(submesh.baseVertex), 0);
	}

	auto RenderContext::beginRendering(const RenderingInfo &p_rendering_info, gpu::CommandBuffer *p_command_buffer, uint32 p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		command_buffer->setRenderArea(p_rendering_info.renderArea);

		std::vector<vk::RenderingAttachmentInfo> colour_rendering_attachment_infos{};
		for (const auto &rendering_attachment: p_rendering_info.colourAttachments)
		{
			auto &info{colour_rendering_attachment_infos.emplace_back()};

			auto image{rendering_attachment.image};
			if (rendering_attachment.image != nullptr)
			{
				info.imageView = image->getImageView();

				// Perform the layout transition on sampled attachment images
				if ((image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					gpu::util::transitionImageLayout(image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
													 command_buffer->getVulkanCommandBuffer());
				}

				info.imageLayout = image->getCurrentImageLayout();
			}
			else
			{
				info.imageView   = rendering_attachment.imageView;
				info.imageLayout = rendering_attachment.imageLayout;
			}

			auto resolve_image{rendering_attachment.resolveImage};
			if (resolve_image != nullptr)
			{
				info.resolveImageView = resolve_image->getImageView();

				// Perform the layout transition on sampled attachment images
				if ((resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						resolve_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					gpu::util::transitionImageLayout(resolve_image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
													 command_buffer->getVulkanCommandBuffer());
				}

				info.resolveImageLayout = resolve_image->getCurrentImageLayout();
			}
			else
			{
				info.resolveImageView   = rendering_attachment.resolveImageView;
				info.resolveImageLayout = rendering_attachment.resolveImageLayout;
			}

			info.resolveMode = rendering_attachment.resolveMode;

			info.loadOp     = getLoadOp(rendering_attachment.attachmentOp);
			info.storeOp    = getStoreOp(rendering_attachment.attachmentOp);
			info.clearValue = rendering_attachment.clearValue;
		}

		vk::RenderingAttachmentInfo depth_attachment_info{};
		if (p_rendering_info.depthAttachment.has_value())
		{
			auto depth_image{p_rendering_info.depthAttachment->image};
			if (depth_image != nullptr)
			{
				depth_attachment_info.imageView = depth_image->getImageView();

				// Perform the layout transition on sampled attachment images
				if ((depth_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						depth_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					gpu::util::transitionImageLayout(depth_image, vk::ImageLayout::eShaderReadOnlyOptimal,
													 p_rendering_info.depthReadOnly ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal,
													 command_buffer->getVulkanCommandBuffer());
				}

				depth_attachment_info.imageLayout = depth_image->getCurrentImageLayout();
			}
			else
			{
				depth_attachment_info.imageView   = p_rendering_info.depthAttachment->imageView;
				depth_attachment_info.imageLayout = p_rendering_info.depthAttachment->imageLayout;
			}

			auto depth_resolve_image{p_rendering_info.depthAttachment->resolveImage};
			if (depth_resolve_image != nullptr)
			{
				depth_attachment_info.resolveImageView = depth_resolve_image->getImageView();
				// Perform the layout transition on sampled attachment images
				if ((depth_resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						depth_resolve_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					gpu::util::transitionImageLayout(depth_resolve_image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthAttachmentOptimal,
													 command_buffer->getVulkanCommandBuffer());
				}

				depth_attachment_info.resolveImageLayout = depth_resolve_image->getCurrentImageLayout();
			}
			else
			{
				depth_attachment_info.resolveImageView   = p_rendering_info.depthAttachment->resolveImageView;
				depth_attachment_info.resolveImageLayout = p_rendering_info.depthAttachment->resolveImageLayout;
			}

			depth_attachment_info.resolveMode = p_rendering_info.depthAttachment->resolveMode;

			depth_attachment_info.loadOp     = getLoadOp(p_rendering_info.depthAttachment->attachmentOp);
			depth_attachment_info.storeOp    = getStoreOp(p_rendering_info.depthAttachment->attachmentOp);
			depth_attachment_info.clearValue = p_rendering_info.depthAttachment->clearValue;
		}

		vk::RenderingAttachmentInfo stencil_attachment_info{};
		if (p_rendering_info.pStencilAttachment != nullptr)
		{
			if (p_rendering_info.pStencilAttachment->image != nullptr)
			{
				stencil_attachment_info.imageView   = p_rendering_info.pStencilAttachment->image->getImageView();
				stencil_attachment_info.imageLayout = p_rendering_info.pStencilAttachment->image->getCurrentImageLayout();
			}
			else
			{
				stencil_attachment_info.imageView   = p_rendering_info.pStencilAttachment->imageView;
				stencil_attachment_info.imageLayout = p_rendering_info.pStencilAttachment->imageLayout;
			}

			if (p_rendering_info.pStencilAttachment->resolveImage != nullptr)
			{
				stencil_attachment_info.resolveImageView   = p_rendering_info.pStencilAttachment->resolveImage->getImageView();
				stencil_attachment_info.resolveImageLayout = p_rendering_info.pStencilAttachment->resolveImage->getCurrentImageLayout();
			}
			else
			{
				stencil_attachment_info.resolveImageView   = p_rendering_info.pStencilAttachment->resolveImageView;
				stencil_attachment_info.resolveImageLayout = p_rendering_info.pStencilAttachment->resolveImageLayout;
			}

			stencil_attachment_info.resolveMode = p_rendering_info.pStencilAttachment->resolveMode;

			stencil_attachment_info.loadOp     = getLoadOp(p_rendering_info.pStencilAttachment->attachmentOp);
			stencil_attachment_info.storeOp    = getStoreOp(p_rendering_info.pStencilAttachment->attachmentOp);
			stencil_attachment_info.clearValue = p_rendering_info.pStencilAttachment->clearValue;
		}

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea           = p_rendering_info.renderArea;
		rendering_info.layerCount           = p_rendering_info.layerCount;
		rendering_info.colorAttachmentCount = colour_rendering_attachment_infos.empty() ? 0u : p_rendering_info.colourAttachments.size();
		rendering_info.pColorAttachments    = colour_rendering_attachment_infos.empty() ? nullptr : colour_rendering_attachment_infos.data();
		rendering_info.pDepthAttachment     = p_rendering_info.depthAttachment.has_value() ? &depth_attachment_info : nullptr;
		rendering_info.pStencilAttachment   = p_rendering_info.pStencilAttachment ? &stencil_attachment_info : nullptr;

		command_buffer->getVulkanCommandBuffer().beginRendering(rendering_info);
	}

	auto RenderContext::endRendering(const RenderingInfo &p_rendering_info, gpu::CommandBuffer *p_command_buffer, uint32 p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		command_buffer->getVulkanCommandBuffer().endRendering();

		// Perform the layout transition on sampled attachment images
		for (const auto &rendering_attachment: p_rendering_info.colourAttachments)
		{
			auto image{rendering_attachment.image};
			if ((image != nullptr) && (image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(image, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												 command_buffer->getVulkanCommandBuffer());
			}
			auto resolve_image{rendering_attachment.resolveImage};
			if ((resolve_image != nullptr) && (resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(resolve_image, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												 command_buffer->getVulkanCommandBuffer());
			}
		}

		if (p_rendering_info.depthAttachment.has_value())
		{
			auto depth_image{p_rendering_info.depthAttachment->image};

			if ((depth_image != nullptr) && (depth_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(depth_image,
												 p_rendering_info.depthReadOnly ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal,
												 vk::ImageLayout::eShaderReadOnlyOptimal, command_buffer->getVulkanCommandBuffer());
			}
			auto depth_resolve_image{p_rendering_info.depthAttachment->resolveImage};
			if ((depth_resolve_image != nullptr) && (depth_resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(depth_resolve_image, vk::ImageLayout::eDepthAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												 command_buffer->getVulkanCommandBuffer());
			}
		}
	}

	auto RenderContext::renderSubmesh(const DynamicMesh *p_mesh, uint32 p_submesh_index, uint64 p_push_constant_offset, gpu::CommandBuffer *p_command_buffer,
									  uint32             p_frame_index) -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		auto &submesh{p_mesh->getSubmeshes()[p_submesh_index]};

		auto &material{p_mesh->getMaterials().getMaterial(submesh.materialIndex)};

		MeshPushConstants pcs{};
		pcs.albedoMap    = material.albedoMap->getAlignedShaderReadHeapID();
		pcs.normalMap    = material.normalMap->getAlignedShaderReadHeapID();
		pcs.hasNormalMap = material.hasNormalMap;
		pcs.albedoColour = {material.albedoColour, 1.0f};
		pcs.roughness    = material.roughness;
		pcs.metalness    = material.metalness;

		pcs.samplerIndex = m_samplers.at(ESamplerType::eDefault);
		pcs.model        = submesh.transform;

		command_buffer->pushData(pcs, p_push_constant_offset);

		p_mesh->getVertexBuffer()->bind(command_buffer);
		p_mesh->getIndexBuffer()->bind(command_buffer);

		command_buffer->drawIndexed(submesh.indexCount, 1, submesh.baseIndex, static_cast<int32>(submesh.baseVertex), 0);
	}

	auto RenderContext::renderFullscreenQuad(gpu::CommandBuffer *p_command_buffer, uint32 p_frame_index) const -> void
	{
		TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX();

		m_globals->fullscreenQuadVertexBuffer()->bind(command_buffer);
		m_globals->fullscreenQuadIndexBuffer()->bind(command_buffer);

		command_buffer->drawIndexed(m_globals->fullscreenQuadIndices().size());
	}

	#undef TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX
}
