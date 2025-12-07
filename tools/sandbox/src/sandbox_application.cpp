#include "core/core.hpp"
#include "platform/application.hpp"
#include "sandbox_layer.hpp"

namespace tst
{
	class SandboxApplication : public Application
	{
	public:
		SandboxApplication(const ApplicationSpecInfo &spec_info) : Application(spec_info)
		{
			FileSystem::setWorkingDirectory("C:\\dev\\Toaster\\tools\\sandbox");

			pushLayer(tnew SandboxLayer);
		}
	};

	extern Application *createApplication(const std::vector<String> &args)
	{
		ApplicationSpecInfo spec_info{};
		spec_info.Name                = "Toaster";
		spec_info.WindowWidth         = 1600;
		spec_info.WindowHeight        = 900;
		spec_info.StartMaximized      = true;
		spec_info.VSync               = true;
		spec_info.CoreThreadingPolicy = EThreadingPolicy::eSingleThreaded;

		return tnew SandboxApplication(spec_info);
	}
}
