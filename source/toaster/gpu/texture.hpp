#pragma once

#include "ptr.hpp"
#include "system_types.h"
#include "io/filesystem.hpp"

namespace toaster::gpu
{
	class Texture
	{
	public:
		static RefPtr<Texture> create(const io::filesystem::Path &p_path);
		virtual                ~Texture() = default;

		virtual void bind();
		virtual void unbind();
	};
}
