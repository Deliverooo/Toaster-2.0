#include <QVulkanWindow>

#include "client_application.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_instance.hpp"

auto cstringArrayToVector(toaster::CString *p_arr, uint32 p_size) -> std::vector<toaster::CString>
{
	std::vector<toaster::CString> vec{p_size};
	for (uint32 i{0u}; i < p_size; ++i)
		vec.emplace_back(p_arr[i]);
	return vec;
}

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
int main(int32 p_argc, char **p_argv) // Maybe_todo, Forward these parameters to the application for it to handle
{
	#endif

	QGuiApplication app{p_argc, p_argv};

	toaster::gpu::VKInstance vk_instance{};

	uint32 extension_count{0u};
	auto   required_extensions{cstringArrayToVector(glfwGetRequiredInstanceExtensions(&extension_count), extension_count)};
	vk_instance.setRequiredExtensions({required_extensions.begin(), required_extensions.end()});

	vk_instance.create();

	QVulkanInstance qvk_instance{};
	qvk_instance.setVkInstance(*vk_instance.getVulkanInstance());


	QVulkanWindow vk_window{};

	vk_window.setTitle("Toaster - QT test :)");
	vk_window.show();

	return app.exec();
}
