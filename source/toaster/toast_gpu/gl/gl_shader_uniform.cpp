/*!
 * @file gl_shader_uniform.cpp
 * @brief Implementation of OpenGL-specific shader reflection classes
 * 
 * Implements the type conversion, size calculation, and buffer management for the
 * OpenGL shader reflection system. These implementations directly use OpenGL constants
 * and handle the conversion between OpenGL's type enums and the engine's type system.
 * 
 * Key responsibilities:
 * - Convert GL_FLOAT, GL_FLOAT_VEC3, etc. to engine types
 * - Calculate byte sizes for all supported uniform types
 * - Manage sequential buffer storage with automatic offset calculation
 * - Provide fast lookup for uniforms by name
 */
#include "gl_shader_uniform.hpp"
#include "toast_lib/logging.hpp"
#include <openglhpp/opengl.hpp>

namespace toaster::gpu
{
	// =====================================================
	// GLShaderUniformDeclaration Implementation
	// =====================================================

	GLShaderUniformDeclaration::GLShaderUniformDeclaration(
		EShaderDomain domain,
		EShaderUniformType type,
		const String& name,
		uint32 count)
		: m_name(name), m_domain(domain), m_type(type), m_count(count)
	{
		// Calculate total size: size of one element * count
		m_size = sizeOfUniformType(type) * count;
	}

	EShaderUniformType GLShaderUniformDeclaration::glTypeToUniformType(uint32 glType)
	{
		// Maps OpenGL type constants to our type enum
		// Called by GLShader::_reflect() when discovering uniforms
		switch (glType)
		{
			case GL_BOOL:            return EShaderUniformType::eBool;
			case GL_INT:             return EShaderUniformType::eInt;
			case GL_UNSIGNED_INT:    return EShaderUniformType::eUInt;
			case GL_FLOAT:           return EShaderUniformType::eFloat;
			case GL_FLOAT_VEC2:      return EShaderUniformType::eVec2;
			case GL_FLOAT_VEC3:      return EShaderUniformType::eVec3;
			case GL_FLOAT_VEC4:      return EShaderUniformType::eVec4;
			case GL_FLOAT_MAT3:      return EShaderUniformType::eMat3;
			case GL_FLOAT_MAT4:      return EShaderUniformType::eMat4;
			default:
				LOG_WARN("Unknown OpenGL uniform type: {}", glType);
				return EShaderUniformType::eNone;
		}
	}

	uint32 GLShaderUniformDeclaration::sizeOfUniformType(EShaderUniformType type)
	{
		// Returns the size in bytes for a single element of the given type
		// Used to calculate buffer offsets and allocate CPU-side storage
		switch (type)
		{
			case EShaderUniformType::eBool:    return 1;      // 1 byte
			case EShaderUniformType::eInt:     return 4;      // 4 bytes (32-bit int)
			case EShaderUniformType::eUInt:    return 4;      // 4 bytes (32-bit uint)
			case EShaderUniformType::eFloat:   return 4;      // 4 bytes (32-bit float)
			case EShaderUniformType::eVec2:    return 4 * 2;  // 8 bytes (2 floats)
			case EShaderUniformType::eVec3:    return 4 * 3;  // 12 bytes (3 floats)
			case EShaderUniformType::eVec4:    return 4 * 4;  // 16 bytes (4 floats)
			case EShaderUniformType::eMat3:    return 4 * 3 * 3; // 36 bytes (9 floats)
			case EShaderUniformType::eMat4:    return 4 * 4 * 4; // 64 bytes (16 floats)
			default:
				return 0;
		}
	}

	// =====================================================
	// GLShaderResourceDeclaration Implementation
	// =====================================================

	GLShaderResourceDeclaration::GLShaderResourceDeclaration(
		EShaderResourceType type,
		const String& name,
		uint32 count)
		: m_name(name), m_type(type), m_count(count)
	{
		// Register is set by the reflection system based on discovery order
	}

	EShaderResourceType GLShaderResourceDeclaration::glTypeToResourceType(uint32 glType)
	{
		// Maps OpenGL sampler types to our resource type enum
		// Called by GLShader::_reflect() when discovering texture samplers
		switch (glType)
		{
			case GL_SAMPLER_2D:      return EShaderResourceType::eTexture2D;
			case GL_SAMPLER_CUBE:    return EShaderResourceType::eTextureCube;
			default:
				// Not a resource type (could be a regular uniform)
				return EShaderResourceType::eNone;
		}
	}

	// =====================================================
	// GLShaderUniformBufferDeclaration Implementation
	// =====================================================

	GLShaderUniformBufferDeclaration::GLShaderUniformBufferDeclaration(
		const String& name,
		EShaderDomain domain)
		: m_name(name), m_domain(domain)
	{
	}

	GLShaderUniformBufferDeclaration::~GLShaderUniformBufferDeclaration()
	{
		// Clean up all dynamically allocated uniform declarations
		for (auto uniform : m_uniforms)
			delete uniform;
	}

	ShaderUniformDeclaration* GLShaderUniformBufferDeclaration::findUniform(const String& name)
	{
		// Linear search through uniforms - typically fast since most buffers
		// contain a small number of uniforms (< 20)
		for (auto uniform : m_uniforms)
		{
			if (uniform->getName() == name)
				return uniform;
		}
		return nullptr;
	}

	void GLShaderUniformBufferDeclaration::pushUniform(GLShaderUniformDeclaration* uniform)
	{
		// Calculate offset: sum of sizes of all previous uniforms
		uint32 offset = 0;
		if (!m_uniforms.empty())
		{
			// Get the last uniform and calculate offset from it
			auto prev = static_cast<GLShaderUniformDeclaration*>(m_uniforms.back());
			offset = prev->getOffset() + prev->getSize();
		}

		// Set the offset on the uniform (triggers protected method override)
		uniform->setOffset(offset);

		// Update total buffer size
		m_size += uniform->getSize();

		// Add to list
		m_uniforms.push_back(uniform);
	}
}
