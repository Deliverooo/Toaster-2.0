#include "toast_render/render_context.hpp"
#include "toast_render/globals.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_lib/os/terminal.hpp"
#include "toast_render/image.hpp"

#include "toast_render/shader_compiler.hpp"
#include "toast_render/shader_reflection.hpp"

namespace toaster::render
{
	#define TST_GET_VALID_CMD_BUFFER() gpu::VKCommandBuffer* command_buffer{(!p_command_buffer) ? getCurrentCommandBuffer() : p_command_buffer}
	#define TST_GET_VALID_FRAME_INDEX() uint32 frame_index{(p_frame_index == UINT32_MAX) ? getCurrentFrameIndex() : p_frame_index}
	#define TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX() TST_GET_VALID_CMD_BUFFER(); TST_GET_VALID_FRAME_INDEX()

	RenderContext::RenderContext(const RenderContextSpecInfo &p_spec_info) : m_specInfo(p_spec_info)
	{
		gpu::GPUContextSpecInfo gpu_context_spec_info{};
		gpu_context_spec_info.printDebugInfo     = m_specInfo.printDebugInfo;
		gpu_context_spec_info.instanceExtensions = m_specInfo.instanceExtensions;
		m_gpuCtx                                 = toaster::make_unique<gpu::VKGPUContext>(gpu_context_spec_info);

		const auto physical_device_props = m_gpuCtx->getPhysicalDevice()->getVulkanPhysicalDevice().getProperties();
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

			m_samplers[ESamplerType::eDefault] = m_gpuCtx->getDescriptorHeap()->allocSampler(default_sampler_create_info);
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

			m_samplers[ESamplerType::eNearest] = m_gpuCtx->getDescriptorHeap()->allocSampler(nearest_sampler_create_info);
		}

		{
			vk::SamplerCreateInfo irradiance_sampler_create_info{};

			irradiance_sampler_create_info.magFilter               = vk::Filter::eLinear;
			irradiance_sampler_create_info.minFilter               = vk::Filter::eLinear;
			irradiance_sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
			irradiance_sampler_create_info.addressModeU            = vk::SamplerAddressMode::eClampToEdge;
			irradiance_sampler_create_info.addressModeV            = vk::SamplerAddressMode::eClampToEdge;
			irradiance_sampler_create_info.addressModeW            = vk::SamplerAddressMode::eClampToEdge;
			irradiance_sampler_create_info.mipLodBias              = 0.0f;
			irradiance_sampler_create_info.anisotropyEnable        = false;
			irradiance_sampler_create_info.maxAnisotropy           = 1.0f;
			irradiance_sampler_create_info.compareEnable           = false;
			irradiance_sampler_create_info.compareOp               = vk::CompareOp::eNever;
			irradiance_sampler_create_info.minLod                  = 0.0f;
			irradiance_sampler_create_info.maxLod                  = 0.0f;
			irradiance_sampler_create_info.borderColor             = vk::BorderColor::eFloatOpaqueWhite;
			irradiance_sampler_create_info.unnormalizedCoordinates = false;

			m_samplers[ESamplerType::eIrradianceMap] = m_gpuCtx->getDescriptorHeap()->allocSampler(irradiance_sampler_create_info);
		}

		{
			vk::SamplerCreateInfo brdf_lut_sampler_create_info{};
			brdf_lut_sampler_create_info.magFilter               = vk::Filter::eLinear;
			brdf_lut_sampler_create_info.minFilter               = vk::Filter::eLinear;
			brdf_lut_sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eNearest;
			brdf_lut_sampler_create_info.addressModeU            = vk::SamplerAddressMode::eClampToEdge;
			brdf_lut_sampler_create_info.addressModeV            = vk::SamplerAddressMode::eClampToEdge;
			brdf_lut_sampler_create_info.addressModeW            = vk::SamplerAddressMode::eClampToEdge;
			brdf_lut_sampler_create_info.mipLodBias              = 0.0f;
			brdf_lut_sampler_create_info.anisotropyEnable        = false;
			brdf_lut_sampler_create_info.maxAnisotropy           = 1.0f;
			brdf_lut_sampler_create_info.compareEnable           = false;
			brdf_lut_sampler_create_info.compareOp               = vk::CompareOp::eNever;
			brdf_lut_sampler_create_info.minLod                  = 0.0f;
			brdf_lut_sampler_create_info.maxLod                  = 0.0f;
			brdf_lut_sampler_create_info.borderColor             = vk::BorderColor::eFloatOpaqueBlack;
			brdf_lut_sampler_create_info.unnormalizedCoordinates = false;

			m_samplers[ESamplerType::eBRDFLUT] = m_gpuCtx->getDescriptorHeap()->allocSampler(brdf_lut_sampler_create_info);
		}

		m_shaderCompiler = toaster::make_unique<ShaderCompiler>(*this);

		if (m_specInfo.createGlobals)
		{
			GlobalsSpecInfo globals_spec_info{};
			globals_spec_info.shaderBinaryDir = io::filesystem::exists(m_specInfo.sdkDir) ? m_specInfo.sdkDir / "shaders" : os::getBinaryDirectory() / "shaders";
			m_globals                         = new Globals{*this, globals_spec_info};

			m_globals->reflectShader("Dynamic_Mesh_PS"); // Create the necessary reflection data for the PBR material
			m_globals->reflectShader("Default_Unlit_PS");
			m_globals->reflectShader("Fullscreen_Quad_PS");
		}
	}

	RenderContext::~RenderContext()
	{
		delete m_globals;

		m_gpuCtx.reset(nullptr);
	}

	auto RenderContext::getBackendInstance() const -> gpu::VKInstance *
	{
		return m_gpuCtx->getBackendInstance();
	}

	auto RenderContext::getPhysicalDevice() const -> gpu::VKPhysicalDevice *
	{
		return m_gpuCtx->getPhysicalDevice();
	}

	auto RenderContext::getLogicalDevice() const -> gpu::VKLogicalDevice *
	{
		return m_gpuCtx->getLogicalDevice();
	}

	auto RenderContext::getDescriptorHeap() const -> gpu::VKDescriptorHeap *
	{
		return m_gpuCtx->getDescriptorHeap();
	}

	auto RenderContext::getGPUContext() const -> gpu::VKGPUContext *
	{
		return m_gpuCtx.get();
	}

	auto RenderContext::getGlobals() const -> const Globals *
	{
		return m_globals;
	}

	auto RenderContext::gpuWaitIdle() const -> void
	{
		m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice().waitIdle();
	}

	auto RenderContext::getCurrentFrameIndex() const -> uint32
	{
		return m_gpuCtx->getCurrentFrameIndex();
	}

	auto RenderContext::setCurrentFrameIndex(uint32 p_index) -> void
	{
		TST_PERMA_ASSERT_MSG(p_index < maxFramesInFlight, "Index is out of bounds!");
		m_gpuCtx->setCurrentFrameIndex(p_index);
	}

	auto RenderContext::performGarbageCollection() const -> void
	{
		m_gpuCtx->performGarbageCollection();
	}

	auto RenderContext::getCurrentCommandBuffer() const -> gpu::CommandBuffer *
	{
		return m_gpuCtx->getCurrentCommandBuffer();
	}

	auto RenderContext::setCurrentCommandBuffer(gpu::CommandBuffer *p_cmd) -> void
	{
		m_gpuCtx->setCurrentCommandBuffer(p_cmd);
	}

	auto RenderContext::getSampler(ESamplerType p_type) const -> gpu::DescriptorSlot
	{
		return m_samplers.at(p_type);
	}

	auto RenderContext::createImageRef(const io::filesystem::Path &p_path) -> RefPtr<Image>
	{
		ImageSpecInfo image_spec_info{};
		image_spec_info.generateMipmaps = true;
		Buffer image_data{gpu::util::loadTextureIntoBuffer(p_path, image_spec_info.format, image_spec_info.size.x, image_spec_info.size.y)};
		if (!image_data)
		{
			LOG_ERROR("Failed to load image: {}", p_path);
			return m_globals->debugImage();
		}

		auto out_image{createRef<Image>(image_spec_info, image_data)}; // The image takes ownership of the image data from here...
		out_image->generateMipmaps();
		return out_image;
	}

	auto RenderContext::createImageUnique(const io::filesystem::Path &p_path) -> UniquePtr<Image>
	{
		ImageSpecInfo image_spec_info{};
		image_spec_info.generateMipmaps = true;
		Buffer image_data{gpu::util::loadTextureIntoBuffer(p_path, image_spec_info.format, image_spec_info.size.x, image_spec_info.size.y)};
		if (!image_data)
		{
			LOG_ERROR("Failed to load image: {}", p_path);
			return nullptr;
		}
		auto out_image{createUnique<Image>(image_spec_info, image_data)}; // The image takes ownership of the image data from here...
		out_image->generateMipmaps();
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

		m_gpuCtx->generateMipmaps(out_image->getImage(), {image_spec_info.size.x, image_spec_info.size.y, 1u}, 1);
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
		attachment_image_spec_info.sampleCount = m_gpuCtx->getPhysicalDevice()->getMaxUsableSampleCount();
		attachment_image_spec_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | gpu::util::getImageUsageFlags(p_image_aspect_flags);
		return createGPURef<gpu::RawImage>(attachment_image_spec_info);
	}

	auto RenderContext::createMultisampleAttachmentImageUnique(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags,
															   vk::Format p_format) const -> gpu::RawImageUnique
	{
		if (p_format == vk::Format::eUndefined)
			p_format = gpu::util::getDefaultFormat(p_image_aspect_flags);

		gpu::ImageSpecInfo attachment_image_spec_info{};
		attachment_image_spec_info.size        = p_size;
		attachment_image_spec_info.format      = p_format;
		attachment_image_spec_info.sampleCount = m_gpuCtx->getPhysicalDevice()->getMaxUsableSampleCount();
		attachment_image_spec_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | gpu::util::getImageUsageFlags(p_image_aspect_flags);
		return createGPUUnique<gpu::RawImage>(attachment_image_spec_info);
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

		gpu::CommandBuffer command_buffer{*m_gpuCtx, vk::QueueFlagBits::eCompute};
		command_buffer.begin();

		m_gpuCtx->bindDescriptorHeap(&command_buffer);

		command_buffer.bindShaders({m_globals->getShader("Equirectangular_To_Cube_Map")});

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

	auto RenderContext::createDiffuseIrradianceMapImage(const RefPtr<Image> &p_environment_map) -> RefPtr<Image>
	{
		static constexpr uint32 c_diffuse_irradiance_resolution{32u};

		ImageSpecInfo env_output_spec_info{};
		env_output_spec_info.size       = {c_diffuse_irradiance_resolution};
		env_output_spec_info.format     = vk::Format::eR16G16B16A16Sfloat;
		env_output_spec_info.layerCount = 6u;
		env_output_spec_info.storage    = true;

		auto out_irradiance_map{createRef<Image>(env_output_spec_info)};

		gpu::CommandBuffer command_buffer{*m_gpuCtx, vk::QueueFlagBits::eCompute};
		command_buffer.begin();

		m_gpuCtx->bindDescriptorHeap(&command_buffer);

		command_buffer.bindShaders({m_globals->getShader("Diffuse_Irradiance_Convolution")});

		Globals::DiffuseIrradianceConvolutionConstants diffuse_irradiance_convolution_constants{};
		diffuse_irradiance_convolution_constants.environmentMapId       = p_environment_map->getAlignedShaderReadHeapID();
		diffuse_irradiance_convolution_constants.diffuseIrradianceMapId = out_irradiance_map->getAlignedStorageHeapID();
		diffuse_irradiance_convolution_constants.samplerId              = m_samplers.at(ESamplerType::eIrradianceMap);

		command_buffer.pushData(diffuse_irradiance_convolution_constants);
		command_buffer.getVulkanCommandBuffer().dispatch((c_diffuse_irradiance_resolution + 15u) / 16u, (c_diffuse_irradiance_resolution + 15u) / 16u, 6);
		command_buffer.endAndSubmit();

		out_irradiance_map->toShaderReadOptimal();

		return out_irradiance_map;
	}

	auto RenderContext::createSpecularIrradianceMapImage(const RefPtr<Image> &p_environment_map) -> RefPtr<Image>
	{
		static constexpr uint32 c_specular_irradiance_resolution{512u};

		ImageSpecInfo env_output_spec_info{};
		env_output_spec_info.size            = {c_specular_irradiance_resolution};
		env_output_spec_info.format          = vk::Format::eR16G16B16A16Sfloat;
		env_output_spec_info.layerCount      = 6u;
		env_output_spec_info.storage         = true;
		env_output_spec_info.generateMipmaps = true;

		auto out_irradiance_map{createRef<Image>(env_output_spec_info)};

		gpu::CommandBuffer command_buffer{*m_gpuCtx, vk::QueueFlagBits::eCompute};
		command_buffer.begin();

		m_gpuCtx->bindDescriptorHeap(&command_buffer);

		command_buffer.bindShaders({m_globals->getShader("Specular_Irradiance_Convolution")});

		Globals::SpecularIrradianceConvolutionConstants specular_irradiance_convolution_constants{};
		specular_irradiance_convolution_constants.environmentMapId = p_environment_map->getAlignedShaderReadHeapID();
		specular_irradiance_convolution_constants.samplerId        = m_samplers.at(ESamplerType::eIrradianceMap);
		specular_irradiance_convolution_constants.numSamples       = 1028u;

		for (uint32 i{0u}; i < out_irradiance_map->getImage()->getSpecInfo().mipCount; ++i)
		{
			out_irradiance_map->createMipHeapID(i);

			specular_irradiance_convolution_constants.specularIrradianceMapId = out_irradiance_map->getMipAlignedStorageHeapID(i);
			specular_irradiance_convolution_constants.roughness               = (float32) i / (float32) (out_irradiance_map->getImage()->getSpecInfo().mipCount - 1u);

			command_buffer.pushData(specular_irradiance_convolution_constants);

			uint32 mip_dim{std::max(1u, c_specular_irradiance_resolution >> i)};
			command_buffer.getVulkanCommandBuffer().dispatch((mip_dim + 15u) / 16u, (mip_dim + 15u) / 16u, 6);

			// command_buffer.getVulkanCommandBuffer().pipelineBarrier2();
		}

		command_buffer.endAndSubmit();

		out_irradiance_map->toShaderReadOptimal();

		return out_irradiance_map;
	}

	auto RenderContext::createShader(const io::filesystem::Path &p_path, EShaderStage p_stage, EShaderStage p_next_stage,
									 EShaderLanguage             p_shader_lang) const -> gpu::ShaderHandle
	{
		return m_shaderCompiler->compileToShaderFromPath(p_path, p_stage, p_next_stage, p_shader_lang);
	}

	auto RenderContext::createShaderFromSpirV(const io::filesystem::Path &p_spir_v_path, EShaderStage p_stage, EShaderStage p_next_stage) const -> gpu::ShaderHandle
	{
		TST_ASSERT(io::filesystem::exists(p_spir_v_path));
		return createGPURef<gpu::Shader>(io::filesystem::readBinary(p_spir_v_path), getVulkanShaderStage(p_stage), getVulkanShaderStage((p_next_stage)));
	}

	auto RenderContext::beginRendering(const RenderingInfo &p_rendering_info, gpu::CommandBuffer *p_command_buffer) const -> void
	{
		TST_GET_VALID_CMD_BUFFER();

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

	auto RenderContext::endRendering(const RenderingInfo &p_rendering_info, gpu::CommandBuffer *p_command_buffer) const -> void
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

	auto RenderContext::renderFullscreenQuad(gpu::CommandBuffer *p_command_buffer) const -> void
	{
		TST_GET_VALID_CMD_BUFFER();

		command_buffer->bindIndexBuffer(m_globals->fullscreenQuadIndexBuffer(), 0u, gpu::EIndexType::eUint8);
		command_buffer->drawIndexed(m_globals->fullscreenQuadIndices().size());
	}

	auto RenderContext::renderFullscreenQuadMeshShader(gpu::CommandBuffer *p_command_buffer) const -> void
	{
		// That's it...
		TST_GET_VALID_CMD_BUFFER();
		command_buffer->getVulkanCommandBuffer().drawMeshTasksEXT(1, 1, 1);
	}

	#undef TST_GET_VALID_CMD_BUFFER_AND_FRAME_INDEX
}
