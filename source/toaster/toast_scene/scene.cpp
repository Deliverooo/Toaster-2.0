#include "scene.hpp"
#include "entity.hpp"
#include "components.hpp"

#include "toast_lib/logging.hpp"

namespace toaster
{
	Scene::Scene(gpu::VKGPUContext *p_ctx, const String &p_name) : m_ctx(p_ctx), m_name(p_name.empty() ? "Untitled Scene" : p_name)
	{
	}

	Scene::~Scene()
	{
	}

	void Scene::onUpdate(float32 p_dt)
	{
		m_registry.view<NativeScriptComponent>().each([this, p_dt](auto p_entity, auto &p_script)
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

	void Scene::onRender(vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, float32 p_dt, const RefPtr<Renderer2D> &p_renderer_2d)
	{
		Camera *  main_camera{nullptr};
		glm::mat4 camera_transform{1.0f};
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
		}

		#if 0
		if (main_camera)
		{
			p_renderer_2d->begin(*main_camera, camera_transform);

			auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity: group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				if (sprite.texture)
					p_renderer_2d->submitQuad(transform.getTransform(), sprite.texture, sprite.colour, sprite.tilingFactor);
				else
					p_renderer_2d->submitQuad(transform.getTransform(), sprite.colour);
			}

			p_renderer_2d->end();
		}
		#endif
	}

	void Scene::onRender(vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, float32 p_dt, const RefPtr<Renderer2D> &p_renderer_2d, const glm::mat4 &p_view,
						 const glm::mat4 &        p_projection)
	{
		// #if 0

		p_renderer_2d->begin(p_cmd, p_frame_index, p_view, p_projection);

		auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity: group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			// if (sprite.texture)
			// p_renderer_2d->submitQuad(transform.getTransform(), sprite.texture, sprite.colour, sprite.tilingFactor, static_cast<uint32>(entity));
			// else
			p_renderer_2d->submitQuad(transform.getTransform(), sprite.colour);
		}

		p_renderer_2d->end(p_cmd, p_frame_index);

		// #endif
	}

	void Scene::setViewportSize(uint32 p_width, uint32 p_height)
	{
		m_viewportWidth  = p_width;
		m_viewportHeight = p_height;

		auto view = m_registry.view<CameraComponent>();
		for (auto entity: view)
		{
			auto &cameraComponent = view.get<CameraComponent>(entity);
			cameraComponent.camera.setViewportSize(p_width, p_height);
		}
	}

	Entity Scene::createEntity(const String &p_name)
	{
		auto entity = Entity{m_registry.create(), this};

		entity.addComponent<TransformComponent>();

		const bool name_empty = p_name.empty();
		entity.addComponent<TagComponent>(name_empty ? "ヌル　エンチチ (" + std::to_string(m_newEntityTagCount) + ")" : p_name);

		if (name_empty)
			++m_newEntityTagCount; // E.g. "New Entity (1)"

		return entity;
	}

	void Scene::destroyEntity(Entity p_entity)
	{
		m_registry.destroy(p_entity);
	}

	Entity Scene::getMainCameraEntity()
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

	entt::registry &Scene::getRegistry()
	{
		return m_registry;
	}

	const entt::registry &Scene::getRegistry() const
	{
		return m_registry;
	}

	void Scene::setName(const String &p_name)
	{
		m_name = p_name;
	}

	String Scene::getName() const
	{
		return m_name;
	}

	template<typename Type>
	void Scene::onComponentAdded([[maybe_unused]] Entity p_entity, [[maybe_unused]] Type &p_component)
	{
		TST_ASSERT(false);
	}

	#define ON_COMPONENT_ADDED(__type)	template<>\
										void Scene::onComponentAdded<__type>([[maybe_unused]] Entity p_entity, __type &p_component)

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
	#undef ON_COMPONENT_ADDED
}
