#include "runtime_layer.hpp"

#include "toast_kernel/input.hpp"
#include "toast_lib/io/file_stream.hpp"
#include "toast_render/globals.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_kernel/application.hpp"
#include "toast_lib/os/terminal.hpp"
#include "toast_render/render_context.hpp"
#include "toast_scene/components.hpp"
#include "toast_scene/entity.hpp"
#include "toast_scene/nsc_templates.hpp"
#include "toast_scene/scene_serializer.hpp"
#include "toast_scene/scriptable_entity.hpp"

namespace toaster
{
	#if 0
	class CameraController : public ScriptableEntity
	{
	public:
		static constexpr tsm::float3 c_forwardDir{0.0f, 0.0f, -1.0f};
		static constexpr tsm::float3 c_rightDir{1.0f, 0.0f, 0.0f};
		static constexpr tsm::float3 c_upDir{0.0f, 1.0f, 0.0f};

		virtual auto onCreate(void *p_user_data) -> void override
		{
			m_inputCtx = static_cast<InputContext *>(p_user_data);

			transform->translation = {0.0f, 1.5f, 2.5f};

			m_camera = &addComponent<CameraComponent>();
			m_camera->camera.setProjectionType(SceneCamera::EProjectionType::ePerspective);
			m_camera->camera.setPerspectiveFov(tsm::radians(90.0f));
			m_camera->primary = true;
		}

		virtual auto onUpdate(float32 p_dt) -> void override
		{
			if (m_inputCtx->isMouseButtonDown(input::EMouseButton::eRight))
			{
				if (m_inputCtx->getCursorMode() != input::ECursorMode::eDisabled)
					m_inputCtx->setCursorMode(input::ECursorMode::eDisabled);

				float fov = m_camera->camera.getPerspectiveFov();
				fov       -= tsm::radians(m_inputCtx->getMouseScrollY());
				m_camera->camera.setPerspectiveFov(fov);

				float32 speed = m_inputCtx->isKeyDown(input::EKeyCode::eLeftControl) ? 30.0f : 10.0f;

				tsm::float3 delta_position{tsm::float3::zero};
				if (m_inputCtx->isKeyDown(input::EKeyCode::eW))
					delta_position += c_forwardDir;
				if (m_inputCtx->isKeyDown(input::EKeyCode::eA))
					delta_position -= c_rightDir;
				if (m_inputCtx->isKeyDown(input::EKeyCode::eS))
					delta_position -= c_forwardDir;
				if (m_inputCtx->isKeyDown(input::EKeyCode::eD))
					delta_position += c_rightDir;

				delta_position = (delta_position.length() == 0.0f) ? tsm::float3::zero : tsm::normalize(delta_position) * p_dt;

				if (m_inputCtx->isKeyPressed(input::EKeyCode::eL))
				{
					if (m_slow)
						m_slow = false;
					else
						m_slow = true;
				}

				speed *= m_slow ? 0.3f : 1.0f;

				tsm::quatf  orientation      = tsm::fromYawPitchRoll(-m_yaw, -m_pitch, 0.0f);
				tsm::float4 rotated_position = tsm::float4{delta_position, 0.0f} * tsm::toMat4(orientation);
				transform->translation       += tsm::float3{rotated_position} * speed;

				LOG_WARN("{}", transform->translation);

				if (m_inputCtx->isKeyDown(input::EKeyCode::eSpace))
					transform->translation += c_upDir * p_dt * speed;
				if (m_inputCtx->isKeyDown(input::EKeyCode::eLeftShift))
					transform->translation -= c_upDir * p_dt * speed;

				tsm::float2 mouse       = {m_inputCtx->getMouseX(), m_inputCtx->getMouseY()};
				tsm::float2 mouse_delta = (mouse - m_initialMousePos) * p_dt;
				m_initialMousePos       = mouse;

				m_yaw   += mouse_delta.x;
				m_pitch += mouse_delta.y;
				if (m_pitch > tsm::radians(89.0f))
					m_pitch = tsm::radians(89.0f);
				if (m_pitch < tsm::radians(-89.0f))
					m_pitch = tsm::radians(-89.0f);

				transform->orientation = orientation;
			}
			else
			{
				if (m_inputCtx->getCursorMode() != input::ECursorMode::eNormal)
					m_inputCtx->setCursorMode(input::ECursorMode::eNormal);

				m_initialMousePos = {m_inputCtx->getMousePos().first, m_inputCtx->getMousePos().second};
			}
		}

	private:
		CameraComponent *m_camera{nullptr};

		NonOwningPtr<InputContext> m_inputCtx{nullptr};

		float32 m_yaw{0.0f};
		float32 m_pitch{0.0f};

		bool32 m_slow{false};

		tsm::float2 m_initialMousePos{0.0f};
	};
	#endif
	auto RuntimeLayer::onInit() -> void
	{
		m_camera = FPCamera{m_inputCtx, 90.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f};
		m_camera.setViewportSize(m_viewportSize.x, m_viewportSize.y);

		auto swapchain{m_app->getWindow().getSwapchain()};

		io::filesystem::Path binary_dir{os::getBinaryDirectory()};

		script::ScriptEngineSpecInfo sesi{};
		sesi.coreAssemblyPath = "C:/dev/Toaster-2.0/examples/Sandbox/bin/Debug/net48/Toaster.dll";
		sesi.appAssemblyPath  = "C:/dev/Toaster-2.0/examples/Sandbox/bin/Debug/net48/Sandbox.dll";
		m_scriptEngine        = make_unique<script::ScriptEngine>(sesi);
		m_inputCtx->registerScriptMethods(m_scriptEngine.get());

		// m_scene               = m_app->createScene("Runtime Scene Test");
		m_scene = make_unique<Scene>(m_renderCtx, m_scriptEngine.get(), "Runtime Test Scene");
		m_scene->setSceneEnvironment(m_renderCtx->createEnvironmentMap("C:/dev/Toaster-2.0/resources/environments/grasslands_sunset_1k.hdr"));
		m_sceneRenderer = m_app->createSceneRenderer(m_scene.get());

		gpu::PipelineSpecInfo fullscreen_pipeline_spec_info{};
		fullscreen_pipeline_spec_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
		fullscreen_pipeline_spec_info.shader             = m_globals->shaderLibrary().get("Composite");
		fullscreen_pipeline_spec_info.cullMode           = vk::CullModeFlagBits::eBack;
		fullscreen_pipeline_spec_info.vertexBufferLayout = render::RenderContext::fullscreenQuadVbl;

		m_fullscreenPipeline   = m_renderCtx->createGPURef<gpu::Pipeline>(fullscreen_pipeline_spec_info);
		m_fullscreenRenderPass = m_renderCtx->createRef<render::RenderPass>(m_fullscreenPipeline);

		m_fullscreenRenderPass->setInput("u_Texture", m_sceneRenderer->getColourTexture()).bake();

		{
			Entity e{m_scene->createEntity("Peeb")};
			e.addComponent<SpriteRendererComponent>();
		}

		{
			Entity e{m_scene->createEntity("Skib")};
			e.addComponent<MeshComponent>().mesh = m_renderCtx->createRef<render::MeshData>("C:/dev/Toaster-2.0/resources/meshes/DJT_sculpt.fbx");
			auto &tc{e.getComponent<TransformComponent>()};
		}
	}

	auto RuntimeLayer::onUpdate(const float32 p_dt) -> void
	{
		m_camera.onUpdate(p_dt);
		m_scene->onUpdate(p_dt);

		Dx::XMMATRIX projection{Dx::XMMatrixPerspectiveFovLH(Dx::XMConvertToRadians(90.0f), m_viewportSize.aspect(), 0.1f, 1000.0f)};
		Dx::XMMATRIX view{
			Dx::XMMatrixLookAtLH(m_camera.getPosition(), Dx::XMVectorAdd(m_camera.getPosition(), m_camera.getForwardDirection()),
								 Dx::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f))
		};

		m_sceneRenderer->onRender(view, projection);

		const auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo({1.0f, 1.0f, 1.0f, 1.0f}, false)};
		const auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};

		m_renderCtx->beginRendering(cmd, rendering_info, m_fullscreenRenderPass);
		m_renderCtx->renderFullscreenQuad(cmd, m_fullscreenRenderPass, nullptr);
		m_renderCtx->endRendering(cmd, rendering_info);
	}

	auto RuntimeLayer::onResize(tsm::uint2 p_size) -> void
	{
		m_viewportSize = p_size;

		m_camera.setViewportSize(p_size.x, p_size.y);
		m_scene->onResize(p_size);
		m_sceneRenderer->onResize(p_size);
	}

	auto RuntimeLayer::onEvent(Event &p_event) -> void
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(RuntimeLayer::_onKeyPressEvent));

		m_scene->onEvent(p_event);
		m_camera.onEvent(p_event);
	}

	auto RuntimeLayer::_onKeyPressEvent(KeyPressEvent &e) -> bool
	{
		auto &window{m_app->getWindow()};
		if (e.getKeyCode() == input::EKeyCode::eF11)
		{
			if (!window.isFullscreen())
				window.setFullscreen();
			else
				window.setWindowed();
		}

		return false;
	}
}
