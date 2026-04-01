/*!
 * @file gl_shader_uniform.hpp
 * @brief OpenGL-specific implementations of shader reflection classes
 * 
 * This file provides OpenGL-specific implementations of the shader reflection system
 * defined in shader_common.hpp. It handles:
 * 
 * - Conversion from OpenGL type enums to engine uniform types
 * - Size calculations for all supported types
 * - GPU location management
 * - Offset calculation for sequential buffer storage
 * 
 * The reflection system uses OpenGL's native glGetActiveUniform() to discover shader
 * metadata without requiring shader source code parsing.
 * 
 * ## Architecture
 * 
 * The OpenGL reflection system has three main classes:
 * 
 * 1. **GLShaderUniformDeclaration**: Represents a single uniform
 *    - Stores name, type, size, offset, GPU location
 *    - Provides GL type conversion
 * 
 * 2. **GLShaderUniformBufferDeclaration**: Container for related uniforms
 *    - Stores multiple uniforms with automatic offset management
 *    - Provides fast name-based lookup
 *    - Calculates total buffer size
 * 
 * 3. **GLShaderResourceDeclaration**: Represents a texture/image resource
 *    - Stores name, type, register/binding point
 *    - Supports 2D textures and cube maps
 * 
 * ## Type Conversion
 * 
 * The GLShaderUniformDeclaration::glTypeToUniformType() method converts from OpenGL
 * type enums (GL_FLOAT, GL_FLOAT_VEC3, etc.) to the engine's EShaderUniformType enum.
 * This enables type-safe handling of uniform values.
 * 
 * Supported conversions:
 * - GL_BOOL → eBool (1 byte)
 * - GL_INT → eInt (4 bytes)
 * - GL_UNSIGNED_INT → eUInt (4 bytes)
 * - GL_FLOAT → eFloat (4 bytes)
 * - GL_FLOAT_VEC2 → eVec2 (8 bytes)
 * - GL_FLOAT_VEC3 → eVec3 (12 bytes)
 * - GL_FLOAT_VEC4 → eVec4 (16 bytes)
 * - GL_FLOAT_MAT3 → eMat3 (36 bytes)
 * - GL_FLOAT_MAT4 → eMat4 (64 bytes)
 * 
 * @see shader_common.hpp for the abstract base classes
 * @see gl_shader.cpp for the reflection pipeline (glGetActiveUniform)
 */
#pragma once

#include "../shader_common.hpp"
#include <unordered_map>

namespace toaster::gpu
{
	/**
	 * @class GLShaderUniformDeclaration
	 * @brief OpenGL implementation of uniform metadata
	 * 
	 * Represents metadata for a single uniform in an OpenGL shader. Stores the uniform's:
	 * - Type (float, vec3, mat4, etc.)
	 * - Name and size
	 * - GPU location (from glGetUniformLocation)
	 * - Offset within a uniform buffer
	 * - Domain (vertex or pixel shader)
	 * 
	 * This class is created by the shader reflection system during GLShader::_reflect()
	 * when glGetActiveUniform() discovers uniforms in the compiled program.
	 * 
	 * @note Size is automatically calculated based on type and count
	 */
	class GLShaderUniformDeclaration : public ShaderUniformDeclaration
	{
	private:
		friend class GLShader;
		friend class GLShaderUniformBufferDeclaration;

	public:
		/**
		 * @brief Construct a uniform declaration
		 * 
		 * @param domain Which shader stage (vertex or pixel) this uniform belongs to
		 * @param type The type of this uniform (float, vec3, mat4, etc.)
		 * @param name The name of the uniform (e.g., "u_ViewMatrix")
		 * @param count Number of elements if array, 1 if scalar (default: 1)
		 * 
		 * The size is automatically calculated based on type and count.
		 * The offset is set later when the uniform is added to a buffer.
		 * 
		 * @code
		 * auto uniform = new GLShaderUniformDeclaration(
		 *     EShaderDomain::eVertex,
		 *     EShaderUniformType::eMat4,
		 *     "u_ViewMatrix",
		 *     1);
		 * @endcode
		 */
		GLShaderUniformDeclaration(
			EShaderDomain domain,
			EShaderUniformType type,
			const String& name,
			uint32 count = 1);

		/// @brief Get the uniform's name
		const String& getName() const override { return m_name; }

		/// @brief Get the size in bytes
		uint32 getSize() const override { return m_size; }

		/// @brief Get the count (>1 for arrays)
		uint32 getCount() const override { return m_count; }

		/// @brief Get the offset in the parent buffer
		uint32 getOffset() const override { return m_offset; }

		/// @brief Get which shader domain this uniform belongs to
		EShaderDomain getDomain() const override { return m_domain; }

		/// @brief Get the data type of this uniform
		EShaderUniformType getType() const override { return m_type; }

		/// @brief Get the GPU location (-1 if not found)
		int32 getLocation() const override { return m_location; }

		/// @brief Check if this is an array uniform
		bool isArray() const { return m_count > 1; }

		/**
		 * @brief Convert an OpenGL type enum to our EShaderUniformType
		 * 
		 * Maps OpenGL type constants (GL_FLOAT, GL_FLOAT_VEC3, etc.) to the
		 * engine's EShaderUniformType enum for type-safe handling.
		 * 
		 * @param glType OpenGL type enum (e.g., GL_FLOAT_VEC3)
		 * @return EShaderUniformType The corresponding engine type, or eNone if unknown
		 * 
		 * Supported mappings:
		 * - GL_BOOL → eBool
		 * - GL_INT → eInt
		 * - GL_UNSIGNED_INT → eUInt
		 * - GL_FLOAT → eFloat
		 * - GL_FLOAT_VEC2 → eVec2
		 * - GL_FLOAT_VEC3 → eVec3
		 * - GL_FLOAT_VEC4 → eVec4
		 * - GL_FLOAT_MAT3 → eMat3
		 * - GL_FLOAT_MAT4 → eMat4
		 */
		static EShaderUniformType glTypeToUniformType(uint32 glType);

		/**
		 * @brief Get the size in bytes for a uniform type
		 * 
		 * Returns the size in bytes needed to store a single element of the given type.
		 * 
		 * @param type The uniform type
		 * @return uint32 Size in bytes (e.g., 4 for float, 16 for vec4, 64 for mat4)
		 * 
		 * Size table:
		 * - eBool: 1 byte
		 * - eInt, eUInt, eFloat: 4 bytes
		 * - eVec2: 8 bytes
		 * - eVec3: 12 bytes
		 * - eVec4: 16 bytes
		 * - eMat3: 36 bytes (3x3 = 9 floats)
		 * - eMat4: 64 bytes (4x4 = 16 floats)
		 * 
		 * @code
		 * uint32 mat4_size = GLShaderUniformDeclaration::sizeOfUniformType(EShaderUniformType::eMat4);
		 * assert(mat4_size == 64);
		 * @endcode
		 */
		static uint32 sizeOfUniformType(EShaderUniformType type);

	protected:
		/// @brief Set the offset within the parent buffer (for buffer initialization)
		void setOffset(uint32 offset) override { m_offset = offset; }

	private:
		String m_name;                          ///< Uniform name
		uint32 m_size{0};                       ///< Size in bytes
		uint32 m_count{1};                      ///< Count (1 for scalar, >1 for arrays)
		uint32 m_offset{0};                     ///< Offset within parent buffer
		EShaderDomain m_domain;                 ///< Vertex or pixel shader domain
		EShaderUniformType m_type;              ///< Type (float, vec3, mat4, etc.)
		mutable int32 m_location{-1};          ///< GPU uniform location
	};

	/**
	 * @class GLShaderResourceDeclaration
	 * @brief OpenGL implementation of shader resource metadata
	 * 
	 * Represents a shader resource such as a texture sampler. Stores:
	 * - Name (e.g., "u_DiffuseTexture")
	 * - Type (Texture2D, TextureCube)
	 * - Register/binding point (texture unit)
	 * 
	 * Resources are discovered during GLShader::_reflect() when glGetActiveUniform()
	 * returns a sampler type (GL_SAMPLER_2D, GL_SAMPLER_CUBE, etc.).
	 */
	class GLShaderResourceDeclaration : public ShaderResourceDeclaration
	{
	private:
		friend class GLShader;

	public:
		/**
		 * @brief Construct a resource declaration
		 * 
		 * @param type Type of resource (Texture2D, TextureCube, etc.)
		 * @param name Name of the resource (e.g., "u_AlbedoMap")
		 * @param count Number of resources if array, 1 if single (default: 1)
		 * 
		 * @code
		 * auto resource = new GLShaderResourceDeclaration(
		 *     EShaderResourceType::eTexture2D,
		 *     "u_DiffuseTexture",
		 *     1);
		 * @endcode
		 */
		GLShaderResourceDeclaration(
			EShaderResourceType type,
			const String& name,
			uint32 count = 1);

		/// @brief Get the resource name
		const String& getName() const override { return m_name; }

		/// @brief Get the binding point/texture unit
		uint32 getRegister() const override { return m_register; }

		/// @brief Get the count (>1 for arrays)
		uint32 getCount() const override { return m_count; }

		/// @brief Get the resource type
		EShaderResourceType getType() const override { return m_type; }

		/**
		 * @brief Convert OpenGL sampler type to EShaderResourceType
		 * 
		 * @param glType OpenGL type enum (e.g., GL_SAMPLER_2D)
		 * @return EShaderResourceType The corresponding engine type, or eNone if not a sampler
		 * 
		 * Supported mappings:
		 * - GL_SAMPLER_2D → eTexture2D
		 * - GL_SAMPLER_CUBE → eTextureCube
		 * - Other types → eNone (not a resource)
		 */
		static EShaderResourceType glTypeToResourceType(uint32 glType);

	private:
		String m_name;                          ///< Resource name
		uint32 m_register{0};                   ///< Texture unit/binding point
		uint32 m_count{1};                      ///< Count (usually 1)
		EShaderResourceType m_type;             ///< Resource type
	};

	/**
	 * @class GLShaderUniformBufferDeclaration
	 * @brief Container for related uniforms with automatic offset management
	 * 
	 * A uniform buffer organizes multiple uniforms into a single contiguous buffer.
	 * It handles:
	 * - Sequential storage with automatic offset calculation
	 * - Fast name-based uniform lookup
	 * - Total size calculation
	 * 
	 * When uniforms are added via pushUniform(), their offsets are automatically
	 * calculated based on the cumulative size of previous uniforms.
	 * 
	 * ## Memory Layout
	 * 
	 * @code
	 * Buffer memory:
	 * [Uniform1 data (offset 0,    size 64)]  // e.g., mat4
	 * [Uniform2 data (offset 64,   size 12)]  // e.g., vec3
	 * [Uniform3 data (offset 76,   size 4)]   // e.g., float
	 * Total size: 80 bytes
	 * @endcode
	 * 
	 * This layout matches how the Material class stores uniform values in CPU buffers.
	 */
	class GLShaderUniformBufferDeclaration : public ShaderUniformBufferDeclaration
	{
	private:
		friend class GLShader;

	public:
		/**
		 * @brief Construct a uniform buffer
		 * 
		 * @param name Name of this buffer (e.g., "Material_VS" for vertex shader materials)
		 * @param domain Which shader stage this buffer is for (vertex or pixel)
		 * 
		 * @code
		 * auto buffer = new GLShaderUniformBufferDeclaration("Material_VS", EShaderDomain::eVertex);
		 * @endcode
		 */
		GLShaderUniformBufferDeclaration(
			const String& name,
			EShaderDomain domain);

		/// @brief Destructor - cleans up all stored uniforms
		~GLShaderUniformBufferDeclaration() override;

		/// @brief Get the buffer name
		const String& getName() const override { return m_name; }

		/// @brief Get the register/binding point
		uint32 getRegister() const override { return m_register; }

		/// @brief Get the total size of all uniforms in bytes
		uint32 getSize() const override { return m_size; }

		/// @brief Get the shader domain (vertex or pixel)
		EShaderDomain getDomain() const override { return m_domain; }

		/// @brief Get all uniforms in this buffer
		const ShaderUniformList& getUniformDeclarations() const override { return m_uniforms; }

		/**
		 * @brief Find a uniform by name
		 * 
		 * Searches the buffer for a uniform with the given name.
		 * 
		 * @param name The uniform name to find
		 * @return ShaderUniformDeclaration* Pointer to the uniform, or nullptr if not found
		 * 
		 * @code
		 * auto uniform = buffer->findUniform("u_ViewMatrix");
		 * if (uniform)
		 *     LOG_INFO("Found uniform: {} bytes", uniform->getSize());
		 * @endcode
		 */
		ShaderUniformDeclaration* findUniform(const String& name) override;

		/**
		 * @brief Add a uniform to the buffer
		 * 
		 * Appends a uniform to the buffer, automatically calculating its offset
		 * based on the cumulative size of previous uniforms.
		 * 
		 * @param uniform The uniform to add
		 * 
		 * Updates:
		 * - The uniform's offset to the current buffer size
		 * - The buffer's total size by the uniform's size
		 * - The uniform list
		 * 
		 * @code
		 * auto uniform = new GLShaderUniformDeclaration(...);
		 * buffer->pushUniform(uniform);  // Offset automatically set
		 * assert(uniform->getOffset() == previous_size);
		 * @endcode
		 */
		void pushUniform(GLShaderUniformDeclaration* uniform);

	private:
		String m_name;                          ///< Buffer name
		EShaderDomain m_domain;                 ///< Vertex or pixel shader
		uint32 m_register{0};                   ///< Binding point
		uint32 m_size{0};                       ///< Total size of all uniforms
		ShaderUniformList m_uniforms;          ///< List of uniforms in this buffer
	};
}
