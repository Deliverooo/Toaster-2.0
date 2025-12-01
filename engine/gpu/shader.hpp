#pragma once

#include "io/stream_writer.hpp"
#include "io/stream_reader.hpp"
#include "io/serializable.hpp"

#include "memory/ref_ptr.hpp"
#include "string/string.hpp"

namespace tst::gpu
{
	enum class EShaderLanguage
	{
		eGLSL, eHLSL
	};

	enum class EShaderUniformType
	{
		eBool,
		eInt,
		eUInt,
		eFloat,
		eVec2,
		eVec3,
		eVec4,
		eMat3,
		eMat4,
		eIVec2,
		eIVec3,
		eIVec4
	};

	class ShaderUniform : public Serializable
	{
	public:
		ShaderUniform(String name, EShaderUniformType uniform_type, uint32 size, uint32 offset);

		const String &     getName() const { return m_name; }
		EShaderUniformType getType() const { return m_type; }
		uint32             getSize() const { return m_size; }
		uint32             getOffset() const { return m_offset; }

		static String uniformTypeToString(EShaderUniformType type);

		void serialize(StreamWriter *stream_writer) const override
		{
			stream_writer->writeString(m_name);
			stream_writer->writeRaw(m_type);
			stream_writer->writeRaw(m_size);
			stream_writer->writeRaw(m_offset);
		}

		void deserialize(StreamReader *stream_reader) override
		{
			stream_reader->readString(m_name);
			stream_reader->readRaw(m_type);
			stream_reader->readRaw(m_size);
			stream_reader->readRaw(m_offset);
		}

	private:
		String             m_name;
		EShaderUniformType m_type;
		uint32             m_size{0};
		uint32             m_offset{0};
	};

	struct ShaderBuffer : Serializable
	{
		String                                    name;
		uint32                                    size{0};
		std::unordered_map<String, ShaderUniform> uniforms;

		void serialize(StreamWriter *stream_writer) const override
		{
			stream_writer->writeString(name);
			stream_writer->writeRaw(size);
			stream_writer->writeRaw(uniforms);
		}

		void deserialize(StreamReader *stream_reader) override
		{
			stream_reader->readString(name);
			stream_reader->readRaw(size);
			stream_reader->readRaw(uniforms);
		}
	};

	class Shader : public RefCounted
	{
	public:
		Shader(const String &name, bool force_compile = false);

		virtual ~Shader() override = default;

		virtual void reload(bool force_compile = false) = 0;
		virtual void _reload(bool force_compile) = 0;

		virtual const String &getName() const = 0;

		virtual void setShaderMacro(const String &macro_name, const String &value) = 0;
	};
}
