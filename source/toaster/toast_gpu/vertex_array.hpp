/*!
 * @file vertex_array.hpp
 * @brief Abstract vertex array object (VAO) interface for GPU vertex management
 * @details
 * Defines the abstract interface for vertex array objects (VAOs), which manage the binding
 * and configuration of vertex buffers and index buffers for the GPU rendering pipeline.
 *
 * A vertex array object (VAO) encapsulates the state required to render geometry:
 * - Multiple vertex buffers (e.g., positions, normals, colors, UV coordinates)
 * - One optional index buffer (for indexed drawing)
 * - Vertex layout configuration (stride, offset, type information)
 *
 * **VAO Concept:**
 * In OpenGL, a VAO captures vertex attribute state, eliminating the need to reconfigure
 * attributes for every draw call. Instead:
 * 1. Configure VAO once (add buffers, set layout)
 * 2. Bind VAO before drawing
 * 3. GPU uses stored configuration automatically
 *
 * **Typical Usage Flow:**
 * @code
 * // Create and configure VAO
 * auto vao = gpu::IVertexArray::create();
 *
 * // Create and fill vertex buffer
 * auto vb = gpu::IVertexBuffer::create(vertices.data(), vertices.size() * sizeof(Vertex));
 * vb->setLayout(VertexBufferLayout({
 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),
 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Normal"),
 *     VertexBufferElement(EShaderDataType::eFloat2, "a_TexCoord")
 * }));
 * vao->addVertexBuffer(vb);
 *
 * // Create and fill index buffer
 * auto ib = gpu::IIndexBuffer::create(indices.data(), indices.size());
 * vao->setIndexBuffer(ib);
 *
 * // Use for rendering
 * vao->bind();
 * glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
 * @endcode
 *
 * **Key Responsibilities:**
 * - Store vertex buffer references
 * - Store index buffer reference
 * - Configure GPU attribute pointers based on layout
 * - Manage binding state for rendering
 *
 * @see IVertexBuffer for individual buffer management
 * @see IIndexBuffer for index buffer management
 * @see VertexBufferLayout for vertex structure definition
 * @see GLVertexArray for OpenGL implementation
 */

#pragma once

#include "index_buffer.hpp"
#include "vertex_buffer.hpp"

namespace toaster::gpu
{
	/*!
	 * @class IVertexArray
	 * @brief Abstract interface for GPU vertex array object (VAO) management
	 * @details
	 * IVertexArray provides an abstraction layer for vertex array object (VAO) operations.
	 * A VAO is a GPU object that encapsulates the configuration needed to render geometry,
	 * specifically which vertex buffers and index buffer to use, and how to interpret the data.
	 *
	 * **Vertex Array Composition:**
	 * A VAO contains:
	 * 1. **Vertex Buffers**: One or more buffers containing vertex attribute data
	 *    - Typical: one buffer with interleaved data (position, normal, UV, etc.)
	 *    - Advanced: multiple buffers with different attributes
	 * 2. **Index Buffer**: Optional buffer containing triangle indices for indexed drawing
	 * 3. **Attribute Configuration**: How to interpret vertex buffer data
	 *
	 * **Rendering Pipeline Integration:**
	 * ```
	 * VAO (configuration)
	 *  ├─ Vertex Buffer(s) (vertex attribute data)
	 *  │  └─ Layout (stride, offset, type info)
	 *  └─ Index Buffer (optional, triangle indices)
	 *
	 * Rendering Flow:
	 * vao->bind()                           // Activate configuration
	 * glDrawElements(..., index_count, ...) // GPU uses VAO to fetch vertex data
	 * ```
	 *
	 * **Memory and State Management:**
	 * - VAO maintains references to buffers (via RefPtr smart pointers)
	 * - Buffers are kept alive as long as VAO is alive
	 * - Deleting VAO doesn't delete buffers (ref counting handles cleanup)
	 * - Multiple VAOs can reference the same buffers
	 *
	 * **Typical Usage Patterns:**
	 *
	 * **Pattern 1: Static Mesh (Created Once)**
	 * @code
	 * class Mesh {
	 *     RefPtr<IVertexArray> m_vao;
	 *     uint32 m_index_count;
	 *
	 * public:
	 *     Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices) {
	 *         m_vao = gpu::IVertexArray::create();
	 *
	 *         auto vb = gpu::IVertexBuffer::create(
	 *             vertices.data(),
	 *             vertices.size() * sizeof(Vertex)
	 *         );
	 *         vb->setLayout(VertexBufferLayout({
	 *             VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),
	 *             VertexBufferElement(EShaderDataType::eFloat3, "a_Normal")
	 *         }));
	 *         m_vao->addVertexBuffer(vb);
	 *
	 *         auto ib = gpu::IIndexBuffer::create(indices.data(), indices.size());
	 *         m_vao->setIndexBuffer(ib);
	 *
	 *         m_index_count = indices.size();
	 *     }
	 *
	 *     void render() {
	 *         m_vao->bind();
	 *         glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, 0);
	 *     }
	 * };
	 * @endcode
	 *
	 * **Pattern 2: Multiple Vertex Buffers (Advanced)**
	 * @code
	 * auto vao = gpu::IVertexArray::create();
	 *
	 * // Position buffer
	 * auto pos_vb = gpu::IVertexBuffer::create(positions.data(), positions.size() * 12);
	 * pos_vb->setLayout(VertexBufferLayout({
	 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position")
	 * }));
	 * vao->addVertexBuffer(pos_vb);
	 *
	 * // Normal buffer (separate)
	 * auto norm_vb = gpu::IVertexBuffer::create(normals.data(), normals.size() * 12);
	 * norm_vb->setLayout(VertexBufferLayout({
	 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Normal")
	 * }));
	 * vao->addVertexBuffer(norm_vb);
	 * @endcode
	 *
	 * **Performance Characteristics:**
	 * - Creation: O(1) - GPU VAO allocation
	 * - Adding buffer: O(1) - storing reference + configuring attributes
	 * - Binding: O(1) - GPU state change
	 * - Memory: Minimal (just stores references; buffers hold the data)
	 *
	 * **Backend Implementation (OpenGL):**
	 * In GLVertexArray:
	 * - VAO ID obtained via glGenVertexArrays()
	 * - Binding via glBindVertexArray()
	 * - Attributes configured via glVertexAttribPointer() and glEnableVertexAttribArray()
	 * - Layout information (stride, offset) used during binding
	 *
	 * @see IVertexBuffer for vertex data management
	 * @see IIndexBuffer for index data management
	 * @see VertexBufferLayout for buffer layout specification
	 * @see GLVertexArray for OpenGL implementation
	 *
	 * @note Always set vertex layout before adding buffer to VAO
	 * @note Index buffer is optional (not all geometries need indexed drawing)
	 * @note Multiple VAOs can share the same buffers
	 * @note Bind VAO before draw calls to use its configuration
	 */
	class IVertexArray
	{
	public:
		/*!
		 * @brief Factory method: Creates a new vertex array object
		 * @details
		 * Allocates a GPU vertex array object (VAO) with no buffers configured.
		 * Buffers must be added via addVertexBuffer() and setIndexBuffer() before rendering.
		 *
		 * **Initialization State:**
		 * - VAO is created empty (no buffers)
		 * - No index buffer configured
		 * - Ready to add vertex buffers
		 *
		 * **Typical Usage:**
		 * @code
		 * auto vao = gpu::IVertexArray::create();
		 *
		 * // Configure with buffers
		 * vao->addVertexBuffer(vertex_buffer);
		 * vao->setIndexBuffer(index_buffer);
		 *
		 * // Use for rendering
		 * vao->bind();
		 * glDrawElements(...);
		 * @endcode
		 *
		 * @return RefPtr to newly created IVertexArray
		 *
		 * **Complexity:** O(1) - GPU VAO allocation
		 *
		 * @see addVertexBuffer() to add vertex buffers
		 * @see setIndexBuffer() to add index buffer
		 */
		static RefPtr<IVertexArray> create();

		virtual                    ~IVertexArray() = default;

		/*!
		 * @brief Binds vertex array to current OpenGL context
		 * @details
		 * Makes this VAO the active vertex array object for subsequent draw operations.
		 * In OpenGL terms, this calls glBindVertexArray(vao_id).
		 *
		 * After binding:
		 * - GPU uses this VAO's configuration for rendering
		 * - All buffers and attribute settings come from this VAO
		 * - Draw calls (glDrawElements, glDrawArrays) use this VAO's state
		 *
		 * **Typical Rendering Sequence:**
		 * @code
		 * // Bind shader
		 * shader->bind();
		 *
		 * // Bind VAO (activates buffers and attribute configuration)
		 * vao->bind();
		 *
		 * // Issue draw command
		 * glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
		 *
		 * // Cleanup
		 * vao->unbind();
		 * @endcode
		 *
		 * @note Must be bound before draw calls to use its configuration
		 * @note Binding persists until another VAO is bound or unbind() is called
		 * @see unbind() to deactivate the VAO
		 * @see Typical usage: bind once per render call
		 */
		virtual void bind() = 0;

		/*!
		 * @brief Unbinds vertex array from current OpenGL context
		 * @details
		 * Deactivates this VAO by binding the null VAO (id 0).
		 * In OpenGL terms, this calls glBindVertexArray(0).
		 *
		 * After unbinding:
		 * - No VAO is active
		 * - Subsequent draw calls would fail without binding a VAO
		 * - Clean state for other VAO operations
		 *
		 * @note Generally not required; binding another VAO automatically unbinds the previous one
		 * @note Can be omitted in typical usage
		 * @see bind() to activate the VAO
		 */
		virtual void unbind() = 0;

		/*!
		 * @brief Adds a vertex buffer to this vertex array
		 * @details
		 * Associates a vertex buffer with this VAO, adding it to the list of buffers
		 * that provide vertex attribute data. Multiple buffers can be added for advanced
		 * configurations, though single buffer (interleaved) is most common.
		 *
		 * **Buffer Configuration:**
		 * The vertex buffer must have a layout set (via IVertexBuffer::setLayout())
		 * before being added to the VAO. The layout defines:
		 * - Which attributes are present
		 * - Stride (bytes per vertex)
		 * - Offset of each attribute within the vertex
		 *
		 * **Single Buffer (Typical):**
		 * @code
		 * auto vb = gpu::IVertexBuffer::create(vertex_data, vertex_data_size);
		 * vb->setLayout(VertexBufferLayout({
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Normal"),
		 *     VertexBufferElement(EShaderDataType::eFloat2, "a_TexCoord")
		 * }));
		 * vao->addVertexBuffer(vb);  // All attributes from one buffer
		 * @endcode
		 *
		 * **Multiple Buffers (Advanced):**
		 * @code
		 * // Position buffer
		 * auto pos_vb = gpu::IVertexBuffer::create(positions, pos_size);
		 * pos_vb->setLayout(VertexBufferLayout({
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position")
		 * }));
		 * vao->addVertexBuffer(pos_vb);
		 *
		 * // Normal buffer (separate buffer)
		 * auto norm_vb = gpu::IVertexBuffer::create(normals, norm_size);
		 * norm_vb->setLayout(VertexBufferLayout({
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Normal")
		 * }));
		 * vao->addVertexBuffer(norm_vb);
		 * @endcode
		 *
		 * **Backend Behavior (OpenGL):**
		 * In GLVertexArray implementation:
		 * - Binds the vertex buffer to GL_ARRAY_BUFFER
		 * - For each layout element:
		 *   - Gets shader attribute location by name
		 *   - Calls glVertexAttribPointer() with layout stride/offset
		 *   - Calls glEnableVertexAttribArray() to activate attribute
		 *
		 * @param p_vertex_buffer RefPtr to vertex buffer to add
		 *
		 * **Complexity:** O(m) where m = number of layout elements
		 *                 (one attribute pointer setup per element)
		 *
		 * **Requirements:**
		 * - Vertex buffer must have layout set (setLayout() called)
		 * - Layout element names must match shader attribute names exactly
		 * - Vertex count must match other buffers if using multiple buffers
		 *
		 * @note Buffer is stored via RefPtr; lifetime guaranteed while VAO exists
		 * @note Multiple calls accumulate buffers (not replacements)
		 * @see IVertexBuffer for buffer creation
		 * @see VertexBufferLayout for layout definition
		 * @see getVertexBuffers() to retrieve added buffers
		 */
		virtual void addVertexBuffer(const RefPtr<IVertexBuffer> &p_vertex_buffer) = 0;

		/*!
		 * @brief Sets the index buffer for indexed drawing
		 * @details
		 * Associates an index buffer with this VAO, configuring it for indexed rendering.
		 * The index buffer stores triangle indices (or other primitive indices) that reference
		 * vertices in the added vertex buffers.
		 *
		 * **Index Rendering:**
		 * With an index buffer set, rendering uses glDrawElements():
		 * @code
		 * vao->bind();
		 * uint32 index_count = ib->getIndexCount();
		 * glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
		 * @endcode
		 *
		 * **Typical Setup:**
		 * @code
		 * // Create VAO with vertex and index buffers
		 * auto vao = gpu::IVertexArray::create();
		 *
		 * auto vb = gpu::IVertexBuffer::create(vertices.data(), vertices.size() * sizeof(Vertex));
		 * vb->setLayout(vertex_layout);
		 * vao->addVertexBuffer(vb);
		 *
		 * auto ib = gpu::IIndexBuffer::create(indices.data(), indices.size());
		 * vao->setIndexBuffer(ib);  // Configure for indexed drawing
		 * @endcode
		 *
		 * **Index Buffer Requirements:**
		 * - Indices must be uint32 (32-bit unsigned integers)
		 * - All index values must be < vertex_count
		 * - Typical grouping: 3 indices per triangle (GL_TRIANGLES)
		 * - Index count = triangle_count * 3
		 *
		 * **Backend Behavior (OpenGL):**
		 * In GLVertexArray implementation:
		 * - Binds index buffer to GL_ELEMENT_ARRAY_BUFFER
		 * - Stores reference for later glDrawElements() calls
		 * - Index buffer state is saved in VAO configuration
		 *
		 * @param p_index_buffer RefPtr to index buffer to use
		 *
		 * **Complexity:** O(1) - Stores reference and binds to GL_ELEMENT_ARRAY_BUFFER
		 *
		 * **Optional:**
		 * Index buffer can be skipped if using non-indexed rendering (glDrawArrays).
		 * However, indexed rendering is more efficient for meshes with shared vertices.
		 *
		 * **Multiple Calls:**
		 * Calling setIndexBuffer() multiple times replaces the previous index buffer.
		 * Useful for LOD (level-of-detail) switching or animation.
		 *
		 * @see IIndexBuffer for index buffer management
		 * @see getIndexBuffer() to retrieve current index buffer
		 * @note Always set vertex buffers before index buffer
		 * @note Index values must reference valid vertices (< vertex count)
		 */
		virtual void setIndexBuffer(const RefPtr<IIndexBuffer> &p_index_buffer) = 0;

		/*!
		 * @brief Gets all vertex buffers in this array
		 * @details
		 * Returns a const reference to the vector of vertex buffers that were added
		 * via addVertexBuffer(). Useful for introspection and debugging.
		 *
		 * **Typical Usage:**
		 * @code
		 * const auto& buffers = vao->getVertexBuffers();
		 * for (const auto& vb : buffers) {
		 *     uint32 vb_id = vb->getID();
		 *     const auto& layout = vb->getLayout();
		 *     // Inspect or use buffer...
		 * }
		 * @endcode
		 *
		 * @return Const reference to vector of RefPtr<IVertexBuffer>
		 *
		 * **Complexity:** O(1)
		 *
		 * @see addVertexBuffer() to add vertex buffers
		 * @see getIndexBuffer() for index buffer access
		 * @note Returns const reference; buffers cannot be modified through this
		 */
		[[nodiscard]] virtual const std::vector<RefPtr<IVertexBuffer> > &getVertexBuffers() const = 0;

		/*!
		 * @brief Gets the index buffer in this array
		 * @details
		 * Returns a const reference to the index buffer configured via setIndexBuffer().
		 * Returns nullptr if no index buffer has been set (for non-indexed rendering).
		 *
		 * **Typical Usage:**
		 * @code
		 * const auto& ib = vao->getIndexBuffer();
		 * if (ib) {
		 *     uint32 index_count = ib->getIndexCount();
		 *     glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
		 * } else {
		 *     // No index buffer - use non-indexed rendering
		 *     uint32 vertex_count = vao->getVertexBuffers()[0]->getLayout().getStride();
		 *     glDrawArrays(GL_TRIANGLES, 0, vertex_count);
		 * }
		 * @endcode
		 *
		 * @return Const reference to RefPtr<IIndexBuffer>
		 *         - Non-null if setIndexBuffer() was called
		 *         - Null (nullptr) if no index buffer configured
		 *
		 * **Complexity:** O(1)
		 *
		 * @see setIndexBuffer() to configure index buffer
		 * @see getVertexBuffers() for vertex buffer access
		 * @note Returns nullptr if no index buffer set; check before dereferencing
		 */
		[[nodiscard]] virtual const RefPtr<IIndexBuffer> &               getIndexBuffer() const = 0;
	};
}
