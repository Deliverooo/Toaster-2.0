#pragma once

#include "toaster/toast_lib/ptr.hpp"
#include "toaster/toast_lib/system_types.h"
#include "toaster/toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	class ITexture
	{
	public:
		virtual ~ITexture() = default;

		virtual void setData(void *p_data, uint32 p_size) = 0;

		virtual void bind(uint32 p_slot = 0) const = 0;

		[[nodiscard]] virtual uint32 getID() const = 0;

		[[nodiscard]] virtual uint32 getWidth() const = 0;
		[[nodiscard]] virtual uint32 getHeight() const = 0;

		virtual bool operator==(const ITexture &p_other) const = 0;
	};

	class ITexture2D : public ITexture
	{
	public:
		static RefPtr<ITexture2D> create(uint32 p_width, uint32 p_height);
		static RefPtr<ITexture2D> create(const io::filesystem::Path &p_path);
	};
}
