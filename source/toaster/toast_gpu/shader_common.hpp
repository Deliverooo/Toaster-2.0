/*!
 * @file shader_common.hpp
 */
#pragma once

#include <span>
#include <string>
#include <vector>
#include "toast_lib/system_types.h"
#include "toast_lib/string.hpp"

namespace toaster::gpu
{
	enum class EShaderType
	{
		eVertex,
		ePixel,
		eCompute,
		eGeometry
	};

	inline std::string shaderStageToString(const EShaderType type)
	{
		switch (type)
		{
			case EShaderType::eVertex: { return "Vertex"; }
			case EShaderType::ePixel: { return "Pixel"; }
			case EShaderType::eCompute: { return "Compute"; }
			case EShaderType::eGeometry: { return "Geometry"; }
		}
		return "";
	}

	class ShaderBlob
	{
	public:
		ShaderBlob(const uint32 *p_data, uint64 p_size) : m_data(p_data), m_size(p_size)
		{
		}

		ShaderBlob(std::span<const uint32> p_data) : m_data(p_data.data()), m_size(p_data.size())
		{
		}

		[[nodiscard]] const uint32 *data() const { return m_data; }
		[[nodiscard]] uint64        size() const { return m_size; }
		[[nodiscard]] uint64        sizeBytes() const { return m_size * sizeof(uint32); }

	private:
		const uint32 *m_data{nullptr};
		uint64        m_size{0u};
	};

	enum class EShaderDomain
	{
		eVertex = 0,
		ePixel = 1
	};

	enum class EShaderUniformType
	{
		eNone,
		eBool,
		eInt,
		eUInt,
		eFloat,
		eVec2,
		eVec3,
		eVec4,
		eMat3,
		eMat4,
		eStruct
	};

	enum class EShaderResourceType
	{
		eNone,
		eTexture2D,
		eTextureCube
	};

	// Base class for uniform declarations
	class ShaderUniformDeclaration
	{
	public:
		virtual ~ShaderUniformDeclaration() = default;

		virtual const String& getName() const = 0;
		virtual uint32 getSize() const = 0;
		virtual uint32 getCount() const = 0;
		virtual uint32 getOffset() const = 0;
		virtual EShaderDomain getDomain() const = 0;
		virtual EShaderUniformType getType() const = 0;
		virtual int32 getLocation() const = 0;

	protected:
		virtual void setOffset(uint32 offset) = 0;

		friend class ShaderUniformBufferDeclaration;
	};

	using ShaderUniformList = std::vector<ShaderUniformDeclaration*>;

	// Uniform buffer declaration
	class ShaderUniformBufferDeclaration
	{
	public:
		virtual ~ShaderUniformBufferDeclaration() = default;

		virtual const String& getName() const = 0;
		virtual uint32 getRegister() const = 0;
		virtual uint32 getSize() const = 0;
		virtual EShaderDomain getDomain() const = 0;
		virtual const ShaderUniformList& getUniformDeclarations() const = 0;

		virtual ShaderUniformDeclaration* findUniform(const String& name) = 0;
	};

	using ShaderUniformBufferList = std::vector<ShaderUniformBufferDeclaration*>;

	// Resource (texture) declaration
	class ShaderResourceDeclaration
	{
	public:
		virtual ~ShaderResourceDeclaration() = default;

		virtual const String& getName() const = 0;
		virtual uint32 getRegister() const = 0;
		virtual uint32 getCount() const = 0;
		virtual EShaderResourceType getType() const = 0;
	};

	using ShaderResourceList = std::vector<ShaderResourceDeclaration*>;

	// Backward compatibility struct (used by old code)
	struct ShaderUniformDeclarationCompat
	{
		String             name;
		int32              size{0};
		int32              location{-1};
		uint32             count{0};
		EShaderUniformType type{EShaderUniformType::eNone};
	};
}
