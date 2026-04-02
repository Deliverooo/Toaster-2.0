#include "client_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"
#include "toast_gpu/gpu_context.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_lib/logging.hpp"

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void ClientLayer::onInit()
	{

		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		auto &instance = ctx->getVulkanInstance();

		auto physical_device = ctx->getPhysicalDevice();
		auto props           = physical_device.getProperties();
		LOG_INFO("Name: {}", props.deviceName.data());
		LOG_INFO("Type: {}", vk::to_string(props.deviceType));
	}

	void ClientLayer::onDestroy()
	{

	}

	void ClientLayer::onUpdate(const float32 p_dt)
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		ctx->drawFrame();
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::onKeyPressEvent));
	}

	bool ClientLayer::onKeyPressEvent(KeyPressEvent &e)
	{
		if (e.getKeyCode() == input::EKeyCode::eEscape)
		{
			getApp().close();
		}

		return false;
	}
}
