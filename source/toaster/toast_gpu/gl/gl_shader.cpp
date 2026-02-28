#include "gl_shader.hpp"

#include "toaster/toast_lib/logging.hpp"
#include "toaster/toast_lib/toast_assert.h"

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

	GLShader::GLShader(std::string p_name, const std::map<EShaderType, ShaderBlob> &p_shader_bytecode_map) : m_name(std::move(p_name))
	{
		TST_ASSERT_MSG(false, "Until I somehow get SPIRV to work with GLSL (that being the syntactical differences), sadly this will not work :(");
		m_programId = gl::createProgram();

		std::vector<gl::UInt> shader_ids;
		for (auto [stage, shader]: p_shader_bytecode_map)
		{
			gl::UInt id = gl::createShader(getShaderStage(stage));
			gl::shaderBinary(1, &id, gl::ShaderBinaryFormat::eSpirV, shader.data(), static_cast<gl::Int>(shader.sizeBytes()));
			gl::specializeShader(id, "main", 0, nullptr, nullptr);
			gl::attachShader(m_programId, id);
			shader_ids.push_back(id);
		}
		gl::linkProgram(m_programId);

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
	}

	GLShader::GLShader(std::string p_name, const std::map<EShaderType, const char *> &p_shader_source_map) : m_name(std::move(p_name))
	{
		m_programId = gl::createProgram();

		std::vector<gl::UInt> shader_ids;
		for (auto [stage, source]: p_shader_source_map)
		{
			gl::UInt id = gl::createShader(getShaderStage(stage));
			gl::shaderSource(id, 1, &source, nullptr);
			gl::compileShader(id);
			gl::attachShader(m_programId, id);
			shader_ids.push_back(id);
		}
		gl::linkProgram(m_programId);

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
	}

	GLShader::~GLShader()
	{
		gl::deleteProgram(m_programId);
	}

	void GLShader::bind() const
	{
		gl::useProgram(m_programId);
	}

	void GLShader::unbind() const
	{
		gl::useProgram(0);
	}

	void GLShader::setUniform(const std::string &p_name, float p_value)
	{
		if (!m_uniformLocations.contains(p_name))
		{
			m_uniformLocations[p_name] = gl::getUniformLocation(m_programId, p_name.c_str());
		}
		gl::uniform1f(m_uniformLocations.at(p_name), p_value);
	}

	void GLShader::setUniform(const std::string &p_name, int p_value)
	{
		if (!m_uniformLocations.contains(p_name))
		{
			m_uniformLocations[p_name] = gl::getUniformLocation(m_programId, p_name.c_str());
		}
		gl::uniform1i(m_uniformLocations.at(p_name), p_value);
	}

	void GLShader::setUniform(const std::string &p_name, uint32 p_value)
	{
		if (!m_uniformLocations.contains(p_name))
		{
			m_uniformLocations[p_name] = gl::getUniformLocation(m_programId, p_name.c_str());
		}
		gl::uniform1ui(m_uniformLocations.at(p_name), p_value);
	}

	void GLShader::setUniform(const std::string &p_name, const glm::vec2 &p_value)
	{
		if (!m_uniformLocations.contains(p_name))
		{
			m_uniformLocations[p_name] = gl::getUniformLocation(m_programId, p_name.c_str());
		}
		gl::uniform2f(m_uniformLocations.at(p_name), p_value.x, p_value.y);
	}

	void GLShader::setUniform(const std::string &p_name, const glm::vec3 &p_value)
	{
		if (!m_uniformLocations.contains(p_name))
		{
			m_uniformLocations[p_name] = gl::getUniformLocation(m_programId, p_name.c_str());
		}
		gl::uniform3f(m_uniformLocations.at(p_name), p_value.x, p_value.y, p_value.z);
	}

	void GLShader::setUniform(const std::string &p_name, const glm::vec4 &p_value)
	{
		if (!m_uniformLocations.contains(p_name))
		{
			m_uniformLocations[p_name] = gl::getUniformLocation(m_programId, p_name.c_str());
		}
		gl::uniform4f(m_uniformLocations.at(p_name), p_value.x, p_value.y, p_value.z, p_value.w);
	}

	void GLShader::setUniform(const std::string &p_name, const glm::mat3 &p_value)
	{
		if (!m_uniformLocations.contains(p_name))
		{
			m_uniformLocations[p_name] = gl::getUniformLocation(m_programId, p_name.c_str());
		}
		gl::uniformMatrix3fv(m_uniformLocations.at(p_name), 1, false, &p_value[0].x);
	}

	void GLShader::setUniform(const std::string &p_name, const glm::mat4 &p_value)
	{
		if (!m_uniformLocations.contains(p_name))
		{
			m_uniformLocations[p_name] = gl::getUniformLocation(m_programId, p_name.c_str());
		}
		gl::uniformMatrix4fv(m_uniformLocations.at(p_name), 1, false, &p_value[0].x);
	}
}
