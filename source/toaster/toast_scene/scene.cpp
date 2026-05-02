#include "scene.hpp"
#include "entity.hpp"
#include "components.hpp"
#include "scene_renderer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

#include "toast_lib/logging.hpp"
#include "toast_render/globals.hpp"

#define TST_ENABLE_2D_SCENE_RENDERING 1

namespace toaster
{
	class TST_API ScriptableEntityCS
	{
	public:
		ScriptableEntityCS(script::Class *p_class, Scene *p_scene, Entity p_entity) : m_obj(p_class)
		{
			TST_ASSERT_MSG(m_obj.getClass()->getScriptEngine(), "Class's script engine is null");

			m_onCreateMethod = m_obj.getClass()->getMethod("OnCreate", 0);
			m_onUpdateMethod = m_obj.getClass()->getMethod("OnUpdate", 1);
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

	Scene::Scene(gpu::VKLogicalDevice *p_device, script::ScriptEngine *p_script_engine, const String &p_name) : m_device(p_device), m_scriptEngine(p_script_engine),
																												m_name(p_name.empty() ? "Untitled Scene" : p_name)
	{
		if (m_scriptEngine)
		{
			MonoImage *          image{m_scriptEngine->getImage()};
			const MonoTableInfo *type_definitions{mono_image_get_table_info(image, MONO_TABLE_TYPEDEF)};
			m_baseEntityClass = make_reference<script::Class>(m_scriptEngine, "Toaster", "Entity");
			for (uint32 row{0u}; row < mono_table_info_get_rows(type_definitions); ++row)
			{
				uint32 cols[MONO_TYPEDEF_SIZE]{};
				mono_metadata_decode_row(type_definitions, row, cols, MONO_TYPEDEF_SIZE);

				auto name_space{mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE])};
				auto type_name{mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME])};

				MonoClass *script_class{mono_class_from_name(image, name_space, type_name)};
				if (script_class && mono_class_is_subclass_of(script_class, m_baseEntityClass->getClass(), false))
				{
					String full_name{fmt::format("{}.{}", name_space, type_name)};
					LOG_ERROR("Is entity: {}", full_name);
					m_entityClassMap[full_name] = toaster::make_reference<script::Class>(m_scriptEngine, script_class);
				}
			}

			for (auto &[class_name, class_]: m_entityClassMap)
			{
				LOG_INFO("Class: {} ", class_name);
			}
		}
	}

	Scene::~Scene()
	{
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
						m_entityScriptMap[entity_id] = make_reference<ScriptableEntityCS>(m_entityClassMap[class_name].get(), this, e);
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
		#if 0
		Camera *main_camera{nullptr}; glm::mat4 camera_transform{1.0f};
		{
			auto view = m_registry.view<TransformComponent, CameraComponent>();
			for (auto entity: view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
				if (camera.primary)
				{
					main_camera      = &camera.camera;
					camera_transform = transform.getTransform();
					break;
				}
			}
		} if (main_camera)
		{
			p_scene_renderer->begin(p_cmd, p_frame_index, camera_transform, main_camera->getProjectionMatrix());

			auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity: group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				// if (sprite.texture)
				// p_scene_renderer->submitQuad(transform.getTransform(), sprite.texture, sprite.colour, sprite.tilingFactor);
				// else
				// p_scene_renderer->submitQuad(transform.getTransform(), sprite.colour);
			}

			p_scene_renderer->end(p_cmd, p_frame_index);
		}
		else
			TST_ASSERT(false);
		#endif
	}

	auto Scene::onRender(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, [[maybe_unused]] float32 p_dt, const RefPtr<SceneRenderer> &p_scene_renderer,
						 const glm::mat4 &              p_view, const glm::mat4 &p_projection) -> void
	{
		m_lightEnvironment.pointLights.clear();

		{
			for (const auto group{m_registry.group<TransformComponent>(entt::get<DirectionalLightComponent>)}; const auto entity: group)
			{
				auto [transform, directional_light]{group.get<TransformComponent, DirectionalLightComponent>(entity)};

				m_lightEnvironment.directionalLights.emplace_back(DirectionalLight{
																	  glm::vec4(glm::normalize(transform.rotation), 1.0f),
																	  glm::vec4(directional_light.radiance, directional_light.multiplier)
																  });
			}
		}
		{
			for (const auto view{m_registry.view<TransformComponent, PointLightComponent>()}; const auto entity: view)
			{
				auto [transform, point_light]{view.get<TransformComponent, PointLightComponent>(entity)};

				m_lightEnvironment.pointLights.emplace_back(PointLight{
																glm::vec4(transform.translation, 1.0f),
																glm::vec4(point_light.radiance, point_light.multiplier),
																point_light.radius,
																point_light.falloff
															});
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
		auto entity = Entity{m_registry.create(), this};

		entity.addComponent<TransformComponent>();

		const bool name_empty = p_name.empty();
		entity.addComponent<TagComponent>(name_empty ? "ヌル　エンチチ (" + std::to_string(m_newEntityTagCount) + ")" : p_name);

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
		auto view = m_registry.view<CameraComponent>();
		for (auto entity: view)
		{
			const auto &camera = view.get<CameraComponent>(entity);
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
