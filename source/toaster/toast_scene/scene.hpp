#pragma once

#include <entt/entt.hpp>

#include "components.hpp"
#include "toast_lib/string.hpp"
#include "toast_render/renderer_2d.hpp"

#include "toast_lib/uuid.hpp"
#include "toast_lib/events/event.hpp"

namespace toaster
{
	namespace script
	{
		class ScriptEngine;
		class Class;
		class Object;
	}

	class Entity;
	class SceneRenderer;
	class ScriptableEntityCS; // Inside the scene.cpp file

	namespace render
	{
		class RenderContext;
	}

	struct TST_API DirectionalLight
	{
		glm::vec4 direction{0.0f};
		glm::vec4 radiance{1.0f};
	};

	struct TST_API PointLight
	{
		glm::vec4 position{0.0f};
		glm::vec4 radiance{1.0f};
		// float32   radius{25.0f};
		// float32   falloff{1.0f};
	};

	struct TST_API SpotLight
	{
		glm::vec3 position{0.0f};
		glm::vec3 radiance{1.0f};
		float32   falloff{1.0f};
		float32   multiplier{1.0f};
		float32   angle{67.0f};
		float32   range{12.0f};
	};

	struct TST_API SceneLightEnvironment
	{
		std::vector<DirectionalLight> directionalLights;
		std::vector<PointLight>       pointLights;
	};

	class TST_API Scene
	{
	public:
		using ComponentType    = void *; // Actually a MonoType*...
		using HasComponentFn   = bool(*)(Entity *);
		using AddComponentFn   = void(*)(Entity *);
		using ResetComponentFn = void(*)(Entity *);

		Scene(render::RenderContext *p_render_ctx, script::ScriptEngine *p_script_engine = nullptr, const String &p_name = "");
		~Scene();

		auto onUpdate(float32 p_dt) -> void;
		auto onEvent(Event &p_event) -> void;

		auto onRender(gpu::VKCommandBuffer *p_cmd, uint32 p_frame_index, float32 p_dt, const RefPtr<SceneRenderer> &p_scene_renderer) -> void;
		auto onRender(gpu::VKCommandBuffer *p_cmd, uint32 p_frame_index, float32 p_dt, const RefPtr<SceneRenderer> &p_scene_renderer, const glm::mat4 &p_view,
					  const glm::mat4 &     p_projection) -> void;

		auto setViewportSize(uint32 p_width, uint32 p_height) -> void;

		auto createEntity(const String &p_name = "") -> Entity;
		auto createEntityWithUUID(UUID p_uuid, const String &p_name = "") -> Entity;
		auto destroyEntity(Entity p_entity) -> void;

		auto getEntityByUUID(UUID p_uuid) -> Entity;
		auto getEntityByName(const String &p_name) -> Entity;

		auto getEntityWorldTransformMatrix(Entity p_entity) const -> glm::mat4;
		auto getEntityWorldTransformComponent(Entity p_entity) const -> TransformComponent;

		auto getMainCameraEntity() -> Entity;

		auto               getRegistry() -> entt::registry &;
		[[nodiscard]] auto getRegistry() const -> const entt::registry &;

		auto               setName(const String &p_name) -> void;
		[[nodiscard]] auto getName() const -> String;

		auto getLightEnvironment() const -> const SceneLightEnvironment &;

		auto getScriptEngine() -> NonOwningPtr<script::ScriptEngine>;
		auto getHasComponentFn(ComponentType p_component_type) -> HasComponentFn &;
		auto getAddComponentFn(ComponentType p_component_type) -> AddComponentFn &;
		auto getResetComponentFn(ComponentType p_component_type) -> ResetComponentFn &;

	private:
		auto _registerScriptMethods() -> void;

		template<typename Type>
		TST_API auto onComponentAdded(Entity p_entity, Type &p_component) -> void;

		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};
		NonOwningPtr<script::ScriptEngine>  m_scriptEngine{nullptr};

		entt::registry                         m_registry;
		std::unordered_map<UUID, entt::entity> m_entityUUIDMap;

		String m_name;

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		uint32 m_newEntityTagCount{0u};

		RefPtr<script::Class>                                 m_baseEntityClass{nullptr};
		std::unordered_map<String, RefPtr<script::Class> >    m_entityClassMap;
		std::unordered_map<UUID, RefPtr<ScriptableEntityCS> > m_entityScriptMap;

		std::unordered_map<ComponentType, HasComponentFn>   m_hasComponentFnMap;
		std::unordered_map<ComponentType, AddComponentFn>   m_addComponentFnMap;
		std::unordered_map<ComponentType, ResetComponentFn> m_resetComponentFnMap;

		SceneLightEnvironment m_lightEnvironment;

		friend class ScriptableEntityCS;
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneRenderer;
		friend class SceneImporter;
	};
}
