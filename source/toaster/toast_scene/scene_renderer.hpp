#pragma once

#include "scene.hpp"

namespace toaster
{
	class SceneRenderer
	{
	public:
		SceneRenderer(RefPtr<Scene> p_scene);

	private:
		RefPtr<Scene> m_scene{nullptr};
	};
}
