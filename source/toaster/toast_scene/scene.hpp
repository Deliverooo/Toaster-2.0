#pragma once

#include <entt/entt.hpp>

#include "toast_lib/string.hpp"
#include "toast_render/renderer_2d.hpp"

#include "toast_gpu/vk/vk_mesh.hpp"

namespace toaster
{
	class Entity;
	class SceneRenderer;

	class Scene
	{
	public:
		Scene(gpu::VKGPUContext *p_ctx, const String &p_name = "");
		~Scene();

		auto onUpdate(float32 p_dt) -> void;

		auto onRender(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, float32 p_dt, const RefPtr<SceneRenderer> &p_scene_renderer) -> void;
		auto onRender(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, float32 p_dt, const RefPtr<SceneRenderer> &p_scene_renderer, const glm::mat4 &p_view,
					  const glm::mat4 &              p_projection) -> void;

		auto setViewportSize(uint32 p_width, uint32 p_height) -> void;

		auto createEntity(const String &p_name = "") -> Entity;
		auto destroyEntity(Entity p_entity) -> void;

		auto getMainCameraEntity() -> Entity;

		auto               getRegistry() -> entt::registry &;
		[[nodiscard]] auto getRegistry() const -> const entt::registry &;

		auto               setName(const String &p_name) -> void;
		[[nodiscard]] auto getName() const -> String;

	private:
		template<typename Type>
		auto onComponentAdded(Entity p_entity, Type &p_component) -> void;

		gpu::VKGPUContext *m_ctx{nullptr};

		entt::registry m_registry;

		String m_name;

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		uint32 m_newEntityTagCount{0u};

		RefPtr<gpu::VKMesh> m_mesh{nullptr};

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneRenderer;
	};
}
