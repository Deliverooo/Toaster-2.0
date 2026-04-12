#pragma once

#include "toaster/toast_scene/entity.hpp"
#include "toaster/toast_scene/scene.hpp"

namespace toaster
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(gpu::VKGPUContext *p_ctx, const RefPtr<Scene> &p_scene);
		~SceneHierarchyPanel();

		void setScene(const RefPtr<Scene> &p_scene);

		void onUIRender();

		Entity getSelectedEntity() const;
		void   setSelectedEntity(Entity p_entity);

	private:
		void _drawEntityNode(Entity p_entity);
		void _drawComponents(Entity p_entity);

		gpu::VKGPUContext *m_ctx{nullptr};

		RefPtr<Scene> m_scene;

		Entity m_selectedEntity;

		template<typename Type, bool Removable, typename UIFunc>
		friend void drawComponent(const String &p_name, Entity p_entity, UIFunc p_func, void *p_caller_id);
	};
}
