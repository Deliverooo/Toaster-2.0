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

		auto setScene(const RefPtr<Scene> &p_scene) -> void;

		auto onUIRender(uint32 p_frame_index) -> void;

		auto getSelectedEntity() const -> Entity;
		auto setSelectedEntity(Entity p_entity) -> void;

	private:
		auto _drawEntityNode(Entity p_entity, uint32 p_frame_index) -> void;
		auto _drawComponents(Entity p_entity, uint32 p_frame_index) -> void;
		auto _drawMaterial(uint32 p_frame_index,const RefPtr<gpu::VKMaterial> &p_mat) -> void;

		gpu::VKGPUContext *m_ctx{nullptr};

		RefPtr<Scene> m_scene;

		Entity m_selectedEntity;

		template<typename Type, bool Removable, typename UIFunc>
		friend auto drawComponent(const String &p_name, Entity p_entity, UIFunc p_func, void *p_caller_id) -> void;
	};
}
