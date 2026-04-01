/*!
 * @file framebuffer.hpp
 * @brief Abstract framebuffer interface for off-screen rendering and render targets
 * @details
 * Defines the abstract interface for GPU framebuffer objects (FBOs), which enable
 * off-screen rendering to textures. Framebuffers are essential for:
 * - Post-processing effects (bloom, blur, tone mapping)
 * - Deferred rendering (G-buffer generation)
 * - Shadow mapping (render to shadow texture)
 * - Screen-space effects (SSAO, SSR)
 * - UI rendering to texture
 * - Scene preview/capture
 *
 * **Framebuffer Concept:**
 * A framebuffer is a GPU object that holds references to attachments:
 * - **Color Attachments**: Textures to render color output (0-8 typically)
 * - **Depth/Stencil Attachment**: Texture for depth and stencil testing
 * - **Dimensions**: Width and height in pixels
 * - **Samples**: MSAA sample count for anti-aliasing
 *
 * Rendering pipeline can be redirected to framebuffer:
 * ```
 * // Normal rendering
 * glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Default framebuffer (screen)
 * render();  // Output to screen
 *
 * // Off-screen rendering
 * fbo->bind();
 * render();  // Output to FBO textures
 * screen_fbo->bind();
 * displayTexture(fbo_color_texture);  // Show result
 * ```
 *
 * @see IFramebuffer for object interface
 * @see FramebufferCreateInfo for configuration
 * @see ITexture2D for render target textures
 * @see GLFramebuffer for OpenGL implementation
 */

#pragma once

#include "toast_lib/ptr.hpp"
#include "toast_lib/system_types.h"

#include "toast_lib/io/filesystem.hpp"

#include "image.hpp"

namespace toaster::gpu
{
	/*!
	 * @struct FramebufferTextureCreateInfo
	 * @brief Configuration for a single framebuffer attachment texture
	 * @details
	 * Specifies how a texture attachment should be created and formatted.
	 * Each attachment can have different formats (color, depth, etc.).
	 *
	 * **Texture Formats:**
	 * - Color formats: RGBA8, RGBA16F, RGBA32F (typical: RGBA8)
	 * - Depth formats: Depth24 (typical)
	 * - Depth-Stencil: Depth24Stencil8 (for depth+stencil)
	 * - HDR Color: RGBA16F or RGBA32F
	 *
	 * **Typical Usage:**
	 * @code
	 * // Single color attachment (RGBA8)
	 * FramebufferTextureCreateInfo color_info(EImageFormat::RGBA8);
	 *
	 * // Depth-stencil attachment
	 * FramebufferTextureCreateInfo depth_info(EImageFormat::Depth24Stencil8);
	 * @endcode
	 *
	 * **Default Initialization:**
	 * Default constructor initializes format to platform-dependent default (usually RGBA8).
	 *
	 * @see FramebufferAttachmentsCreateInfo for collecting attachments
	 * @see EImageFormat for format enumeration
	 */
	struct FramebufferTextureCreateInfo
	{
		/*!
		 * @brief Default constructor
		 * @details Initializes with default format (platform-dependent)
		 */
		FramebufferTextureCreateInfo() = default;

		/*!
		 * @brief Constructs with specified format
		 * @param p_format Image format for this attachment
		 */
		FramebufferTextureCreateInfo(EImageFormat p_format) : format(p_format)
		{
		}

		EImageFormat format;  //!< Pixel format for this attachment texture
	};

	/*!
	 * @struct FramebufferAttachmentsCreateInfo
	 * @brief Collection of framebuffer attachment configurations
	 * @details
	 * Groups multiple attachment configurations for use in framebuffer creation.
	 * Framebuffers can have multiple color attachments plus optional depth-stencil.
	 *
	 * **Typical Configurations:**
	 * - **Single Color**: One RGBA8 attachment (simplest case)
	 * - **Color + Depth**: RGBA8 color + Depth24Stencil8 (normal rendering)
	 * - **Multiple Colors**: 4x RGBA8 + Depth24Stencil8 (deferred rendering)
	 * - **HDR**: RGBA16F color + Depth24Stencil8 (for post-processing)
	 *
	 * **Usage Pattern:**
	 * @code
	 * // Define attachments
	 * FramebufferAttachmentsCreateInfo attachments({
	 *     FramebufferTextureCreateInfo(EImageFormat::RGBA8),
	 *     FramebufferTextureCreateInfo(EImageFormat::Depth24Stencil8)
	 * });
	 *
	 * // Use in create info
	 * FramebufferCreateInfo info;
	 * info.attachments = attachments;
	 * info.width = 1920;
	 * info.height = 1080;
	 * @endcode
	 *
	 * @see FramebufferCreateInfo for complete framebuffer setup
	 * @see FramebufferTextureCreateInfo for individual attachment format
	 */
	struct FramebufferAttachmentsCreateInfo
	{
		/*!
		 * @brief Default constructor
		 * @details Creates empty attachment list
		 */
		FramebufferAttachmentsCreateInfo() = default;

		/*!
		 * @brief Constructs from initializer list
		 * @param p_attachments List of attachment format specifications
		 *
		 * @code
		 * auto info = FramebufferAttachmentsCreateInfo({
		 *     FramebufferTextureCreateInfo(EImageFormat::RGBA8),
		 *     FramebufferTextureCreateInfo(EImageFormat::Depth24Stencil8)
		 * });
		 * @endcode
		 */
		FramebufferAttachmentsCreateInfo(const std::initializer_list<FramebufferTextureCreateInfo> &p_attachments) : attachments(p_attachments)
		{
		}

		std::vector<FramebufferTextureCreateInfo> attachments;  //!< List of attachment format specs
	};

	/*!
	 * @struct FramebufferCreateInfo
	 * @brief Complete framebuffer creation specification
	 * @details
	 * Specifies all parameters needed to create and configure a framebuffer object.
	 * Groups attachment configurations, dimensions, and sampling parameters.
	 *
	 * **Configuration Members:**
	 * - attachments: Formats for color and depth textures
	 * - width, height: Render target dimensions in pixels
	 * - samples: MSAA sample count (1 = no MSAA, 4 or 8 typical)
	 *
	 * **Typical Setups:**
	 *
	 * **Setup 1: Simple Off-Screen Rendering**
	 * @code
	 * FramebufferCreateInfo info;
	 * info.attachments = FramebufferAttachmentsCreateInfo({
	 *     FramebufferTextureCreateInfo(EImageFormat::RGBA8),
	 *     FramebufferTextureCreateInfo(EImageFormat::Depth24Stencil8)
	 * });
	 * info.width = 512;
	 * info.height = 512;
	 * info.samples = 1;  // No MSAA
	 * @endcode
	 *
	 * **Setup 2: HDR Post-Processing**
	 * @code
	 * FramebufferCreateInfo info;
	 * info.attachments = FramebufferAttachmentsCreateInfo({
	 *     FramebufferTextureCreateInfo(EImageFormat::RGBA16F),  // HDR
	 *     FramebufferTextureCreateInfo(EImageFormat::Depth24Stencil8)
	 * });
	 * info.width = 1920;
	 * info.height = 1080;
	 * info.samples = 1;
	 * @endcode
	 *
	 * **Setup 3: Anti-Aliased Rendering**
	 * @code
	 * FramebufferCreateInfo info;
	 * info.attachments = FramebufferAttachmentsCreateInfo({
	 *     FramebufferTextureCreateInfo(EImageFormat::RGBA8),
	 *     FramebufferTextureCreateInfo(EImageFormat::Depth24Stencil8)
	 * });
	 * info.width = 1920;
	 * info.height = 1080;
	 * info.samples = 4;  // 4x MSAA
	 * @endcode
	 *
	 * **Setup 4: Deferred Rendering (G-Buffer)**
	 * @code
	 * FramebufferCreateInfo info;
	 * info.attachments = FramebufferAttachmentsCreateInfo({
	 *     FramebufferTextureCreateInfo(EImageFormat::RGBA8),     // Albedo
	 *     FramebufferTextureCreateInfo(EImageFormat::RGBA8),     // Normal
	 *     FramebufferTextureCreateInfo(EImageFormat::RGBA8),     // Position
	 *     FramebufferTextureCreateInfo(EImageFormat::RGBA8),     // Roughness/Metallic
	 *     FramebufferTextureCreateInfo(EImageFormat::Depth24Stencil8)
	 * });
	 * info.width = 1920;
	 * info.height = 1080;
	 * info.samples = 1;
	 * @endcode
	 *
	 * **Dimension Constraints:**
	 * - Minimum: 1×1 (rarely useful)
	 * - Typical: Match screen resolution (1920×1080, etc.)
	 * - Maximum: GPU-dependent (usually 16384×16384 or higher)
	 *
	 * **Sample Count (MSAA):**
	 * - 1: No multi-sampling (fastest, default)
	 * - 2: 2x MSAA (rarely used)
	 * - 4: 4x MSAA (good quality/performance balance)
	 * - 8: 8x MSAA (good quality, higher cost)
	 * - 16: 16x MSAA (excellent quality, expensive)
	 *
	 * @see IFramebuffer::create() for framebuffer creation
	 * @see FramebufferAttachmentsCreateInfo for attachment details
	 *
	 * @note Width and height must be > 0
	 * @note Samples must be >= 1 (0 is invalid)
	 * @note Not all sample counts are supported on all GPUs
	 */
	struct FramebufferCreateInfo
	{
		FramebufferAttachmentsCreateInfo attachments;  //!< Attachment format specifications

		uint32 width{0u};   //!< Render target width in pixels
		uint32 height{0u};  //!< Render target height in pixels

		uint32 samples{1u}; //!< MSAA sample count (1 = no MSAA, must be >= 1)
	};

	/*!
	 * @class IFramebuffer
	 * @brief Abstract interface for GPU framebuffer object (FBO) management
	 * @details
	 * IFramebuffer provides an abstraction layer for GPU framebuffer operations.
	 * A framebuffer is a rendering target that can contain multiple color attachments
	 * and a depth-stencil attachment, enabling off-screen rendering.
	 *
	 * **Framebuffer Lifecycle:**
	 *
	 * 1. **Creation**: Create with desired attachments, dimensions, samples
	 *    ```cpp
	 *    FramebufferCreateInfo info{...};
	 *    auto fbo = gpu::IFramebuffer::create(info);
	 *    ```
	 *
	 * 2. **Binding**: Make framebuffer active for rendering
	 *    ```cpp
	 *    fbo->bind();
	 *    // Rendering now goes to FBO, not screen
	 *    ```
	 *
	 * 3. **Rendering**: Issue draw calls (they render to FBO)
	 *    ```cpp
	 *    glDrawElements(...);  // Renders to fbo attachments
	 *    ```
	 *
	 * 4. **Unbinding**: Switch back to screen or other FBO
	 *    ```cpp
	 *    fbo->unbind();
	 *    screen_fbo->bind();  // Or default framebuffer
	 *    ```
	 *
	 * 5. **Texture Access**: Get color/depth textures for further use
	 *    ```cpp
	 *    uint32 color_id = fbo->getColourAttachmentID(0);
	 *    // Use texture in subsequent rendering
	 *    ```
	 *
	 * **Common Use Cases:**
	 *
	 * **Use Case 1: Post-Processing**
	 * @code
	 * // Create FBO for scene rendering
	 * auto scene_fbo = createSceneFramebuffer();
	 *
	 * // Render scene to FBO
	 * scene_fbo->bind();
	 * renderScene();
	 * scene_fbo->unbind();
	 *
	 * // Post-process FBO output
	 * post_process_shader->bind();
	 * post_process_shader->setUniform("u_SceneTexture", scene_fbo->getColourAttachmentID(0));
	 * renderFullscreenQuad();
	 * @endcode
	 *
	 * **Use Case 2: Shadow Mapping**
	 * @code
	 * // Create depth FBO for shadow map
	 * auto shadow_fbo = createShadowFramebuffer();
	 *
	 * // Render from light perspective
	 * shadow_fbo->bind();
	 * renderScene(light_view_proj);
	 * shadow_fbo->unbind();
	 *
	 * // Use shadow map in scene rendering
	 * shader->setUniform("u_ShadowMap", shadow_fbo->getDepthStencilAttachmentID());
	 * @endcode
	 *
	 * **Use Case 3: Deferred Rendering**
	 * @code
	 * // Create G-buffer FBO with multiple attachments
	 * auto gbuffer_fbo = createGBufferFramebuffer();
	 *
	 * // Render geometry to G-buffer
	 * gbuffer_fbo->bind();
	 * renderGeometry();  // Outputs: albedo, normal, position, etc.
	 * gbuffer_fbo->unbind();
	 *
	 * // Deferred lighting pass
	 * lighting_fbo->bind();
	 * lighting_shader->setUniform("u_Albedo", gbuffer_fbo->getColourAttachmentID(0));
	 * lighting_shader->setUniform("u_Normal", gbuffer_fbo->getColourAttachmentID(1));
	 * // ... more attachments ...
	 * renderLightingQuad();
	 * @endcode
	 *
	 * **Performance Characteristics:**
	 * - Creation: O(n) where n = attachment count (GPU memory allocation)
	 * - Binding: O(1) GPU state change
	 * - Rendering: Same cost as screen rendering
	 * - Unbinding: O(1)
	 * - Texture access: O(1)
	 * - Resize: O(n) (reallocate all attachments)
	 *
	 * **Important Considerations:**
	 * - Don't bind framebuffer during rendering to that FBO
	 * - Framebuffer dimensions affect texture memory usage
	 * - MSAA increases memory and rendering cost
	 * - Depth-stencil texture not readable; must resolve/copy for sampling
	 * - Typical pattern: render to FBO, then display result as texture
	 *
	 * @see IFramebuffer::create() factory method
	 * @see FramebufferCreateInfo for configuration
	 * @see ITexture2D for attachment textures
	 * @see GLFramebuffer for OpenGL implementation
	 *
	 * @note Don't bind same FBO for reading and writing in pipeline
	 * @note Dimensions > screen resolution use extra memory
	 * @note Clear framebuffer before rendering (use gl clear or shader)
	 * @note Some implementations may have texture format restrictions
	 */
	class IFramebuffer
	{
	public:
		/*!
		 * @brief Factory method: Creates a framebuffer object
		 * @details
		 * Allocates a GPU framebuffer with the specified attachment textures,
		 * dimensions, and sampling configuration.
		 *
		 * **Creation Process:**
		 * 1. Create framebuffer object (GPU FBO)
		 * 2. Create and attach color textures
		 * 3. Create and attach depth-stencil texture
		 * 4. Validate framebuffer completeness
		 * 5. Store configuration for later queries
		 *
		 * **Typical Usage:**
		 * @code
		 * // Define configuration
		 * FramebufferCreateInfo info;
		 * info.attachments = FramebufferAttachmentsCreateInfo({
		 *     FramebufferTextureCreateInfo(EImageFormat::RGBA8),
		 *     FramebufferTextureCreateInfo(EImageFormat::Depth24Stencil8)
		 * });
		 * info.width = 1920;
		 * info.height = 1080;
		 * info.samples = 1;
		 *
		 * // Create framebuffer
		 * auto fbo = gpu::IFramebuffer::create(info);
		 * @endcode
		 *
		 * **Memory Allocation:**
		 * Memory allocated depends on configuration:
		 * - Per color attachment: width × height × bytes_per_pixel (4 for RGBA8, etc.)
		 * - Depth attachment: width × height × 4 (Depth24 + padding)
		 * - With MSAA: multiply by sample count
		 *
		 * **Example Memory Usage:**
		 * - 1920×1080 RGBA8 color: 8.3 MB
		 * - 1920×1080 Depth24: 8.3 MB
		 * - With 4x MSAA: 33.2 MB per color + 33.2 MB depth
		 *
		 * @param p_framebuffer_create_info Complete framebuffer specification
		 *
		 * @return RefPtr to newly created IFramebuffer
		 *
		 * **Complexity:** O(n) where n = attachment count
		 *                 (allocation + texture creation)
		 *
		 * @see FramebufferCreateInfo for parameter details
		 * @see IFramebuffer for usage
		 *
		 * @throws or returns nullptr on invalid configuration
		 * @throws or returns nullptr if GPU memory allocation fails
		 * @note Framebuffer completeness validated automatically
		 */
		static RefPtr<IFramebuffer> create(const FramebufferCreateInfo &p_framebuffer_create_info);

		virtual                     ~IFramebuffer() = default;

		/*!
		 * @brief Binds framebuffer as the rendering target
		 * @details
		 * Makes this framebuffer the active target for rendering.
		 * In OpenGL terms, this calls glBindFramebuffer(GL_FRAMEBUFFER, fbo_id).
		 *
		 * After binding:
		 * - All rendering output goes to this FBO's attachments
		 * - Color writes go to color attachments
		 * - Depth testing uses depth attachment
		 * - Previous framebuffer is inactive
		 *
		 * **Typical Rendering Pattern:**
		 * @code
		 * // Render to framebuffer
		 * fbo->bind();
		 * glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		 * renderScene();
		 * fbo->unbind();
		 *
		 * // Display result
		 * quad->bind();
		 * postprocess_shader->bind();
		 * glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		 * @endcode
		 *
		 * @note Const method: doesn't modify FBO state, only binding
		 * @note Binding is persistent until another FBO/framebuffer bound
		 * @see unbind() to deactivate the framebuffer
		 * @see getColourAttachmentID() for texture access after rendering
		 *
		 * **Complexity:** O(1) - GPU state change
		 */
		virtual void bind() const = 0;

		/*!
		 * @brief Unbinds framebuffer, switching to default framebuffer
		 * @details
		 * Deactivates this framebuffer by binding the default framebuffer (id 0).
		 * In OpenGL terms, this calls glBindFramebuffer(GL_FRAMEBUFFER, 0).
		 *
		 * After unbinding:
		 * - Rendering output goes to screen/window
		 * - No active user framebuffer
		 * - Default framebuffer becomes active
		 *
		 * **Usage:**
		 * @code
		 * fbo->bind();
		 * render_to_fbo();
		 * fbo->unbind();  // Back to screen
		 *
		 * // Display FBO result
		 * display_quad->bind();
		 * glDrawArrays(...);
		 * @endcode
		 *
		 * @note Can be omitted if binding another FBO immediately
		 * @see bind() to activate the framebuffer
		 *
		 * **Complexity:** O(1) - GPU state change
		 */
		virtual void unbind() const = 0;

		/*!
		 * @brief Resizes framebuffer attachments
		 * @details
		 * Changes the dimensions of all attachments. Reallocates GPU memory
		 * for all color and depth textures.
		 *
		 * **Typical Usage:**
		 * Resize when window/viewport size changes:
		 * @code
		 * // Window resize callback
		 * void on_window_resized(uint32 new_width, uint32 new_height) {
		 *     scene_fbo->resize(new_width, new_height);
		 * }
		 * @endcode
		 *
		 * **Performance:**
		 * Resizing deallocates and reallocates all textures.
		 * - Old textures become invalid
		 * - Any existing texture references are stale
		 * - Should not resize during rendering
		 *
		 * **Side Effects:**
		 * - All attachment textures are reallocated
		 * - GPU memory is freed and reallocated
		 * - Previous texture IDs may become invalid
		 * - Content of previous attachments is lost
		 *
		 * @param p_width New width in pixels (must be > 0)
		 * @param p_height New height in pixels (must be > 0)
		 *
		 * **Complexity:** O(n) where n = attachment count
		 *                 (deallocate + reallocate all textures)
		 *
		 * @note Must resize when window dimensions change
		 * @note Can be expensive; batch resizes if possible
		 * @note Framebuffer must be unbound during resize
		 * @see getCreateInfo() to query current dimensions
		 */
		virtual void resize(uint32 p_width, uint32 p_height) = 0;

		/*!
		 * @brief Gets the GPU framebuffer identifier
		 * @details
		 * Returns the backend-specific GPU resource handle.
		 * In OpenGL, this is the framebuffer object name (uint32 ID).
		 *
		 * @return GPU resource ID (OpenGL FBO name)
		 *
		 * @note Used internally; rarely needed by end users
		 * @note Don't use ID for direct GL calls; use the interface instead
		 */
		[[nodiscard]] virtual uint32 getID() const = 0;

		/*!
		 * @brief Gets the color attachment texture ID
		 * @details
		 * Returns the GPU texture ID for a color attachment.
		 * Multiple color attachments indexed from 0.
		 *
		 * **Typical Usage:**
		 * @code
		 * uint32 color_texture = fbo->getColourAttachmentID(0);
		 *
		 * // Use in shader or further rendering
		 * post_shader->setUniform("u_ColorTexture", color_texture);
		 * @endcode
		 *
		 * **Index Range:**
		 * - 0 to (attachment_count - 1)
		 * - First color attachment: index 0
		 * - Returns depth ID if accessing past color count (implementation-dependent)
		 *
		 * **Typical Indices:**
		 * - 0: Primary color output (usual case)
		 * - 1-3: Additional color attachments (deferred rendering)
		 *
		 * @param p_attachment_index Color attachment index (default: 0)
		 *
		 * @return GPU texture ID (OpenGL texture name)
		 *
		 * @note Index must be < number of color attachments
		 * @note Default index (0) is typical for single-target rendering
		 * @see getDepthStencilAttachmentID() for depth texture
		 *
		 * **Complexity:** O(1)
		 */
		[[nodiscard]] virtual uint32 getColourAttachmentID(uint32 p_attachment_index = 0) const = 0;

		/*!
		 * @brief Gets the depth-stencil attachment texture ID
		 * @details
		 * Returns the GPU texture ID for the depth-stencil attachment.
		 * Note: Depth-stencil textures typically cannot be sampled directly;
		 * must be resolved/copied for reading.
		 *
		 * **Typical Usage:**
		 * @code
		 * uint32 depth_texture = fbo->getDepthStencilAttachmentID();
		 * // Limited use: mainly for debugging or special techniques
		 * @endcode
		 *
		 * **Limitations:**
		 * - Depth textures require special sampling in shaders
		 * - MSAA depth cannot be sampled; must resolve first
		 * - Usually used internally, not for shader sampling
		 *
		 * **Alternative - Shadow Mapping:**
		 * For shadow maps, typically use dedicated depth framebuffer:
		 * @code
		 * auto shadow_fbo = createShadowFramebuffer();  // Has depth attachment
		 * shadow_fbo->bind();
		 * renderFromLight();
		 * shadow_fbo->unbind();
		 *
		 * // Use depth texture in lighting pass
		 * lighting_shader->setUniform("u_ShadowMap", shadow_fbo->getDepthStencilAttachmentID());
		 * @endcode
		 *
		 * @return GPU texture ID (OpenGL texture name) for depth attachment
		 *         0 if no depth attachment configured
		 *
		 * @note Returns 0 if no depth attachment
		 * @note May require special sampler types (sampler2DShadow, etc.)
		 * @see getColourAttachmentID() for color texture access
		 *
		 * **Complexity:** O(1)
		 */
		[[nodiscard]] virtual uint32 getDepthStencilAttachmentID() const = 0;

		/*!
		 * @brief Gets the framebuffer creation configuration
		 * @details
		 * Returns a const reference to the creation specification used
		 * to create this framebuffer. Useful for introspection.
		 *
		 * **Typical Usage:**
		 * @code
		 * const auto& info = fbo->getCreateInfo();
		 * uint32 width = info.width;
		 * uint32 height = info.height;
		 * uint32 samples = info.samples;
		 * @endcode
		 *
		 * **Use Cases:**
		 * - Querying current dimensions (for resize logic)
		 * - Checking sample count (for MSAA handling)
		 * - Verifying configuration (debugging)
		 *
		 * @return Const reference to FramebufferCreateInfo used for creation
		 *
		 * **Complexity:** O(1)
		 *
		 * @see FramebufferCreateInfo for structure details
		 * @see resize() to change dimensions
		 * @note Information is constant throughout FBO lifetime
		 */
		[[nodiscard]] virtual const FramebufferCreateInfo &getCreateInfo() const = 0;

		/*!
		 * @brief Reads a pixel value from color attachment
		 * @details
		 * Reads the color value of a single pixel from a framebuffer attachment.
		 * This is a CPU-GPU synchronization point; use sparingly.
		 *
		 * **Typical Usage:**
		 * @code
		 * // Read pixel from FBO
		 * int32 pixel_value = fbo->readPixel(0, mouse_x, mouse_y);
		 *
		 * // Decode RGBA from packed value
		 * uint8 r = (pixel_value >> 0) & 0xFF;
		 * uint8 g = (pixel_value >> 8) & 0xFF;
		 * uint8 b = (pixel_value >> 16) & 0xFF;
		 * uint8 a = (pixel_value >> 24) & 0xFF;
		 * @endcode
		 *
		 * **Use Cases:**
		 * - Object picking (mouse click detection)
		 * - Debugging (inspect render output)
		 * - Tone mapping probe (read HDR value)
		 * - Screenshot/capture
		 *
		 * **Performance Warning:**
		 * Pixel readback causes GPU-CPU synchronization stall.
		 * For picking, better approaches:
		 * - Render object IDs to texture, read in next frame
		 * - Use geometry picking (ray-triangle tests)
		 * - Use transform feedback or compute shaders
		 *
		 * **Pixel Format:**
		 * Returned value encoding depends on attachment format:
		 * - RGBA8: Packed as RGBA bytes (R=0-7, G=8-15, B=16-23, A=24-31)
		 * - Other formats: Implementation-specific conversion
		 *
		 * @param p_attachment_index Color attachment to read from (default: 0)
		 * @param p_x Pixel X coordinate (0 = left)
		 * @param p_y Pixel Y coordinate (0 = top)
		 *
		 * @return Pixel value (format depends on attachment type)
		 *         Typically RGBA packed into int32
		 *
		 * **Complexity:** O(1) per pixel (but stalls GPU pipeline!)
		 *
		 * @note VERY SLOW - causes GPU-CPU sync stall
		 * @note Use sparingly (max once per frame for UI feedback)
		 * @note Coordinates must be in bounds [0, width) × [0, height)
		 * @note Consider CPU-side picking alternatives for better performance
		 * @see https://www.khronos.org/opengl/wiki/Asynchronous_Buffer_Transfer for async reads
		 */
		virtual int32 readPixel(uint32 p_attachment_index, int32 p_x, int32 p_y) = 0;
	};
}
