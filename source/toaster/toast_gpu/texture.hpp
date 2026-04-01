/*!
 * @file texture.hpp
 * @brief Abstract texture interface for GPU image resource management
 * @details
 * Defines abstract interfaces for GPU texture resources, supporting both generic texture operations
 * and 2D texture-specific functionality. Textures store image data on the GPU for use in rendering,
 * enabling efficient shader sampling and pixel operations.
 *
 * **Texture Concepts:**
 * - Textures are GPU image resources (similar to CPU buffers)
 * - Store pixel data in specialized GPU formats for fast random access
 * - Support sampling in shaders (texture lookups during fragment processing)
 * - Can be read from files, created from raw pixel data, or rendered to (FBO attachment)
 * - Various formats supported: RGBA8, RGBA32F, Depth24Stencil8, etc.
 *
 * **Texture Types:**
 * - ITexture: Base class with common operations
 * - ITexture2D: 2D texture (most common; created from files or pixel buffers)
 * - Implied: Texture3D, TextureCube (derived from ITexture)
 *
 * **Typical Usage:**
 * @code
 * // Create from file
 * auto diffuse = gpu::ITexture2D::create("assets/textures/wood.png");
 *
 * // Create from size (e.g., for rendering)
 * auto render_target = gpu::ITexture2D::create(1920, 1080);
 *
 * // Use in shader
 * material->setTexture("u_DiffuseTexture", diffuse);
 *
 * // Bind and sample
 * diffuse->bind(0);  // Bind to texture unit 0
 * // In shader: sampler2D u_Diffuse; ... texture(u_Diffuse, uv);
 * @endcode
 *
 * @see ITexture2D for 2D texture creation and management
 * @see Material for texture resource binding in rendering
 * @see GLTexture2D for OpenGL implementation
 */

#pragma once

#include "toast_lib/ptr.hpp"
#include "toast_lib/system_types.h"
#include "toast_lib/io/filesystem.hpp"

#include <optional>

namespace toaster::gpu
{
	/*!
	 * @class ITexture
	 * @brief Abstract base class for GPU texture resources
	 * @details
	 * ITexture provides the common interface for all texture types (2D, 3D, Cube, etc.).
	 * It defines essential texture operations like binding, data updates, and metadata queries.
	 *
	 * **Texture Lifecycle:**
	 * 1. **Creation**: Implemented by derived classes (ITexture2D, etc.)
	 * 2. **Configuration**: setData() to upload pixel data if needed
	 * 3. **Binding**: bind(slot) to activate for rendering
	 * 4. **Sampling**: Shader samples texture during rendering
	 * 5. **Destruction**: Automatic via RefPtr smart pointer
	 *
	 * **Memory Layout:**
	 * Textures use specialized GPU memory with:
	 * - Hardware filtering (linear, bilinear, trilinear)
	 * - Swizzling and tiling for cache efficiency
	 * - Compression support (DXT, BC formats)
	 * - Mipmapping (for levels of detail)
	 *
	 * **Binding and Texture Units:**
	 * Textures are bound to texture units (typically 0-31) for shader access:
	 * ```cpp
	 * texture->bind(0);  // Bind to unit 0
	 * // In shader: uniform sampler2D u_Tex0; texture(u_Tex0, uv);
	 * ```
	 *
	 * **Typical Derived Classes:**
	 * - ITexture2D: Most common (files, renderable)
	 * - Texture3D: Volume textures (not shown in interface yet)
	 * - TextureCube: Environment/cubemaps (not shown in interface yet)
	 *
	 * **Performance Characteristics:**
	 * - Memory: Variable by format; typical 4 bytes/pixel (RGBA8)
	 * - Binding: O(1) GPU state change
	 * - Sampling: O(1) GPU operation (hardware accelerated)
	 * - Filtering: Hardware support (linear, trilinear, etc.)
	 *
	 * @see ITexture2D for concrete 2D texture implementation
	 * @see Material for texture binding in rendering
	 * @see GLTexture for OpenGL backend
	 *
	 * @note Don't use ITexture directly; use derived classes
	 * @note Bind to texture unit before shaders sample
	 * @note Format and filtering affect shader output quality and performance
	 */
	class ITexture
	{
	public:
		virtual ~ITexture() = default;

		/*!
		 * @brief Uploads pixel data to GPU texture
		 * @details
		 * Copies pixel data from CPU memory to GPU texture memory.
		 * This can be called multiple times to update texture contents.
		 *
		 * **Data Format:**
		 * - Data layout depends on texture format (RGBA8, RGBA32F, etc.)
		 * - Typical: 4 bytes per pixel for RGBA8 (Red, Green, Blue, Alpha)
		 * - Size = width * height * bytes_per_pixel
		 *
		 * **Usage Patterns:**
		 * @code
		 * // Pattern 1: Load from file (typically wrapped by ITexture2D::create)
		 * auto png_data = loadPNGFile("texture.png");
		 * texture->setData(png_data.data(), png_data.size());
		 *
		 * // Pattern 2: Procedural texture
		 * std::vector<uint8_t> pixels(width * height * 4);
		 * generateCheckerboard(pixels);
		 * texture->setData(pixels.data(), pixels.size());
		 *
		 * // Pattern 3: Dynamic texture (e.g., video playback)
		 * for (auto& frame : video_frames) {
		 *     texture->setData(frame.data(), frame.size());
		 *     renderFrame();
		 * }
		 * @endcode
		 *
		 * **Parameters:**
		 * @param p_data Pointer to pixel data in CPU memory
		 * @param p_size Size in bytes of pixel data (width * height * bpp)
		 *
		 * **Performance:**
		 * - Memory transfer: ~20-50 GB/s typical PCIe bandwidth
		 * - Synchronization: CPU may block until GPU receives data
		 * - Large textures (4K+) can cause frame rate hiccups
		 *
		 * @note Size must match texture dimensions and format
		 * @note Large uploads should be done offline or via streaming
		 * @see ITexture2D::create() for typical creation patterns
		 */
		virtual void setData(void *p_data, uint32 p_size) = 0;

		/*!
		 * @brief Binds texture to a GPU texture unit
		 * @details
		 * Activates this texture for shader sampling by binding it to a texture unit.
		 * In OpenGL terms, this:
		 * 1. Calls glActiveTexture(GL_TEXTURE0 + p_slot)
		 * 2. Calls glBindTexture(GL_TEXTURE_2D, texture_id) or similar
		 *
		 * **Texture Units:**
		 * Graphics hardware typically supports 16-32 texture units (0-31).
		 * Each unit can hold one texture for shader sampling.
		 *
		 * **Shader Binding:**
		 * After binding, shader samples from the unit:
		 * @code
		 * // C++ code
		 * texture->bind(0);
		 *
		 * // Shader code
		 * uniform sampler2D u_Texture0;  // Implicitly uses unit 0
		 * vec4 color = texture(u_Texture0, uv);
		 * @endcode
		 *
		 * **Typical Pattern:**
		 * @code
		 * // Bind multiple textures for multi-texturing
		 * diffuse_texture->bind(0);
		 * normal_texture->bind(1);
		 * specular_texture->bind(2);
		 *
		 * // Set shader uniform samplers if needed
		 * shader->setUniform("u_Diffuse", 0);
		 * shader->setUniform("u_Normal", 1);
		 * shader->setUniform("u_Specular", 2);
		 *
		 * // Render geometry
		 * renderMesh();
		 * @endcode
		 *
		 * **Parameters:**
		 * @param p_slot Texture unit index (0-31, default: 0)
		 *
		 * **Complexity:** O(1) - GPU state change (minimal overhead)
		 *
		 * @note Typical default slot: 0 (for single textures)
		 * @note Binding persists until changed or texture deleted
		 * @note Const: doesn't modify texture state, only binds to units
		 * @note Multi-texturing requires multiple bind calls with different slots
		 */
		virtual void bind(uint32 p_slot = 0) const = 0;

		/*!
		 * @brief Gets the GPU resource identifier
		 * @details
		 * Returns the backend-specific GPU resource handle.
		 * In OpenGL, this is the texture object name (uint32 ID).
		 *
		 * @return GPU resource ID (OpenGL texture name)
		 *
		 * @note Used internally; rarely needed by end users
		 * @note Don't use ID for direct GL calls; use the interface instead
		 */
		[[nodiscard]] virtual uint32 getID() const = 0;

		/*!
		 * @brief Gets texture width in pixels
		 * @details
		 * Returns the horizontal dimension of the texture.
		 * For 2D textures, this is the number of pixels wide.
		 *
		 * **Typical Usage:**
		 * @code
		 * auto texture = gpu::ITexture2D::create("image.png");
		 * uint32 width = texture->getWidth();
		 * uint32 height = texture->getHeight();
		 *
		 * // Use in aspect ratio calculations
		 * float aspect = (float)width / height;
		 * @endcode
		 *
		 * @return Width in pixels
		 *
		 * @note Useful for UI scaling and aspect ratio calculations
		 * @see getHeight() for vertical dimension
		 */
		[[nodiscard]] virtual uint32 getWidth() const = 0;

		/*!
		 * @brief Gets texture height in pixels
		 * @details
		 * Returns the vertical dimension of the texture.
		 * For 2D textures, this is the number of pixels tall.
		 *
		 * @return Height in pixels
		 *
		 * @see getWidth() for horizontal dimension
		 */
		[[nodiscard]] virtual uint32 getHeight() const = 0;

		/*!
		 * @brief Compares two textures for equality
		 * @details
		 * Determines if two texture objects refer to the same GPU texture resource.
		 * Useful for:
		 * - Detecting duplicate texture uploads
		 * - Material caching (same texture = same visual result)
		 * - Optimization (skip redundant texture switching)
		 *
		 * **Comparison Semantics:**
		 * Compares GPU resource IDs (GL buffer names), not pixel data.
		 * Two textures with identical pixels but different IDs are not equal.
		 *
		 * **Typical Usage:**
		 * @code
		 * if (texture1 == texture2) {
		 *     // Same GPU resource - safe to skip texture switch
		 * } else {
		 *     // Different textures - need to rebind
		 * }
		 * @endcode
		 *
		 * @param p_other Reference to texture to compare with
		 * @return true if textures are the same resource, false otherwise
		 *
		 * @note Compares object identity, not visual similarity
		 * @note Used in material system for texture caching
		 */
		virtual bool operator==(const ITexture &p_other) const = 0;
	};

	/*!
	 * @class ITexture2D
	 * @brief Concrete 2D texture interface for image-based rendering
	 * @details
	 * ITexture2D provides creation and management of 2D texture resources.
	 * This is the most common texture type, used for:
	 * - Image assets (diffuse, normal, specular maps)
	 * - Render targets (framebuffer attachments)
	 * - Generated textures (procedural, dynamic)
	 *
	 * **Creation Methods:**
	 * 1. From size (empty texture for rendering):
	 *    ```cpp
	 *    auto target = gpu::ITexture2D::create(1920, 1080);
	 *    ```
	 * 2. From file (load image from disk):
	 *    ```cpp
	 *    auto diffuse = gpu::ITexture2D::create("assets/wood.png");
	 *    ```
	 *
	 * **File Support:**
	 * Supported formats typically include:
	 * - PNG (most common; 8-bit RGBA)
	 * - JPEG (compressed; RGB only)
	 * - TGA (uncompressed; various formats)
	 * - HDR (high dynamic range)
	 * - KTX (compressed GPU formats)
	 *
	 * **Typical Workflow:**
	 * @code
	 * // Load textures
	 * auto diffuse = gpu::ITexture2D::create("textures/brick_diffuse.png");
	 * auto normal = gpu::ITexture2D::create("textures/brick_normal.png");
	 *
	 * // Use in material
	 * material->setTexture("u_DiffuseTexture", diffuse);
	 * material->setTexture("u_NormalTexture", normal);
	 *
	 * // Rendering (handled by material)
	 * material->use();
	 * renderMesh();
	 * @endcode
	 *
	 * **Render Target Usage:**
	 * 2D textures are frequently used as render targets:
	 * @code
	 * // Create render target texture
	 * auto target = gpu::ITexture2D::create(512, 512);
	 *
	 * // Attach to framebuffer
	 * auto fbo = gpu::IFramebuffer::create();
	 * fbo->attachTexture2D(target, FramebufferAttachment::Color0);
	 *
	 * // Render to texture
	 * fbo->bind();
	 * renderScene();  // Renders to texture instead of screen
	 * screen_fbo->bind();
	 * renderQuad(target);  // Display rendered texture
	 * @endcode
	 *
	 * @see ITexture for base interface
	 * @see Material for texture binding in rendering
	 * @see GLTexture2D for OpenGL implementation
	 * @see IFramebuffer for render target usage
	 *
	 * @note Path support via io::filesystem::Path (supports relative and absolute paths)
	 * @note File errors throw exceptions or return nullptr (implementation-dependent)
	 */
	class ITexture2D : public ITexture
	{
	public:
		/*!
		 * @brief Factory method: Creates a 2D texture of specified size
		 * @details
		 * Allocates GPU memory for a 2D texture without initializing pixel data.
		 * Useful for:
		 * - Creating render targets for framebuffer attachments
		 * - Pre-allocating textures for dynamic updates
		 * - Off-screen rendering surfaces
		 *
		 * **Typical Usage:**
		 * @code
		 * // Create render target (e.g., for post-processing)
		 * const uint32 width = 1920, height = 1080;
		 * auto render_target = gpu::ITexture2D::create(width, height);
		 *
		 * // Use in framebuffer
		 * auto fbo = gpu::IFramebuffer::create();
		 * fbo->attachTexture2D(render_target, FramebufferAttachment::Color0);
		 * @endcode
		 *
		 * **Initial State:**
		 * - GPU memory allocated but uninitialized
		 * - Can be filled via setData() if needed
		 * - Ready for framebuffer attachment
		 *
		 * **Memory Allocation:**
		 * - Typical format: RGBA8 (4 bytes per pixel)
		 * - Memory: width * height * 4 bytes
		 *
		 * @param p_width Texture width in pixels
		 * @param p_height Texture height in pixels
		 *
		 * @return RefPtr to newly allocated ITexture2D
		 *
		 * **Complexity:** O(1) - GPU memory allocation (no data transfer)
		 *
		 * @see create(const io::filesystem::Path&) for file-based creation
		 * @see setData() for updating pixel data after creation
		 * @see IFramebuffer for render target usage
		 */
		static RefPtr<ITexture2D> create(uint32 p_width, uint32 p_height);

		/*!
		 * @brief Factory method: Creates a 2D texture from an image file
		 * @details
		 * Loads an image file from disk and creates a GPU texture from its pixel data.
		 * This is the typical way to load game assets (diffuse maps, normal maps, etc.).
		 *
		 * **File Format Support:**
		 * Depends on implementation; typically includes:
		 * - PNG: Most common format for game assets
		 * - JPEG: Compressed; no alpha channel
		 * - TGA: Uncompressed; various formats
		 * - HDR: High dynamic range (for lighting)
		 *
		 * **Path Handling:**
		 * Paths can be:
		 * - Relative: "assets/textures/wood.png" (relative to working directory)
		 * - Absolute: "C:/Project/Assets/wood.png" (Windows) or "/home/user/wood.png" (Linux)
		 *
		 * **Typical Usage:**
		 * @code
		 * // Load texture asset
		 * const auto asset_dir = io::filesystem::getAssetDirectory();
		 * auto diffuse = gpu::ITexture2D::create(asset_dir / "textures" / "brick.png");
		 *
		 * // Load from string path
		 * auto normal = gpu::ITexture2D::create("assets/brick_normal.png");
		 *
		 * // Use in material
		 * material->setTexture("u_Diffuse", diffuse);
		 * @endcode
		 *
		 * **Error Handling:**
		 * File loading errors typically result in:
		 * - Exception thrown
		 * - nullptr returned (implementation-dependent)
		 * - Default placeholder texture (implementation-dependent)
		 *
		 * **Performance:**
		 * File loading is I/O and decode-bound. For multiple textures:
		 * - Async loading recommended (e.g., thread pool)
		 * - Stream texture loading for large scenes
		 * - Pre-compute compressed formats (DXT, BC)
		 *
		 * @param p_path Path to image file (relative or absolute)
		 *
		 * @return RefPtr to newly loaded ITexture2D
		 *
		 * **Complexity:** O(n) where n = file size (I/O + decompression)
		 *
		 * @see create(uint32, uint32) for empty texture allocation
		 * @see io::filesystem::Path for path utilities
		 *
		 * @throws or returns nullptr on file not found
		 * @throws or returns nullptr on unsupported format
		 * @note Loading is synchronous; consider async loading for UI responsiveness
		 * @note Path must be accessible from working directory or be absolute
		 */
		static RefPtr<ITexture2D> create(const io::filesystem::Path &p_path);

		/*!
		 * @brief Gets the file path this texture was loaded from
		 * @details
		 * Returns the path of the image file used to create this texture (if loaded from file).
		 * Returns std::nullopt if the texture was created from size or generated procedurally.
		 *
		 * **Typical Usage:**
		 * @code
		 * auto texture = gpu::ITexture2D::create("assets/wood.png");
		 * if (const auto path = texture->getPath()) {
		 *     std::cout << "Loaded from: " << path->string() << '\n';
		 * } else {
		 *     std::cout << "No file source (procedural or render target)" << '\n';
		 * }
		 * @endcode
		 *
		 * **Use Cases:**
		 * - Logging and debugging
		 * - Asset reloading (hot-reload on file change)
		 * - Texture caching (prevent duplicate loads)
		 *
		 * @return std::optional<io::filesystem::Path>
		 *         - Holds path if texture was loaded from file
		 *         - Empty if texture was created from size or procedurally
		 *
		 * **std::optional Semantics:**
		 * @code
		 * if (auto path = texture->getPath()) {
		 *     // path holds valid value
		 *     std::cout << *path;
		 * } else {
		 *     // path is empty
		 * }
		 * @endcode
		 *
		 * @see create(const io::filesystem::Path&) for file-based creation
		 * @note Path is const throughout texture lifetime
		 * @note Useful for asset management and debugging
		 */
		virtual std::optional<io::filesystem::Path> getPath() = 0;
	};
}
