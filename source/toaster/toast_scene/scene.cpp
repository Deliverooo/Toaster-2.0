#include "scene.hpp"
#include "entity.hpp"
#include "components.hpp"
#include "scene_renderer.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

#include "toast_lib/logging.hpp"
#include "toast_render/globals.hpp"

#define TST_ENABLE_2D_SCENE_RENDERING 1

namespace toaster
{
	class TST_API ScriptableEntityCS
	{
	public:
		ScriptableEntityCS(const script::Class &p_class, Scene *p_scene, Entity p_entity) : m_obj(p_class)
		{
			TST_ASSERT_MSG(m_obj.getClass().getScriptEngine(), "Class's script engine is null");

			m_onCreateMethod = m_obj.getClass().getMethod("OnCreate", 0);
			m_onUpdateMethod = m_obj.getClass().getMethod("OnUpdate", 1);
			m_obj.invoke(p_scene->m_baseEntityClass->getMethod(".ctor", 1), static_cast<uint32>(p_entity));
		}

		void onCreate()
		{
			m_obj.invoke(m_onCreateMethod);
		}

		void onUpdate(float32 p_dt)
		{
			m_obj.invoke(m_onUpdateMethod, p_dt);
		}

	private:
		script::Object m_obj;

		script::Method *m_onCreateMethod{nullptr};
		script::Method *m_onUpdateMethod{nullptr};
	};

	static Scene *s_activeScene{nullptr};

	Scene::Scene(gpu::VKLogicalDevice *p_device, script::ScriptEngine *p_script_engine, const String &p_name) : m_device(p_device), m_scriptEngine(p_script_engine),
																												m_name(p_name.empty() ? "Untitled Scene" : p_name)
	{
		s_activeScene = this;

		if (m_scriptEngine)
		{
			MonoImage *          image{m_scriptEngine->getAppImage()};
			const MonoTableInfo *type_definitions{mono_image_get_table_info(image, MONO_TABLE_TYPEDEF)};
			m_baseEntityClass = make_reference<script::Class>(m_scriptEngine, "Toaster", "Entity", script::EClassScope::eCore);
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
				LOG_INFO("Found Entity class: {} ", class_name);

				void *          iterator{nullptr};
				MonoClassField *field{nullptr};
				while ((field = mono_class_get_fields(klass->getClass(), &iterator)) != nullptr)
				{
					CString name{mono_field_get_name(field)};
					LOG_INFO("Field: {}", name);
				}
			}

			#define REGISTER_COMPONENT_TYPE(__type) {MonoType *managed_type{mono_reflection_type_from_name((char *) "Toaster."#__type, m_scriptEngine->getCoreImage())};\
													if(!managed_type) { LOG_FATAL("Could not find component: "#__type); TST_ASSERT(false);}\
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

				uint32 entity_id{static_cast<uint32>(entity)};
				Entity e{entity, this};
				if (!m_entityScriptMap.contains(entity_id))
				{
					if (m_entityClassMap.contains(class_name))
					{
						m_entityScriptMap[entity_id] = make_reference<ScriptableEntityCS>(*m_entityClassMap[class_name].get(), this, e);
						m_entityScriptMap[entity_id]->onCreate();
					}
					else
					{
						LOG_ERROR("Invalid script class name: {}", class_name);
					}
				}

				if (m_entityScriptMap.contains(entity_id))
					m_entityScriptMap[entity_id]->onUpdate(p_dt);
			}
		}

		m_registry.view<NativeScriptComponent>().each([this, p_dt](auto p_entity, auto &p_script) -> void
		{
			if (!p_script.instance)
			{
				p_script.instance           = p_script.instantiateFn();
				p_script.instance->m_entity = {p_entity, this};
				p_script.instance->onCreate();
			}

			p_script.instance->onUpdate(p_dt);
		});
	}

	auto Scene::onRender([[maybe_unused]] const vk::raii::CommandBuffer &p_cmd, [[maybe_unused]] uint32 p_frame_index, [[maybe_unused]] float32 p_dt,
						 [[maybe_unused]] const RefPtr<SceneRenderer> &  p_scene_renderer) -> void
	{
		Camera *  main_camera{nullptr};
		glm::mat4 camera_transform{1.0f};

		Entity main_camera_entity{getMainCameraEntity()};
		if (static_cast<entt::entity>(main_camera_entity) != entt::null)
		{
			main_camera      = &main_camera_entity.getComponent<CameraComponent>().camera;
			camera_transform = main_camera_entity.getComponent<TransformComponent>().getTransform();
		}

		if (!main_camera)
		{
			LOG_WARN("Scene has no main camera!");
			return;
		}

		m_lightEnvironment.pointLights.clear();
		m_lightEnvironment.directionalLights.clear();

		{
			for (const auto group{m_registry.group<TransformComponent>(entt::get<DirectionalLightComponent>)}; const auto entity: group)
			{
				auto [transform, directional_light]{group.get<TransformComponent, DirectionalLightComponent>(entity)};

				glm::mat4 rotation{glm::eulerAngleXYZ(transform.rotation.x, transform.rotation.y, transform.rotation.z)};
				glm::vec4 forward{rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)};
				glm::vec3 direction{glm::normalize(glm::vec3(forward))};
				m_lightEnvironment.directionalLights.emplace_back(DirectionalLight{
																	  glm::vec4(direction, 1.0f),
																	  glm::vec4(directional_light.radiance, directional_light.multiplier)
																  });
			}
		}
		{
			for (const auto view{m_registry.view<TransformComponent, PointLightComponent>()}; const auto entity: view)
			{
				auto [transform, point_light]{view.get<TransformComponent, PointLightComponent>(entity)};

				m_lightEnvironment.pointLights.emplace_back(PointLight{glm::vec4(transform.translation, 1.0f), glm::vec4(point_light.radiance, point_light.multiplier)});
			}
		}

		glm::mat4 camera_view{glm::inverse(camera_transform)};
		{
			p_scene_renderer->begin(p_cmd, p_frame_index, camera_view, main_camera->getProjectionMatrix());
			for (const auto view{m_registry.view<TransformComponent, MeshComponent>()}; const auto entity: view)
			{
				if (auto [transform, mesh]{view.get<TransformComponent, MeshComponent>(entity)}; mesh.mesh)
				{
					p_scene_renderer->renderMesh(mesh.mesh, transform.getTransform());
				}
			}
			p_scene_renderer->end(p_cmd, p_frame_index);
		}
		#if TST_ENABLE_2D_SCENE_RENDERING
		{
			auto renderer_2d{p_scene_renderer->getRenderer2D()};
			renderer_2d->begin(p_cmd, p_frame_index, camera_view, main_camera->getProjectionMatrix());

			for (const auto view{m_registry.view<TransformComponent, SpriteRendererComponent>()}; const auto entity: view)
			{
				auto [transform, src]{view.get<TransformComponent, SpriteRendererComponent>(entity)};
				if (src.texture)
					renderer_2d->submitQuad(transform.getTransform(), src.texture, src.colour);
				else
					renderer_2d->submitQuad(transform.getTransform(), src.colour);
			}

			gpu::RenderingAttachmentInfo colour_attachment_info{};
			colour_attachment_info.clearValue = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f};
			colour_attachment_info.image      = p_scene_renderer->getOutputColourTexture()->getImage();
			colour_attachment_info.loadOp     = vk::AttachmentLoadOp::eNone;
			colour_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;

			gpu::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue = vk::ClearDepthStencilValue{1.0f, 0u};
			depth_attachment_info.image      = p_scene_renderer->getOutputDepthTexture()->getImage();
			depth_attachment_info.loadOp     = vk::AttachmentLoadOp::eLoad;
			depth_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;

			renderer_2d->end(p_cmd, p_frame_index, &colour_attachment_info, &depth_attachment_info);
		}
		#endif
	}

	auto Scene::onRender(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, [[maybe_unused]] float32 p_dt, const RefPtr<SceneRenderer> &p_scene_renderer,
						 const glm::mat4 &              p_view, const glm::mat4 &p_projection) -> void
	{
		m_lightEnvironment.pointLights.clear();
		m_lightEnvironment.directionalLights.clear();

		{
			for (const auto group{m_registry.group<TransformComponent>(entt::get<DirectionalLightComponent>)}; const auto entity: group)
			{
				auto [transform, directional_light]{group.get<TransformComponent, DirectionalLightComponent>(entity)};

				glm::mat4 rotation{glm::eulerAngleXYZ(transform.rotation.x, transform.rotation.y, transform.rotation.z)};
				glm::vec4 forward{rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)};
				glm::vec3 direction{glm::normalize(glm::vec3(forward))};
				m_lightEnvironment.directionalLights.emplace_back(DirectionalLight{
																	  glm::vec4(direction, 1.0f),
																	  glm::vec4(directional_light.radiance, directional_light.multiplier)
																  });
			}
		}
		{
			for (const auto view{m_registry.view<TransformComponent, PointLightComponent>()}; const auto entity: view)
			{
				auto [transform, point_light]{view.get<TransformComponent, PointLightComponent>(entity)};

				m_lightEnvironment.pointLights.emplace_back(PointLight{glm::vec4(transform.translation, 1.0f), glm::vec4(point_light.radiance, point_light.multiplier)});
			}
		}
		{
			p_scene_renderer->begin(p_cmd, p_frame_index, p_view, p_projection);
			for (const auto view{m_registry.view<TransformComponent, MeshComponent>()}; const auto entity: view)
			{
				if (auto [transform, mesh]{view.get<TransformComponent, MeshComponent>(entity)}; mesh.mesh)
				{
					p_scene_renderer->renderMesh(mesh.mesh, transform.getTransform());
				}
			}
			p_scene_renderer->end(p_cmd, p_frame_index);
		}
		#if TST_ENABLE_2D_SCENE_RENDERING
		{
			auto renderer_2d{p_scene_renderer->getRenderer2D()};
			renderer_2d->begin(p_cmd, p_frame_index, p_view, p_projection);

			for (const auto view{m_registry.view<TransformComponent, SpriteRendererComponent>()}; const auto entity: view)
			{
				auto [transform, src]{view.get<TransformComponent, SpriteRendererComponent>(entity)};
				if (src.texture)
					renderer_2d->submitQuad(transform.getTransform(), src.texture, src.colour);
				else
					renderer_2d->submitQuad(transform.getTransform(), src.colour);
			}

			gpu::RenderingAttachmentInfo colour_attachment_info{};
			colour_attachment_info.clearValue = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f};
			colour_attachment_info.image      = p_scene_renderer->getOutputColourTexture()->getImage();
			colour_attachment_info.loadOp     = vk::AttachmentLoadOp::eNone;
			colour_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;

			gpu::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue = vk::ClearDepthStencilValue{1.0f, 0u};
			depth_attachment_info.image      = p_scene_renderer->getOutputDepthTexture()->getImage();
			depth_attachment_info.loadOp     = vk::AttachmentLoadOp::eLoad;
			depth_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;

			renderer_2d->end(p_cmd, p_frame_index, &colour_attachment_info, &depth_attachment_info);
		}
		#endif
	}

	auto Scene::setViewportSize(uint32 p_width, uint32 p_height) -> void
	{
		m_viewportWidth  = p_width;
		m_viewportHeight = p_height;

		auto view{m_registry.view<CameraComponent>()};
		for (auto entity: view)
		{
			auto &cameraComponent{view.get<CameraComponent>(entity)};
			cameraComponent.camera.setViewportSize(p_width, p_height);
		}
	}

	auto Scene::createEntity(const String &p_name) -> Entity
	{
		auto entity{Entity{m_registry.create(), this}};

		entity.addComponent<TransformComponent>();

		const bool name_empty = p_name.empty();
		entity.addComponent<TagComponent>(name_empty ? fmt::format("ヌル　エンチチ ({})", m_newEntityTagCount) : p_name);

		if (name_empty)
			++m_newEntityTagCount; // E.g. "New Entity (1)"

		return entity;
	}

	auto Scene::destroyEntity(Entity p_entity) -> void
	{
		m_registry.destroy(p_entity);
	}

	auto Scene::getMainCameraEntity() -> Entity
	{
		for (const auto view = m_registry.view<CameraComponent>(); const auto &entity: view)
		{
			if (const auto &camera = view.get<CameraComponent>(entity); camera.primary)
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
		m_scriptEngine->registerMethod("Toaster.InternalCalls::HasComponent", +[](uint32 p_entity_id, MonoReflectionType *p_component_type) -> bool
		{
			MonoType *type{mono_reflection_type_get_type(p_component_type)};
			Entity    entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			return s_activeScene->getHasComponentFn(type)(&entity);
		});

		m_scriptEngine->registerMethod("Toaster.InternalCalls::AddComponent", +[](uint32 p_entity_id, MonoReflectionType *p_component_type) -> void
		{
			MonoType *type{mono_reflection_type_get_type(p_component_type)};
			Entity    entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			s_activeScene->getAddComponentFn(type)(&entity);
		});

		m_scriptEngine->registerMethod("Toaster.Component::ResetInternal", +[](uint32 p_entity_id, MonoReflectionType *p_component_type) -> void
		{
			MonoType *type{mono_reflection_type_get_type(p_component_type)};
			Entity    entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			s_activeScene->getResetComponentFn(type)(&entity);
		});

		#pragma region Tag Component
		m_scriptEngine->registerMethod("Toaster.TagComponent::GetTag", +[](uint32 p_entity_id, MonoString **p_out_tag) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			*p_out_tag = mono_string_new(s_activeScene->getScriptEngine()->getAppDomain(), entity.getComponent<TagComponent>().tag.c_str());
		});

		m_scriptEngine->registerMethod("Toaster.TagComponent::SetTag", +[](uint32 p_entity_id, MonoString **p_tag) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};

			char *new_string{mono_string_to_utf8(*p_tag)};
			entity.getComponent<TagComponent>().tag = String{new_string};
			mono_free(new_string);
		});
		#pragma endregion

		#pragma region Transform Component
		m_scriptEngine->registerMethod("Toaster.TransformComponent::GetTranslation", +[](uint32 p_entity_id, glm::vec3 *p_out_translation) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			*p_out_translation = entity.getComponent<TransformComponent>().translation;
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::SetTranslation", +[](uint32 p_entity_id, const glm::vec3 *p_translation) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			entity.getComponent<TransformComponent>().translation = *p_translation;
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::GetRotation", +[](uint32 p_entity_id, glm::vec4 *p_out_rotation) -> void
		{
			Entity           entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const glm::quat &rotation{entity.getComponent<TransformComponent>().rotation};
			*p_out_rotation = glm::vec4{rotation.x, rotation.y, rotation.z, rotation.w};
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::SetRotation", +[](uint32 p_entity_id, const glm::vec4 *p_rotation) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			entity.getComponent<TransformComponent>().rotation = glm::quat{p_rotation->w, p_rotation->x, p_rotation->y, p_rotation->z};
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::GetScale", +[](uint32 p_entity_id, glm::vec3 *p_out_scale) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			*p_out_scale = entity.getComponent<TransformComponent>().scale;
		});

		m_scriptEngine->registerMethod("Toaster.TransformComponent::SetScale", +[](uint32 p_entity_id, const glm::vec3 *p_scale) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			entity.getComponent<TransformComponent>().scale = *p_scale;
		});
		#pragma endregion

		#pragma region Sprite Renderer Component
		m_scriptEngine->registerMethod("Toaster.SpriteRendererComponent::GetColour", +[](uint32 p_entity_id, glm::vec4 *p_out_colour) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			*p_out_colour = entity.getComponent<SpriteRendererComponent>().colour;
		});

		m_scriptEngine->registerMethod("Toaster.SpriteRendererComponent::SetColour", +[](uint32 p_entity_id, const glm::vec4 *p_colour) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			entity.getComponent<SpriteRendererComponent>().colour = *p_colour;
		});
		#pragma endregion

		#pragma region Mesh Component
		m_scriptEngine->registerMethod("Toaster.MeshComponent::HasMaterialInternal", +[](uint32 p_entity_id, uint32 p_index) -> bool
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & mesh{entity.getComponent<MeshComponent>()};
			return mesh.mesh->getMaterials().size() > p_index;
		});
		#pragma endregion

		#pragma region Camera Component
		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetPrimary", +[](uint32 p_entity_id) -> bool
		{
			Entity      entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const auto &cam{entity.getComponent<CameraComponent>()};
			return cam.primary;
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetPrimary", +[](uint32 p_entity_id, const bool *p_primary) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.primary = *p_primary;
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetProjectionType", +[](uint32 p_entity_id, SceneCamera::EProjectionType *p_out_projection_type) -> void
		{
			Entity      entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_projection_type = cam.camera.getProjectionType();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetProjectionType",
									   +[](uint32 p_entity_id, const SceneCamera::EProjectionType *p_projection_type) -> void
									   {
										   Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
										   auto & cam{entity.getComponent<CameraComponent>()};
										   cam.camera.setProjectionType(*p_projection_type);
									   });

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetPerspectiveFov", +[](uint32 p_entity_id, float32 *p_out_perspective_fov) -> void
		{
			Entity      entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_perspective_fov = cam.camera.getPerspectiveFov();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetPerspectiveFov", +[](uint32 p_entity_id, const float32 *p_perspective_fov) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setPerspectiveFov(*p_perspective_fov);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetPerspectiveNear", +[](uint32 p_entity_id, float32 *p_out_perspective_near) -> void
		{
			Entity      entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_perspective_near = cam.camera.getPerspectiveNearClip();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetPerspectiveNear", +[](uint32 p_entity_id, const float32 *p_perspective_near) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setPerspectiveNearClip(*p_perspective_near);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetPerspectiveFar", +[](uint32 p_entity_id, float32 *p_out_perspective_far) -> void
		{
			Entity      entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_perspective_far = cam.camera.getPerspectiveFarClip();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetPerspectiveFar", +[](uint32 p_entity_id, const float32 *p_perspective_far) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setPerspectiveFarClip(*p_perspective_far);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetOrthoSize", +[](uint32 p_entity_id, float32 *p_out_ortho_size) -> void
		{
			Entity      entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_ortho_size = cam.camera.getOrthoSize();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetOrthoSize", +[](uint32 p_entity_id, const float32 *p_ortho_size) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setOrthoSize(*p_ortho_size);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetOrthoNear", +[](uint32 p_entity_id, float32 *p_out_ortho_near) -> void
		{
			Entity      entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_ortho_near = cam.camera.getOrthoNearClip();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetOrthoNear", +[](uint32 p_entity_id, const float32 *p_ortho_near) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setOrthoNearClip(*p_ortho_near);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetOrthoFar", +[](uint32 p_entity_id, float32 *p_out_ortho_far) -> void
		{
			Entity      entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_ortho_far = cam.camera.getOrthoFarClip();
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::SetOrthoFar", +[](uint32 p_entity_id, const float32 *p_ortho_far) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & cam{entity.getComponent<CameraComponent>()};
			cam.camera.setOrthoFarClip(*p_ortho_far);
		});

		m_scriptEngine->registerMethod("Toaster.CameraComponent::GetProjectionMatrix", +[](uint32 p_entity_id, glm::mat4 *p_out_projection_matrix) -> void
		{
			Entity      entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			const auto &cam{entity.getComponent<CameraComponent>()};
			*p_out_projection_matrix = glm::transpose(cam.camera.getProjectionMatrix());
		});
		#pragma endregion

		#pragma region Directional Light Component
		m_scriptEngine->registerMethod("Toaster.DirectionalLightComponent::GetRadiance", +[](uint32 p_entity_id, glm::vec3 *p_out_colour) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			*p_out_colour = entity.getComponent<DirectionalLightComponent>().radiance;
		});

		m_scriptEngine->registerMethod("Toaster.DirectionalLightComponent::SetRadiance", +[](uint32 p_entity_id, const glm::vec3 *p_colour) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			entity.getComponent<DirectionalLightComponent>().radiance = *p_colour;
		});

		m_scriptEngine->registerMethod("Toaster.DirectionalLightComponent::GetMultiplier", +[](uint32 p_entity_id, float32 *p_multiplier) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			*p_multiplier = entity.getComponent<DirectionalLightComponent>().multiplier;
		});

		m_scriptEngine->registerMethod("Toaster.DirectionalLightComponent::SetMultiplier", +[](uint32 p_entity_id, const float32 *p_multiplier) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			entity.getComponent<DirectionalLightComponent>().multiplier = *p_multiplier;
		});
		#pragma endregion

		#pragma region Point Light Component
		m_scriptEngine->registerMethod("Toaster.PointLightComponent::GetRadiance", +[](uint32 p_entity_id, glm::vec3 *p_out_colour) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			*p_out_colour = entity.getComponent<PointLightComponent>().radiance;
		});

		m_scriptEngine->registerMethod("Toaster.PointLightComponent::SetRadiance", +[](uint32 p_entity_id, const glm::vec3 *p_colour) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			entity.getComponent<PointLightComponent>().radiance = *p_colour;
		});

		m_scriptEngine->registerMethod("Toaster.PointLightComponent::GetMultiplier", +[](uint32 p_entity_id, float32 *p_multiplier) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			*p_multiplier = entity.getComponent<PointLightComponent>().multiplier;
		});

		m_scriptEngine->registerMethod("Toaster.PointLightComponent::SetMultiplier", +[](uint32 p_entity_id, const float32 *p_multiplier) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			entity.getComponent<PointLightComponent>().multiplier = *p_multiplier;
		});
		#pragma endregion

		#pragma region Material
		m_scriptEngine->registerMethod("Toaster.Material::GetAlbedoColour", +[](uint32 p_entity_id, uint32 p_index, glm::vec3 *p_out_colour) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & mesh{entity.getComponent<MeshComponent>()};
			auto & material{mesh.mesh->getMaterials().at(p_index)};
			*p_out_colour = material->get<glm::vec3>("u_Material.albedoColour");
		});

		m_scriptEngine->registerMethod("Toaster.Material::SetAlbedoColour", +[](uint32 p_entity_id, uint32 p_index, glm::vec3 *p_colour) -> void
		{
			Entity entity{static_cast<entt::entity>(p_entity_id), s_activeScene};
			auto & mesh{entity.getComponent<MeshComponent>()};
			auto & material{mesh.mesh->getMaterials().at(p_index)};
			material->set("u_Material.albedoColour", *p_colour);
		});
		#pragma endregion
	}

	template<typename Type>
	auto Scene::onComponentAdded([[maybe_unused]] Entity p_entity, [[maybe_unused]] Type &p_component) -> void
	{
		TST_ASSERT(false);
	}

	#define ON_COMPONENT_ADDED(__type)	template<>\
											TST_API auto Scene::onComponentAdded<__type>([[maybe_unused]] Entity p_entity, __type &p_component) -> void

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

	ON_COMPONENT_ADDED(CameraComponent)
	{
		(void) p_entity;
		p_component.camera.setViewportSize(m_viewportWidth, m_viewportHeight);
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
