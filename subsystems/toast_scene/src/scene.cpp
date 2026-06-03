#include "toast_scene/scene.hpp"
#include "toast_scene/entity.hpp"
#include "toast_scene/scriptable_entity.hpp"

#include "toast_lib/events/window_event.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"

namespace toaster
{
	class ScriptableEntityCS
	{
	public:
		struct WindowResizeEventCS
		{
			tsm::float2 size{0.0f};
		};

		ScriptableEntityCS(const script::Class &p_class, Scene *p_scene, Entity p_entity) : m_obj(p_class)
		{
			TST_PERMA_ASSERT_MSG(m_obj.getClass().getScriptEngine(), "Class's script engine is null");

			m_onCreateMethod = m_obj.getClass().getMethod("OnCreate", 0);
			m_onUpdateMethod = m_obj.getClass().getMethod("OnUpdate", 1);

			m_onWindowResizeEventMethod = m_obj.getClass().getMethod("OnWindowResizeEvent", 1);
			// if (!m_onWindowResizeEventMethod)
			// m_onWindowResizeEventMethod = p_scene->m_baseEntityClass->getMethod("OnWindowResizeEvent", 1);

			m_obj.invoke(p_scene->m_baseEntityClass->getMethod(".ctor", 1), p_entity.getComponent<UUIDComponent>().uuid);
		}

		auto onCreate() -> void
		{
			m_obj.invoke(m_onCreateMethod);
		}

		auto onUpdate(float32 p_dt) -> void
		{
			m_obj.invoke(m_onUpdateMethod, p_dt);
		}

		auto onEvent(Event &p_event) -> void
		{
			if (p_event.getEventType() == WindowResizeEvent::getStaticType())
			{
				if (m_onWindowResizeEventMethod)
				{
					auto                window_resize_event{static_cast<WindowResizeEvent &>(p_event)};
					WindowResizeEventCS window_resize_event_data{};
					window_resize_event_data.size = tsm::float2{
						static_cast<float32>(window_resize_event.getWidth()),
						static_cast<float32>(window_resize_event.getHeight())
					};
					m_obj.invoke(m_onWindowResizeEventMethod, window_resize_event_data);
				}
			}
		}

		auto getObject() -> script::Object &
		{
			return m_obj;
		}

	private:
		script::Object m_obj;

		script::Method *m_onCreateMethod{nullptr};
		script::Method *m_onUpdateMethod{nullptr};
		script::Method *m_onWindowResizeEventMethod{nullptr};
	};

	static Scene *s_activeScene{nullptr};

	Scene::Scene(render::RenderContext *p_render_ctx, script::ScriptEngine *p_script_engine, const String &p_name) : m_renderCtx(p_render_ctx),
																													 m_scriptEngine(p_script_engine),
																													 m_name(p_name.empty() ? "Untitled Scene" : p_name)
	{
		s_activeScene = this;

		if (m_scriptEngine)
		{
			MonoImage *          image{m_scriptEngine->getAppImage()};
			const MonoTableInfo *type_definitions{mono_image_get_table_info(image, MONO_TABLE_TYPEDEF)};
			m_baseEntityClass = make_reference<script::Class>(m_scriptEngine, "Toaster", "Entity", script::EClassScope::eCore);

			// Register the default classes from the core library
			m_entityClassMap["Toaster.CameraController"] = make_reference<script::Class>(m_scriptEngine, "Toaster", "CameraController", script::EClassScope::eCore);

			for (uint32 row{0u}; row < mono_table_info_get_rows(type_definitions); ++row)
			{
				uint32 cols[MONO_TYPEDEF_SIZE]{};
				mono_metadata_decode_row(type_definitions, static_cast<int32>(row), cols, MONO_TYPEDEF_SIZE);

				auto   name_space{mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE])};
				auto   type_name{mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME])};
				String full_name{fmt::format("{}.{}", name_space, type_name)};

				if (full_name == "Toaster.Entity" || full_name == ".<Module>")
					continue;

				MonoClass *script_class{mono_class_from_name(image, name_space, type_name)};
				if (script_class && mono_class_is_subclass_of(script_class, m_baseEntityClass->getClass(), false))
				{
					m_entityClassMap[full_name] = toaster::make_reference<script::Class>(m_scriptEngine, script_class);
				}
			}

			for (const auto &[class_name, klass]: m_entityClassMap)
			{
				DEBUG_LOG_INFO("Found Entity class: {} ", class_name);
			}

			#define REGISTER_COMPONENT_TYPE(__type) {MonoType *managed_type{mono_reflection_type_from_name((char *) "Toaster."#__type, m_scriptEngine->getCoreImage())};\
													if(!managed_type) { LOG_FATAL("Could not find component: "#__type); TST_PERMA_ASSERT(false);}\
													m_hasComponentFnMap[managed_type] = +[](Entity *p_entity) -> bool { return p_entity->hasComponent<__type>(); };\
													m_addComponentFnMap[managed_type] = +[](Entity *p_entity) -> void { __type& comp{p_entity->addComponent<__type>()}; (void)comp; };\
													m_resetComponentFnMap[managed_type] = +[](Entity *p_entity) -> void { __type& comp{p_entity->getComponent<__type>()}; comp.reset(); };}
			REGISTER_COMPONENT_TYPE(TagComponent);
			REGISTER_COMPONENT_TYPE(TransformComponent);
			REGISTER_COMPONENT_TYPE(SpriteRendererComponent);
			REGISTER_COMPONENT_TYPE(MeshComponent);
			REGISTER_COMPONENT_TYPE(CameraComponent);
			REGISTER_COMPONENT_TYPE(DirectionalLightComponent);
			REGISTER_COMPONENT_TYPE(PointLightComponent);

			#undef REGISTER_COMPONENT_TYPE

			_registerScriptMethods();
		}
	}

	Scene::~Scene()
	{
		s_activeScene = nullptr;
	}

	auto Scene::onUpdate(float32 p_dt) -> void
	{
		{
			for (const auto view{m_registry.view<ScriptComponent>()}; const auto entity: view)
			{
				auto [class_name]{view.get<ScriptComponent>(entity)};

				Entity e{entity, this};
				UUID   uuid{e.getComponent<UUIDComponent>().uuid};
				if (!m_entityScriptMap.contains(uuid))
				{
					if (m_entityClassMap.contains(class_name))
					{
						m_entityScriptMap[uuid] = make_reference<ScriptableEntityCS>(*m_entityClassMap[class_name].get(), this, e);
						m_entityScriptMap[uuid]->onCreate();
					}
					else
					{
						LOG_ERROR("Invalid script class name: {}", class_name);
					}
				}

				if (m_entityScriptMap.contains(uuid))
					m_entityScriptMap[uuid]->onUpdate(p_dt);
			}
		}

		m_registry.view<NativeScriptComponent>().each([this, p_dt](auto p_entity, auto &p_script) -> void
		{
			if (!p_script.instance)
			{
				p_script.instance           = p_script.instantiateFn();
				p_script.instance->m_entity = {p_entity, this};
				p_script.instance->_superInit();
				p_script.instance->onCreate(p_script.userData);
			}

			p_script.instance->onUpdate(p_dt);
		});
	}

	auto Scene::onEvent(Event &p_event) -> void
	{
		m_registry.view<NativeScriptComponent>().each([&p_event](auto p_entity, auto &p_script) -> void
		{
			(void)p_entity;
			if (p_script.instance)
				p_script.instance->onEvent(p_event);
		});

		for (const auto view{m_registry.view<ScriptComponent>()}; const auto entity: view)
		{
			auto [class_name]{view.get<ScriptComponent>(entity)};

			Entity e{entity, this};
			UUID   uuid{e.getComponent<UUIDComponent>().uuid};

			if (m_entityScriptMap.contains(uuid))
				m_entityScriptMap[uuid]->onEvent(p_event);
		}
	}

	auto Scene::onResize(tsm::uint2 p_size) -> void
	{
		m_viewportSize = p_size;

		m_registry.view<NativeScriptComponent>().each([this](auto p_entity, NativeScriptComponent& p_script) -> void
		{
			if (p_script.instance)
				p_script.instance->onResize(m_viewportSize);
		});

		auto view{m_registry.view<CameraComponent>()};
		for (auto entity: view)
		{
			auto &cameraComponent{view.get<CameraComponent>(entity)};
			cameraComponent.camera.setViewportSize(m_viewportSize);
		}
	}

	auto Scene::createEntity(const String &p_name) -> Entity
	{
		return createEntityWithUUID({}, p_name);
	}

	auto Scene::createEntityWithUUID(UUID p_uuid, const String &p_name) -> Entity
	{
		TST_PERMA_ASSERT_MSG(!m_entityUUIDMap.contains(p_uuid), "Bradar wat is dis?");
		auto entity{Entity{m_registry.create(), this}};

		entity.addComponent<UUIDComponent>(p_uuid);
		entity.addComponent<RelationshipComponent>();

		m_entityUUIDMap[p_uuid] = entity;

		entity.addComponent<TransformComponent>();

		const bool name_empty = p_name.empty();
		entity.addComponent<TagComponent>(name_empty ? fmt::format("ヌル　エンチチ ({})", m_newEntityTagCount) : p_name);

		if (name_empty)
			++m_newEntityTagCount; // E.g. "New Entity (1)"

		return entity;
	}

	auto Scene::destroyEntity(Entity p_entity) -> void
	{
		m_entityUUIDMap.erase(p_entity.getComponent<UUIDComponent>().uuid);
		m_registry.destroy(p_entity);
	}

	auto Scene::getEntityByUUID(UUID p_uuid) -> Entity
	{
		if (!m_entityUUIDMap.contains(p_uuid))
			return {};
		return {m_entityUUIDMap.at(p_uuid), this};
	}

	auto Scene::getEntityByName(const String &p_name) -> Entity
	{
		for (const auto view = m_registry.view<TagComponent>(); const auto &entity: view)
		{
			if (const auto &tag = view.get<TagComponent>(entity); tag.tag == p_name)
				return Entity{entity, this};
		}
		return {};
	}

	auto Scene::getEntityWorldTransformMatrix(Entity p_entity) const -> Dx::XMMATRIX
	{
		Dx::XMMATRIX transform{Dx::XMMatrixIdentity()};
		if (const Entity parent{p_entity.getParent()})
			transform = getEntityWorldTransformMatrix(parent);

		return Dx::XMMatrixMultiply(transform, p_entity.getTransform());
	}

	auto Scene::getEntityWorldTransformComponent(Entity p_entity) const -> TransformComponent
	{
		Dx::XMMATRIX       transform{getEntityWorldTransformMatrix(p_entity)};
		TransformComponent transform_component{};
		transform_component.setTransform(transform);
		return transform_component;
	}

	auto Scene::getMainCameraEntity() -> Entity
	{
		const auto view = m_registry.view<CameraComponent>();
		for (const auto entity: view)
		{
			auto &camera = view.get<CameraComponent>(entity);

			if (camera.primary)
				return Entity{entity, this};
		}
		return {};
	}

	auto Scene::getRegistry() -> entt::registry &
	{
		return m_registry;
	}

	auto Scene::getRegistry() const -> const entt::registry &
	{
		return m_registry;
	}

	auto Scene::setName(const String &p_name) -> void
	{
		m_name = p_name;
	}

	auto Scene::getName() const -> String
	{
		return m_name;
	}

	auto Scene::getLightEnvironment() const -> const SceneLightEnvironment &
	{
		return m_lightEnvironment;
	}

	auto Scene::getSceneEnvironment() const -> const gpu::Texture3DHandle &
	{
		return m_sceneEnvironment.skyboxMap;
	}

	auto Scene::setSceneEnvironment(const gpu::Texture3DHandle &p_environment) -> void
	{
		m_sceneEnvironment.skyboxMap            = p_environment;
		m_sceneEnvironment.diffuseIrradianceMap = m_renderCtx->createDiffuseIrradianceMap(p_environment);
		m_reloadEnvironment                     = true;
	}

	auto Scene::initNativeScripts() -> void
	{
		m_registry.view<NativeScriptComponent>().each([this](auto p_entity, auto &p_script) -> void
		{
			if (!p_script.instance)
			{
				p_script.instance           = p_script.instantiateFn();
				p_script.instance->m_entity = {p_entity, this};
				p_script.instance->_superInit();
				p_script.instance->onCreate(p_script.userData);
			}
		});
	}

	auto Scene::getScriptEngine() -> NonOwningPtr<script::ScriptEngine>
	{
		return m_scriptEngine;
	}

	auto Scene::getHasComponentFn(ComponentType p_component_type) -> HasComponentFn &
	{
		return m_hasComponentFnMap.at(p_component_type);
	}

	auto Scene::getAddComponentFn(ComponentType p_component_type) -> AddComponentFn &
	{
		return m_addComponentFnMap.at(p_component_type);
	}

	auto Scene::getResetComponentFn(ComponentType p_component_type) -> ResetComponentFn &
	{
		return m_resetComponentFnMap.at(p_component_type);
	}

	auto Scene::_registerScriptMethods() -> void
	{
		m_scriptEngine->registerMethod("Toaster.Entity::HasComponentInternal", +[](uint64 p_entity_id, MonoReflectionType *p_component_type) -> bool
		{
			MonoType *type{mono_reflection_type_get_type(p_component_type)};
			Entity    entity{s_activeScene->getEntityByUUID(p_entity_id)};
			return s_activeScene->getHasComponentFn(type)(&entity);
		});

		m_scriptEngine->registerMethod("Toaster.Entity::AddComponentInternal", +[](uint64 p_entity_id, MonoReflectionType *p_component_type) -> void
		{
			MonoType *type{mono_reflection_type_get_type(p_component_type)};
			Entity    entity{s_activeScene->getEntityByUUID(p_entity_id)};
			s_activeScene->getAddComponentFn(type)(&entity);
		});

		m_scriptEngine->registerMethod("Toaster.Entity::GetEntityByNameInternal", +[](MonoString *p_name) -> uint64
		{
			char * name_str{mono_string_to_utf8(p_name)};
			Entity entity{s_activeScene->getEntityByName(name_str)};
			mono_free(name_str);

			if (!entity)
				return 0;
			return entity.getComponent<UUIDComponent>().uuid;
		});

		m_scriptEngine->registerMethod("Toaster.Entity::GetScriptInstance", +[](uint64 p_entity_id) -> MonoObject *
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};

			auto instance{s_activeScene->m_entityScriptMap[entity.getComponent<UUIDComponent>().uuid]};
			return instance->getObject().getObject();
		});

		m_scriptEngine->registerMethod("Toaster.Entity::CreateEntityInternal", +[](MonoString *p_name) -> uint64
		{
			char * name_str{mono_string_to_utf8(p_name)};
			Entity entity{s_activeScene->createEntity(name_str)};
			mono_free(name_str);

			return entity.getComponent<UUIDComponent>().uuid;
		});

		m_scriptEngine->registerMethod("Toaster.Component::ResetInternal", +[](uint64 p_entity_id, MonoReflectionType *p_component_type) -> void
		{
			MonoType *type{mono_reflection_type_get_type(p_component_type)};
			Entity    entity{s_activeScene->getEntityByUUID(p_entity_id)};
			s_activeScene->getResetComponentFn(type)(&entity);
		});

		#pragma region Tag Component
		m_scriptEngine->registerMethod("Toaster.TagComponent::GetTag", +[](uint64 p_entity_id, MonoString **p_out_tag) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			*p_out_tag = mono_string_new(s_activeScene->getScriptEngine()->getAppDomain(), entity.getComponent<TagComponent>().tag.c_str());
		});

		m_scriptEngine->registerMethod("Toaster.TagComponent::SetTag", +[](uint64 p_entity_id, MonoString **p_tag) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};

			char *new_string{mono_string_to_utf8(*p_tag)};
			entity.getComponent<TagComponent>().tag = String{new_string};
			mono_free(new_string);
		});
		#pragma endregion

		#pragma region Transform Component
		m_scriptEngine->registerMethod("Toaster.TransformComponent::GetTranslation", +[](uint64 p_entity_id, Dx::XMFLOAT3 *p_out_translation) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			*p_out_translation = entity.getComponent<TransformComponent>().translation;
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::SetTranslation", +[](uint64 p_entity_id, Dx::XMFLOAT3 *p_translation) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			entity.getComponent<TransformComponent>().translation = *p_translation;
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::GetRotation", +[](uint64 p_entity_id, Dx::XMFLOAT4 *p_out_rotation) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			*p_out_rotation = entity.getOrientation();
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::SetRotation", +[](uint64 p_entity_id, Dx::XMFLOAT4 *p_rotation) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			entity.getComponent<TransformComponent>().orientation = *p_rotation;
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::GetScale", +[](uint64 p_entity_id, Dx::XMFLOAT3 *p_out_scale) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			*p_out_scale = entity.getComponent<TransformComponent>().scale;
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::SetScale", +[](uint64 p_entity_id, Dx::XMFLOAT3 *p_scale) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			entity.getComponent<TransformComponent>().scale = *p_scale;
		});
		#pragma endregion

		#pragma region Sprite Renderer Component
		m_scriptEngine->registerMethod("Toaster.SpriteRendererComponent::GetColour", +[](uint64 p_entity_id, tsm::float4 *p_out_colour) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			*p_out_colour = entity.getComponent<SpriteRendererComponent>().colour;
		});

		m_scriptEngine->registerMethod("Toaster.SpriteRendererComponent::SetColour", +[](uint64 p_entity_id, tsm::float4 *p_colour) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			entity.getComponent<SpriteRendererComponent>().colour = *p_colour;
		});
		#pragma endregion

		#pragma region Mesh Component
		m_scriptEngine->registerMethod("Toaster.MeshComponent::HasMaterialInternal", +[](uint64 p_entity_id, uint32 p_index) -> bool
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & mc{entity.getComponent<MeshComponent>()};
			return mc.mesh->getMaterials().hasMaterial(p_index);
		});
		#pragma endregion

		#pragma region Camera Component
		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetPrimary", +[](uint64 p_entity_id) -> bool
		{
			Entity      entity{s_activeScene->getEntityByUUID(p_entity_id)};
			const auto &cam{entity.getComponent<CameraComponent>()};
			return cam.primary;
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetPrimary", +[](uint64 p_entity_id, const bool *p_primary) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.primary = *p_primary;
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetProjectionType", +[](uint64 p_entity_id, SceneCamera::EProjectionType *p_out_projection_type) -> void
		{
			Entity      entity{s_activeScene->getEntityByUUID(p_entity_id)};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_projection_type = cam.camera.getProjectionType();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetProjectionType",
									   +[](uint64 p_entity_id, const SceneCamera::EProjectionType *p_projection_type) -> void
									   {
										   Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
										   auto & cam{entity.getComponent<CameraComponent>()};
										   cam.camera.setProjectionType(*p_projection_type);
									   });

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetPerspectiveFov", +[](uint64 p_entity_id, float32 *p_out_perspective_fov) -> void
		{
			Entity      entity{s_activeScene->getEntityByUUID(p_entity_id)};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_perspective_fov = cam.camera.getPerspectiveFov();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetPerspectiveFov", +[](uint64 p_entity_id, const float32 *p_perspective_fov) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setPerspectiveFov(*p_perspective_fov);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetPerspectiveNear", +[](uint64 p_entity_id, float32 *p_out_perspective_near) -> void
		{
			Entity      entity{s_activeScene->getEntityByUUID(p_entity_id)};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_perspective_near = cam.camera.getPerspectiveNearClip();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetPerspectiveNear", +[](uint64 p_entity_id, const float32 *p_perspective_near) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setPerspectiveNearClip(*p_perspective_near);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetPerspectiveFar", +[](uint64 p_entity_id, float32 *p_out_perspective_far) -> void
		{
			Entity      entity{s_activeScene->getEntityByUUID(p_entity_id)};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_perspective_far = cam.camera.getPerspectiveFarClip();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetPerspectiveFar", +[](uint64 p_entity_id, const float32 *p_perspective_far) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setPerspectiveFarClip(*p_perspective_far);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetOrthoSize", +[](uint64 p_entity_id, float32 *p_out_ortho_size) -> void
		{
			Entity      entity{s_activeScene->getEntityByUUID(p_entity_id)};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_ortho_size = cam.camera.getOrthoSize();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetOrthoSize", +[](uint64 p_entity_id, const float32 *p_ortho_size) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setOrthoSize(*p_ortho_size);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetOrthoNear", +[](uint64 p_entity_id, float32 *p_out_ortho_near) -> void
		{
			Entity      entity{s_activeScene->getEntityByUUID(p_entity_id)};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_ortho_near = cam.camera.getOrthoNearClip();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetOrthoNear", +[](uint64 p_entity_id, const float32 *p_ortho_near) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setOrthoNearClip(*p_ortho_near);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetOrthoFar", +[](uint64 p_entity_id, float32 *p_out_ortho_far) -> void
		{
			Entity      entity{s_activeScene->getEntityByUUID(p_entity_id)};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_ortho_far = cam.camera.getOrthoFarClip();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetOrthoFar", +[](uint64 p_entity_id, const float32 *p_ortho_far) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setOrthoFarClip(*p_ortho_far);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetProjectionMatrix", +[](uint64 p_entity_id, Dx::XMFLOAT4X4 *p_out_projection_matrix) -> void
		{
			Entity         entity{s_activeScene->getEntityByUUID(p_entity_id)};
			const auto &   cam{entity.getComponent<CameraComponent>()};
			Dx::XMFLOAT4X4 proj;
			Dx::XMStoreFloat4x4(&proj, cam.camera.getProjectionMatrix());
			*p_out_projection_matrix = proj;
		});
		#pragma endregion

		#pragma region Directional Light Component
		m_scriptEngine->registerMethod("Toaster.DirectionalLightComponent::GetRadiance", +[](uint64 p_entity_id, tsm::float3 *p_out_colour) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			*p_out_colour = entity.getComponent<DirectionalLightComponent>().radiance;
		});

		m_scriptEngine->registerMethod("Toaster.DirectionalLightComponent::SetRadiance", +[](uint64 p_entity_id, const tsm::float3 *p_colour) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			entity.getComponent<DirectionalLightComponent>().radiance = *p_colour;
		});

		m_scriptEngine->registerMethod("Toaster.DirectionalLightComponent::GetMultiplier", +[](uint64 p_entity_id, float32 *p_multiplier) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			*p_multiplier = entity.getComponent<DirectionalLightComponent>().multiplier;
		});

		m_scriptEngine->registerMethod("Toaster.DirectionalLightComponent::SetMultiplier", +[](uint64 p_entity_id, const float32 *p_multiplier) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			entity.getComponent<DirectionalLightComponent>().multiplier = *p_multiplier;
		});
		#pragma endregion

		#pragma region Point Light Component
		m_scriptEngine->registerMethod("Toaster.PointLightComponent::GetRadiance", +[](uint64 p_entity_id, tsm::float3 *p_out_colour) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			*p_out_colour = entity.getComponent<PointLightComponent>().radiance;
		});

		m_scriptEngine->registerMethod("Toaster.PointLightComponent::SetRadiance", +[](uint64 p_entity_id, const tsm::float3 *p_colour) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			entity.getComponent<PointLightComponent>().radiance = *p_colour;
		});

		m_scriptEngine->registerMethod("Toaster.PointLightComponent::GetMultiplier", +[](uint64 p_entity_id, float32 *p_multiplier) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			*p_multiplier = entity.getComponent<PointLightComponent>().multiplier;
		});

		m_scriptEngine->registerMethod("Toaster.PointLightComponent::SetMultiplier", +[](uint64 p_entity_id, const float32 *p_multiplier) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			entity.getComponent<PointLightComponent>().multiplier = *p_multiplier;
		});
		#pragma endregion

		#pragma region Material
		m_scriptEngine->registerMethod("Toaster.Material::GetAlbedoColour", +[](uint64 p_entity_id, uint32 p_index, tsm::float3 *p_out_colour) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & mc{entity.getComponent<MeshComponent>()};
			auto & material{mc.mesh->getMaterials().getMaterial(p_index).material};
			*p_out_colour = material->get<tsm::float3>("u_Material.albedoColour");
		});

		m_scriptEngine->registerMethod("Toaster.Material::SetAlbedoColour", +[](uint64 p_entity_id, uint32 p_index, tsm::float3 *p_colour) -> void
		{
			Entity entity{s_activeScene->getEntityByUUID(p_entity_id)};
			auto & mc{entity.getComponent<MeshComponent>()};
			auto & material{mc.mesh->getMaterials().getMaterial(p_index).material};
			material->set("u_Material.albedoColour", *p_colour);
		});
		#pragma endregion
	}

	template<typename Type>
	auto Scene::onComponentAdded([[maybe_unused]] Entity p_entity, [[maybe_unused]] Type &p_component) -> void
	{
		TST_PERMA_ASSERT(false);
	}

	#define ON_COMPONENT_ADDED(__type)	template<>\
											TST_SCENE_API auto Scene::onComponentAdded<__type>([[maybe_unused]] Entity p_entity, __type &p_component) -> void

	ON_COMPONENT_ADDED(UUIDComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(RelationshipComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(TagComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(TransformComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(SpriteRendererComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(MeshComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(SubmeshComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(CameraComponent)
	{
		(void) p_entity;
		p_component.camera.setViewportSize(m_viewportSize);
	}

	ON_COMPONENT_ADDED(NativeScriptComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(ScriptComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(DirectionalLightComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(PointLightComponent)
	{
		(void) p_entity;
		(void) p_component;
	}

	ON_COMPONENT_ADDED(SpotLightComponent)
	{
		(void) p_entity;
		(void) p_component;
	}
	#undef ON_COMPONENT_ADDED
}
