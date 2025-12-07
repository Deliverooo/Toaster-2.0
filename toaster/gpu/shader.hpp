#pragma once

#include <filesystem>

#include "io/serializable.hpp"
#include "io/stream_reader.hpp"
#include "io/stream_writer.hpp"

#include "core/ref_ptr.hpp"
#include "core/string.hpp"

#include <nvrhi/nvrhi.h>
#include <utility>
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

namespace tst
{
	enum class ERenderResourceType
	{
		eNone = 0,
		eUniformBuffer,
		eUniformBufferSet,
		eStorageBuffer,
		eStorageBufferSet,
		eTexture2D,
		eTextureCube,
		eImage2D,
		eSampler
	};

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
		ShaderUniform() = default;

		ShaderUniform(String name, EShaderUniformType uniform_type, uint32 size, uint32 offset) : m_name(name), m_type(uniform_type), m_size(size), m_offset(offset)
		{
		}

		[[nodiscard]] const String &     getName() const { return m_name; }
		[[nodiscard]] EShaderUniformType getType() const { return m_type; }
		[[nodiscard]] uint32             getSize() const { return m_size; }
		[[nodiscard]] uint32             getOffset() const { return m_offset; }

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

	class ShaderUniformBuffer : public Serializable
	{
	public:
		ShaderUniformBuffer() = default;
		VkDescriptorBufferInfo descriptor;
		uint32                 size{0};
		uint32                 bindingPoint{0};
		String                 name;
		nvrhi::ShaderType      shaderStage{nvrhi::ShaderType::None};

		void serialize(StreamWriter *stream_writer) const override
		{
			stream_writer->writeRaw(descriptor);
			stream_writer->writeRaw(size);
			stream_writer->writeRaw(bindingPoint);
			stream_writer->writeString(name);
			stream_writer->writeRaw(shaderStage);
		}

		void deserialize(StreamReader *stream_reader) override
		{
			stream_reader->readRaw(descriptor);
			stream_reader->readRaw(size);
			stream_reader->readRaw(bindingPoint);
			stream_reader->readString(name);
			stream_reader->readRaw(shaderStage);
		}
	};

	class ShaderStorageBuffer : public Serializable
	{
	public:
		VmaAllocation          memoryAlloc = nullptr;
		VkDescriptorBufferInfo descriptor;
		uint32                 size{0};
		uint32                 bindingPoint{0};
		String                 name;
		nvrhi::ShaderType      shaderStage{nvrhi::ShaderType::None};
		bool                   readOnly{false};

		void serialize(StreamWriter *stream_writer) const override
		{
			stream_writer->writeRaw(descriptor);
			stream_writer->writeRaw(size);
			stream_writer->writeRaw(bindingPoint);
			stream_writer->writeString(name);
			stream_writer->writeRaw(shaderStage);
			stream_writer->writeRaw(readOnly);
		}

		void deserialize(StreamReader *stream_reader) override
		{
			stream_reader->readRaw(descriptor);
			stream_reader->readRaw(size);
			stream_reader->readRaw(bindingPoint);
			stream_reader->readString(name);
			stream_reader->readRaw(shaderStage);
			stream_reader->readRaw(readOnly);
		}
	};

	class ShaderImageSampler : public Serializable
	{
	public:
		uint32_t          bindingPoint  = 0;
		uint32_t          descriptorSet = 0;
		uint32_t          dimension     = 0;
		uint32_t          arraySize     = 0;
		String            name;
		nvrhi::ShaderType shaderStage{nvrhi::ShaderType::None};

		void serialize(StreamWriter *stream_writer) const override
		{
			stream_writer->writeRaw(bindingPoint);
			stream_writer->writeRaw(descriptorSet);
			stream_writer->writeRaw(dimension);
			stream_writer->writeRaw(arraySize);
			stream_writer->writeString(name);
			stream_writer->writeRaw(shaderStage);
		}

		void deserialize(StreamReader *stream_reader) override
		{
			stream_reader->readRaw(bindingPoint);
			stream_reader->readRaw(descriptorSet);
			stream_reader->readRaw(dimension);
			stream_reader->readRaw(arraySize);
			stream_reader->readString(name);
			stream_reader->readRaw(shaderStage);
		}
	};

	enum class ERenderInputType
	{
		eNone = 0,
		eUniformBuffer,
		eStorageBuffer,
		eImageSampler,
		eImageSampler1D,
		eImageSampler2D,
		eImageSampler3D,
		eStorageImage1D,
		eStorageImage2D,
		eStorageImage3D
	};

	class RenderInputDeclaration : public Serializable
	{
	public:
		ERenderInputType type    = ERenderInputType::eNone;
		uint32_t         set     = 0;
		uint32_t         binding = 0;
		uint32_t         count   = 0;
		String           name;

		void serialize(StreamWriter *stream_writer) const override
		{
			stream_writer->writeRaw(type);
			stream_writer->writeRaw(set);
			stream_writer->writeRaw(binding);
			stream_writer->writeRaw(count);
			stream_writer->writeString(name);
		}

		void deserialize(StreamReader *stream_reader) override
		{
			stream_reader->readRaw(type);
			stream_reader->readRaw(set);
			stream_reader->readRaw(binding);
			stream_reader->readRaw(count);
			stream_reader->readString(name);
		}
	};

	struct ShaderDescriptorSet
	{
		std::unordered_map<uint32, ShaderUniformBuffer> uniformBuffers;
		std::unordered_map<uint32, ShaderStorageBuffer> storageBuffers;
		std::unordered_map<uint32, ShaderImageSampler>  imageSamplers;
		std::unordered_map<uint32, ShaderImageSampler>  storageImages;
		std::unordered_map<uint32, ShaderImageSampler>  separateTextures;
		std::unordered_map<uint32, ShaderImageSampler>  separateSamplers;

		std::unordered_map<String, RenderInputDeclaration> inputDeclarations;

		operator bool() const
		{
			return !(storageBuffers.empty() && uniformBuffers.empty() && imageSamplers.empty() && storageImages.empty() && separateTextures.empty() && separateSamplers.
					 empty());
		}
	};

	struct ShaderPushConstantRange : Serializable
	{
		nvrhi::ShaderType shaderStage = nvrhi::ShaderType::None;
		uint32            offset      = 0;
		uint32            size        = 0;

		void serialize(StreamWriter *writer) const override
		{
			writer->writeRaw(shaderStage);
			writer->writeRaw(offset);
			writer->writeRaw(size);
		}

		void deserialize(StreamReader *reader) override
		{
			reader->readRaw(shaderStage);
			reader->readRaw(offset);
			reader->readRaw(size);
		}
	};

	class ShaderResourceDeclaration : public Serializable
	{
	public:
		ShaderResourceDeclaration() = default;

		ShaderResourceDeclaration(String name, uint32 set, uint32 resourceRegister, uint32 count)
			: m_name(std::move(name)), m_set(set), m_register(resourceRegister), m_count(count)
		{
		}

		[[nodiscard]] const String &getName() const { return m_name; }
		[[nodiscard]] uint32        getSet() const { return m_set; }
		[[nodiscard]] uint32        getRegister() const { return m_register; }
		[[nodiscard]] uint32        getCount() const { return m_count; }

		void serialize(StreamWriter *serializer) const override
		{
			serializer->writeString(m_name);
			serializer->writeRaw(m_set);
			serializer->writeRaw(m_register);
			serializer->writeRaw(m_count);
		}

		void deserialize(StreamReader *deserializer) override
		{
			deserializer->readString(m_name);
			deserializer->readRaw(m_set);
			deserializer->readRaw(m_register);
			deserializer->readRaw(m_count);
		}

	private:
		std::string m_name;
		uint32      m_set      = 0;
		uint32      m_register = 0;
		uint32      m_count    = 0;
	};

	class Shader : public RefCounted
	{
	public:
		static RefPtr<Shader> create(const String &name, bool force_compile = false);
		virtual               ~Shader() = default;

		virtual void reload(bool force_compile = false) = 0;
		virtual void _reload(bool force_compile) = 0;

		virtual const String &getName() const = 0;
		virtual uint32        getHash() const = 0;

		virtual void setShaderMacro(const String &macro_name, const String &value) = 0;

		virtual const std::unordered_map<std::string, ShaderBuffer> &             getShaderBuffers() const = 0;
		virtual const std::unordered_map<std::string, ShaderResourceDeclaration> &getResources() const = 0;

		virtual nvrhi::ShaderHandle                                     getHandle() const = 0;
		virtual nvrhi::ShaderHandle                                     getHandle(nvrhi::ShaderType type) const = 0;
		virtual const std::map<nvrhi::ShaderType, nvrhi::ShaderHandle> &getHandles() const = 0;
	};

	class ShaderLibrary : public RefCounted
	{
	public:
		ShaderLibrary()  = default;
		~ShaderLibrary() = default;

		void add(const RefPtr<Shader> &shader);
		void load(const std::filesystem::path &path, bool force_compile = false);
		void load(StringView name, const std::filesystem::path &path);

		const RefPtr<Shader> &get(const std::string &name) const;
		uint64                getSize() const { return m_shaders.size(); }

		std::unordered_map<std::string, RefPtr<Shader> > &      GetShaders() { return m_shaders; }
		const std::unordered_map<std::string, RefPtr<Shader> > &GetShaders() const { return m_shaders; }

	private:
		std::unordered_map<std::string, RefPtr<Shader> > m_shaders;
	};
}
