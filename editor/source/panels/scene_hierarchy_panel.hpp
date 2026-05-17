#pragma once

#include "toaster/toast_scene/entity.hpp"
#include "toaster/toast_scene/scene.hpp"
#include "toast_render/mesh.hpp"

namespace toaster
{
	namespace ui
	{
		class UITextureManager;
	}

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(render::RenderContext *p_render_ctx, const RefPtr<Scene> &p_scene, ui::UITextureManager *p_texture_manager);
		~SceneHierarchyPanel();

		auto setScene(const RefPtr<Scene> &p_scene) -> void;

		auto onUIRender(uint32 p_frame_index) -> void;

		auto getSelectedEntity() const -> Entity;
		auto setSelectedEntity(Entity p_entity) -> void;

	private:
		auto _drawEntityNode(Entity p_entity, uint32 p_frame_index) -> void;
		auto _drawComponents(Entity p_entity, uint32 p_frame_index) -> void;
		auto _drawMaterial(uint32 p_frame_index, render::MeshMaterialData &p_mat) -> void;

		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};
		RefPtr<Scene>                       m_scene{nullptr};
		NonOwningPtr<ui::UITextureManager>  m_textureManager{nullptr};

		Entity m_selectedEntity;

		template<typename Type, bool Removable, typename UIFunc>
		friend auto drawComponent(const String &p_name, Entity p_entity, UIFunc p_func, void *p_caller_id) -> void;
	};
}
