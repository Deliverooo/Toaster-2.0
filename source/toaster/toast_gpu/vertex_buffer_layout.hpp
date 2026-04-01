/*!
 * @file vertex_buffer_layout.hpp
 * @brief Vertex buffer layout system for defining GPU vertex attribute structure
 * @details
 * This file defines the type system and layout management for vertex attributes in GPU buffers.
 * It provides:
 * - Enumeration of supported shader data types (scalars, vectors, matrices)
 * - Size calculation for each type (crucial for memory layout)
 * - Element structure for individual vertex attributes
 * - Layout class for organizing multiple attributes with automatic offset calculation
 *
 * The layout system automatically calculates:
 * - Byte offset of each attribute within the vertex structure
 * - Total stride (size in bytes of one complete vertex)
 * 
 * This is used in conjunction with IVertexBuffer::setLayout() to describe how vertex data
 * is organized in GPU memory, enabling proper attribute binding in the rendering pipeline.
 *
 * @see VertexBufferLayout for auto-calculation of offsets and stride
 * @see IVertexBuffer for usage pattern with GPU buffer binding
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "toast_lib/system_types.h"
#include "toast_lib/toast_assert.h"

namespace toaster::gpu
{
	/*!
	 * @enum EShaderDataType
	 * @brief Enumeration of all supported vertex shader data types
	 * @details
	 * Represents the fundamental data types that can be used for vertex attributes.
	 * Each type is 4-byte aligned (float, int) or multiples thereof (vec2-4, mat3-4).
	 *
	 * Size mapping (in bytes):
	 * - eFloat, eInt: 4 bytes (1 component)
	 * - eFloat2, eInt2: 8 bytes (2 components)
	 * - eFloat3, eInt3: 12 bytes (3 components)
	 * - eFloat4, eInt4: 16 bytes (4 components)
	 * - eMat3: 36 bytes (3×3 matrix = 3 rows of float3)
	 * - eMat4: 64 bytes (4×4 matrix = 4 rows of float4)
	 * - eBool: 1 byte (packed boolean)
	 *
	 * @note Use shaderDataTypeSize() function to retrieve size at compile-time
	 * @note Component counts available via VertexBufferElement::getComponentCount()
	 */
	enum class EShaderDataType
	{
		eFloat,  //!< Single 32-bit float (4 bytes)
		eFloat2, //!< 2D vector of floats (8 bytes)
		eFloat3, //!< 3D vector of floats (12 bytes)
		eFloat4, //!< 4D vector of floats (16 bytes)
		eMat3,   //!< 3×3 matrix of floats (36 bytes)
		eMat4,   //!< 4×4 matrix of floats (64 bytes)
		eInt,    //!< Single 32-bit integer (4 bytes)
		eInt2,   //!< 2D vector of integers (8 bytes)
		eInt3,   //!< 3D vector of integers (12 bytes)
		eInt4,   //!< 4D vector of integers (16 bytes)
		eBool    //!< Single boolean (1 byte)
	};

	/*!
	 * @brief Compile-time size calculator for shader data types
	 * @details
	 * Calculates the byte size of any EShaderDataType at compile-time.
	 * This constexpr function enables use in template parameters and constant expressions.
	 *
	 * Size mapping:
	 * | Type      | Size | Components |
	 * |-----------|------|------------|
	 * | eFloat    | 4    | 1          |
	 * | eFloat2   | 8    | 2          |
	 * | eFloat3   | 12   | 3          |
	 * | eFloat4   | 16   | 4          |
	 * | eMat3     | 36   | 3×3 (9)    |
	 * | eMat4     | 64   | 4×4 (16)   |
	 * | eInt      | 4    | 1          |
	 * | eInt2     | 8    | 2          |
	 * | eInt3     | 12   | 3          |
	 * | eInt4     | 16   | 4          |
	 * | eBool     | 1    | 1          |
	 *
	 * @param p_type The shader data type to measure
	 * @return Size in bytes, or UINT32_MAX if type is unknown
	 *
	 * @note All scalar types are 4-byte aligned (GL standard)
	 * @note Vector/matrix types are multiples of component size
	 * @note Use this function rather than hardcoding sizes for type safety
	 *
	 * @code
	 * // Compile-time size calculation
	 * constexpr uint32 vec3_size = shaderDataTypeSize(EShaderDataType::eFloat3); // 12
	 * static_assert(vec3_size == 12);
	 * @endcode
	 *
	 * @see VertexBufferElement for runtime use
	 */
	constexpr uint32 shaderDataTypeSize(const EShaderDataType p_type)
	{
		switch (p_type)
		{
			case EShaderDataType::eFloat: { return 4; }
			case EShaderDataType::eFloat2: { return 4 * 2; }
			case EShaderDataType::eFloat3: { return 4 * 3; }
			case EShaderDataType::eFloat4: { return 4 * 4; }
			case EShaderDataType::eMat3: { return 4 * 3 * 3; }
			case EShaderDataType::eMat4: { return 4 * 4 * 4; }
			case EShaderDataType::eInt: { return 4; }
			case EShaderDataType::eInt2: { return 4 * 2; }
			case EShaderDataType::eInt3: { return 4 * 3; }
			case EShaderDataType::eInt4: { return 4 * 4; }
			case EShaderDataType::eBool: { return 1; }
		}
		return UINT32_MAX;
	}

	/*!
	 * @struct VertexBufferElement
	 * @brief Describes a single vertex attribute in a GPU buffer
	 * @details
	 * Represents metadata about one vertex attribute, including:
	 * - name: Shader variable name (e.g., "a_Position", "a_TexCoord")
	 * - type: Data type (EShaderDataType enum)
	 * - size: Byte size of this element (calculated from type)
	 * - offset: Byte offset from start of vertex structure (auto-calculated by VertexBufferLayout)
	 * - normalized: Whether integer attributes should be normalized to [0,1] or [-1,1]
	 *
	 * Typical usage pattern:
	 * @code
	 * // Define a vertex with position, color, and texture coordinates
	 * const auto layout = VertexBufferLayout({
	 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),
	 *     VertexBufferElement(EShaderDataType::eFloat4, "a_Color"),
	 *     VertexBufferElement(EShaderDataType::eFloat2, "a_TexCoord")
	 * });
	 * 
	 * // Layout automatically calculates:
	 * // a_Position offset: 0, size: 12
	 * // a_Color offset: 12, size: 16
	 * // a_TexCoord offset: 28, size: 8
	 * // Total stride: 36 bytes per vertex
	 * @endcode
	 *
	 * @note Offsets and stride are automatically calculated by VertexBufferLayout constructor
	 * @note Names must match exactly with shader attribute declarations
	 * @note For interleaved vertex data (common case), VertexBufferLayout handles offset math
	 *
	 * @see VertexBufferLayout for automatic offset calculation
	 * @see EShaderDataType for supported attribute types
	 * @see getComponentCount() for component information
	 */
	struct VertexBufferElement
	{
		std::string     name;         //!< Attribute name in shader (e.g., "a_Position")
		EShaderDataType type;         //!< Data type from EShaderDataType enum
		uint32          size;         //!< Size in bytes (calculated from type)
		uint32          offset;       //!< Byte offset within vertex struct (set by VertexBufferLayout)
		bool            normalized;   //!< Whether to normalize integer values (GL_TRUE/GL_FALSE)

		/*!
		 * @brief Constructs a vertex buffer element
		 * @details
		 * Initializes element metadata with automatic size calculation.
		 * The offset is initially set to 0 and will be recalculated by VertexBufferLayout.
		 *
		 * @param p_type The shader data type for this attribute
		 * @param p_name The shader variable name (must match shader code exactly)
		 * @param p_normalized Whether integer attributes should be normalized (default: false)
		 *                      - true: integers normalized to [0,1] (unsigned) or [-1,1] (signed)
		 *                      - false: integers passed as-is
		 *
		 * @code
		 * // Create elements with automatic size calculation
		 * VertexBufferElement pos(EShaderDataType::eFloat3, "a_Position");
		 * VertexBufferElement norm(EShaderDataType::eFloat3, "a_Normal");
		 * 
		 * // Element with normalization
		 * VertexBufferElement color(EShaderDataType::eInt4, "a_Color", true); // normalize integers
		 * @endcode
		 *
		 * @note Size is automatically calculated based on p_type using shaderDataTypeSize()
		 * @note Offset will be calculated when added to VertexBufferLayout
		 * @see VertexBufferLayout for offset calculation and stride management
		 */
		VertexBufferElement(EShaderDataType p_type, std::string p_name, bool p_normalized = false)
			: name(std::move(p_name)), type(p_type), size(shaderDataTypeSize(p_type)), offset(0), normalized(p_normalized)
		{
		}

		/*!
		 * @brief Gets the number of components in this element's type
		 * @details
		 * Returns the component count for the element's data type:
		 * - Scalars (Float, Int, Bool): 1 component
		 * - Vectors (Float2-4, Int2-4): 2, 3, or 4 components
		 * - Matrices (Mat3, Mat4): 3 or 4 (for layout/shader purposes)
		 *
		 * Component mapping:
		 * | Type    | Components |
		 * |---------|------------|
		 * | eFloat  | 1          |
		 * | eFloat2 | 2          |
		 * | eFloat3 | 3          |
		 * | eFloat4 | 4          |
		 * | eInt    | 1          |
		 * | eInt2   | 2          |
		 * | eInt3   | 3          |
		 * | eInt4   | 4          |
		 * | eBool   | 1          |
		 * | eMat3   | 3 (rows)   |
		 * | eMat4   | 4 (rows)   |
		 *
		 * @return Component count (1-4)
		 *
		 * @note For matrices, returns row count, not total components
		 * @note For layout purposes, matrices are treated as multiple attributes
		 * @see IVertexBuffer::setLayout() for layout usage
		 *
		 * @code
		 * VertexBufferElement pos(EShaderDataType::eFloat3, "a_Position");
		 * assert(pos.getComponentCount() == 3);
		 * 
		 * VertexBufferElement mat(EShaderDataType::eMat4, "a_Transform");
		 * assert(mat.getComponentCount() == 4);
		 * @endcode
		 */
		[[nodiscard]] uint32 getComponentCount() const
		{
			switch (type)
			{
				case EShaderDataType::eFloat: { return 1; }
				case EShaderDataType::eFloat2: { return 2; }
				case EShaderDataType::eFloat3: { return 3; }
				case EShaderDataType::eFloat4: { return 4; }
				case EShaderDataType::eMat3: { return 3; }
				case EShaderDataType::eMat4: { return 4; }
				case EShaderDataType::eInt: { return 1; }
				case EShaderDataType::eInt2: { return 2; }
				case EShaderDataType::eInt3: { return 3; }
				case EShaderDataType::eInt4: { return 4; }
				case EShaderDataType::eBool: { return 1; }
			}
			TST_ASSERT_MSG(false, "Unknown shader data type");

			return 0;
		}
	};

	/*!
	 * @class VertexBufferLayout
	 * @brief Manages the structure and layout of vertex attributes in a GPU buffer
	 * @details
	 * VertexBufferLayout is the cornerstone of vertex data organization in the rendering pipeline.
	 * It performs several critical functions:
	 *
	 * **Automatic Offset Calculation:**
	 * When initialized with vertex elements, it automatically calculates the byte offset of each
	 * element within the vertex structure. This eliminates manual offset bookkeeping and prevents errors.
	 *
	 * **Stride Management:**
	 * Calculates the total stride (bytes per vertex) by summing element sizes.
	 * The stride is needed by the GPU to know how to advance through vertex data during rendering.
	 *
	 * **Interleaved Vertex Data Support:**
	 * Designed for interleaved vertex data (common pattern):
	 * ```
	 * struct Vertex {
	 *     glm::vec3 position;   // offset 0, size 12
	 *     glm::vec4 color;      // offset 12, size 16
	 *     glm::vec2 texCoord;   // offset 28, size 8
	 * };  // total stride: 36 bytes
	 * ```
	 *
	 * **Usage Pattern:**
	 * @code
	 * // Define vertex layout with interleaved attributes
	 * const auto layout = VertexBufferLayout({
	 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),
	 *     VertexBufferElement(EShaderDataType::eFloat4, "a_Color"),
	 *     VertexBufferElement(EShaderDataType::eFloat2, "a_TexCoord")
	 * });
	 *
	 * // Get layout information
	 * uint32 stride = layout.getStride();  // 36 bytes
	 * const auto& elements = layout.getElements();
	 *
	 * // Iterate and use
	 * for (const auto& element : layout) {
	 *     // element.name, element.offset, element.size, etc.
	 * }
	 *
	 * // Set layout on vertex buffer (GPU binding)
	 * vertexBuffer->setLayout(layout);
	 * @endcode
	 *
	 * **Iteration Support:**
	 * Provides range-based for loop support via begin()/end() iterators for convenient
	 * element iteration during shader binding setup.
	 *
	 * **Performance Characteristics:**
	 * - Construction: O(n) where n = number of elements (one pass to calculate offsets)
	 * - Iteration: O(1) per element
	 * - Stride/Elements access: O(1) (const reference returns)
	 *
	 * **Integration with Rendering:**
	 * The calculated offsets and stride are used by:
	 * - GPU drivers to interpret vertex buffer memory layout
	 * - Vertex array objects to bind attributes correctly
	 * - Shaders to access per-vertex data during rasterization
	 *
	 * @see VertexBufferElement for individual attribute metadata
	 * @see IVertexBuffer::setLayout() to apply layout to GPU buffers
	 * @see EShaderDataType for supported attribute types
	 *
	 * @note Stride includes ALL elements; gaps not supported (tightly packed)
	 * @note Default construction creates empty layout (useful for dynamic setup)
	 */
	class VertexBufferLayout
	{
	public:
		/*!
		 * @brief Default constructor for an empty layout
		 * @details Creates a VertexBufferLayout with no elements and zero stride.
		 * Useful when layout will be modified dynamically or copied from another.
		 *
		 * @code
		 * VertexBufferLayout layout;  // Empty initially
		 * @endcode
		 */
		VertexBufferLayout() = default;

		/*!
		 * @brief Constructs layout from initializer list of elements
		 * @details
		 * Initializes the layout with the provided elements and automatically calculates:
		 * - Offset for each element (in order, starting at 0)
		 * - Total stride (sum of all element sizes)
		 *
		 * The elements are stored in the order provided, and offsets are calculated
		 * sequentially to support interleaved vertex data.
		 *
		 * @param p_elements Initializer list of VertexBufferElement objects
		 *
		 * **Example:**
		 * @code
		 * const auto layout = VertexBufferLayout({
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),  // offset 0, size 12
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Normal"),    // offset 12, size 12
		 *     VertexBufferElement(EShaderDataType::eFloat2, "a_TexCoord")   // offset 24, size 8
		 * });
		 * // Resulting stride: 32 bytes per vertex
		 * @endcode
		 *
		 * **Complexity:** O(n) where n = number of elements (one pass for offset calculation)
		 *
		 * @note Elements are copied into internal storage
		 * @note Order matters: offsets are calculated in the order elements are provided
		 * @note Stride is accumulative: sum of all element.size values
		 */
		VertexBufferLayout(std::initializer_list<VertexBufferElement> p_elements) : m_elements(p_elements)
		{
			size_t offset = 0;
			for (auto &element: m_elements)
			{
				element.offset = offset;
				offset         += element.size;
				m_stride       += element.size;
			}
		}

		/*!
		 * @brief Gets the total stride (bytes per vertex)
		 * @details
		 * Returns the byte stride of one complete vertex, which is the sum of all element sizes.
		 * This value tells the GPU how many bytes to advance when moving to the next vertex
		 * in the buffer during rendering.
		 *
		 * @return Stride in bytes (sum of all element.size values)
		 *
		 * **Example:**
		 * @code
		 * const auto layout = VertexBufferLayout({
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Position"),  // 12 bytes
		 *     VertexBufferElement(EShaderDataType::eFloat3, "a_Normal"),    // 12 bytes
		 *     VertexBufferElement(EShaderDataType::eFloat2, "a_TexCoord")   // 8 bytes
		 * });
		 * 
		 * uint32 stride = layout.getStride();  // Returns 32
		 * @endcode
		 *
		 * **Usage:**
		 * The stride is used when:
		 * - Binding vertex attributes to shaders (glVertexAttribPointer stride parameter)
		 * - Managing memory layout of vertex data in CPU buffers
		 * - Calculating vertex buffer offsets in interleaved data
		 *
		 * @note Complexity: O(1)
		 * @note Stride is 0 for empty layouts
		 * @see VertexBufferElement for individual element sizes
		 */
		[[nodiscard]] uint32                                  getStride() const { return m_stride; }

		/*!
		 * @brief Gets all vertex buffer elements
		 * @details
		 * Returns a const reference to the vector of vertex buffer elements.
		 * Useful for iterating over elements or querying the complete layout structure.
		 *
		 * @return Const reference to vector of VertexBufferElement objects
		 *
		 * **Example:**
		 * @code
		 * const auto& elements = layout.getElements();
		 * for (const auto& elem : elements) {
		 *     std::cout << elem.name << " at offset " << elem.offset << '\n';
		 * }
		 * @endcode
		 *
		 * @note Complexity: O(1)
		 * @see begin()/end() const for range-based iteration
		 */
		[[nodiscard]] const std::vector<VertexBufferElement> &getElements() const { return m_elements; }

		/*!
		 * @brief Non-const begin iterator
		 * @return Iterator to first element
		 * @note Use for element modification (rarely needed)
		 */
		std::vector<VertexBufferElement>::iterator                     begin() { return m_elements.begin(); }

		/*!
		 * @brief Non-const end iterator
		 * @return Iterator past last element
		 * @note Use for element modification (rarely needed)
		 */
		std::vector<VertexBufferElement>::iterator                     end() { return m_elements.end(); }

		/*!
		 * @brief Const begin iterator for range-based for loops
		 * @details
		 * Returns const iterator to first element, enabling convenient iteration
		 * over layout elements in the typical use case.
		 *
		 * @return Const iterator to first element
		 *
		 * **Example:**
		 * @code
		 * for (const auto& element : layout) {
		 *     // element is const VertexBufferElement&
		 *     setupVertexAttribute(element);
		 * }
		 * @endcode
		 *
		 * @note Complexity: O(1)
		 */
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const { return m_elements.begin(); }

		/*!
		 * @brief Const end iterator for range-based for loops
		 * @return Const iterator past last element
		 * @note Enables range-based for loop termination
		 */
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const { return m_elements.end(); }

	private:
		std::vector<VertexBufferElement> m_elements;  //!< Ordered list of vertex attributes
		uint32                           m_stride{0u}; //!< Total bytes per vertex (sum of element sizes)
	};
}
