#include "vertex_array.hpp"
#include "gl/gl_vertex_array.hpp"

namespace toaster::gpu
{
	RefPtr<IVertexArray> IVertexArray::create()
	{
		return make_reference<GLVertexArray>();
	}

}