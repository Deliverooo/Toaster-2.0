#pragma once

#include <string>
#include <glm/glm.hpp>
#include <utility>

#include "toaster/toast_lib/ptr.hpp"
#include "toaster/toast_lib/system_types.h"

#include "toaster/toast_gpu/texture.hpp"

#define DEFINE_COMPONENT(__name) struct __name

namespace toaster
{
	DEFINE_COMPONENT(TagComponent)
	{
		TagComponent()  = default;
		~TagComponent() = default;

		TagComponent(std::wstring p_tag) : tag(std::move(p_tag))
		{
		}

		std::wstring tag;
	};

	DEFINE_COMPONENT(TransformComponent)
	{
		TransformComponent()  = default;
		~TransformComponent() = default;

		TransformComponent(const glm::mat4 &p_transform) : transform(p_transform)
		{
		}

		glm::mat4 transform{1.0f};
	};

	DEFINE_COMPONENT(SpriteRendererComponent)
	{
		glm::vec4              colour{1.0f};
		RefPtr<gpu::Texture2D> texture{nullptr};
	};
}
