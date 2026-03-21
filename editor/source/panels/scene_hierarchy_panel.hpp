#pragma once

#include "toaster/toast_scene/entity.hpp"
#include "toaster/toast_scene/scene.hpp"

namespace toaster
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(const RefPtr<Scene> &p_scene);
		~SceneHierarchyPanel();

		void setScene(const RefPtr<Scene> &p_scene);

		void onUIRender();

	private:
		RefPtr<Scene> m_scene;
	};
}
