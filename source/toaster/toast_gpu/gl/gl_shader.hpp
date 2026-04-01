/*!
 * @file gl_shader.hpp
 * @brief OpenGL shader implementation with native reflection system
 * 
 * GLShader provides the main OpenGL implementation of IShader with built-in reflection.
 * It uses OpenGL's native glGetActiveUniform() and related functions to discover shader
 * metadata without requiring source code parsing or external tools.
 * 
 * ## Reflection Pipeline
 * 
 * The reflection system works in three phases:
 * 
 * 1. **Shader Compilation** (in constructor)
 *    - Creates shader objects for each stage (VS, GS, FS)
 *    - Compiles shader source code
 *    - Links shaders into a program
 * 
 * 2. **Reflection** (in _reflect())
 *    - Queries active uniforms with glGetActiveUniform()
 *    - Converts GL types to engine types (GL_FLOAT → eFloat)
 *    - Organizes uniforms into per-domain buffers
 *    - Discovers texture resources and their binding points
 * 
 * 3. **Material Usage** (in Material class)
 *    - Material queries the shader for uniform buffers
 *    - Allocates CPU storage matching buffer sizes
 *    - User sets uniform values in buffers
 *    - During rendering, values are uploaded to GPU
 * 
 * ## Uniform Organization
 * 
 * The reflection system organizes uniforms into buffers by shader domain:
 * - Vertex shader material uniforms → m_vsMaterialUniforms
 * - Pixel shader material uniforms → m_psMaterialUniforms
 * - Resources (samplers, images) → m_resources
 * 
 * This organization enables efficient batch uploads during Material::use().
 * 
 * @note The current implementation uses a conservative approach: all discovered
 *       uniforms go to the VS buffer. Proper per-stage reflection would require
 *       parsing shader source or using GL_ARB_separate_shader_objects.
 * 
 * @see gl_shader_uniform.hpp for uniform and resource classes
 * @see material.hpp for how uniforms are used
 */
#pragma once

#include <unordered_map>
#include <openglhpp/opengl.hpp>

#include "../shader.hpp"
#include "gl_shader_uniform.hpp"

namespace toaster::gpu
{
	/**
	 * @brief Convert engine shader type to OpenGL shader stage
	 * 
	 * Maps EShaderType enums (eVertex, ePixel, etc.) to OpenGL's gl::ShaderStage
	 * enum values for use with OpenGL shader creation functions.
	 * 
	 * @param p_stage The engine shader type
	 * @return gl::ShaderStage The corresponding OpenGL shader stage
	 */
	gl::ShaderStage getShaderStage(EShaderType p_stage);

	/**
	 * @class GLShader
	 * @brief OpenGL implementation of IShader with reflection
	 * 
	 * Manages shader compilation, linking, and reflection for OpenGL. Features:
	 * - Compiles multiple shader stages into a single program
	 * - Reflects over compiled program to discover uniforms and resources
	 * - Provides uniform setting via glProgramUniform*() DSA functions
	 * - Organizes uniforms by shader domain for efficient management
	 * 
	 * ## Usage Example
	 * 
	 * @code
	 * // Create a shader from source code
	 * auto shader = gpu::IShader::create("MyShader", {
	 *     {EShaderType::eVertex, vertex_source},
	 *     {EShaderType::ePixel, pixel_source}
	 * });
	 * 
	 * // Shader is automatically reflected - uniforms are discovered
	 * auto& vs_uniforms = shader->getVSMaterialUniforms();
	 * 
	 * // Create material from shader
	 * auto material = Material::create(shader);
	 * 
	 * // Use material
	 * material->set("u_Time", elapsed_time);
	 * material->use();
	 * @endcode
	 */
	class GLShader : public IShader
	{
	public:
		/**
		 * @brief Construct and compile a shader
		 * 
		 * @param p_name Human-readable name for debugging
		 * @param p_shader_source_map Map of EShaderType to source code strings
		 * 
		 * Compiles each shader stage, links them into a program, and runs reflection.
		 * If compilation fails, logs an error and continues (partial shader).
		 * 
		 * @code
		 * auto shader = new GLShader("MyShader", {
		 *     {EShaderType::eVertex, "#version 460\n..."},
		 *     {EShaderType::ePixel, "#version 460\n..."}
		 * });
		 * @endcode
		 */
		GLShader(String p_name, const std::unordered_map<EShaderType, String> &p_shader_source_map);

		/// @brief Destructor - cleans up GPU resources and reflection data
		~GLShader() override;

		/// @brief Get the OpenGL program ID
		uint32_t getID() const override;

		/// @brief Bind this shader for rendering (glUseProgram)
		void bind() const override;

		/// @brief Unbind any shader (glUseProgram(0))
		void unbind() const override;

		/**
		 * @brief Set a float uniform
		 * 
		 * Uses glProgramUniform1f() to set the uniform without requiring a shader bind.
		 * Uniform locations are cached for performance.
		 * 
		 * @param p_name Name of the uniform (e.g., "u_Time")
		 * @param p_value The value to set
		 */
		void setUniform(const String &p_name, float32 p_value) override;

		/**
		 * @brief Set a signed integer uniform
		 * @param p_name Name of the uniform
		 * @param p_value The value to set
		 */
		void setUniform(const String &p_name, int32 p_value) override;

		/**
		 * @brief Set an unsigned integer uniform
		 * @param p_name Name of the uniform
		 * @param p_value The value to set
		 */
		void setUniform(const String &p_name, uint32 p_value) override;

		/**
		 * @brief Set a 2D vector uniform
		 * @param p_name Name of the uniform
		 * @param p_value The vector to set
		 */
		void setUniform(const String &p_name, const glm::vec2 &p_value) override;

		/**
		 * @brief Set a 3D vector uniform
		 * @param p_name Name of the uniform
		 * @param p_value The vector to set
		 */
		void setUniform(const String &p_name, const glm::vec3 &p_value) override;

		/**
		 * @brief Set a 4D vector uniform
		 * @param p_name Name of the uniform
		 * @param p_value The vector to set
		 */
		void setUniform(const String &p_name, const glm::vec4 &p_value) override;

		/**
		 * @brief Set a 3x3 matrix uniform
		 * @param p_name Name of the uniform
		 * @param p_value The matrix to set
		 */
		void setUniform(const String &p_name, const glm::mat3 &p_value) override;

		/**
		 * @brief Set a 4x4 matrix uniform
		 * @param p_name Name of the uniform
		 * @param p_value The matrix to set
		 */
		void setUniform(const String &p_name, const glm::mat4 &p_value) override;

		/**
		 * @brief Set an array of float uniforms
		 * @param p_name Name of the uniform
		 * @param p_values Pointer to array
		 * @param p_count Number of elements
		 */
		void setUniform(const String &p_name, float32 *p_values, uint32 p_count) override;

		/**
		 * @brief Set an array of signed integer uniforms
		 * @param p_name Name of the uniform
		 * @param p_values Pointer to array
		 * @param p_count Number of elements
		 */
		void setUniform(const String &p_name, int32 *p_values, uint32 p_count) override;

		/**
		 * @brief Set an array of unsigned integer uniforms
		 * @param p_name Name of the uniform
		 * @param p_values Pointer to array
		 * @param p_count Number of elements
		 */
		void setUniform(const String &p_name, uint32 *p_values, uint32 p_count) override;

		/**
		 * @brief Get vertex shader material uniform buffers
		 * 
		 * Returns all uniform buffers for the vertex shader stage.
		 * Typically contains one buffer with all discovered VS uniforms.
		 * 
		 * @return const ShaderUniformBufferList& List of uniform buffers
		 * 
		 * @see getVSMaterialUniforms() in material.hpp for usage
		 */
		const ShaderUniformBufferList &getVSMaterialUniforms() const override { return m_vsMaterialUniforms; }

		/**
		 * @brief Get pixel shader material uniform buffers
		 * 
		 * Returns all uniform buffers for the pixel/fragment shader stage.
		 * May be empty if the shader has no pixel-stage uniforms.
		 * 
		 * @return const ShaderUniformBufferList& List of uniform buffers
		 */
		const ShaderUniformBufferList &getPSMaterialUniforms() const override { return m_psMaterialUniforms; }

		/**
		 * @brief Get shader resources (textures, images)
		 * 
		 * Returns all discovered shader resources (samplers) and their binding points.
		 * 
		 * @return const ShaderResourceList& List of resources
		 */
		const ShaderResourceList &     getResources() const override { return m_resources; }

		/**
		 * @brief Find a uniform declaration by name
		 * 
		 * Searches through all uniform buffers to find a uniform by name.
		 * 
		 * @param name The uniform name to search for
		 * @return ShaderUniformDeclaration* Pointer to the uniform, or nullptr if not found
		 * 
		 * @see Material::findUniformDeclaration() which delegates to this method
		 */
		ShaderUniformDeclaration * findUniformDeclaration(const String &name) override;

		/**
		 * @brief Find a resource declaration by name
		 * 
		 * Searches through resources to find a texture or image by name.
		 * 
		 * @param name The resource name to search for
		 * @return ShaderResourceDeclaration* Pointer to the resource, or nullptr if not found
		 * 
		 * @see Material::findResourceDeclaration() which delegates to this method
		 */
		ShaderResourceDeclaration *findResourceDeclaration(const String &name) override;

		/**
		 * @brief Get uniform declarations (backward compatibility)
		 * 
		 * Returns an empty vector for backward compatibility with old code.
		 * New code should use getVSMaterialUniforms() or getPSMaterialUniforms() instead.
		 * 
		 * @return const std::vector<ShaderUniformDeclaration*>& Always empty
		 * 
		 * @deprecated Use getVSMaterialUniforms() instead
		 */
		const std::vector<ShaderUniformDeclaration *> &getUniformDeclarations() const override;

	private:
		/**
		 * @brief Perform shader reflection
		 * 
		 * Queries the compiled program for active uniforms and resources using
		 * OpenGL's reflection API (glGetActiveUniform, glGetUniformLocation).
		 * 
		 * This method is called after shader linking and populates:
		 * - m_vsMaterialUniforms: all uniforms organized for VS
		 * - m_psMaterialUniforms: empty (current conservative approach)
		 * - m_resources: texture samplers and their binding points
		 * 
		 * The reflection pipeline:
		 * 1. Query total number of active uniforms
		 * 2. For each uniform:
		 *    a. Get metadata (name, type, size)
		 *    b. Convert GL type to engine type
		 *    c. Classify as uniform or resource
		 *    d. Add to appropriate buffer with offset management
		 * 
		 * @internal
		 */
		void _reflect();

		String m_name;                          ///< Shader name for debugging
		gl::ID m_programId{0u};                ///< OpenGL program ID

		std::unordered_map<EShaderType, String> m_shaderSourceMap; ///< Original source code
		std::unordered_map<String, gl::Int>     m_uniformLocations; ///< Cached uniform locations

		// Reflection results organized by domain
		ShaderUniformBufferList m_vsMaterialUniforms; ///< Vertex shader uniforms
		ShaderUniformBufferList m_psMaterialUniforms; ///< Pixel shader uniforms (usually empty)
		ShaderResourceList      m_resources;         ///< Texture samplers
	};
}
