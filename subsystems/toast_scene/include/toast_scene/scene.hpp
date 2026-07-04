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

	class Project;
	class Entity;
	class ScriptableEntityCS; // Inside the scene.cpp file

	namespace render
	{
		class RenderContext;
	}

	struct TST_SCENE_API DirectionalLight
	{
		Dx::XMFLOAT3 direction{0.0f, 0.0f, 0.0f};
		char         _padd[4];
		tsm::float3  radiance{1.0f, 1.0f, 1.0f};
		float32      multiplier{1.0f};
	};

	struct TST_SCENE_API PointLight
	{
		Dx::XMFLOAT3 position{0.0f, 0.0f, 0.0f};
		char         _padd[4];
		tsm::float3  radiance{1.0f, 1.0f, 1.0f};
		float32      multiplier{1.0f};
		// float32   radius{25.0f};
		// float32   falloff{1.0f};
	};

	struct TST_SCENE_API SpotLight
	{
		tsm::float3 position{0.0f};
		tsm::float3 radiance{1.0f};
		float32     falloff{1.0f};
		float32     multiplier{1.0f};
		float32     angle{67.0f};
		float32     range{12.0f};
	};

	struct TST_SCENE_API SceneLightEnvironment
	{
		std::vector<DirectionalLight> directionalLights;
		std::vector<PointLight>       pointLights;
	};

	class TST_SCENE_API Scene
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

		auto onResize(tsm::uint2 p_size) -> void;

		auto createEntity(const String &p_name = "") -> Entity;
		auto createEntityWithUUID(UUID p_uuid, const String &p_name = "") -> Entity;
		auto destroyEntity(Entity p_entity) -> void;

		auto getEntityByUUID(UUID p_uuid) -> Entity;
		auto getEntityByName(const String &p_name) -> Entity;

		auto XM_CALLCONV getEntityWorldTransformMatrix(Entity p_entity) const -> Dx::XMMATRIX;
		auto             getEntityWorldTransformComponent(Entity p_entity) const -> TransformComponent;

		auto getMainCameraEntity() -> Entity;

		auto               getRegistry() -> entt::registry &;
		[[nodiscard]] auto getRegistry() const -> const entt::registry &;

		auto               setName(const String &p_name) -> void;
		[[nodiscard]] auto getName() const -> String;

		auto getLightEnvironment() const -> const SceneLightEnvironment &;

		auto getSceneEnvironment() const -> const gpu::Texture3DHandle &;
		auto setSceneEnvironment(const gpu::Texture3DHandle &p_environment) -> void;

		auto getSceneEnvironmentImage() const -> const render::ImageHandle &;
		auto setSceneEnvironmentImage(const render::ImageHandle &p_environment) -> void;

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

		SceneLightEnvironment m_lightEnvironment;

		// The asset the represents the .hdr / environment map file for the scene renderer to use
		struct
		{
			render::ImageHandle  skyboxMapImage{nullptr};
			render::ImageHandle  diffuseIrradianceMapImage{nullptr};
			render::ImageHandle  specularIrradianceMapImage{nullptr};
			gpu::Texture3DHandle skyboxMap{nullptr};            // TODO: Remove
			gpu::Texture3DHandle diffuseIrradianceMap{nullptr}; // Created from the skybox and updated if it changes
		} m_sceneEnvironment;

		bool m_reloadEnvironment{false};

		friend class ScriptableEntityCS;
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneRenderer;
		friend class DynamicSceneRenderer;
		friend class SceneImporter;
	};

	template<>
	inline auto Scene::onComponentAdded<CameraComponent>(Entity &p_entity, CameraComponent &p_component) -> void
	{
		(void) p_entity;
		p_component.camera.setViewportSize(m_viewportSize);
	}
}
