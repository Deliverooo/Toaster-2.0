#include "core/core.hpp"
#include "platform/application.hpp"
#include "sandbox_layer.hpp"

namespace tst
{
	class SandboxApplication : public Application
	{
	public:
		explicit SandboxApplication(const std::vector<String> &args) : Application(args)
		{
			pushLayer(tnew SandboxLayer);
		}
	};

	extern Application *createApplication(const std::vector<String> &args)
	{
		TST_INFO("Orbo!!");

		return tnew SandboxApplication(args);
	}
}
