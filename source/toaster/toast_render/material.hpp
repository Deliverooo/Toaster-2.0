/*!
 * @file material.hpp
 * @brief Material system for managing shader uniforms and resources
 * 
 * The Material class provides a high-level interface for working with shaders and their
 * uniforms. It abstracts away the complexity of GPU uniform management by:
 * 
 * - Providing type-safe uniform setting via templates
 * - Automatically managing CPU-side uniform storage
 * - Handling GPU uploads during rendering
 * - Tracking texture resources and their bindings
 * - Supporting both immediate (setUniform) and deferred (set<T>) uniform updates
 * 
 * ## Architecture
 * 
 * Materials are built on top of the shader reflection system:
 * 1. When a Material is created, it queries the shader for all uniforms
 * 2. The shader reflection (_reflect) discovers uniforms using OpenGL's glGetActiveUniform
 * 3. Uniforms are organized into per-domain buffers (VS/PS)
 * 4. Material allocates CPU buffers matching these sizes
 * 5. Uniforms are stored in these buffers and uploaded to GPU on use()
 * 
 * ## Usage Example
 * 
 * @code
 * // Create a material from a shader
 * auto material = Material::create(my_shader);
 * 
 * // Set uniforms using type-safe template
 * material->set("u_ViewMatrix", view_matrix);
 * material->set("u_Projection", proj_matrix);
 * material->set("u_Color", glm::vec3(1.0f, 0.5f, 0.2f));
 * material->set("u_MyTexture", my_texture);
 * 
 * // During rendering, upload uniforms to GPU
 * material->use();  // Binds shader and uploads all stored uniforms
 * 
 * // Draw calls go here
 * render_command->drawIndexed(vertex_array);
 * @endcode
 * 
 * ## Uniform Storage
 * 
 * Materials maintain separate uniform storage for vertex and pixel shaders:
 * - `m_vsUniformStorageBuffer`: Contains all uniforms for the vertex shader
 * - `m_psUniformStorageBuffer`: Contains all uniforms for the pixel shader
 * 
 * Each buffer is continuous memory with offsets managed automatically. When a uniform
 * is set, it's written to the buffer at its calculated offset. When use() is called,
 * all buffered uniforms are uploaded to the GPU.
 * 
 * ## Performance Considerations
 * 
 * - Setting a uniform is O(1) - just a buffer write
 * - Uploading is O(n) where n = number of uniforms
 * - Texture binding is O(m) where m = number of textures
 * - Consider batching materials or using instancing for performance-critical code
 * 
 * @see shader_common.hpp for reflection system types
 * @see gl_shader_uniform.hpp for OpenGL-specific implementations
 */
#pragma once

#include "toast_gpu/shader.hpp"
#include "toast_gpu/texture.hpp"
#include "toast_lib/logging.hpp"
#include "toast_lib/buffer.hpp"

#include <unordered_map>

#include "toast_gpu/buffer.hpp"

namespace toaster
{
	/**
	 * @class Material
	 * @brief Manages shader uniforms, resources, and their GPU state
	 * 
	 * A Material represents a shader with its associated uniforms and resources.
	 * It provides a convenient interface for setting uniform values and ensuring
	 * they're properly uploaded to the GPU.
	 * 
	 * Materials are typically not created directly; instead use Material::create()
	 * to construct them from a shader.
	 * 
	 * @note Materials are reference-counted using RefPtr<Material>
	 */
	class Material
	{
	public:
		/**
		 * @brief Factory method to create a new material
		 * 
		 * @param p_shader The shader this material will use
		 * @return RefPtr<Material> Reference-counted pointer to the created material
		 * 
		 * @code
		 * auto my_material = Material::create(my_shader);
		 * @endcode
		 */
		static RefPtr<Material> create(const RefPtr<gpu::IShader> &p_shader);

		/**
		 * @brief Constructor - initializes material from a shader
		 * 
		 * Called by create(); don't use directly. Initializes uniform storage buffers
		 * based on the shader's discovered uniforms.
		 * 
		 * @param p_shader The shader to base this material on
		 * @throws assertion if p_shader is null
		 */
		Material(const RefPtr<gpu::IShader> &p_shader);

		/**
		 * @brief Binds the material for rendering
		 * 
		 * This method:
		 * 1. Binds the shader to the GPU
		 * 2. Uploads all stored vertex shader uniforms
		 * 3. Uploads all stored pixel shader uniforms
		 * 4. Binds all textures to their assigned slots
		 * 
		 * Call this before any draw commands that use this material.
		 * 
		 * @code
		 * material->set("u_Colour", my_colour);
		 * material->use();
		 * render_command->drawIndexed(vertex_array);
		 * @endcode
		 */
		void use();

		/**
		 * @brief Get the underlying shader
		 * 
		 * @return RefPtr<gpu::IShader> The shader this material uses
		 */
		RefPtr<gpu::IShader> getShader();

		/**
		 * @brief Set a uniform value (type-safe template version)
		 * 
		 * This template version automatically handles any copyable type by writing
		 * it to the appropriate uniform buffer. Type checking happens at compile time.
		 * 
		 * @tparam Type The type of value being set (must be memcpy-safe)
		 * @param p_name The uniform name (e.g., "u_ViewMatrix")
		 * @param p_value The value to set
		 * 
		 * @code
		 * material->set("u_TimeValue", 0.5f);           // float
		 * material->set("u_ScreenSize", glm::vec2(...)); // vec2
		 * material->set("u_Transform", matrix);         // mat4
		 * material->set("u_Colors", colour_array, 10);  // array (if supported)
		 * @endcode
		 * 
		 * @note If the uniform is not found, an error is logged but no exception is thrown
		 * @see set(const String&, const RefPtr<gpu::ITexture2D>&) for texture specialization
		 */
		template<typename Type>
		void set(const String &p_name, const Type &p_value)
		{
			auto decl = _findUniformDeclaration(p_name);
			if (decl)
			{
				auto &buffer = _getUniformBufferTarget(decl);
				buffer.write(&p_value, decl->getSize(), decl->getOffset());
			}
			else
			{
				LOG_ERROR("Could not find uniform {}", p_name);
			}
		}

		/**
		 * @brief Set a texture resource
		 * 
		 * Specialization of set<T>() for texture resources. Finds the texture resource
		 * by name and stores it for binding during use().
		 * 
		 * @param p_name The resource name (e.g., "u_DiffuseTexture")
		 * @param p_value The texture to bind
		 * 
		 * @code
		 * material->set("u_AlbedoMap", albedo_texture);
		 * material->set("u_NormalMap", normal_texture);
		 * @endcode
		 * 
		 * @note If the resource is not found, a warning is logged
		 */
		void set(const String &p_name, const RefPtr<gpu::ITexture2D> &p_value);

		/**
		 * @brief Set a floating-point uniform
		 * 
		 * Convenience method for float uniforms. Forwards to the underlying shader's
		 * setUniform() method for immediate GPU upload.
		 * 
		 * @param p_name The uniform name
		 * @param p_value The value to set
		 * 
		 * @deprecated Use set<T>() instead for consistency; this method bypasses
		 *             the material's buffering system
		 * @see set<T>()
		 */
		void setUniform(const String &p_name, float32 p_value);

		/**
		 * @brief Set a signed integer uniform
		 * @param p_name The uniform name
		 * @param p_value The value to set
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, int32 p_value);

		/**
		 * @brief Set an unsigned integer uniform
		 * @param p_name The uniform name
		 * @param p_value The value to set
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, uint32 p_value);

		/**
		 * @brief Set a 2D vector uniform
		 * @param p_name The uniform name
		 * @param p_value The vector to set
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, const glm::vec2 &p_value);

		/**
		 * @brief Set a 3D vector uniform
		 * @param p_name The uniform name
		 * @param p_value The vector to set
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, const glm::vec3 &p_value);

		/**
		 * @brief Set a 4D vector uniform
		 * @param p_name The uniform name
		 * @param p_value The vector to set
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, const glm::vec4 &p_value);

		/**
		 * @brief Set a 3x3 matrix uniform
		 * @param p_name The uniform name
		 * @param p_value The matrix to set
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, const glm::mat3 &p_value);

		/**
		 * @brief Set a 4x4 matrix uniform
		 * @param p_name The uniform name
		 * @param p_value The matrix to set
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, const glm::mat4 &p_value);

		/**
		 * @brief Set an array of floats
		 * @param p_name The uniform name
		 * @param p_values Pointer to array
		 * @param p_count Number of elements
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, float32 *p_values, uint32 p_count);

		/**
		 * @brief Set an array of signed integers
		 * @param p_name The uniform name
		 * @param p_values Pointer to array
		 * @param p_count Number of elements
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, int32 *p_values, uint32 p_count);

		/**
		 * @brief Set an array of unsigned integers
		 * @param p_name The uniform name
		 * @param p_values Pointer to array
		 * @param p_count Number of elements
		 * @deprecated Use set<T>() instead
		 */
		void setUniform(const String &p_name, uint32 *p_values, uint32 p_count);

		/**
		 * @brief Get vertex shader material uniforms
		 * 
		 * Returns the list of uniform buffers for the vertex shader stage.
		 * Typically contains one buffer with all VS uniforms.
		 * 
		 * @return const gpu::ShaderUniformBufferList& List of uniform buffers
		 * 
		 * @code
		 * auto& vs_uniforms = material->getVSMaterialUniforms();
		 * if (!vs_uniforms.empty())
		 * {
		 *     auto buffer = vs_uniforms.front();
		 *     LOG_INFO("VS buffer size: {} bytes", buffer->getSize());
		 * }
		 * @endcode
		 */
		const gpu::ShaderUniformBufferList &getVSMaterialUniforms() const;

		/**
		 * @brief Get pixel shader material uniforms
		 * 
		 * Returns the list of uniform buffers for the pixel/fragment shader stage.
		 * May be empty if the shader has no pixel-stage uniforms.
		 * 
		 * @return const gpu::ShaderUniformBufferList& List of uniform buffers
		 */
		const gpu::ShaderUniformBufferList &getPSMaterialUniforms() const;

		/**
		 * @brief Get shader resources (textures)
		 * 
		 * Returns all discovered shader resources (samplers, images).
		 * 
		 * @return const gpu::ShaderResourceList& List of resources
		 */
		const gpu::ShaderResourceList &     getResources() const;

		/**
		 * @brief Find a uniform declaration by name
		 * 
		 * Searches through all uniform buffers to locate a uniform by name.
		 * 
		 * @param name The uniform name to search for
		 * @return gpu::ShaderUniformDeclaration* Pointer to the uniform, or nullptr if not found
		 * 
		 * @code
		 * auto uniform = material->findUniformDeclaration("u_ViewMatrix");
		 * if (uniform)
		 *     LOG_INFO("Found uniform: {} bytes at offset {}", 
		 *              uniform->getSize(), uniform->getOffset());
		 * @endcode
		 */
		gpu::ShaderUniformDeclaration * findUniformDeclaration(const String &name);

		/**
		 * @brief Find a resource declaration by name
		 * 
		 * Searches for a texture or other resource by name.
		 * 
		 * @param name The resource name to search for
		 * @return gpu::ShaderResourceDeclaration* Pointer to the resource, or nullptr if not found
		 * 
		 * @code
		 * auto resource = material->findResourceDeclaration("u_DiffuseMap");
		 * if (resource)
		 *     LOG_INFO("Bind texture to slot {}", resource->getRegister());
		 * @endcode
		 */
		gpu::ShaderResourceDeclaration *findResourceDeclaration(const String &name);

	private:
		/**
		 * @brief Allocate uniform storage buffers
		 * 
		 * Called during construction to allocate CPU-side buffers for storing uniform values.
		 * One buffer is allocated for VS uniforms and one for PS uniforms, sized to match
		 * the shader's uniform requirements.
		 * 
		 * @internal
		 */
		void _allocateStorage();

		/**
		 * @brief Find a uniform declaration in the shader
		 * 
		 * Internal method that searches the shader's reflection data for a uniform by name.
		 * 
		 * @param p_name The uniform name
		 * @return gpu::ShaderUniformDeclaration* Pointer to the uniform, or nullptr
		 * 
		 * @internal
		 */
		gpu::ShaderUniformDeclaration *_findUniformDeclaration(const String &p_name);

		/**
		 * @brief Get the appropriate buffer for a uniform
		 * 
		 * Determines whether a uniform belongs to the VS or PS buffer based on its domain.
		 * 
		 * @param decl The uniform declaration
		 * @return Buffer& Reference to either m_vsUniformStorageBuffer or m_psUniformStorageBuffer
		 * 
		 * @internal
		 */
		Buffer &                       _getUniformBufferTarget(gpu::ShaderUniformDeclaration *decl);

		/**
		 * @brief Upload a single uniform value to the GPU
		 * 
		 * Takes raw binary data from the CPU buffer, interprets it according to the uniform's type,
		 * and sends it to the GPU via the shader's setUniform() method.
		 * 
		 * This method handles the type dispatch - it reads the appropriate number of bytes from
		 * the buffer and casts them to the correct type before uploading.
		 * 
		 * @param uniform The uniform declaration
		 * @param data Pointer to the raw binary data in the CPU buffer
		 * 
		 * Supports: bool, int, uint, float, vec2-4, mat3-4, and arrays of these types
		 * 
		 * @internal
		 */
		void                           _uploadUniformFromBuffer(gpu::ShaderUniformDeclaration *uniform, uint8 *data);

		/// The shader this material is based on
		RefPtr<gpu::IShader>                  m_shader;

		/// Stored texture resources, indexed by shader register/slot
		std::vector<RefPtr<gpu::ITexture2D> > m_textures;

		/// CPU-side storage for vertex shader uniforms
		Buffer m_vsUniformStorageBuffer;

		/// CPU-side storage for pixel shader uniforms
		Buffer m_psUniformStorageBuffer;
	};
}
