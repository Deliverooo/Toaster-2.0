#pragma once

#include "ptr.hpp"
#include "system_types.h"
#include "io/filesystem.hpp"

namespace toaster::gpu
{
	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual void bind(uint32 p_slot = 0) const = 0;

		[[nodiscard]] virtual uint32 getWidth() const = 0;
		[[nodiscard]] virtual uint32 getHeight() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static RefPtr<Texture2D> create(const io::filesystem::Path &p_path);
	};
}
