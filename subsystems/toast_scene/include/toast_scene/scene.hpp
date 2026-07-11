#pragma once

#include <entt/entt.hpp>

#include "components.hpp"
#include "toast_lib/string.hpp"

#include "toast_lib/uuid.hpp"
#include "toast_lib/events/event.hpp"

#include "light_environment.hpp"

namespace toaster
{
	namespace script
	{
		class ScriptEngine;
		class Class;
		class Object;
	}

	class Project;

	namespace render
	{
		class RenderContext;
	}
}

namespace toaster::scene
{
	class Entity;
	class ScriptableEntityCS; // Inside the scene.cpp file

	class TST_SCENE_API Scene
	{
	public:
		using ComponentType    = void *; // Actually a MonoType*...
		using HasComponentFn   = bool(*)(Entity *);
		using AddComponentFn   = void(*)(Entity *);
		using ResetComponentFn = void(*)(Entity *);

		Scene(render::RenderContext *p_render_ctx, script::ScriptEngine *p_script_engine = nullptr, const String &p_name = "");
		~Scene();

		[[nodiscard]] auto getRenderCtx() const -> NonOwningPtr<render::RenderContext>;

		auto onUpdate(float32 p_dt) -> void;
		auto onEvent(Event &p_event) -> void;

		auto onResize(tsm::uint2 p_size) -> void;

		auto createEntity(const String &p_name = "") -> Entity;
		auto createEntityWithUUID(UUID p_uuid, const String &p_name = "") -> Entity;
		auto destroyEntity(Entity p_entity) -> void;

		auto getEntityByUUID(UUID p_uuid) const -> Entity;
		auto getEntityByName(const String &p_name) const -> Entity;

		auto XM_CALLCONV getEntityWorldTransformMatrix(Entity p_entity) const -> Dx::XMMATRIX;
		auto             getEntityWorldTransformComponent(Entity p_entity) const -> TransformComponent;

		auto getMainCameraEntity() const -> Entity;

		auto               getRegistry() -> entt::registry &;
		[[nodiscard]] auto getRegistry() const -> const entt::registry &;

		auto               setName(const String &p_name) -> void;
		[[nodiscard]] auto getName() const -> String;

		auto getSkyboxMap() const -> const render::ImageHandle &;
		auto getDiffuseIrradianceMap() const -> const render::ImageHandle &;
		auto getSpecularIrradianceMap() const -> const render::ImageHandle &;
		auto setSkyboxMap(const render::ImageHandle &p_skybox_map) -> void;

		auto submitPointLight(const PointLight &p_point_light) -> void; // Ts has to be called every frame, because the environment gets cleared
		auto getLightEnvironment() const -> const LightEnvironment &;

		auto initNativeScripts() -> void; // You can actually call this any time you need to immediately construct a script

		auto getScriptEngine() -> NonOwningPtr<script::ScriptEngine>;
		auto getHasComponentFn(ComponentType p_component_type) -> HasComponentFn &;
		auto getAddComponentFn(ComponentType p_component_type) -> AddComponentFn &;
		auto getResetComponentFn(ComponentType p_component_type) -> ResetComponentFn &;

	private:
		auto _registerScriptMethods() -> void;

		template<typename Type>
		auto onComponentAdded(Entity &p_entity, Type &p_component) -> void
		{
		}

		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};
		NonOwningPtr<script::ScriptEngine>  m_scriptEngine{nullptr};

		entt::registry                         m_registry;
		std::unordered_map<UUID, entt::entity> m_entityUUIDMap;

		String m_name;

		tsm::uint2 m_viewportSize{0u};

		uint32 m_newEntityTagCount{0u};

		RefPtr<script::Class>                                 m_baseEntityClass{nullptr};
		std::unordered_map<String, RefPtr<script::Class> >    m_entityClassMap;
		std::unordered_map<UUID, RefPtr<ScriptableEntityCS> > m_entityScriptMap;

		std::unordered_map<ComponentType, HasComponentFn>   m_hasComponentFnMap;
		std::unordered_map<ComponentType, AddComponentFn>   m_addComponentFnMap;
		std::unordered_map<ComponentType, ResetComponentFn> m_resetComponentFnMap;

		struct
		{
			render::ImageHandle skyboxMap{nullptr};
			render::ImageHandle diffuseIrradianceMap{nullptr};
			render::ImageHandle specularIrradianceMap{nullptr};
		} m_environment;

		LightEnvironment m_lightEnvironment;

		friend class ScriptableEntityCS;
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneImporter;
	};

	template<>
	inline auto Scene::onComponentAdded<CameraComponent>(Entity &p_entity, CameraComponent &p_component) -> void
	{
		(void) p_entity;
		p_component.camera.setViewportSize(m_viewportSize);
	}
}
