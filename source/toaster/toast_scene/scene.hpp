#pragma once

#include <entt/entt.hpp>

#include "toast_lib/string.hpp"
#include "toast_lib/math/colours.hpp"
#include "toast_render/renderer_2d.hpp"

#include "toast_gpu/vk/vk_mesh.hpp"

namespace toaster
{
	class Entity;
	class SceneRenderer;

	struct DirectionalLight
	{
		glm::vec4 direction{0.0f};
		glm::vec4 radiance{1.0f};
	};

	struct PointLight
	{
		glm::vec4 position{0.0f};
		glm::vec4 radiance{1.0f};
		float32 radius{25.0f};
		float32 falloff{1.0f};
	};

	struct SpotLight
	{
		glm::vec3 position{0.0f};
		glm::vec3 radiance{1.0f};
		float32   falloff{1.0f};
		float32   multiplier{1.0f};
		float32   angle{67.0f};
		float32   range{12.0f};
	};

	struct SceneLightEnvironment
	{
		std::vector<DirectionalLight> directionalLights;
		std::vector<PointLight>       pointLights;
		// std::vector<SpotLight>        spotLights;
	};

	class Scene
	{
	public:
		Scene(gpu::VKLogicalDevice *p_ctx, const String &p_name = "");
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

		auto getLightEnvironment() const -> const SceneLightEnvironment &;

	private:
		template<typename Type>
		auto onComponentAdded(Entity p_entity, Type &p_component) -> void;

		gpu::VKLogicalDevice *m_device{nullptr};

		entt::registry m_registry;

		String m_name;

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		uint32 m_newEntityTagCount{0u};

		SceneLightEnvironment m_lightEnvironment;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneRenderer;
	};
}
