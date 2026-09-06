#include <print>
#include <toast_os/console.hpp>
#include <toast_os/entry_points.hpp>

#include <toast_gpu/api.hpp>
#include <toast_gpu/frame.hpp>
#include <toast_gpu/upload.hpp>

using namespace toaster;

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#undef min
#undef max

#include <test.vert.h>
#include <test.frag.h>

#include <stb/stb_image.h>

TST_WINMAIN()
{
	os::createOutputConsole();
	{
		constexpr uint32 max_frames_in_flight{3u};

		gpu::GPUContextDesc ctx_desc{};
		ctx_desc.enableDebugInfo                 = true;
		ctx_desc.maxConcurrentSwapchainWorkloads = max_frames_in_flight;

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		gpu::initGPUContext(ctx_desc);
		gpu::frame::initFrameContext({max_frames_in_flight});
		gpu::upload::initUploadContext({});

		GLFWwindow *window{glfwCreateWindow(1920, 1080, "Toaster Test", nullptr, nullptr)};

		struct CallbackData
		{
			bool   is_resized{false};
			uint32 width{0u};
			uint32 height{0u};
		} cb_data{};

		cb_data.width  = 1920u;
		cb_data.height = 1080u;

		glfwSetWindowUserPointer(window, &cb_data);

		glfwSetFramebufferSizeCallback(window, +[](GLFWwindow *p_window, int32 p_width, int32 p_height) -> void
		{
			auto data{static_cast<CallbackData *>(glfwGetWindowUserPointer(p_window))};
			data->is_resized = true;
			data->width      = static_cast<uint32>(p_width);
			data->height     = static_cast<uint32>(p_height);
		});

		gpu::SurfaceHandle   surface{gpu::createSurface(glfwGetWin32Window(window))};
		gpu::SwapchainHandle swapchain{gpu::createSwapchain(surface, {1920u, 1080u})};

		gpu::ResourceDescriptorHeapHandle resource_heap{gpu::createResourceDescriptorHeap({})};
		gpu::SamplerDescriptorHeapHandle  sampler_heap{gpu::createSamplerDescriptorHeap({})};

		int32  image_width, image_height, nr_channels;
		uint8 *data{stbi_load("../test/resources/textures/doorbell_pig.jpg", &image_width, &image_height, &nr_channels, 4)};
		TST_PERMA_ASSERT(data);

		gpu::TextureHandle test_tex{
			gpu::createTexture(gpu::TextureDesc{
								   {(uint32) image_width, (uint32) image_height, 1u},
								   1u,
								   1u,
								   gpu::ETextureType::e2D,
								   gpu::ESampleCount::e1,
								   gpu::EFormat::eR8G8B8A8Srgb,
								   gpu::ETextureUsageFlagBits::eTransferDst | gpu::ETextureUsageFlagBits::eSampled
							   })
		};

		gpu::upload::uploadDataToTexture(test_tex, data, image_width * image_height * sizeof(uint32), gpu::upload::TextureUploadDesc{{0u, 0u, 0u}, 0u, 0u, 1u});

		stbi_image_free(data);

		gpu::SamplerHandle sampler{gpu::createSampler(gpu::SamplerDesc{})};

		uint32 texture_heap_slot{gpu::allocTextureHeapSlot(resource_heap)};
		gpu::writeTextureDescriptor(resource_heap, texture_heap_slot, test_tex, false);

		uint32 sampler_heap_slot{gpu::allocSamplerHeapSlot(sampler_heap)};
		gpu::writeSamplerDescriptor(sampler_heap, sampler_heap_slot, sampler);

		gpu::ShaderHandle vertex_shader{
			gpu::createShader(gpu::ShaderDesc{
								  "main",
								  c_test_vert_bytecode,
								  sizeof(c_test_vert_bytecode) / sizeof(uint32),
								  gpu::EShaderStageFlagBits::eVertex,
								  gpu::EShaderStageFlagBits::ePixel
							  })
		};

		gpu::ShaderHandle pixel_shader{
			gpu::createShader(gpu::ShaderDesc{
								  "main",
								  c_test_frag_bytecode,
								  sizeof(c_test_frag_bytecode) / sizeof(uint32),
								  gpu::EShaderStageFlagBits::ePixel,
								  gpu::EShaderStageFlagBits::eNone
							  })
		};

		uint32 frame_index{0u};
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			if (cb_data.is_resized)
			{
				if (cb_data.width == 0u || cb_data.height == 0u)
					continue;

				if (!gpu::resizeSwapchain(swapchain, {cb_data.width, cb_data.height}))
					continue;

				cb_data.is_resized = false;
			}

			gpu::upload::flushUploads();
			gpu::frame::beginFrame(frame_index);

			gpu::TextureHandle tex{gpu::acquireNextImage(swapchain)};
			if (!tex)
			{
				gpu::resizeSwapchain(swapchain, {cb_data.width, cb_data.height});
				continue;
			}

			gpu::CommandListHandle cmd{gpu::getOrCreateCommandList(gpu::EQueueType::eGraphics)};

			gpu::insertPreRenderSwapchainResourceBarrier(cmd, tex);

			gpu::RenderingInfo rendering_info{};
			rendering_info.colourAttachments = {
				gpu::RenderingAttachmentInfo{gpu::ClearColourValue{1.0f, 0.0f, 0.0f, 1.0f}, tex, nullptr, gpu::EAttachmentUsageOP::eClearStore}
			};
			rendering_info.renderArea = tsm::Rect{{cb_data.width, cb_data.height}};
			gpu::beginRendering(cmd, rendering_info);

			gpu::bindShaders(cmd, {vertex_shader, pixel_shader});
			gpu::bindResourceHeap(cmd, resource_heap);
			gpu::bindSamplerHeap(cmd, sampler_heap);

			gpu::setPrimitiveTopology(cmd, gpu::EPrimitiveTopology::eTriangleList);
			gpu::setPrimitiveRestart(cmd, false);

			gpu::setViewport(cmd, tsm::Viewport{rendering_info.renderArea});
			gpu::setScissor(cmd, rendering_info.renderArea);

			gpu::setRasterizerDiscardEnable(cmd, false);
			gpu::setPolygonMode(cmd, gpu::EPolygonMode::eFill);
			gpu::setCullMode(cmd, gpu::ECullMode::eBack);
			gpu::setFrontFace(cmd, gpu::EFrontFace::eCCW);
			gpu::setDepthBias(cmd, false);
			gpu::setLineWidth(cmd, 1.0f);

			gpu::setRasterizationSamples(cmd, gpu::ESampleCount::e1);

			gpu::setDepthState(cmd, false);
			gpu::setStencilState(cmd, false);

			struct PushData
			{
				uint32 textureHeapSlot;
				uint32 samplerHeapSlot;
			};

			gpu::pushData(cmd, PushData{texture_heap_slot, sampler_heap_slot});

			gpu::draw(cmd, 3u, 1u);

			gpu::endRendering(cmd);

			gpu::frame::submitAndPresent(swapchain, cmd);

			frame_index = (frame_index + 1u) % max_frames_in_flight;
		}

		gpu::waitIdle();

		gpu::destroyResourceDescriptorHeap(resource_heap);
		gpu::destroySamplerDescriptorHeap(sampler_heap);

		gpu::destroyShader(pixel_shader);
		gpu::destroyShader(vertex_shader);

		gpu::destroySwapchain(swapchain);

		gpu::destroySurface(surface);
		glfwDestroyWindow(window);

		gpu::upload::shutdownUploadContext();
		gpu::frame::shutdownFrameContext();
		gpu::shutdownGPUContext();

		glfwTerminate();
	}

	os::destroyOutputConsole();

	return 0;
}
