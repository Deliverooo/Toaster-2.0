/*!
 * @file shader_common.hpp
 * @brief Common shader definitions and abstract interfaces for the reflection system
 * 
 * This file defines the core type system and abstract base classes for the shader reflection
 * and material system. It provides:
 * 
 * - Enumerations for shader stages, uniform types, and resource types
 * - Abstract base classes for shader metadata discovery
 * - Type-safe interfaces for uniform declarations and resources
 * 
 * The reflection system is designed to support multiple graphics backends (OpenGL, Vulkan, D3D)
 * through these abstract interfaces. Backend-specific implementations (e.g., GLShaderUniformDeclaration)
 * inherit from these base classes.
 * 
 * @see gl_shader_uniform.hpp for OpenGL-specific implementations
 * @see material.hpp for how these types are used in materials
 */
#pragma once

#include <span>
#include <string>
#include <vector>
#include "toast_lib/system_types.h"
#include "toast_lib/string.hpp"

namespace toaster::gpu
{
	/**
	 * @brief Enumeration of supported shader stages
	 * 
	 * Defines the different programmable stages in the graphics pipeline that can be used
	 * when creating shaders.
	 */
	enum class EShaderType
	{
		eVertex,
		///< Vertex shader stage (processes individual vertices)
		ePixel,
		///< Fragment/pixel shader stage (processes individual fragments/pixels)
		eCompute,
		///< Compute shader stage (general-purpose compute)
		eGeometry ///< Geometry shader stage (processes primitives)
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

	/**
	 * @brief Opaque blob containing compiled shader bytecode
	 * 
	 * Used to store compiled shader code (e.g., SPIR-V bytecode) for backends
	 * that require bytecode-based compilation rather than source-based.
	 */
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

	/**
	 * @brief Enumeration of shader domains for uniform organization
	 * 
	 * Used to categorize uniforms based on which shader stage they affect.
	 * This allows the Material system to organize uniforms into per-stage buffers
	 * for efficient GPU uploads.
	 */
	enum class EShaderDomain
	{
		eVertex = 0,
		///< Vertex shader domain
		ePixel = 1 ///< Fragment/pixel shader domain
	};

	/**
	 * @brief Enumeration of supported uniform types in shaders
	 * 
	 * Defines the types of data that can be stored as shader uniforms.
	 * Used for type-safe uniform storage, CPU-GPU transfers, and memory layout calculation.
	 */
	enum class EShaderUniformType
	{
		eNone,
		///< Unknown or no type
		eBool,
		///< Boolean (1 byte)
		eInt,
		///< Signed 32-bit integer (4 bytes)
		eUInt,
		///< Unsigned 32-bit integer (4 bytes)
		eFloat,
		///< 32-bit floating point (4 bytes)
		eVec2,
		///< 2D floating point vector (8 bytes)
		eVec3,
		///< 3D floating point vector (12 bytes)
		eVec4,
		///< 4D floating point vector (16 bytes)
		eMat3,
		///< 3x3 floating point matrix (36 bytes)
		eMat4,
		///< 4x4 floating point matrix (64 bytes)
		eStruct ///< User-defined struct (not directly supported yet)
	};

	/**
	 * @brief Enumeration of shader resource types
	 * 
	 * Defines the types of resources (like textures) that shaders can access.
	 * Used for resource discovery and binding.
	 */
	enum class EShaderResourceType
	{
		eNone,
		///< Unknown or no resource
		eTexture2D,
		///< 2D texture sampler
		eTextureCube ///< Cube map texture sampler
	};

	/**
	 * @brief Abstract base class for shader uniform declarations
	 * 
	 * Represents metadata about a single uniform in a shader. Provides access to:
	 * - Name and size information
	 * - Type information for type-safe handling
	 * - GPU location (uniform location as returned by OpenGL)
	 * - Offset for CPU buffer storage
	 * - Domain (which shader stage uses this uniform)
	 * 
	 * Backend-specific implementations (e.g., GLShaderUniformDeclaration) inherit from this
	 * to provide complete uniform metadata.
	 * 
	 * @note Uniforms are typically organized within ShaderUniformBufferDeclaration containers
	 *       for efficient management and GPU upload.
	 */
	class ShaderUniformDeclaration
	{
	public:
		virtual ~ShaderUniformDeclaration() = default;

		/// @brief Get the name of this uniform (e.g., "u_ViewMatrix")
		virtual const String &getName() const = 0;

		/// @brief Get the size in bytes of this uniform (e.g., 64 for mat4)
		virtual uint32 getSize() const = 0;

		/// @brief Get the count for array uniforms (>1 if array, 1 if scalar)
		virtual uint32 getCount() const = 0;

		/// @brief Get the byte offset of this uniform within its parent buffer
		virtual uint32 getOffset() const = 0;

		/// @brief Get which shader domain (VS/PS) this uniform belongs to
		virtual EShaderDomain getDomain() const = 0;

		/// @brief Get the type of this uniform (float, vec3, mat4, etc.)
		virtual EShaderUniformType getType() const = 0;

		/// @brief Get the GPU uniform location (-1 if not found)
		virtual int32 getLocation() const = 0;

	protected:
		/// @brief Set the offset within the parent buffer (for initialization)
		virtual void setOffset(uint32 offset) = 0;

		friend class ShaderUniformBufferDeclaration;
	};

	/// @brief Typedef for a list of uniform declarations
	using ShaderUniformList = std::vector<ShaderUniformDeclaration *>;

	/**
	 * @brief Abstract base class for shader uniform buffers
	 * 
	 * A uniform buffer is a container that holds multiple related uniforms for a specific
	 * shader domain (vertex or pixel). The buffer manages:
	 * - Sequential storage of uniforms with automatic offset calculation
	 * - Fast lookup of uniforms by name
	 * - Total buffer size for memory allocation
	 * 
	 * Example usage:
	 * @code
	 * // Get the vertex shader material buffer
	 * auto& vs_buffer = shader->getVSMaterialUniforms().front();
	 * 
	 * // Find a specific uniform
	 * auto uniform = vs_buffer->findUniform("u_ViewMatrix");
	 * 
	 * // Get all uniforms in the buffer
	 * for (auto& uniform : vs_buffer->getUniformDeclarations())
	 *     upload_to_gpu(uniform);
	 * @endcode
	 */
	class ShaderUniformBufferDeclaration
	{
	public:
		virtual ~ShaderUniformBufferDeclaration() = default;

		/// @brief Get the name of this buffer (e.g., "Material_VS")
		virtual const String &getName() const = 0;

		/// @brief Get the register/binding point (typically 0)
		virtual uint32 getRegister() const = 0;

		/// @brief Get the total size in bytes of all uniforms in this buffer
		virtual uint32 getSize() const = 0;

		/// @brief Get which shader domain (vertex or pixel) this buffer is for
		virtual EShaderDomain getDomain() const = 0;

		/// @brief Get all uniform declarations in this buffer
		virtual const ShaderUniformList &getUniformDeclarations() const = 0;

		/// @brief Find a uniform by name (returns nullptr if not found)
		virtual ShaderUniformDeclaration *findUniform(const String &name) = 0;
	};

	/// @brief Typedef for a list of uniform buffers
	using ShaderUniformBufferList = std::vector<ShaderUniformBufferDeclaration *>;

	/**
	 * @brief Abstract base class for shader resource declarations
	 * 
	 * Represents metadata about a shader resource, such as a texture sampler.
	 * Used for resource discovery and binding to the correct texture units.
	 * 
	 * Example:
	 * @code
	 * auto resource = shader->findResourceDeclaration("u_Texture");
	 * if (resource)
	 *     texture->bind(resource->getRegister()); // Bind to the right slot
	 * @endcode
	 */
	class ShaderResourceDeclaration
	{
	public:
		virtual ~ShaderResourceDeclaration() = default;

		/// @brief Get the name of this resource (e.g., "u_Texture")
		virtual const String &getName() const = 0;

		/// @brief Get the texture unit/binding point (e.g., 0, 1, 2...)
		virtual uint32 getRegister() const = 0;

		/// @brief Get the count for arrays of resources
		virtual uint32 getCount() const = 0;

		/// @brief Get the type of this resource (Texture2D, TextureCube, etc.)
		virtual EShaderResourceType getType() const = 0;
	};

	/// @brief Typedef for a list of resource declarations
	using ShaderResourceList = std::vector<ShaderResourceDeclaration *>;

	/**
	 * @brief Backward compatibility struct for older uniform code
	 * 
	 * DEPRECATED: This structure is maintained for backward compatibility only.
	 * New code should use ShaderUniformDeclaration and the reflection system instead.
	 * 
	 * Contains basic uniform metadata in a simple format.
	 */
	struct ShaderUniformDeclarationCompat
	{
		String             name;                            ///< Uniform name
		int32              size{0};                         ///< Size in bytes
		int32              location{-1};                    ///< GPU location (-1 if invalid)
		uint32             count{0};                        ///< Array count (0 if scalar)
		EShaderUniformType type{EShaderUniformType::eNone}; ///< Data type
	};
}
