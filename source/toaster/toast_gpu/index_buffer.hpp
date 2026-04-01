/*!
 * @file index_buffer.hpp
 * @brief Abstract index buffer interface for GPU-accelerated indexed rendering
 * @details
 * Defines the abstract interface for GPU index buffers, which store vertex indices
 * for indexed drawing. Index buffers enable efficient rendering by reusing vertices
 * instead of duplicating them in the vertex buffer.
 *
 * **Index Buffer Concepts:**
 * An index buffer contains indices (typically uint32) that reference vertices in a vertex buffer.
 * Instead of sending all vertex data multiple times, you send vertex data once and use indices
 * to reuse vertices efficiently. This is critical for memory-intensive meshes.
 *
 * **Example - Indexed vs Non-indexed Rendering:**
 * ```cpp
 * // Non-indexed (inefficient - duplicate vertices):
 * struct Vertex { glm::vec3 pos; };
 * Vertex vertices[] = {
 *     {0,0,0}, {1,0,0}, {0,1,0},  // Triangle 1
 *     {0,1,0}, {1,0,0}, {1,1,0},  // Triangle 2 (reuses some vertices!)
 * };
 * vb->setData(vertices, 6 * sizeof(Vertex));
 *
 * // Indexed (efficient - vertices used once, indices reuse them):
 * Vertex vertices[] = {
 *     {0,0,0}, {1,0,0}, {0,1,0}, {1,1,0}  // 4 unique vertices
 * };
 * uint32 indices[] = {
 *     0, 1, 2,  // Triangle 1
 *     2, 1, 3   // Triangle 2 (reuses vertices via indices)
 * };
 * vb->setData(vertices, 4 * sizeof(Vertex));
 * ib->setData(indices, 6 * sizeof(uint32));
 * // Draw with: drawIndexed(6 indices, 0 start, 0 base vertex)
 * ```
 *
 * **Integration with Rendering Pipeline:**
 * - Index buffer works with IVertexArray to manage both vertex and index data
 * - Must be bound via IVertexArray::setIndexBuffer()
 * - Used during draw calls (glDrawElements, glDrawElementsInstanced)
 *
 * @see IVertexArray for buffer binding and integrated rendering
 * @see IVertexBuffer for vertex data storage
 * @see GLIndexBuffer for OpenGL implementation
 *
 * @note Index buffers are optional; non-indexed rendering is also supported
 * @note Typically used for mesh data with shared vertices (models, terrain, etc.)
 */

#pragma once

#include "toast_lib/ptr.hpp"
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	/*!
	 * @class IIndexBuffer
	 * @brief Abstract interface for GPU index buffer management
	 * @details
	 * IIndexBuffer provides an abstraction layer for GPU index buffer operations.
	 * An index buffer stores unsigned 32-bit integers that reference vertices in a
	 * vertex buffer, enabling efficient indexed drawing (glDrawElements).
	 *
	 * **Index Buffer Lifecycle:**
	 * 1. **Creation**: Use static create() factories to allocate GPU memory
	 *    - create(count): Allocate empty buffer for 'count' indices
	 *    - create(data, count): Allocate and initialize with index data
	 *
	 * 2. **Data Binding**: Add buffer to vertex array using setIndexBuffer()
	 *    ```cpp
	 *    auto ib = IIndexBuffer::create(indices.data(), indices.size());
	 *    vertex_array->setIndexBuffer(ib);
	 *    ```
	 *
	 * 3. **Rendering**: Indices are automatically used during indexed draw calls
	 *    ```cpp
	 *    vertex_array->bind();
	 *    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	 *    ```
	 *
	 * **Data Format:**
	 * - Indices are 32-bit unsigned integers (uint32)
	 * - Each index references a vertex in the bound vertex buffer (0-based indexing)
	 * - Index values must be < vertex count in the vertex buffer
	 * - Typical usage: triangle lists (indices in groups of 3)
	 *
	 * **Memory Layout:**
	 * Index buffer stores indices in a tightly packed array:
	 * ```cpp
	 * uint32 indices[count] = { 0, 1, 2, 2, 1, 3, ... };
	 * //                        ^^^^^^^^^^^^^  ^^^^^^^
	 * //                        Triangle 1    Triangle 2
	 * ```
	 *
	 * **Performance Characteristics:**
	 * - Creation: O(1) - GPU memory allocation
	 * - Binding: O(1) - GPU state change
	 * - Drawing: O(count) - GPU processes 'count' indices
	 * - Memory usage: count * 4 bytes (one uint32 per index)
	 *
	 * **Common Usage Patterns:**
	 * @code
	 * // Pattern 1: Create from mesh indices
	 * const auto mesh = loadOBJMesh("mesh.obj");
	 * auto ib = gpu::IIndexBuffer::create(
	 *     (uint32*)mesh.indices.data(),
	 *     mesh.indices.size()
	 * );
	 *
	 * // Pattern 2: Dynamic indices (e.g., LOD switching)
	 * auto ib = gpu::IIndexBuffer::create(max_indices);  // Allocate max
	 * // Later, update indices for different LOD level:
	 * ib->setData(lod1_indices.data(), lod1_indices.size());
	 * @endcode
	 *
	 * **Typical Index Patterns:**
	 * - **Triangle List**: Every 3 indices form a triangle (most common)
	 * - **Indexed Cube**: 36 indices (6 faces × 2 triangles × 3 vertices)
	 * - **Indexed Sphere**: Variable based on tessellation
	 * - **Indexed Mesh**: Depends on mesh topology
	 *
	 * @see IVertexBuffer for complementary vertex data storage
	 * @see IVertexArray for integrated vertex+index buffer management
	 * @see GLIndexBuffer for OpenGL implementation details
	 * @see VertexBufferLayout for vertex structure definition
	 *
	 * @note Index buffers require IVertexArray integration for rendering
	 * @note Always bind vertex buffer before index buffer in VAO
	 * @note Index count must match actual indices during draw calls
	 * @note Proper index values (< vertex_count) prevent GPU access violations
	 */
	class IIndexBuffer
	{
	public:
		/*!
		 * @brief Factory method: Creates an empty index buffer
		 * @details
		 * Allocates GPU memory for an index buffer that can store 'p_count' indices.
		 * The buffer is initially uninitialized; data can be filled using setData().
		 *
		 * **Memory Allocation:**
		 * - GPU memory allocated: p_count * 4 bytes (one uint32 per index)
		 * - Memory is uninitialized; use setData() to populate
		 * - Useful for pre-allocating buffers for dynamic data
		 *
		 * **Typical Usage:**
		 * @code
		 * // Allocate for maximum possible indices
		 * const uint32 max_indices = 100000;
		 * auto ib = gpu::IIndexBuffer::create(max_indices);
		 *
		 * // Update with actual indices later
		 * const uint32 actual_indices = 1234;
		 * ib->setData(index_data, actual_indices);
		 * @endcode
		 *
		 * **Parameters:**
		 * @param p_count Number of indices to allocate space for
		 *
		 * **Returns:**
		 * RefPtr to newly allocated IIndexBuffer
		 *
		 * **Complexity:** O(1) - GPU memory allocation (no data transfer)
		 *
		 * @see create(uint32*, uint32) for immediate initialization
		 * @see setData() for filling buffer after creation
		 */
		static RefPtr<IIndexBuffer> create(uint32 p_count);

		/*!
		 * @brief Factory method: Creates and initializes index buffer with data
		 * @details
		 * Allocates GPU memory and immediately fills it with provided index data.
		 * More efficient than separate create() and setData() calls.
		 *
		 * **Index Data Format:**
		 * - Indices must be uint32 (32-bit unsigned integers)
		 * - Each index references a vertex: valid range [0, vertex_count)
		 * - Typical grouping: 3 indices per triangle (GL_TRIANGLES mode)
		 *
		 * **Typical Usage:**
		 * @code
		 * // Load mesh from file
		 * struct Mesh {
		 *     std::vector<Vertex> vertices;
		 *     std::vector<uint32> indices;
		 * };
		 * const auto mesh = loadOBJMesh("model.obj");
		 *
		 * // Create and init buffer in one call
		 * auto ib = gpu::IIndexBuffer::create(
		 *     mesh.indices.data(),
		 *     mesh.indices.size()
		 * );
		 *
		 * // Use in rendering
		 * vertex_array->setIndexBuffer(ib);
		 * @endcode
		 *
		 * **Parameters:**
		 * @param p_data Pointer to index array in CPU memory
		 * @param p_count Number of indices (size of p_data array)
		 *
		 * **Returns:**
		 * RefPtr to newly allocated and initialized IIndexBuffer
		 *
		 * **Memory Layout:**
		 * GPU buffer receives p_count * 4 bytes from p_data.
		 *
		 * **Complexity:** O(n) where n = p_count (GPU memory transfer)
		 *
		 * @see create(uint32) for empty buffer allocation
		 * @see setData() for updating indices after creation
		 *
		 * @note Data is copied to GPU; source can be freed after call
		 * @note All p_count indices are copied; ensure array is valid
		 * @note For validation, ensure all indices < vertex_count
		 */
		static RefPtr<IIndexBuffer> create(uint32 *p_data, uint32 p_count);

		virtual                     ~IIndexBuffer() = default;

		/*!
		 * @brief Binds index buffer to current OpenGL context
		 * @details
		 * Makes this index buffer the active index buffer for subsequent indexed draw operations.
		 * In OpenGL terms, this calls glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id).
		 *
		 * **Binding State:**
		 * - Binding affects the current vertex array state
		 * - Bound index buffer is used by glDrawElements() calls
		 * - Only one index buffer can be bound at a time
		 *
		 * **Typical Usage:**
		 * @code
		 * vao->bind();
		 * ib->bind();
		 * glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
		 * @endcode
		 *
		 * @note Usually called via IVertexArray::setIndexBuffer(), not directly
		 * @see unbind() to deactivate the buffer
		 * @see IVertexArray for proper buffer management
		 */
		virtual void bind() = 0;

		/*!
		 * @brief Unbinds index buffer from current OpenGL context
		 * @details
		 * Deactivates this index buffer by binding the null buffer (id 0).
		 * In OpenGL terms, this calls glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0).
		 *
		 * **Usage:**
		 * Generally not required as binding another buffer or changing VAO handles cleanup.
		 *
		 * @note Can be omitted if binding another buffer immediately
		 * @see bind() to activate the buffer
		 */
		virtual void unbind() = 0;

		/*!
		 * @brief Gets the GPU resource identifier
		 * @details
		 * Returns the backend-specific GPU resource handle.
		 * In OpenGL, this is the buffer object name (uint32 ID).
		 *
		 * @return GPU resource ID (OpenGL buffer name)
		 *
		 * @note Used internally; rarely needed by end users
		 * @note Don't use ID for direct GL calls; use the interface instead
		 */
		[[nodiscard]] virtual uint32 getID() const = 0;

		/*!
		 * @brief Gets the number of indices in buffer
		 * @details
		 * Returns the count of indices stored in this buffer.
		 * Used during draw calls to specify how many indices to process.
		 *
		 * **Typical Usage:**
		 * @code
		 * uint32 index_count = ib->getIndexCount();
		 * glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
		 * @endcode
		 *
		 * @return Number of indices (uint32 values) in buffer
		 *
		 * **Relationship to Memory:**
		 * - Memory usage = index_count * 4 bytes
		 * - Triangles rendered = index_count / 3 (for GL_TRIANGLES mode)
		 *
		 * @note Index count set at creation (create factories)
		 * @note Index count set by setData() if implemented
		 */
		[[nodiscard]] virtual uint32 getIndexCount() const = 0;
	};
}
