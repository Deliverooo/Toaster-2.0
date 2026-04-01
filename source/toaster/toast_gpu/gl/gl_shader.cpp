#include "gl_shader.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

#include "toast_lib/io/filesystem.hpp"

#include <fstream>
#include <ranges>

namespace toaster::gpu
{
	gl::ShaderStage getShaderStage(EShaderType p_stage)
	{
		switch (p_stage)
		{
			case EShaderType::eVertex: { return gl::ShaderStage::eVertex; }
			case EShaderType::ePixel: { return gl::ShaderStage::eFragment; }
			case EShaderType::eCompute: { return gl::ShaderStage::eCompute; }
			case EShaderType::eGeometry: { return gl::ShaderStage::eGeometry; }
		}
		return static_cast<gl::ShaderStage>(0);
	}

	#define RETRIEVE_UNIFORM(__name) if (!m_uniformLocations.contains(p_name)) {	m_uniformLocations[p_name] = gl::getUniformLocation(m_programId, p_name.c_str()); }

	GLShader::GLShader(String p_name, const std::unordered_map<EShaderType, String> &p_shader_source_map) : m_name(std::move(p_name)),
																											m_shaderSourceMap(p_shader_source_map)
	{
		m_programId = gl::createProgram();

		std::vector<gl::UInt> shader_ids;
		for (auto [stage, source]: p_shader_source_map)
		{
			gl::UInt id         = gl::createShader(getShaderStage(stage));
			auto     source_str = m_shaderSourceMap[stage].c_str();
			gl::shaderSource(id, 1, &source_str, nullptr);
			gl::compileShader(id);
			gl::attachShader(m_programId, id);
			shader_ids.push_back(id);
		}
		gl::linkProgram(m_programId);

		_reflect();

		gl::Int isLinked;
		gl::getProgramiv(m_programId, gl::ProgramQuery::eLinkStatus, &isLinked);
		if (!isLinked)
		{
			gl::Int maxLength;
			gl::getProgramiv(m_programId, gl::ProgramQuery::eInfoLogLength, &maxLength);

			std::vector<gl::Char> infoLog(maxLength);
			gl::getProgramInfoLog(m_programId, maxLength, &maxLength, infoLog.data());
			LOG_ERROR("Shader linking failed: {}", infoLog.data());

			gl::deleteProgram(m_programId);

			for (const auto id: shader_ids)
			{
				gl::deleteShader(id);
			}
		}

		for (const auto id: shader_ids)
		{
			gl::detachShader(m_programId, id);
			gl::deleteShader(id);
		}
		// gl::getProgramResourceName(m_programId, gl::ProgramInterface::eUniform, 0)
	}

	GLShader::~GLShader()
	{
		// Don't delete m_uniformDeclarations - they're stored in the buffers
		// The buffers will clean them up
		for (auto& buf : m_vsMaterialUniforms)
			delete buf;
		for (auto& buf : m_psMaterialUniforms)
			delete buf;
		for (auto& res : m_resources)
			delete res;
		gl::deleteProgram(m_programId);
	}

	uint32_t GLShader::getID() const
	{
		return m_programId;
	}

	void GLShader::bind() const
	{
		gl::useProgram(m_programId);
	}

	void GLShader::unbind() const
	{
		gl::useProgram(0);
	}

	void GLShader::setUniform(const String &p_name, float32 p_value)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniform1f(m_programId, m_uniformLocations.at(p_name), p_value);
	}

	void GLShader::setUniform(const String &p_name, int32 p_value)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniform1i(m_programId, m_uniformLocations.at(p_name), p_value);
	}

	void GLShader::setUniform(const String &p_name, uint32 p_value)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniform1ui(m_programId, m_uniformLocations.at(p_name), p_value);
	}

	void GLShader::setUniform(const String &p_name, const glm::vec2 &p_value)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniform2f(m_programId, m_uniformLocations.at(p_name), p_value.x, p_value.y);
	}

	void GLShader::setUniform(const String &p_name, const glm::vec3 &p_value)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniform3f(m_programId, m_uniformLocations.at(p_name), p_value.x, p_value.y, p_value.z);
	}

	void GLShader::setUniform(const String &p_name, const glm::vec4 &p_value)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniform4f(m_programId, m_uniformLocations.at(p_name), p_value.x, p_value.y, p_value.z, p_value.w);
	}

	void GLShader::setUniform(const String &p_name, const glm::mat3 &p_value)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniformMatrix3fv(m_programId, m_uniformLocations.at(p_name), 1, false, &p_value[0].x);
	}

	void GLShader::setUniform(const String &p_name, const glm::mat4 &p_value)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniformMatrix4fv(m_programId, m_uniformLocations.at(p_name), 1, false, &p_value[0].x);
	}

	void GLShader::setUniform(const String &p_name, float32 *p_values, uint32 p_count)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniform1fv(m_programId, m_uniformLocations.at(p_name), static_cast<gl::Int>(p_count), p_values);
	}

	void GLShader::setUniform(const String &p_name, int32 *p_values, uint32 p_count)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniform1iv(m_programId, m_uniformLocations.at(p_name), static_cast<gl::Int>(p_count), p_values);
	}

	void GLShader::setUniform(const String &p_name, uint32 *p_values, uint32 p_count)
	{
		RETRIEVE_UNIFORM(p_name);
		gl::programUniform1uiv(m_programId, m_uniformLocations.at(p_name), static_cast<gl::Int>(p_count), p_values);
	}
	#undef RETRIEVE_UNIFORM

	const std::vector<ShaderUniformDeclaration *> &GLShader::getUniformDeclarations() const
	{
		// Return empty vector for backward compatibility
		// The new system uses getVSMaterialUniforms() and getPSMaterialUniforms() instead
		static const std::vector<ShaderUniformDeclaration *> emptyVector;
		return emptyVector;
	}

	ShaderUniformDeclaration* GLShader::findUniformDeclaration(const String& name)
	{
		for (auto& buffer : m_vsMaterialUniforms)
		{
			auto uniform = buffer->findUniform(name);
			if (uniform)
				return uniform;
		}

		for (auto& buffer : m_psMaterialUniforms)
		{
			auto uniform = buffer->findUniform(name);
			if (uniform)
				return uniform;
		}

		return nullptr;
	}

	ShaderResourceDeclaration* GLShader::findResourceDeclaration(const String& name)
	{
		for (auto& resource : m_resources)
		{
			if (resource->getName() == name)
				return resource;
		}

		return nullptr;
	}

	void GLShader::_reflect()
	{
		gl::Int count;
		gl::getProgramiv(m_programId, gl::ProgramQuery::eActiveUniforms, &count);

		gl::Int max_length;
		gl::getProgramiv(m_programId, gl::ProgramQuery::eActiveUniformMaxLength, &max_length);

		// Create material uniform buffers for each domain
		auto vsMaterialBuffer = new GLShaderUniformBufferDeclaration("Material_VS", EShaderDomain::eVertex);
		auto psMaterialBuffer = new GLShaderUniformBufferDeclaration("Material_PS", EShaderDomain::ePixel);

		for (uint32 i{0u}; i < count; ++i)
		{
			String uniform_name;
			uniform_name.resize(max_length);

			gl::Enum  type;
			gl::SizeI size_count;
			gl::SizeI actual_length;

			gl::getActiveUniform(m_programId, i, max_length, &actual_length, &size_count, &type, &uniform_name[0]);

			uniform_name.resize(actual_length);

			gl::Int location = gl::getUniformLocation(m_programId, uniform_name.c_str());

			// Skip built-in uniforms
			if (uniform_name.find("gl_") != std::string::npos)
				continue;

			// Determine if this is a resource (texture) or a uniform
			auto resourceType = GLShaderResourceDeclaration::glTypeToResourceType(type);
			if (resourceType != EShaderResourceType::eNone)
			{
				auto resource = new GLShaderResourceDeclaration(resourceType, uniform_name, 1);
				m_resources.push_back(resource);
			}
			else
			{
				// This is a regular uniform
				auto uniformType = GLShaderUniformDeclaration::glTypeToUniformType(type);
				if (uniformType == EShaderUniformType::eNone)
					continue;

				// Only add to VS buffer - OpenGL's glGetActiveUniform() doesn't provide
				// per-stage usage info. For proper per-stage reflection, you would need to:
				// 1. Parse shader source code
				// 2. Use GL_ARB_separate_shader_objects per-stage queries
				// 3. Maintain separate reflection for each stage
				// For now, VS buffer contains all uniforms (typically they're defined
				// once in the linked program and available to all stages)
				auto uniformDeclVS = new GLShaderUniformDeclaration(
					EShaderDomain::eVertex,
					uniformType,
					uniform_name,
					static_cast<uint32>(size_count));
				uniformDeclVS->m_location = location;
				vsMaterialBuffer->pushUniform(uniformDeclVS);
			}
		}

		// Only add buffers if they have uniforms
		if (!vsMaterialBuffer->getUniformDeclarations().empty())
			m_vsMaterialUniforms.push_back(vsMaterialBuffer);
		else
			delete vsMaterialBuffer;

		// PS buffer typically stays empty with this approach
		if (!psMaterialBuffer->getUniformDeclarations().empty())
			m_psMaterialUniforms.push_back(psMaterialBuffer);
		else
			delete psMaterialBuffer;
	}
}
