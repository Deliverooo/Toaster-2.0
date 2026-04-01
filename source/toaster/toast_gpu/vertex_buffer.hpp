/*!
 * @file vertex_buffer.hpp
 * @brief Abstract vertex buffer interface for GPU memory management
 * @details
 * Defines the abstract interface for GPU vertex buffers, which store vertex attribute data
 * on the GPU device. This is a key abstraction that separates the buffer API from its OpenGL
 * implementation, enabling future support for other graphics backends (Direct3D, Vulkan, etc.).
 *
 * Vertex buffers work in conjunction with:
 * - VertexBufferLayout: Describes the structure of vertex data
 * - IVertexArray: Manages multiple buffers and attribute binding
 * - IShader: Consumes vertex data according to layout specification
 *
 * The typical usage flow is:
 * 1. Create vertex buffer with desired size (or size + initial data)
 * 2. Define layout using VertexBufferLayout
 * 3. Set layout on buffer using setLayout()
 * 4. Add buffer to vertex array using IVertexArray::addVertexBuffer()
 * 5. Bind vertex array and draw
 *
 * @see VertexBufferLayout for vertex data structure definition
 * @see IVertexArray for buffer binding and rendering
 * @see IShader for shader attribute binding
 */

#pragma once

#include "toast_lib/ptr.hpp"
#include "toast_lib/system_types.h"

#include "vertex_buffer_layout.hpp"

namespace toaster::gpu
{
	/*!
	 * @class IVertexBuffer
	 * @brief Abstract interface for GPU vertex buffer management
	 * @details
	 * IVertexBuffer provides an abstraction layer for GPU vertex buffer operations.
	 * A vertex buffer stores vertex attributes (positions, colors, normals, etc.) on the GPU,
	 * enabling efficient rendering of geometry.
	 *
	 * **Vertex Buffer Lifecycle:**
	 * 1. **Creation**: Use static create() factories to allocate GPU memory
	 *    - create(size): Allocate empty buffer
	 *    - create(data, size): Allocate and initialize with data
	 *
	 * 2. **Configuration**: Define vertex layout with setLayout()
	 *    ```cpp
	 *    auto vb = IVertexBuffer::create(vertices.size() * sizeof(Vertex));
	 *    vb->setLayout(VertexBufferLayout({
	 *        VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),
	 *        VertexBufferElement(EShaderDataType::eFloat3, "a_Normal")
	 *    }));
	 *    ```
	 *
	 * 3. **Data Management**: Update GPU data with setData()
	 *    ```cpp
	 *    vb->setData(newVertexData, newDataSize);
	 *    ```
	 *
	 * 4. **Rendering**: Bind buffer before drawing
	 *    ```cpp
	 *    vb->bind();
	 *    // ... draw commands ...
	 *    vb->unbind();
	 *    ```
	 *
	 * **Implementation Details:**
	 * - Backend-specific implementation (e.g., GLVertexBuffer) handles GPU memory
	 * - Stride and offset information comes from VertexBufferLayout
	 * - Bind/unbind calls manage OpenGL state or equivalent
	 *
	 * **Performance Considerations:**
	 * - Prefer creating buffers once and reusing them
	 * - Use setData() for dynamic data; avoid recreating buffers
	 * - Batch vertex data when possible to reduce state changes
	 * - Combine multiple buffers in a vertex array when appropriate
	 *
	 * **Memory Layout Example:**
	 * For a vertex structure like:
	 * ```cpp
	 * struct Vertex {
	 *     glm::vec3 position;   // offset 0, size 12
	 *     glm::vec3 normal;     // offset 12, size 12
	 *     glm::vec2 texCoord;   // offset 24, size 8
	 * };  // stride: 32 bytes
	 * ```
	 *
	 * The GPU stores vertices in interleaved format:
	 * ```
	 * Vertex 0:    Vertex 1:
	 * [pos][norm]  [pos][norm]
	 * [tx] [Pad]   [tx] [Pad]
	 * ```
	 *
	 * @see VertexBufferLayout for layout specification
	 * @see IVertexArray for buffer binding in rendering
	 * @see GLVertexBuffer for OpenGL implementation
	 *
	 * @note Use smart pointers (RefPtr) to manage buffer lifetime
	 * @note Always set layout before using buffer in rendering
	 * @note Don't bind multiple buffers to same vertex array slot
	 */
	class IVertexBuffer
	{
	public:
		/*!
		 * @brief Factory method: Creates an empty vertex buffer
		 * @details
		 * Allocates GPU memory for a vertex buffer of the specified size.
		 * The buffer is initially empty; data can be filled using setData().
		 *
		 * **Memory Allocation:**
		 * - GPU memory is allocated but uninitialized
		 * - Use setData() to populate with actual vertex data
		 * - Useful for creating reusable buffers or preallocating space
		 *
		 * **Typical Usage:**
		 * @code
		 * // Allocate buffer for 1000 vertices of custom struct (e.g., 32 bytes each)
		 * const uint32 vertex_count = 1000;
		 * const uint32 vertex_size = sizeof(MyVertexStruct);
		 * auto vb = gpu::IVertexBuffer::create(vertex_count * vertex_size);
		 *
		 * // Later, fill with data
		 * vb->setData(vertices.data(), vertices.size() * vertex_size);
		 * @endcode
		 *
		 * **Parameters:**
		 * @param p_size Total size in bytes to allocate
		 *               Note: This should be vertex_count * sizeof(VertexStruct)
		 *
		 * **Returns:**
		 * RefPtr to newly allocated IVertexBuffer
		 *
		 * **Complexity:** O(1) - GPU memory allocation
		 *
		 * @see create(void*, uint32) for immediate data initialization
		 * @see setData() for filling buffer after creation
		 * @see setLayout() for vertex structure definition
		 *
		 * @note Backend (GLVertexBuffer) determines actual GL buffer target (GL_ARRAY_BUFFER)
		 */
		static RefPtr<IVertexBuffer> create(uint32 p_size);

		/*!
		 * @brief Factory method: Creates and initializes vertex buffer with data
		 * @details
		 * Allocates GPU memory for a vertex buffer and immediately fills it with provided data.
		 * This is more efficient than separate create() and setData() calls.
		 *
		 * **Typical Usage Pattern:**
		 * @code
		 * // Common pattern: vertex struct + vector of vertices
		 * struct MeshVertex {
		 *     glm::vec3 position;
		 *     glm::vec3 normal;
		 *     glm::vec2 texCoord;
		 * };  // 32 bytes per vertex
		 *
		 * std::vector<MeshVertex> vertices = loadMeshVertices("mesh.obj");
		 *
		 * // Create and initialize buffer in one call
		 * auto vertex_buffer = gpu::IVertexBuffer::create(
		 *     (void*)vertices.data(),
		 *     vertices.size() * sizeof(MeshVertex)
		 * );
		 *
		 * // Set layout
		 * vertex_buffer->setLayout(VertexBufferLayout({
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Normal"),
		 *     VertexBufferElement(EShaderDataType::eFloat2, "a_TexCoord")
		 * }));
		 * @endcode
		 *
		 * **Parameters:**
		 * @param p_data Pointer to vertex data in CPU memory
		 * @param p_size Total size in bytes of the data
		 *
		 * **Returns:**
		 * RefPtr to newly allocated and initialized IVertexBuffer
		 *
		 * **Complexity:** O(n) where n = p_size (GPU memory transfer)
		 *
		 * @see create(uint32) for empty buffer allocation
		 * @see setData() for updating buffer after creation
		 *
		 * @note Data is copied to GPU; source memory can be freed after call returns
		 * @note All bytes from p_data are copied; ensure p_size is accurate
		 * @note For CPU-GPU synchronization, GPU memory write operation typically blocks CPU
		 */
		static RefPtr<IVertexBuffer> create(void *p_data, uint32 p_size);

		virtual ~IVertexBuffer() = default;

		/*!
		 * @brief Binds vertex buffer to current OpenGL context
		 * @details
		 * Makes this vertex buffer the active buffer for subsequent vertex attribute operations.
		 * In OpenGL terms, this calls glBindBuffer(GL_ARRAY_BUFFER, buffer_id).
		 *
		 * After binding:
		 * - This buffer is the target for attribute pointer setup
		 * - Vertex data fetches come from this buffer
		 * - Subsequent calls to setData() affect this buffer
		 *
		 * **Typical Usage:**
		 * @code
		 * vertex_buffer->bind();
		 * // ... set layout, attach to VAO, etc ...
		 * vertex_buffer->unbind();
		 * @endcode
		 *
		 * **Semantics:**
		 * Binding is a stateful operation that affects the rendering context.
		 * Multiple buffers can exist, but only one is active at a time.
		 *
		 * @note Usually called as part of IVertexArray setup, not directly
		 * @note Overhead: typically 1-2 GPU cycles (minimal)
		 * @see unbind() to deactivate the buffer
		 * @see IVertexArray for typical buffer binding usage
		 */
		virtual void bind() = 0;

		/*!
		 * @brief Unbinds vertex buffer from current OpenGL context
		 * @details
		 * Deactivates this vertex buffer by binding the null buffer (id 0).
		 * In OpenGL terms, this calls glBindBuffer(GL_ARRAY_BUFFER, 0).
		 *
		 * After unbinding:
		 * - No buffer is active for vertex attributes
		 * - Subsequent operations need to bind a buffer first
		 * - Clean state for other buffer operations
		 *
		 * **Good Practice:**
		 * @code
		 * vertex_buffer->bind();
		 * // ... perform buffer operations ...
		 * vertex_buffer->unbind();  // Clean up after use
		 * @endcode
		 *
		 * @note Usually called as part of VAO cleanup, not typically needed
		 * @note Can be omitted if binding another buffer immediately
		 * @see bind() to activate the buffer
		 */
		virtual void unbind() = 0;

		/*!
		 * @brief Uploads data to GPU vertex buffer
		 * @details
		 * Copies vertex data from CPU memory to GPU memory.
		 * This can be called multiple times to update buffer contents.
		 *
		 * **Data Transfer:**
		 * Copies p_size bytes from p_data to GPU memory starting at offset 0.
		 * This is a CPU→GPU synchronization point that typically blocks until transfer completes.
		 *
		 * **Usage Patterns:**
		 * @code
		 * // Pattern 1: Immediate data after buffer creation
		 * auto vb = IVertexBuffer::create(size);
		 * vb->setData(vertices.data(), vertices.size() * sizeof(Vertex));
		 *
		 * // Pattern 2: Update dynamic data (e.g., animated mesh)
		 * vb->setData(updatedVertices.data(), updatedVertices.size() * sizeof(Vertex));
		 *
		 * // Pattern 3: Partial updates (reusing existing buffer)
		 * std::vector<Vertex> mesh_frame;
		 * for (int frame = 0; frame < num_frames; ++frame) {
		 *     updateMeshFrame(mesh_frame, frame);
		 *     vb->setData(mesh_frame.data(), mesh_frame.size() * sizeof(Vertex));
		 * }
		 * @endcode
		 *
		 * **Parameters:**
		 * @param p_data Pointer to CPU memory containing vertex data
		 * @param p_size Number of bytes to copy
		 *
		 * **Performance:**
		 * - Memory transfer: ~20-50 GB/s typical PCIe bandwidth
		 * - Synchronization: CPU blocks until GPU receives data
		 * - For frequent updates, consider using persistent mapped buffers (advanced)
		 *
		 * @note Buffer must be bound before calling setData()
		 * @note Size should match actual vertex data size
		 * @note For large buffers, consider streaming or partial updates
		 * @see setLayout() for describing data structure
		 */
		virtual void setData(const void *p_data, uint32 p_size) = 0;

		/*!
		 * @brief Sets the vertex buffer layout
		 * @details
		 * Associates a VertexBufferLayout with this buffer, defining how vertex attributes
		 * are organized in memory. This layout information is critical for:
		 * - Shader attribute binding (which shader inputs get which data)
		 * - GPU stride and offset calculations
		 * - Memory layout validation
		 *
		 * **Layout Setup Flow:**
		 * @code
		 * // Define buffer layout
		 * const auto layout = VertexBufferLayout({
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Normal"),
		 *     VertexBufferElement(EShaderDataType::eFloat2, "a_TexCoord")
		 * });
		 *
		 * // Apply layout to buffer
		 * vertex_buffer->setLayout(layout);
		 *
		 * // Afterward, shader can bind attributes using layout information
		 * @endcode
		 *
		 * **Backend Behavior:**
		 * In OpenGL implementation (GLVertexBuffer):
		 * - Layout is stored for later use during vertex array binding
		 * - Stride and offset are extracted and used in glVertexAttribPointer calls
		 * - Layout persists until changed via another setLayout() call
		 *
		 * **Common Layouts:**
		 * - **Simple Position Only**: {eFloat3, "a_Position"}
		 * - **Position + Color**: {eFloat3, "a_Position"}, {eFloat4, "a_Color"}
		 * - **Full Mesh Vertex**: {eFloat3, "a_Position"}, {eFloat3, "a_Normal"}, {eFloat2, "a_TexCoord"}
		 * - **Instanced Transform**: {eMat4, "a_Transform"}
		 *
		 * @param p_layout VertexBufferLayout describing vertex structure
		 *
		 * @note Layout must be set before using buffer in rendering
		 * @note Layout affects vertex array attribute binding
		 * @note Multiple setLayout() calls update the layout
		 * @see VertexBufferLayout for layout specification
		 * @see getLayout() to retrieve current layout
		 */
		virtual void setLayout(const VertexBufferLayout &p_layout) = 0;

		/*!
		 * @brief Gets the GPU resource identifier
		 * @details
		 * Returns the backend-specific GPU resource handle.
		 * In OpenGL, this is the buffer object name (uint32 ID).
		 *
		 * **Usage:**
		 * Typically used internally by vertex arrays and debugging tools.
		 * End users rarely need to call this directly.
		 *
		 * @return GPU resource ID (OpenGL buffer name)
		 *
		 * @note ID validity depends on buffer lifetime
		 * @note Don't use ID for direct GL calls; use the interface instead
		 */
		[[nodiscard]] virtual uint32 getID() const = 0;

		/*!
		 * @brief Gets the vertex buffer layout (non-const version)
		 * @return Reference to non-const VertexBufferLayout
		 * @see getLayout() const for const version
		 */
		virtual const VertexBufferLayout &              getLayout() = 0;

		/*!
		 * @brief Gets the vertex buffer layout (const version)
		 * @details
		 * Retrieves the VertexBufferLayout previously set via setLayout().
		 * Provides read-only access to the vertex structure definition.
		 *
		 * **Usage:**
		 * Typically used by vertex array implementations to extract stride and offset information.
		 *
		 * @return Const reference to VertexBufferLayout
		 *
		 * **Example:**
		 * @code
		 * const auto& layout = vertex_buffer->getLayout();
		 * uint32 stride = layout.getStride();
		 * for (const auto& element : layout) {
		 *     bindVertexAttribute(element);
		 * }
		 * @endcode
		 *
		 * @see setLayout() to set the layout
		 * @see VertexBufferLayout for layout details
		 */
		[[nodiscard]] virtual const VertexBufferLayout &getLayout() const = 0;
	};
}
