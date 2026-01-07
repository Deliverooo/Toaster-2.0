#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "system_types.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include "gpu_context.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include "swapchain.hpp"
#include "window.hpp"

#define RAY_TRACING_ENABLED 0

namespace toaster
{
	class Application
	{
	public:
		Application();
		~Application() noexcept;

		void run();

	private:
		void _processInput();
		void _drawFrame();

		Window *m_window{nullptr};

		nvrhi::CommandListHandle m_commandList{nullptr};

		gpu::Shader *m_testShader{nullptr};

		Camera    m_camera;
		glm::vec2 m_lastMousePos{0.0f, 0.0f};
		bool      m_firstMouse{true};
		bool      m_cursorCaptured{false};

		float32 m_deltaTime{0.0f};
	};
}
