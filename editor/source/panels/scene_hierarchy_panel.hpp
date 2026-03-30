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

		Entity getSelectedEntity() const;

	private:
		void _drawEntityNode(Entity p_entity);
		void _drawComponents(Entity p_entity);

		RefPtr<Scene> m_scene;

		Entity m_selectedEntity;
	};
}
