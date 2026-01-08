#include "application.hpp"

#include <set>
#include <glm/gtx/transform.hpp>
#include <nvrhi/utils.h>
#include <spirv_cross/spirv_glsl.hpp>

#include "io/filesystem.hpp"
#include "io/file_stream.hpp"

#include "input.hpp"
#include "logging.hpp"
#include "shader_compiler.hpp"

#define SHADER_REFLECTION_TEST 0
#define FILE_STREAM_TEST 1

namespace shaders::vulkan
{
	#include "test.vert.glsl.spv.inl"
	#include "test.pixel.glsl.spv.inl"
	#include "mesh.vert.glsl.spv.inl"
	#include "mesh.pixel.glsl.spv.inl"
	#include "mandlebrot.pixel.glsl.spv.inl"
}

namespace toaster
{
	Application::Application()
		: m_camera(glm::vec3(0.0f, 2.0f, 5.0f))
	{
		Window::initWindowingAPI();

		m_window = new Window(1280, 720, "Toaster: v0.314");

		input::setCurrentWindowContext(m_window->getNativeWindow());

		gpu::GPUContext *gpu_context = m_window->getGPUContext();
		auto             nv_device   = gpu_context->getNVRHIDevice();

		m_commandList = nv_device->createCommandList();

		std::map<nvrhi::ShaderType, gpu::ShaderBlob> shader_bytecode_map{
			{nvrhi::ShaderType::Vertex, {shaders::vulkan::g_vs_test}},
			{nvrhi::ShaderType::Pixel, {shaders::vulkan::g_ps_test}}
		};

		m_testShader = new gpu::Shader(gpu_context, shader_bytecode_map);

		#if FILE_STREAM_TEST
		{
			io::FileStreamWriter writer{"orbo.bin"};
			writer.writeString("Orbo is sigma!");
		}
		std::string test_str;
		{
			io::FileStreamReader reader{"orbo.bin"};
			reader.readString(test_str);
		}
		LOG_TRACE("{}", test_str);
		#endif
	}

	Application::~Application() noexcept
	{
		delete m_testShader;

		delete m_window;

		Window::shutdownWindowingAPI();
	}

	void Application::run()
	{
		auto gpu_context = m_window->getGPUContext();
		auto nv_device   = gpu_context->getNVRHIDevice();

		while (!glfwWindowShouldClose(m_window->getNativeWindow()))
		{
			const auto startTime = static_cast<float32>(glfwGetTime());

			m_window->processEvents();
			m_window->beginFrame();

			_processInput();
			_drawFrame();

			m_window->endFrame();
			gpu_context->getNVRHIDevice()->runGarbageCollection();

			const auto endTime = static_cast<float32>(glfwGetTime());
			m_deltaTime        = endTime - startTime;
		}
		gpu_context->getLogicalDevice().waitIdle();
	}

	void Application::_processInput()
	{
		if (m_cursorCaptured)
		{
			if (input::isKeyDown(input::EKeyCode::eW))
				m_camera.processKeyboard(Camera::EMovement::eForward, m_deltaTime);
			if (input::isKeyDown(input::EKeyCode::eS))
				m_camera.processKeyboard(Camera::EMovement::eBackward, m_deltaTime);
			if (input::isKeyDown(input::EKeyCode::eA))
				m_camera.processKeyboard(Camera::EMovement::eLeft, m_deltaTime);
			if (input::isKeyDown(input::EKeyCode::eD))
				m_camera.processKeyboard(Camera::EMovement::eRight, m_deltaTime);
			if (input::isKeyDown(input::EKeyCode::eSpace))
				m_camera.processKeyboard(Camera::EMovement::eUp, m_deltaTime);
			if (input::isKeyDown(input::EKeyCode::eLeftControl))
				m_camera.processKeyboard(Camera::EMovement::eDown, m_deltaTime);

			// Mouse look
			auto [mouseX, mouseY] = input::getMousePos();

			if (m_firstMouse)
			{
				m_lastMousePos = {mouseX, mouseY};
				m_firstMouse   = false;
			}

			float xOffset  = mouseX - m_lastMousePos.x;
			float yOffset  = m_lastMousePos.y - mouseY; // Reversed: y goes down in screen coords
			m_lastMousePos = {mouseX, mouseY};

			m_camera.processMouseMovement(xOffset, yOffset);
		}
	}

	void Application::_drawFrame()
	{
		auto            gpu_context = m_window->getGPUContext();
		nvrhi::IDevice *nv_device   = gpu_context->getNVRHIDevice();
		vk::Device      vk_device   = gpu_context->getLogicalDevice();

		m_commandList->open();

		nvrhi::utils::ClearColorAttachment(m_commandList, m_window->getSwapchain()->getCurrentFramebuffer(), 0, {1.0f, 0.0f, 1.0f, 1.0f});

		m_commandList->close();
		nv_device->executeCommandList(m_commandList);
	}
}
