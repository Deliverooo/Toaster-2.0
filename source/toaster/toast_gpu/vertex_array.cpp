#include "vertex_array.hpp"
#include "gl/gl_vertex_array.hpp"

namespace toaster::gpu
{
	RefPtr<VertexArray> VertexArray::create()
	{
		return std::make_shared<GLVertexArray>();
	}

}