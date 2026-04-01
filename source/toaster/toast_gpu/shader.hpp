/*!
 * @file shader.hpp
 * @brief Abstract shader interface with reflection and uniform management
 * @details
 * Defines the abstract interface for GPU shader programs, which contain the compiled GPU code
 * for vertex, pixel, geometry, and compute stages. This interface provides:
 * - Shader program creation and lifecycle management
 * - Uniform variable setting (legacy direct interface)
 * - Shader reflection (introspection of uniforms and resources)
 * - Material system integration (automatic uniform buffer management)
 *
 * **Shader Concepts:**
 * A shader is a specialized program that runs on the GPU during rendering:
 * - **Vertex Shader**: Processes each vertex (transform, lighting, etc.)
 * - **Pixel/Fragment Shader**: Determines final pixel color
 * - **Geometry Shader**: Creates/modifies vertices on GPU
 * - **Compute Shader**: General-purpose GPU computation
 *
 * **Reflection System:**
 * This interface includes a reflection system that discovers and manages shader inputs:
 * - Uniform variables (per-draw, per-object, per-frame data)
 * - Texture resources (diffuse, normal, etc.)
 * - Uniform buffers (grouped uniforms for efficient GPU transfer)
 *
 * **Integration with Material System:**
 * The Material class uses shader reflection to:
 * 1. Discover what uniforms the shader needs
 * 2. Automatically manage CPU-side uniform storage
 * 3. Upload data to GPU in optimized buffers
 * 4. Track texture dependencies
 *
 * **Typical Usage:**
 * @code
 * // Create shader from source files
 * auto shader = gpu::IShader::create("PBR", {
 *     {gpu::EShaderType::eVertex, vertex_source},
 *     {gpu::EShaderType::ePixel, pixel_source}
 * });
 *
 * // Query reflection information
 * const auto& vs_uniforms = shader->getVSMaterialUniforms();
 * const auto& resources = shader->getResources();
 *
 * // Create material from shader (typical usage)
 * auto material = gpu::Material::create(shader);
 * material->set("u_Color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
 * material->setTexture("u_DiffuseMap", texture);
 * @endcode
 *
 * @see shader_common.hpp for type system and reflection classes
 * @see Material for typical shader usage pattern
 * @see GLShader for OpenGL implementation
 *
 * @note Direct uniform setting (setUniform methods) is legacy; Material system preferred
 * @note Reflection requires OpenGL 3.3+ for introspection via glGetActiveUniform()
 */

#pragma once

#include "toast_lib/ptr.hpp"
#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"

#include "shader_common.hpp"

#include <map>
#include <glm/glm.hpp>

namespace toaster::gpu
{
	/*!
	 * @class IShader
	 * @brief Abstract interface for GPU shader program management and reflection
	 * @details
	 * IShader provides an abstraction layer for GPU shader program operations including:
	 * - Creation from source code
	 * - Uniform variable management
	 * - Shader reflection (discovering uniforms and resources)
	 * - Integration with Material system for automatic uniform handling
	 *
	 * **Shader Architecture:**
	 * A shader program consists of multiple stages (vertex, pixel, etc.) that work together:
	 * ```
	 * Input Data (Vertex Array)
	 *  ↓
	 * Vertex Shader (per-vertex processing)
	 *  ↓
	 * Fragment/Pixel Shader (per-pixel processing)
	 *  ↓
	 * Output (Rendered Image)
	 *
	 * Both stages can access:
	 * - Uniforms (constant during draw call)
	 * - Texture resources (sampled data)
	 * - Vertex attributes (position, normal, UV, etc.)
	 * ```
	 *
	 * **Reflection System:**
	 * The shader reflection interface automatically discovers shader inputs without
	 * manual specification. Two categories:
	 *
	 * 1. **Uniforms**: Scalar/vector/matrix values
	 *    ```cpp
	 *    uniform float u_Time;
	 *    uniform vec3 u_LightPos;
	 *    uniform mat4 u_Transform;
	 *    ```
	 *    - Grouped by stage (vertex, pixel)
	 *    - Organized into uniform buffers
	 *    - Automatic size and offset calculation
	 *
	 * 2. **Resources**: Textures and samplers
	 *    ```cpp
	 *    uniform sampler2D u_DiffuseMap;
	 *    uniform samplerCube u_EnvironmentMap;
	 *    ```
	 *    - Named resource declaration
	 *    - Type and dimension information
	 *
	 * **Uniform Management:**
	 * Two approaches to setting uniforms:
	 *
	 * 1. **Direct Method (Legacy)**: One-off setUniform() calls
	 *    ```cpp
	 *    shader->setUniform("u_Color", glm::vec4(1,0,0,1));
	 *    shader->setUniform("u_Transform", matrix);
	 *    // Slow: each call transfers to GPU individually
	 *    ```
	 *
	 * 2. **Material System (Recommended)**: Batch updates via reflection
	 *    ```cpp
	 *    auto material = Material::create(shader);
	 *    material->set("u_Color", glm::vec4(1,0,0,1));
	 *    material->set("u_Transform", matrix);
	 *    material->use();  // Single efficient GPU transfer
	 *    ```
	 *
	 * **Performance Characteristics:**
	 * - Creation: O(n) where n = source code size (compile + link)
	 * - Direct setUniform(): O(1) per call (but multiple calls are slow)
	 * - Reflection discovery: O(m) where m = number of uniforms/resources
	 * - Material batch upload: O(1) amortized (one GPU transfer for all)
	 *
	 * **Typical Rendering Flow:**
	 * @code
	 * // Setup (once)
	 * auto shader = IShader::create("MyShader", {...});
	 * auto material = Material::create(shader);
	 *
	 * // Per-frame
	 * for (const auto& object : scene.objects) {
	 *     // Update material parameters
	 *     material->set("u_MVP", mvp_matrix);
	 *     material->set("u_Color", object.color);
	 *     material->setTexture("u_DiffuseMap", object.texture);
	 *
	 *     // Render with optimized upload
	 *     material->use();
	 *     vao->bind();
	 *     glDrawElements(...);
	 * }
	 * @endcode
	 *
	 * @see shader_common.hpp for reflection type definitions
	 * @see Material for recommended shader usage pattern
	 * @see GLShader for OpenGL implementation details
	 *
	 * @note Use Material system for efficient uniform management
	 * @note Direct setUniform() useful for one-off changes only
	 * @note Reflection requires OpenGL 3.3+ (automatic via glGetActiveUniform)
	 * @note Shader lifecycle: Create → Use → Destroy (automatic via RefPtr)
	 */
	class IShader
	{
	public:
		/*!
		 * @brief Factory method: Creates a shader program from source code
		 * @details
		 * Compiles and links shader source code into a GPU shader program.
		 * Accepts a map of shader stages (vertex, pixel, etc.) with their source strings.
		 *
		 * **Shader Source Format:**
		 * - GLSL (OpenGL Shading Language) source code
		 * - Version directives (e.g., "#version 450")
		 * - Function definitions (varying per stage)
		 *
		 * **Typical Usage:**
		 * @code
		 * // Load shader files
		 * const auto shader_dir = io::filesystem::getAssetDirectory() / "shaders";
		 * const auto vs_source = io::filesystem::readFile(shader_dir / "mesh.vert.glsl");
		 * const auto ps_source = io::filesystem::readFile(shader_dir / "mesh.pixel.glsl");
		 *
		 * // Create shader program
		 * auto shader = gpu::IShader::create("MeshShader", {
		 *     {gpu::EShaderType::eVertex, vs_source},
		 *     {gpu::EShaderType::ePixel, ps_source}
		 * });
		 *
		 * // After creation, shader is ready for use
		 * shader->bind();  // Optional (mainly for debug)
		 * @endcode
		 *
		 * **Shader Stages:**
		 * Map entries:
		 * - EShaderType::eVertex: Vertex processing (required)
		 * - EShaderType::ePixel: Fragment/pixel processing (required)
		 * - EShaderType::eGeometry: Geometry generation (optional)
		 * - EShaderType::eCompute: General compute (optional, mutually exclusive with other stages)
		 *
		 * **Compilation Process:**
		 * 1. For each shader stage in map:
		 *    - Compile GLSL source to GPU bytecode
		 *    - Verify compilation success
		 * 2. Link all stages into single program
		 * 3. Verify link success
		 * 4. Perform reflection (discover uniforms/resources)
		 *
		 * **Error Handling:**
		 * Compilation/link errors typically:
		 * - Throw exceptions with error details
		 * - Return nullptr with error logging
		 * - Print to debug output
		 *
		 * **Reflection Side-Effect:**
		 * Creating a shader triggers automatic reflection discovery:
		 * - All uniform variables discovered
		 * - All texture resources found
		 * - Uniforms grouped by stage (vertex, pixel)
		 * - Organize into uniform buffers
		 * Ready for Material system use
		 *
		 * **Performance:**
		 * - Compilation: 50ms-500ms (depends on complexity)
		 * - Linking: 10ms-100ms
		 * - Reflection: 1ms-10ms
		 * - **Total**: ~100ms-1s typical (do during load, not per-frame!)
		 *
		 * @param p_name Human-readable shader name (for debugging)
		 * @param p_shader_source_map Map of EShaderType → source code string
		 *                             At minimum: eVertex + ePixel
		 *
		 * @return RefPtr to newly created and linked IShader
		 *
		 * **Complexity:** O(n) where n = total source code size
		 *                 (compilation and linking are linear in code size)
		 *
		 * @see EShaderType for available shader stages
		 * @see Material::create(IShader*) for typical shader usage
		 *
		 * @throws or returns nullptr on compilation failure
		 * @throws or returns nullptr on link failure
		 * @note Compile at load time, not during rendering
		 * @note Shader names help with debugging and caching
		 */
		static RefPtr<IShader> create(const String &p_name, const std::unordered_map<EShaderType, String> &p_shader_source_map);

		virtual ~IShader() = default;

		/*!
		 * @brief Gets the GPU shader program identifier
		 * @details
		 * Returns the backend-specific GPU resource handle.
		 * In OpenGL, this is the program object name (uint32 ID).
		 *
		 * **Typical Usage:**
		 * Mainly for debugging and internal use.
		 * @code
		 * uint32 program_id = shader->getID();
		 * // Debug: std::cout << "Shader program ID: " << program_id << '\n';
		 * @endcode
		 *
		 * @return GPU resource ID (OpenGL program name)
		 *
		 * @note Used internally; rarely needed by end users
		 * @note Don't use ID for direct GL calls; use the interface instead
		 */
		virtual uint32 getID() const = 0;

		/*!
		 * @brief Binds shader program to current OpenGL context
		 * @details
		 * Makes this shader the active program for subsequent rendering.
		 * In OpenGL terms, this calls glUseProgram(program_id).
		 *
		 * **Note on Direct State Access (DSA):**
		 * The OpenGL implementation uses Direct State Access (DSA) via
		 * glProgramUniform*() functions, which don't require binding the program.
		 * Therefore, bind() is mainly for organizational purposes and compatibility.
		 *
		 * **Typical Usage (Optional):**
		 * @code
		 * // Traditional binding (not required for Material system)
		 * shader->bind();
		 * // Now shader is active - unnecessary with DSA
		 * shader->unbind();
		 * @endcode
		 *
		 * **Material System Usage:**
		 * The Material class handles binding internally.
		 * Manual binding is rarely needed when using Materials.
		 *
		 * @note Modern OpenGL prefers Direct State Access (glProgramUniform*())
		 * @note Binding is unnecessary if using glProgramUniform*() for uniforms
		 * @see unbind() to deactivate the shader
		 * @see Material for recommended shader usage
		 */
		// Not really used because the OpenGL implementation uses Direct State Access (DSA)
		virtual void bind() const = 0;

		/*!
		 * @brief Unbinds shader program from current OpenGL context
		 * @details
		 * Deactivates this shader by binding the null program (id 0).
		 * In OpenGL terms, this calls glUseProgram(0).
		 *
		 * Generally not required with DSA (Direct State Access).
		 *
		 * @note Can be omitted when using glProgramUniform*()
		 * @see bind() to activate the shader
		 */
		virtual void unbind() const = 0;

		/*!
		 * @brief Sets a uniform float value
		 * @details
		 * Sets a shader uniform variable to a float value.
		 * **Note:** Prefer Material system for batch efficiency.
		 *
		 * Uses Direct State Access (DSA) via glProgramUniformf(), no binding required.
		 *
		 * @param p_name Shader uniform name (e.g., "u_Time")
		 * @param p_value Float value to set
		 *
		 * **Typical Usage (Legacy):**
		 * @code
		 * shader->setUniform("u_Time", elapsed_time);
		 * shader->setUniform("u_Opacity", 0.5f);
		 * @endcode
		 *
		 * **Recommended (Material System):**
		 * @code
		 * material->set("u_Time", elapsed_time);
		 * material->use();  // Efficient batch upload
		 * @endcode
		 *
		 * @note Legacy interface; use Material system for efficiency
		 * @note Each call sends value to GPU individually
		 * @see Material for batch uniform management
		 */
		virtual void setUniform(const String &p_name, float32 p_value) = 0;

		/*!
		 * @brief Sets a uniform int value
		 * @see setUniform(const String&, float32) for documentation
		 */
		virtual void setUniform(const String &p_name, int32 p_value) = 0;

		/*!
		 * @brief Sets a uniform uint value
		 * @see setUniform(const String&, float32) for documentation
		 */
		virtual void setUniform(const String &p_name, uint32 p_value) = 0;

		/*!
		 * @brief Sets a uniform vec2 value
		 * @see setUniform(const String&, float32) for documentation
		 */
		virtual void setUniform(const String &p_name, const glm::vec2 &p_value) = 0;

		/*!
		 * @brief Sets a uniform vec3 value
		 * @see setUniform(const String&, float32) for documentation
		 */
		virtual void setUniform(const String &p_name, const glm::vec3 &p_value) = 0;

		/*!
		 * @brief Sets a uniform vec4 value
		 * @see setUniform(const String&, float32) for documentation
		 */
		virtual void setUniform(const String &p_name, const glm::vec4 &p_value) = 0;

		/*!
		 * @brief Sets a uniform mat3 value
		 * @see setUniform(const String&, float32) for documentation
		 */
		virtual void setUniform(const String &p_name, const glm::mat3 &p_value) = 0;

		/*!
		 * @brief Sets a uniform mat4 value
		 * @see setUniform(const String&, float32) for documentation
		 */
		virtual void setUniform(const String &p_name, const glm::mat4 &p_value) = 0;

		/*!
		 * @brief Sets a uniform array of floats
		 * @details
		 * Sets a shader uniform array to float values.
		 *
		 * @param p_name Shader uniform name (array)
		 * @param p_values Pointer to float array
		 * @param p_count Number of floats in array
		 *
		 * @note Legacy interface; use Material system for efficiency
		 * @see setUniform(const String&, float32) for documentation
		 */
		virtual void setUniform(const String &p_name, float32 *p_values, uint32 p_count) = 0;

		/*!
		 * @brief Sets a uniform array of ints
		 * @see setUniform(const String&, float32*, uint32) for documentation
		 */
		virtual void setUniform(const String &p_name, int32 *p_values, uint32 p_count) = 0;

		/*!
		 * @brief Sets a uniform array of uints
		 * @see setUniform(const String&, float32*, uint32) for documentation
		 */
		virtual void setUniform(const String &p_name, uint32 *p_values, uint32 p_count) = 0;

		/*!
		 * @brief Gets material uniforms for vertex shader stage
		 * @details
		 * Returns const reference to the list of uniform declarations for the vertex shader stage.
		 * These uniforms are used by the Material system to build the CPU-side uniform buffer
		 * for efficient batch uploads.
		 *
		 * **Reflection Information:**
		 * List contains ShaderUniformBufferDeclaration objects, each representing:
		 * - Uniform variable name
		 * - Data type (float, vec3, mat4, etc.)
		 * - Byte size and offset
		 * - Stage affinity (vertex vs pixel)
		 *
		 * **Typical Usage:**
		 * @code
		 * auto material = Material::create(shader);
		 * const auto& vs_uniforms = shader->getVSMaterialUniforms();
		 *
		 * // Material uses this to set up CPU storage
		 * for (const auto& uniform : vs_uniforms) {
		 *     // Build buffer, allocate storage, etc.
		 * }
		 * @endcode
		 *
		 * **Example Uniforms (Vertex Stage):**
		 * - u_MVP: mat4 (transformation matrix)
		 * - u_ModelMatrix: mat4 (object transform)
		 * - u_NormalMatrix: mat3 (normal transformation)
		 * - u_Time: float (animation parameter)
		 *
		 * @return Const reference to ShaderUniformBufferList (vector of uniform declarations)
		 *
		 * @see getPSMaterialUniforms() for pixel stage uniforms
		 * @see getResources() for texture resources
		 * @see Material system for integration
		 *
		 * @note Returns type: ShaderUniformBufferList (typedef for list of declarations)
		 * @note Empty list if no uniforms in vertex stage
		 */
		virtual const ShaderUniformBufferList &getVSMaterialUniforms() const = 0;

		/*!
		 * @brief Gets material uniforms for pixel/fragment shader stage
		 * @details
		 * Returns const reference to the list of uniform declarations for the pixel shader stage.
		 * Similar to getVSMaterialUniforms() but for fragment processing.
		 *
		 * **Example Uniforms (Pixel Stage):**
		 * - u_Color: vec4 (diffuse color)
		 * - u_Roughness: float (material roughness)
		 * - u_Metallic: float (material metallic factor)
		 * - u_EmissionColor: vec3 (self-illumination)
		 *
		 * @return Const reference to ShaderUniformBufferList (vertex of uniform declarations)
		 *
		 * @see getVSMaterialUniforms() for vertex stage uniforms
		 * @see getResources() for texture resources
		 * @see Material system for integration
		 *
		 * @note Empty list if no uniforms in pixel stage
		 */
		virtual const ShaderUniformBufferList &getPSMaterialUniforms() const = 0;

		/*!
		 * @brief Gets texture and sampler resources
		 * @details
		 * Returns const reference to the list of shader resource declarations
		 * (textures, samplers, etc.) used by the shader.
		 *
		 * **Resource Information:**
		 * List contains ShaderResourceDeclaration objects, each representing:
		 * - Resource name (e.g., "u_DiffuseMap")
		 * - Resource type (Texture2D, TextureCube, etc.)
		 * - Binding slot (texture unit)
		 * - Dimension/format info
		 *
		 * **Typical Usage:**
		 * @code
		 * const auto& resources = shader->getResources();
		 *
		 * for (const auto& resource : resources) {
		 *     if (resource->getResourceType() == EShaderResourceType::eTexture2D) {
		 *         // Material needs a 2D texture for this resource
		 *     }
		 * }
		 * @endcode
		 *
		 * **Example Resources:**
		 * - u_DiffuseMap: Texture2D (color)
		 * - u_NormalMap: Texture2D (surface normals)
		 * - u_SpecularMap: Texture2D (specularity)
		 * - u_EnvironmentMap: TextureCube (reflections/IBL)
		 *
		 * @return Const reference to ShaderResourceList (vector of resource declarations)
		 *
		 * @see getVSMaterialUniforms() for uniform variables
		 * @see getPSMaterialUniforms() for uniform variables
		 * @see findResourceDeclaration() to find specific resource
		 * @see Material system for integration
		 *
		 * @note Returns type: ShaderResourceList (typedef for list of resource declarations)
		 * @note Empty list if shader uses no textures/resources
		 */
		virtual const ShaderResourceList &getResources() const = 0;

		/*!
		 * @brief Finds a uniform declaration by name
		 * @details
		 * Searches for a uniform variable declaration in the shader reflection data.
		 * Used to look up specific uniforms without iterating all declarations.
		 *
		 * **Typical Usage:**
		 * @code
		 * if (const auto* decl = shader->findUniformDeclaration("u_MVP")) {
		 *     // Found the MVP matrix uniform
		 *     uint32 size = decl->getSize();
		 *     EShaderUniformType type = decl->getType();
		 *     // Use declaration info
		 * }
		 * @endcode
		 *
		 * **Search Scope:**
		 * Searches across:
		 * - All shader stages (vertex, pixel, etc.)
		 * - Both per-stage and global uniforms
		 * - All uniform names in the shader
		 *
		 * @param name Uniform variable name to search for
		 *
		 * @return Pointer to ShaderUniformDeclaration if found
		 *         nullptr if not found
		 *
		 * **Complexity:** O(n) where n = number of uniforms (linear search)
		 *
		 * @see getVSMaterialUniforms() to list all vertex stage uniforms
		 * @see getPSMaterialUniforms() to list all pixel stage uniforms
		 * @see findResourceDeclaration() for texture resources
		 *
		 * @note Returns nullptr if name not found
		 * @note Check for nullptr before dereferencing
		 */
		virtual ShaderUniformDeclaration *findUniformDeclaration(const String &name) = 0;

		/*!
		 * @brief Finds a resource declaration by name
		 * @details
		 * Searches for a texture/resource declaration in the shader reflection data.
		 * Used to look up specific resources (textures, samplers) without iterating.
		 *
		 * **Typical Usage:**
		 * @code
		 * if (const auto* decl = shader->findResourceDeclaration("u_DiffuseMap")) {
		 *     // Found the diffuse texture
		 *     EShaderResourceType type = decl->getResourceType();
		 *     // Use resource info
		 * }
		 * @endcode
		 *
		 * **Resource Types:**
		 * Finds:
		 * - Texture2D samplers
		 * - Texture3D samplers
		 * - TextureCube samplers
		 * - Storage textures
		 * - Custom resource types
		 *
		 * @param name Resource variable name to search for
		 *
		 * @return Pointer to ShaderResourceDeclaration if found
		 *         nullptr if not found
		 *
		 * **Complexity:** O(n) where n = number of resources (linear search)
		 *
		 * @see getResources() to list all resources
		 * @see findUniformDeclaration() for uniform variables
		 *
		 * @note Returns nullptr if name not found
		 * @note Check for nullptr before dereferencing
		 */
		virtual ShaderResourceDeclaration *findResourceDeclaration(const String &name) = 0;

		/*!
		 * @brief Gets all uniform declarations (backward compatibility)
		 * @details
		 * Returns a vector of all uniform declarations across all shader stages.
		 * This is a backward compatibility interface; prefer getVSMaterialUniforms()
		 * and getPSMaterialUniforms() for stage-specific information.
		 *
		 * **Deprecated Note:**
		 * This method is provided for backward compatibility with older code.
		 * New code should use getVSMaterialUniforms() and getPSMaterialUniforms()
		 * to distinguish between vertex and pixel stage uniforms.
		 *
		 * **Typical Usage (Legacy):**
		 * @code
		 * const auto& all_uniforms = shader->getUniformDeclarations();
		 * for (const auto* decl : all_uniforms) {
		 *     // Process uniform
		 * }
		 * @endcode
		 *
		 * **Recommended (Modern):**
		 * @code
		 * const auto& vs_uniforms = shader->getVSMaterialUniforms();
		 * const auto& ps_uniforms = shader->getPSMaterialUniforms();
		 * // Process stage-specific uniforms
		 * @endcode
		 *
		 * @return Const reference to vector of ShaderUniformDeclaration pointers
		 *         Contains all uniforms from all stages
		 *
		 * @see getVSMaterialUniforms() for vertex stage only
		 * @see getPSMaterialUniforms() for pixel stage only
		 *
		 * @deprecated Use stage-specific getVSMaterialUniforms() / getPSMaterialUniforms()
		 * @note Included for backward compatibility with existing code
		 * @note Empty vector if no uniforms in shader
		 */
		virtual const std::vector<ShaderUniformDeclaration *> &getUniformDeclarations() const = 0;
	};
}
