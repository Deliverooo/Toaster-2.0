#include "scene.hpp"
#include "entity.hpp"
#include "components.hpp"

#include "toaster/toast_lib/logging.hpp"

namespace toaster
{
	Scene::Scene()
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

	void Scene::onRender(const RefPtr<Renderer2D> &p_renderer_2d, float32 p_dt)
	{
		Camera *   main_camera{nullptr};
		glm::mat4 *camera_transform{nullptr};
		{
			auto view = m_registry.view<TransformComponent, CameraComponent>();
			for (auto entity: view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
				if (camera.primary)
				{
					main_camera      = &camera.camera;
					camera_transform = &transform.transform;
					break;
				}
			}
		}

		if (main_camera)
		{
			p_renderer_2d->begin(*main_camera, *camera_transform);

			auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity: group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				if (sprite.texture)
					p_renderer_2d->submitQuad(transform.transform, sprite.texture, sprite.colour);
				else
					p_renderer_2d->submitQuad(transform.transform, sprite.colour);
			}

			p_renderer_2d->end();
		}
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

	Entity Scene::createEntity(const U8String &p_name)
	{
		auto entity = Entity{m_registry.create(), this};

		entity.addComponent<TransformComponent>();

		entity.addComponent<TagComponent>(p_name.empty() ? u8"ヌル　エンチチ" : p_name);

		return entity;
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
}
