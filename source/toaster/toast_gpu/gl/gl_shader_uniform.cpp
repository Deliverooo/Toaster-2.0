/*!
 * @file gl_shader_uniform.cpp
 */
#include "gl_shader_uniform.hpp"
#include "toast_lib/logging.hpp"
#include <openglhpp/opengl.hpp>

namespace toaster::gpu
{
	// =====================================================
	// GLShaderUniformDeclaration
	// =====================================================

	GLShaderUniformDeclaration::GLShaderUniformDeclaration(
		EShaderDomain domain,
		EShaderUniformType type,
		const String& name,
		uint32 count)
		: m_name(name), m_domain(domain), m_type(type), m_count(count)
	{
		m_size = sizeOfUniformType(type) * count;
	}

	EShaderUniformType GLShaderUniformDeclaration::glTypeToUniformType(uint32 glType)
	{
		switch (glType)
		{
			case GL_BOOL:      return EShaderUniformType::eBool;
			case GL_INT:       return EShaderUniformType::eInt;
			case GL_UNSIGNED_INT: return EShaderUniformType::eUInt;
			case GL_FLOAT:     return EShaderUniformType::eFloat;
			case GL_FLOAT_VEC2: return EShaderUniformType::eVec2;
			case GL_FLOAT_VEC3: return EShaderUniformType::eVec3;
			case GL_FLOAT_VEC4: return EShaderUniformType::eVec4;
			case GL_FLOAT_MAT3: return EShaderUniformType::eMat3;
			case GL_FLOAT_MAT4: return EShaderUniformType::eMat4;
			default:
				LOG_WARN("Unknown OpenGL uniform type: {}", glType);
				return EShaderUniformType::eNone;
		}
	}

	uint32 GLShaderUniformDeclaration::sizeOfUniformType(EShaderUniformType type)
	{
		switch (type)
		{
			case EShaderUniformType::eBool:    return 1;
			case EShaderUniformType::eInt:     return 4;
			case EShaderUniformType::eUInt:    return 4;
			case EShaderUniformType::eFloat:   return 4;
			case EShaderUniformType::eVec2:    return 4 * 2;
			case EShaderUniformType::eVec3:    return 4 * 3;
			case EShaderUniformType::eVec4:    return 4 * 4;
			case EShaderUniformType::eMat3:    return 4 * 3 * 3;
			case EShaderUniformType::eMat4:    return 4 * 4 * 4;
			default:
				return 0;
		}
	}

	// =====================================================
	// GLShaderResourceDeclaration
	// =====================================================

	GLShaderResourceDeclaration::GLShaderResourceDeclaration(
		EShaderResourceType type,
		const String& name,
		uint32 count)
		: m_name(name), m_type(type), m_count(count)
	{
	}

	EShaderResourceType GLShaderResourceDeclaration::glTypeToResourceType(uint32 glType)
	{
		switch (glType)
		{
			case GL_SAMPLER_2D:  return EShaderResourceType::eTexture2D;
			case GL_SAMPLER_CUBE: return EShaderResourceType::eTextureCube;
			default:
				return EShaderResourceType::eNone;
		}
	}

	// =====================================================
	// GLShaderUniformBufferDeclaration
	// =====================================================

	GLShaderUniformBufferDeclaration::GLShaderUniformBufferDeclaration(
		const String& name,
		EShaderDomain domain)
		: m_name(name), m_domain(domain)
	{
	}

	GLShaderUniformBufferDeclaration::~GLShaderUniformBufferDeclaration()
	{
		for (auto uniform : m_uniforms)
			delete uniform;
	}

	ShaderUniformDeclaration* GLShaderUniformBufferDeclaration::findUniform(const String& name)
	{
		for (auto uniform : m_uniforms)
		{
			if (uniform->getName() == name)
				return uniform;
		}
		return nullptr;
	}

	void GLShaderUniformBufferDeclaration::pushUniform(GLShaderUniformDeclaration* uniform)
	{
		uint32 offset = 0;
		if (!m_uniforms.empty())
		{
			auto prev = static_cast<GLShaderUniformDeclaration*>(m_uniforms.back());
			offset = prev->getOffset() + prev->getSize();
		}
		uniform->setOffset(offset);
		m_size += uniform->getSize();
		m_uniforms.push_back(uniform);
	}
}
