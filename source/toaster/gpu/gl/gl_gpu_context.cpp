#include "gl/gl_gpu_context.hpp"

#include "openglhpp/opengl.hpp"
#include <GLFW/glfw3.h>

#include "openglhpp/gl_to_string.hpp"

namespace toaster::gpu
{
	static void __stdcall debugMessageCallback(gl::Enum    p_source, gl::Enum p_type, gl::UInt p_id, gl::Enum p_severity, gl::SizeI p_length, const gl::Char *p_message,
											   const void *p_user_param)
	{
		LOG_ERROR("GL CALLBACK: {} | Source: {} | Severity: {} | Message: {}", gl::to_string(static_cast<gl::Error>(p_type)),
				  gl::to_string(static_cast<gl::DebugSource>(p_source)), gl::to_string(static_cast<gl::DebugSeverity>(p_severity)), p_message);
	}

	GLGPUContext::GLGPUContext(GLFWwindow *p_window) : m_window(p_window)
	{
		glfwMakeContextCurrent(m_window);

		gl::loadGL();

		gl::enable(gl::Capability::eDebugOutput);
		gl::enable(gl::Capability::eDebugOutputSynchronous);

		gl::debugMessageCallback(debugMessageCallback, nullptr);
	}
}
